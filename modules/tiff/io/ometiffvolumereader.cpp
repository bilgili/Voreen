/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "ometiffvolumereader.h"

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/io/progressbar.h"
#include "voreen/core/utils/stringutils.h"

#include <tinyxml/tinyxml.h>

#include <fstream>
#include <iostream>
#include <assert.h>

#include "tgt/exception.h"
#include "tgt/vector.h"
#include "tgt/texture.h"
#include "tgt/filesystem.h"

using std::string;
using tgt::vec3;
using tgt::ivec3;
using tgt::Texture;

namespace {
void raiseCorruptedFileException(const std::string& msg, const std::string& filename) {
    LERRORC("voreen.ome.OmeTiffVolumeReader", msg);
    throw tgt::CorruptedFileException(msg, filename);
}
}

namespace voreen {

const std::string OMETiffVolumeReader::loggerCat_ = "voreen.ome.OmeTiffVolumeReader";

OMETiffVolumeReader::OMETiffVolumeReader(ProgressBar* progress) : VolumeReader(progress)
{
    extensions_.push_back("tiff");
    extensions_.push_back("tif");
}

VolumeList* OMETiffVolumeReader::read(const std::string &url)
    throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{
    VolumeURL origin(url);
    const std::string masterFileName = origin.getPath();
    const std::string path = tgt::FileSystem::dirName(masterFileName);

    ivec3 dimensions;
    int band = 1;

    LINFO("Reading OME XML from master file: " << masterFileName);

    // determine number of directories within the tiff file
    TIFF* tiffFile = TIFFOpen(masterFileName.c_str(), "r");
    if (tiffFile) {
        int dircount = 0;
        do {
            dircount++;
        } while (TIFFReadDirectory(tiffFile));
        if (dircount == 0)
            raiseCorruptedFileException("No directories found in Tiff file", masterFileName);
        else if (dircount > 1)
            LWARNING(dircount << " directories found in Tiff file. Using only the first one.");
        TIFFClose(tiffFile);
    }
    else
        raiseCorruptedFileException("Failed to open Tiff file", masterFileName);

    tiffFile = TIFFOpen(masterFileName.c_str(), "r");
    if (!tiffFile)
        raiseCorruptedFileException("Failed to open file", masterFileName);

    // extract meta data from OME XML, stored in the image description field of the tiff file
    int sizeC, sizeT;
    tgt::vec3 spacing;
    OMEDataType dataType;
    std::vector<std::string> filenames;
    try {
        extractMetaData(tiffFile, dimensions, sizeC, sizeT, dataType, spacing, filenames);
    }
    catch (tgt::CorruptedFileException& e) {
        raiseCorruptedFileException(e.what(), masterFileName);
    }

    // check extracted meta data
    if (tgt::hor(tgt::lessThanEqual(dimensions, tgt::ivec3::zero)) || tgt::hor(tgt::greaterThan(dimensions, tgt::ivec3(99999)))) {
        std::ostringstream stream;
        stream << dimensions;
        raiseCorruptedFileException("Invalid dimensions: " + stream.str(), masterFileName);
    }
    if (tgt::hor(tgt::lessThan(spacing, tgt::vec3::zero))) {
        std::ostringstream stream;
        stream << spacing;
        LWARNING("negative spacing: " + stream.str() + ", overwriting with 1.0");
        spacing = tgt::vec3(1.f);
    }
    else if (tgt::hor(tgt::equal(spacing, tgt::vec3::zero))) {
        std::ostringstream stream;
        stream << spacing;
        LWARNING("zero spacing: " + stream.str() + ", overwriting with 1.0");
        spacing = tgt::vec3(1.f);
    }
    if (sizeC != 1 || sizeT != 1) {
        std::string errorMsg = "Currently only TIFF-OME data sets with one channel and one time point supported. ";
        errorMsg += "Data set properties: SizeC=" + itos(sizeC) + ", SizeT=" + itos(sizeT);
        LERROR(errorMsg);
        throw tgt::IOException(errorMsg);
    }
    if (filenames.size() != dimensions.z) {
        throw tgt::CorruptedFileException("OME XML: number of data files (" + itos(filenames.size()) +
            ") does not match number of z-slices (" + itos(dimensions.z) + ")");
    }

    LINFO("dimensions: " << dimensions);
    LINFO("spacing: " << spacing << " micron");
    LINFO("data type: " << omeDataTypeToStr(dataType));

    // Voreen base length unit is mm
    spacing /= 1000.f;

    // create volume
    VolumeRAM* volume = 0;
    switch (dataType) {
        case OME_UINT8:
            volume = new VolumeRAM_UInt8(dimensions);
            break;
        case OME_UINT16:
            volume = new VolumeRAM_UInt16(dimensions);
            break;
        case OME_UINT32:
            volume = new VolumeRAM_UInt32(dimensions);
            break;
        case OME_INT8:
            volume = new VolumeRAM_Int8(dimensions);
            break;
        case OME_INT16:
            volume = new VolumeRAM_Int16(dimensions);
            break;
        case OME_INT32:
            volume = new VolumeRAM_Int32(dimensions);
            break;
        case OME_FLOAT:
            volume = new VolumeRAM_Float(dimensions);
            break;
        default:
            LERROR("Data type " << omeDataTypeToStr(dataType) << " not supported");
            throw tgt::IOException("Data type " + omeDataTypeToStr(dataType) + " not supported", masterFileName);
    }

    // read slices
    if (getProgressBar()) {
        getProgressBar()->setTitle("Loading OME-TIFF data set");
        getProgressBar()->show();
    }

    for (size_t slice=0; slice<filenames.size(); slice++) {
        std::string curFile = path + "/" + filenames.at(slice);

        if (getProgressBar()) {
            getProgressBar()->setMessage("Loading " + filenames.at(slice) + " ...");
            getProgressBar()->setProgress(static_cast<float>(slice) / static_cast<float>(filenames.size()-1));
            getProgressBar()->forceUpdate();
        }

        try {
            switch (dataType) {
            case OME_UINT8:
                readTiffSlice<uint8_t>(static_cast<VolumeRAM_UInt8*>(volume), IL_UNSIGNED_BYTE, curFile, slice);
                break;
            case OME_UINT16:
                readTiffSlice<uint16_t>(static_cast<VolumeRAM_UInt16*>(volume), IL_UNSIGNED_SHORT, curFile, slice);
                break;
            case OME_UINT32:
                readTiffSlice<uint32_t>(static_cast<VolumeRAM_UInt32*>(volume), IL_UNSIGNED_INT, curFile, slice);
                break;
            case OME_INT8:
                readTiffSlice<int8_t>(static_cast<VolumeRAM_Int8*>(volume), IL_BYTE, curFile, slice);
                break;
            case OME_INT16:
                readTiffSlice<int16_t>(static_cast<VolumeRAM_Int16*>(volume), IL_SHORT, curFile, slice);
                break;
            case OME_INT32:
                readTiffSlice<int32_t>(static_cast<VolumeRAM_Int32*>(volume), IL_INT, curFile, slice);
                break;
            case OME_FLOAT:
                readTiffSlice<float>(static_cast<VolumeRAM_Float*>(volume), IL_FLOAT, curFile, slice);
                break;
            default:
                tgtAssert(false, "");
                if (getProgressBar())
                    getProgressBar()->hide();
                throw tgt::IOException("Data type " + omeDataTypeToStr(dataType) + " not supported", masterFileName);
            }
        }
        catch (tgt::CorruptedFileException& e) {
            delete volume;
            if (getProgressBar())
                getProgressBar()->hide();
            raiseCorruptedFileException("Failed to read tiff slice: " + std::string(e.what()), curFile);
        }
    }
    if (getProgressBar())
        getProgressBar()->hide();

    VolumeList* volumeList = new VolumeList();
    for (int i = 0; i < band; i++ ) {
        Volume* volumeHandle = new Volume(volume, spacing, vec3(0.0f));
        volumeHandle->setTimestep(static_cast<float>(i));
        //oldVolumePosition(volumeHandle);
        volumeList->add(volumeHandle);
    }
    if (!volumeList->empty())
        volumeList->first()->setOrigin(VolumeURL(masterFileName));
    return volumeList;
}

VolumeReader* OMETiffVolumeReader::create(ProgressBar* progress) const {
    return new OMETiffVolumeReader(progress);
}

const TiXmlNode* OMETiffVolumeReader::getXMLNode(const TiXmlNode* parent, const std::string& path) const {
    tgtAssert(parent, "parent node is null");
    tgtAssert(!path.empty(), "path string is empty");

    std::vector<std::string> pathSplit = strSplit(path, '/');
    if (pathSplit.back().empty())
        pathSplit.pop_back();
    std::string subPath;
    const TiXmlNode* currentNode = parent;
    for (size_t i=0; i<pathSplit.size(); i++) {
        subPath += pathSplit.at(i) + "/";
        const TiXmlNode* childNode = 0;
        while ((childNode = currentNode->IterateChildren(childNode)) != 0) {
            if (childNode->Value() == pathSplit.at(i))
                break;
        }
        if (!childNode)
            throw new tgt::CorruptedFileException("Path '" + subPath  + "' does not exist beneath node '" + parent->Value() + "'");
        currentNode = childNode;
    }

    tgtAssert(currentNode, "node is null");
    return currentNode;
}

std::vector<const TiXmlNode*> OMETiffVolumeReader::getXMLNodeList(const TiXmlNode* parent,
        const std::string& path, const std::string& nodeName) const {

    tgtAssert(parent, "parent node is null");
    tgtAssert(!nodeName.empty(), "node name is empty");

    // get parent node
    const TiXmlNode* nodeListParent = 0;
    if (path.empty())
        nodeListParent = parent;
    else
        getXMLNode(parent, path);
    tgtAssert(nodeListParent, "parent node null"); //< otherwise exception expected

    // iterate over children and collect all nodes with matching name
    std::vector<const TiXmlNode*> result;
    const TiXmlNode* childNode = 0;
    while ((childNode = parent->IterateChildren(childNode)) != 0) {
        if (childNode->Value() == nodeName)
            result.push_back(childNode);
    }
    return result;
}

void OMETiffVolumeReader::extractMetaData(TIFF* tiffFile, tgt::ivec3& dimensions, int& sizeC, int& sizeT, OMEDataType& dataType,
        tgt::vec3& spacing, std::vector<std::string>& filenames) const throw (tgt::CorruptedFileException) {

    tgtAssert(tiffFile, "no tiff file");

    char* desc = 0;
    TIFFGetField(tiffFile, TIFFTAG_IMAGEDESCRIPTION, &desc);
    if (!desc)
        throw tgt::CorruptedFileException("Failed to extract image description from Tiff file");

    TiXmlDocument xmlDoc;
    xmlDoc.Parse(desc);
    if (xmlDoc.Error()) {
        LDEBUG("OME XML string: " << desc);
        throw tgt::CorruptedFileException("Failed to parse OME XML string: " + std::string(xmlDoc.ErrorDesc()));
    }

    // log xml string for debugging
    TiXmlPrinter printer;
    xmlDoc.Accept(&printer);
    LDEBUG("OME XML string: " << printer.Str());

    // Is there a root element?
    const TiXmlElement* rootElem = xmlDoc.RootElement();
    if (!rootElem)
        throw tgt::CorruptedFileException("OME XML has no root element");

    // get 'Pixels' element
    const TiXmlNode* pixelsNode = 0;
    try {
        pixelsNode = getXMLNode(rootElem, "Image/Pixels");
    }
    catch (tgt::CorruptedFileException& e) {
        throw tgt::CorruptedFileException("Node 'Pixels' not not found in OME XML: " + std::string(e.what()));
    }
    tgtAssert(pixelsNode, "no pixels node");
    const TiXmlElement* pixelsElem = pixelsNode->ToElement();
    if (!pixelsElem)
        throw tgt::CorruptedFileException("Node 'Pixels' is not an element");

    // extract dimensions from 'Pixels' elem
    if (pixelsElem->QueryIntAttribute("SizeX", &dimensions.x) != TIXML_SUCCESS)
        throw tgt::CorruptedFileException("Failed to read attribute 'SizeX' of element 'Pixels' in OME XML");
    if (pixelsElem->QueryIntAttribute("SizeY", &dimensions.y) != TIXML_SUCCESS)
        throw tgt::CorruptedFileException("Failed to read attribute 'SizeY' of element 'Pixels' in OME XML");
    if (pixelsElem->QueryIntAttribute("SizeZ", &dimensions.z) != TIXML_SUCCESS)
        throw tgt::CorruptedFileException("Failed to read attribute 'SizeZ' of element 'Pixels' in OME XML");
    if (pixelsElem->QueryIntAttribute("SizeT", &sizeT) != TIXML_SUCCESS)
        throw tgt::CorruptedFileException("Failed to read attribute 'SizeT' of element 'Pixels' in OME XML");
    if (pixelsElem->QueryIntAttribute("SizeT", &sizeC) != TIXML_SUCCESS)
        throw tgt::CorruptedFileException("Failed to read attribute 'SizeC' of element 'Pixels' in OME XML");

    // extract data type from 'Pixels' elem
    std::string pixelTypeStr;
    if (pixelsElem->QueryValueAttribute("PixelType", &pixelTypeStr) != TIXML_SUCCESS)
        throw tgt::CorruptedFileException("Failed to read attribute 'PixelType' of element 'Pixels' in OME XML");
    dataType = omeDataTypeFromStr(pixelTypeStr);
    if (dataType == OME_UNKNOWN)
        throw tgt::CorruptedFileException("Unknown OME PixelType: " + dataType);

    // extract spacing from 'Pixels' elem
    if (pixelsElem->QueryFloatAttribute("PhysicalSizeX", &spacing.x) != TIXML_SUCCESS)
        throw tgt::CorruptedFileException("Failed to read attribute 'PhysicalSizeX' of element 'Pixels' in OME XML");
    if (pixelsElem->QueryFloatAttribute("PhysicalSizeY", &spacing.y) != TIXML_SUCCESS)
        throw tgt::CorruptedFileException("Failed to read attribute 'PhysicalSizeY' of element 'Pixels' in OME XML");
    if (pixelsElem->QueryFloatAttribute("PhysicalSizeZ", &spacing.z) != TIXML_SUCCESS)
        throw tgt::CorruptedFileException("Failed to read attribute 'PhysicalSizeZ' of element 'Pixels' in OME XML");

    // collect filenames from 'TiffData' nodes
    std::vector<const TiXmlNode*> tiffdataNodes;
    try {
        tiffdataNodes = getXMLNodeList(pixelsNode, "", "TiffData");
    }
    catch (tgt::CorruptedFileException& e) {
        throw tgt::CorruptedFileException("Failed to retrieve 'TiffData' nodes from OME XML: " + std::string(e.what()));
    }

    filenames.clear();
    try {
        for (size_t i=0; i<tiffdataNodes.size(); i++) {
            tgtAssert(tiffdataNodes.at(i), "node is null");
            const TiXmlNode* uuidNode = getXMLNode(tiffdataNodes.at(i), "UUID");
            tgtAssert(uuidNode, "node is null");
            const char* c_filename = uuidNode->ToElement()->Attribute("FileName");
            if (!c_filename)
                throw tgt::CorruptedFileException("OME XML: failed to read attribute 'FileName' from 'UUID' node");
            std::string filename(c_filename);
            if (filename.empty())
                throw tgt::CorruptedFileException("OME XML: 'FileName' attribute of 'UUID' node is empty");
            filenames.push_back(filename);
        }
    }
    catch (tgt::CorruptedFileException& e) {
        throw tgt::CorruptedFileException("Failed to read filenames from 'TiffData' nodes in OME XML: " + std::string(e.what()));
    }

}

OMETiffVolumeReader::OMEDataType OMETiffVolumeReader::omeDataTypeFromStr(const std::string& pixelTypeStr) const {
    if (pixelTypeStr == "int8")
        return OME_INT8;
    else if (pixelTypeStr == "int16")
        return OME_INT16;
    else if (pixelTypeStr == "int32")
        return OME_INT32;
    else if (pixelTypeStr == "uint8")
        return OME_UINT8;
    else if (pixelTypeStr == "uint16")
        return OME_UINT16;
    else if (pixelTypeStr == "uint32")
        return OME_UINT32;
    else if (pixelTypeStr == "float")
        return OME_FLOAT;
    else
        return OME_UNKNOWN;
}

std::string OMETiffVolumeReader::omeDataTypeToStr(OMEDataType type) const {
    if (type == OME_INT8)
        return "int8";
    else if (type == OME_INT16)
        return "int16";
    else if (type == OME_INT32)
        return "int32";
    else if (type == OME_UINT8)
        return "uint8";
    else if (type == OME_UINT16)
        return "uint16";
    else if (type == OME_UINT32)
        return "uint32";
    else if (type == OME_FLOAT)
        return "float";
    else if (type == OME_UNKNOWN)
        return "unknown";
    else {
        tgtAssert(false, "unknown OME data type");
        return "";
    }
}

} // namespace voreen
