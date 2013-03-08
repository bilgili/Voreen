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
#include "voreen/core/datastructures/volume/volumefactory.h"

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
using tgt::svec3;
using tgt::Texture;

namespace { // anonymous helper functions

void raiseIOException(const std::string& msg, const std::string& filename, voreen::ProgressBar* progress = 0) {
    LERRORC("voreen.ome.OmeTiffVolumeReader", msg + ": " + filename);
    if (progress)
        progress->hide();
    throw tgt::IOException(msg, filename);
}

void deleteVolumes(const std::vector<std::vector<voreen::VolumeRAM*> >& volumes) {
    for (size_t i=0; i<volumes.size(); i++) {
        for (size_t j=0; j<volumes[i].size(); j++) {
            delete volumes[i][j];
        }
    }
}

} // namespace anonymous

// ------------------------------------------------------------------------------------------------

namespace voreen {

OMETiffVolumeReader::OMETiffFile::OMETiffFile(std::string filename, size_t numDirectories, size_t firstZ, size_t firstT, size_t firstC) 
    : filename_(filename)
    , numDirectories_(numDirectories)
    , firstZ_(firstZ)
    , firstT_(firstT)
    , firstC_(firstC)
{}

std::string OMETiffVolumeReader::OMETiffFile::toString() const {
    std::string result = "OMETiffFile[";
    result += "filename=" + filename_ + ", ";
    result += "numDirectories=" + itos(numDirectories_) + ", ";
    result += "firstZ=" + itos(firstZ_) + ", ";
    result += "firstT=" + itos(firstT_) + ", ";
    result += "firstC=" + itos(firstC_) + "]";
    return result;
}

// ------------------------------------------------------------------------------------------------

const std::string OMETiffVolumeReader::loggerCat_ = "voreen.ome.OmeTiffVolumeReader";

OMETiffVolumeReader::OMETiffVolumeReader(ProgressBar* progress) : VolumeReader(progress)
{
    extensions_.push_back("ome.tiff");
    extensions_.push_back("ome.tif");

    protocols_.push_back("ome-tiff");
}

VolumeReader* OMETiffVolumeReader::create(ProgressBar* progress) const {
    return new OMETiffVolumeReader(progress);
}

VolumeList* OMETiffVolumeReader::read(const std::string &url)
    throw (tgt::FileException, std::bad_alloc)
{
    VolumeURL origin(url);
    const std::string masterFileName = origin.getPath();

    // determine selected channel/timestep
    int requestedChannel = -1;
    if (origin.getSearchParameter("channel") != "")
        requestedChannel = stoi(origin.getSearchParameter("channel"));
    int requestedTimestep = -1;
    if (origin.getSearchParameter("timestep") != "")
        requestedTimestep = stoi(origin.getSearchParameter("timestep"));
    if ((requestedChannel == -1) != (requestedTimestep == -1)) {
        LWARNING("Either channel and timestep need both to be specified, or none of them. Ignoring.");
        requestedChannel = -1;
        requestedTimestep = -1;
    }
    const bool singleVolume = requestedChannel != -1;

    // open master ome tiff file
    TIFF* tiffFile = TIFFOpen(masterFileName.c_str(), "r");
    if (!tiffFile)
        raiseIOException("Failed to open file", masterFileName);

    LINFO("Reading OME XML from master file: " << masterFileName);

    if (getProgressBar()) {
        getProgressBar()->setTitle("Loading OME-TIFF data set");
        getProgressBar()->setMessage("Scanning files...");
        getProgressBar()->show();
        getProgressBar()->forceUpdate();
    }

    // extract meta data from OME XML, stored in the image description field of the tiff file
    std::string dimensionOrder;
    svec3 dimensions;
    int sizeC, sizeT;
    tgt::vec3 spacing;
    std::string dataType;
    std::vector<OMETiffFile> files;
    try {
        extractMetaData(tiffFile, tgt::FileSystem::dirName(masterFileName), dimensionOrder, dimensions, sizeC, sizeT, dataType, spacing, files);
        TIFFClose(tiffFile);
        tiffFile = 0;
    }
    catch (tgt::Exception& e) {
        TIFFClose(tiffFile);
        raiseIOException(e.what(), masterFileName, getProgressBar());
    }

    // in single mode, check requested channel and timestep
    if (singleVolume) {
        if (requestedChannel >= sizeC) {
            raiseIOException("Requested channel (" + itos(requestedChannel) + ") is greater/equal than number of channels in stack (" + itos(sizeC) + ")", 
                masterFileName, getProgressBar());
        }
        if (requestedTimestep >= sizeT) {
            raiseIOException("Requested timestep (" + itos(requestedTimestep) + ") is greater/equal than number of timesteps in stack (" + itos(sizeT) + ")", 
                masterFileName, getProgressBar());
        }
    }

    // determine number of directories stored in each tiff file (requires to open files)
    size_t numSlices = 0;
    try {
        determineDirectoryCount(files, numSlices);
    }
    catch (tgt::Exception& e) {
        raiseIOException(e.what(), masterFileName, getProgressBar());
    }

    // check extracted meta data
    if (tgt::hor(tgt::lessThanEqual(dimensions, tgt::svec3::zero)) || tgt::hor(tgt::greaterThan(dimensions, tgt::svec3(99999)))) {
        std::ostringstream stream;
        stream << dimensions;
        raiseIOException("Invalid dimensions: " + stream.str(), masterFileName, getProgressBar());
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
    if (numSlices != dimensions.z*sizeC*sizeT) {
        raiseIOException("Total number of slices in TIFF files (" + itos(numSlices) + 
            ") does not match stack size (dim.z*sizeC*sizeT = " + itos(dimensions.z*sizeC*sizeT) + ") ", masterFileName, getProgressBar());
    }

    // log extracted data
    LINFO("DimensionOrder: " << dimensionOrder);
    LINFO("Volume dimensions: " << dimensions);
    LINFO("Voxel spacing: " << spacing << " micron");
    LINFO("Data type: " << dataType);
    LINFO("Num channels: " << sizeC);
    LINFO("Num timesteps: " << sizeT);
    if (singleVolume)
        LINFO("Loading channel/timestep: " << requestedChannel << "/" << requestedTimestep);

    LDEBUG("TIFF files:");
    for (size_t i=0; i<files.size(); i++)
        LDEBUG(files.at(i).toString());

    // adapt spacing: OME base length is micron, Voreen base length unit is mm
    spacing /= 1000.f;

    // create two nested vectors of output volumes: first coordinate=channel, second coordinate=time
    // (volumes will be created on demand)
    std::vector<std::vector<VolumeRAM*> > volumes;
    for (size_t c=0; c<sizeC; c++) {
        std::vector<VolumeRAM*> timeSeries(sizeT, reinterpret_cast<VolumeRAM*>(0));
        volumes.push_back(timeSeries);
    }
    tgtAssert(volumes.size() == sizeC, "invalid size of volume vector");

    // insert current and max z,c,t values into helper vectors according to DimensionOrder
    int curZ = 0;
    int curC = 0;
    int curT = 0;
    std::vector<int*> curStackIndices = std::vector<int*>(3, reinterpret_cast<int*>(0));
    std::vector<int> dimSizes = std::vector<int>(3, 0);
    std::string::size_type posZ = dimensionOrder.find("Z");
    std::string::size_type posC = dimensionOrder.find("C");
    std::string::size_type posT = dimensionOrder.find("T");
    if (posZ < 2 || posZ > 4 || posC < 2 || posC > 4 || posT < 2 || posT > 4)
        raiseIOException("Invalid DimensionOrder: " + dimensionOrder, masterFileName, getProgressBar());
    curStackIndices.at(posZ-2) = &curZ;
    dimSizes.at(posZ-2) = static_cast<int>(dimensions.z);
    curStackIndices.at(posC-2) = &curC;
    dimSizes.at(posC-2) = sizeC;
    curStackIndices.at(posT-2) = &curT;
    dimSizes.at(posT-2) = sizeT;
    tgtAssert(curStackIndices.at(0) != 0 && dimSizes.at(0) != 0 && curStackIndices.at(1) != 0 && dimSizes.at(1) != 0 && curStackIndices.at(2) != 0 && dimSizes.at(2) != 0,
        "dimension vector not properly initialized");


    //
    // Iterate over TIFF files and copy pixel content to respective positions in the volume stack
    //
    VolumeFactory volumeFac;
    size_t curSlice = 0;
    for (int fileID = 0; fileID < files.size(); fileID++) {
        OMETiffFile& curFile = files.at(fileID);
        if (getProgressBar())
            getProgressBar()->setMessage("Loading " + curFile.filename_ + " ...");

        // open current tiff file
        TIFF* curTiffFile = TIFFOpen(curFile.filename_.c_str(), "r");
        if (!curTiffFile) {
            deleteVolumes(volumes);
            raiseIOException("Failed to open TIFF file", curFile.filename_, getProgressBar());
        }

        // check firstZ, firstC, firstT parameters of current file against current coordinates
        if (curFile.firstZ_ != curZ || curFile.firstC_ != curC || curFile.firstT_ != curT) {
            LWARNING("First Z/T/C values of Tiff file '" << curFile.filename_ << "' do not match expected values");
            //deleteVolumes(volumes);
            //raiseIOException("First Z/T/C values of Tiff file do not match expected values", curFile.filename_, getProgressBar());
        }

        //
        // iterate over directories of current TIFF file
        //
        for (size_t tiffDir = 0; tiffDir < files.at(fileID).numDirectories_; tiffDir++) {
            tgtAssert(curC < volumes.size(), "current C value larger than volumes vector");
            tgtAssert(curT < volumes[curC].size(), "current T value larger than volumes vector");

            if (getProgressBar()) {
                getProgressBar()->setProgress(static_cast<float>(curSlice) / static_cast<float>(numSlices-1));
                getProgressBar()->forceUpdate();
            }

            // ignore slice, if only a single volume is loaded and current channel/timestep do not match requested volume
            if (!singleVolume || (curC == requestedChannel && curT == requestedTimestep)) {
                
                // retrieve current volume and create it, if not created yet
                VolumeRAM*& currentVolume = volumes[curC][curT];
                if (!currentVolume) {
                    try {
                        currentVolume = volumeFac.create(dataType, dimensions);
                    }
                    catch (std::exception& e) {
                        LERROR(e.what());
                        deleteVolumes(volumes);
                        if (getProgressBar())
                            getProgressBar()->hide();
                        throw tgt::IOException(e.what(), masterFileName);
                    }
                }
                tgtAssert(currentVolume, "current volume is null");

                // read current directory/slice
                try {
                    size_t curSliceByteOffset = tgt::hmul(dimensions.xy()) * curZ * currentVolume->getBytesPerVoxel();
                    void* curSlicePointer = reinterpret_cast<void*>(reinterpret_cast<char*>(currentVolume->getData()) + curSliceByteOffset);
                    readTiffDirectory(curTiffFile, dataType, dimensions, curSlicePointer);
                
                    // alternative using devil (works only for single-slice files!):
                    //readTiffDirectory(curFile, dataType, dimensions, curSlicePointer);
                }
                catch (tgt::Exception& e) {
                    deleteVolumes(volumes);
                    raiseIOException("Failed to read tiff slice: " + std::string(e.what()), curFile.filename_, getProgressBar());
                }
            } // single volume

            // increment directory in tiff file
            TIFFReadDirectory(curTiffFile);

            // update stack indices (pointers to curZ, curC, curT)
            curSlice++;
            (*curStackIndices[0])++; //< first index changes fastest
            if (*curStackIndices[0] >= dimSizes[0]) { //< handle overvlow
                *curStackIndices[0] = 0;
                (*curStackIndices[1])++;
                if (*curStackIndices[1] >= dimSizes[1]) {
                    *curStackIndices[1] = 0;
                    (*curStackIndices[2])++;
                }
            }
            tgtAssert((*curStackIndices[0] < dimSizes[0] && *curStackIndices[1] < dimSizes[1] && *curStackIndices[2] < dimSizes[2]) ||
                      (curSlice == numSlices), "invalid stack indices");

        } // directory iteration

        TIFFClose(curTiffFile);
        curTiffFile = 0;

    } // file iteration


    if (getProgressBar())
        getProgressBar()->hide();

    // collect created volumes in volume list
    VolumeList* volumeList = new VolumeList();
    for (size_t c=0; c<volumes.size(); c++) {
        for (size_t t=0; t<volumes[c].size(); t++) {
            tgtAssert(singleVolume || volumes[c][t], "missing volume"); //< if not single volume, all volumes should have been created
            if (volumes[c][t]) {
                Volume* volumeHandle = new Volume(volumes[c][t], spacing, vec3(0.0f));
                volumeHandle->setTimestep(static_cast<float>(t));
                volumeHandle->setMetaDataValue<IntMetaData, int>("Channel", (int)c);

                VolumeURL origin("ome-tiff", masterFileName);
                origin.addSearchParameter("channel", itos(c));
                origin.addSearchParameter("timestep", itos(t));
                volumeHandle->setOrigin(origin);

                volumeList->add(volumeHandle);
            }
        }
    }
    tgtAssert(!volumeList->empty(), "volume list is empty");

    return volumeList;
}

VolumeBase* OMETiffVolumeReader::read(const VolumeURL& origin) 
    throw (tgt::FileException, std::bad_alloc) 
{
    VolumeList* volumeList = read(origin.getURL());
    tgtAssert(!volumeList->empty(), "volume list is empty");

    VolumeBase* volume = volumeList->first();
    tgtAssert(volume, "volume is null");

    if (volumeList->size() > 1) {
        LWARNING("read(origin): more than one volume loaded. Discarding redundant volumes.");
        for (size_t i=1; i<volumeList->size(); i++)
            delete volumeList->at(i);
    }

    return volume;
}

std::vector<VolumeURL> OMETiffVolumeReader::listVolumes(const std::string& urlStr) const 
    throw (tgt::FileException) 
{
    VolumeURL url(urlStr);
    std::string filepath = url.getPath();

    TIFF* tiffFile = TIFFOpen(filepath.c_str(), "r");
    if (!tiffFile)
        raiseIOException("Failed to open file", filepath);

    // extract meta data from OME XML, stored in the image description field of the tiff file
    std::string dimensionOrder;
    svec3 dimensions;
    int sizeC, sizeT;
    tgt::vec3 spacing;
    std::string dataType;
    std::vector<OMETiffFile> files;
    try {
        extractMetaData(tiffFile, filepath, dimensionOrder, dimensions, sizeC, sizeT, dataType, spacing, files);
    }
    catch (tgt::Exception& e) {
        raiseIOException(e.what(), urlStr);
    }

    // identify volumes by channel and timestep
    std::vector<VolumeURL> volumeURLs;
    for (size_t c=0; c<sizeC; c++) {
        for (size_t t=0; t<sizeT; t++) {
            VolumeURL subUrl("ome-tiff", filepath);
            subUrl.addSearchParameter("channel", itos(c));
            subUrl.addSearchParameter("timestep", itos(t));
            subUrl.getMetaDataContainer().addMetaData("Channel", new IntMetaData((int)c));
            subUrl.getMetaDataContainer().addMetaData("Timestep", new IntMetaData((int)t));
            subUrl.getMetaDataContainer().addMetaData("Volume Dimensions", new IVec3MetaData(static_cast<ivec3>(dimensions)));
            subUrl.getMetaDataContainer().addMetaData("Voxel Spacing", new StringMetaData(genericToString(spacing) + " micron"));
            volumeURLs.push_back(subUrl);
        }
    }
    
    return volumeURLs;
}


// protected/private methods
// -------------------------

void OMETiffVolumeReader::readTiffDirectory(TIFF* tiffFile, const std::string& dataType, const tgt::svec3& volumeDim, void* destBuffer) const 
    throw (tgt::Exception) 
{
    tgtAssert(tiffFile, "null pointer passed");

    // determine bits per voxel from pass data type
    int bitsPerVoxel = 0;
    if (dataType == "uint8" || dataType == "int8")
        bitsPerVoxel = 8;
    else if (dataType == "uint16" || dataType == "int16")
        bitsPerVoxel = 16;
    else if (dataType == "uint32" || dataType == "int32" || dataType == "float")
        bitsPerVoxel = 32;
    else {
        tgtAssert(false, "unknown data type"); //< should have been checked before
        throw tgt::Exception("Unknown data type: " + dataType);   
    }
    tgtAssert(bitsPerVoxel > 0, "invalid bits per voxel");
        
    // read properties from tiff file and check against passed parameters
    uint32 width, height;
    uint16 depth, bps;
    TIFFGetField(tiffFile, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tiffFile, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField(tiffFile, TIFFTAG_SAMPLESPERPIXEL, &depth);
    TIFFGetField(tiffFile, TIFFTAG_BITSPERSAMPLE, &bps);
    if ((volumeDim.x != static_cast<int>(width)) || (volumeDim.y != static_cast<int>(height))) {
        throw tgt::Exception("Slice dimensions (" + genericToString(tgt::ivec2(width, height)) + ") differ from volume dimensions (" + genericToString(volumeDim) + ")");
    }
    else if (depth != 1) {
        throw tgt::Exception("Samples per pixel != 1 (" + itos(depth) + ")");
    }
    else if (bitsPerVoxel != bps) {
        throw tgt::Exception("Bits per sample (" + itos(bitsPerVoxel) + ") does not match data type " + dataType + " (expected: " + itos(bitsPerVoxel) + ")");
    }

    // determine strip parameters
    tsize_t stripCount = TIFFNumberOfStrips(tiffFile);
    tsize_t stripSize = TIFFStripSize(tiffFile);
    
    // iterate over strips and copy them to dest buffer
    for (tstrip_t stripID=0; stripID<static_cast<tstrip_t>(stripCount); stripID++) {
        if (TIFFReadEncodedStrip(tiffFile, stripID, destBuffer, stripSize) == -1)
            throw tgt::CorruptedFileException("Failed to read strip " + itos(static_cast<int>(stripID)));
        destBuffer = reinterpret_cast<void*>(reinterpret_cast<char*>(destBuffer) + stripSize);
    }

}

// see OME XML schema definition: http://www.openmicroscopy.org/Schemas/Documentation/Generated/OME-2012-06/ome.html
void OMETiffVolumeReader::extractMetaData(TIFF* tiffFile, const std::string& path, std::string& dimensionOrder, tgt::svec3& dimensions, int& sizeC, int& sizeT, std::string& dataType,
    tgt::vec3& spacing, std::vector<OMETiffFile>& files) const 
    throw (tgt::Exception) 
{
    tgtAssert(tiffFile, "no tiff file");

    char* desc = 0;
    TIFFGetField(tiffFile, TIFFTAG_IMAGEDESCRIPTION, &desc);
    if (!desc)
        throw tgt::Exception("Failed to extract image description from Tiff file");

    TiXmlDocument xmlDoc;
    xmlDoc.Parse(desc);
    if (xmlDoc.Error()) {
        LDEBUG("OME XML string: " << desc);
        throw tgt::Exception("Failed to parse OME XML: " + std::string(xmlDoc.ErrorDesc()));
    }

    // log xml string for debugging
    TiXmlPrinter printer;
    xmlDoc.Accept(&printer);
    LDEBUG("OME XML string: " << printer.Str());

    // Is there a root element?
    const TiXmlElement* rootElem = xmlDoc.RootElement();
    if (!rootElem)
        throw tgt::Exception("OME XML has no root element");

    // get 'Pixels' element
    const TiXmlNode* pixelsNode = 0;
    try {
        pixelsNode = getXMLNode(rootElem, "Image/Pixels");
    }
    catch (tgt::Exception& e) {
        throw tgt::Exception("Node 'Pixels' not not found in OME XML: " + std::string(e.what()));
    }
    tgtAssert(pixelsNode, "no pixels node");
    const TiXmlElement* pixelsElem = pixelsNode->ToElement();
    if (!pixelsElem)
        throw tgt::Exception("Node 'Pixels' is not an element");

    // extract 'DimensionOrder' from 'Pixels' elem (see )
    if (pixelsElem->QueryValueAttribute("DimensionOrder", &dimensionOrder) != TIXML_SUCCESS)
        throw tgt::Exception("Failed to read attribute 'DimensionOrder' of element 'Pixels' in OME XML");
    dimensionOrder = toUpper(trim(dimensionOrder));
    if (dimensionOrder != "XYZCT" &&  
        dimensionOrder != "XYZTC" &&
        dimensionOrder != "XYCTZ" &&
        dimensionOrder != "XYCZT" &&
        dimensionOrder != "XYTCZ" &&
        dimensionOrder != "XYTZC"    )
    {
        throw VoreenException("Unknown DimensionOrder: " + dimensionOrder);
    }

    // extract dimensions from 'Pixels' elem
    ivec3 intDim;
    if (pixelsElem->QueryIntAttribute("SizeX", &intDim.x) != TIXML_SUCCESS)
        throw tgt::Exception("Failed to read attribute 'SizeX' of element 'Pixels' in OME XML");
    if (pixelsElem->QueryIntAttribute("SizeY", &intDim.y) != TIXML_SUCCESS)
        throw tgt::Exception("Failed to read attribute 'SizeY' of element 'Pixels' in OME XML");
    if (pixelsElem->QueryIntAttribute("SizeZ", &intDim.z) != TIXML_SUCCESS)
        throw tgt::Exception("Failed to read attribute 'SizeZ' of element 'Pixels' in OME XML");
    if (pixelsElem->QueryIntAttribute("SizeT", &sizeT) != TIXML_SUCCESS)
        throw tgt::Exception("Failed to read attribute 'SizeT' of element 'Pixels' in OME XML");
    if (pixelsElem->QueryIntAttribute("SizeC", &sizeC) != TIXML_SUCCESS)
        throw tgt::Exception("Failed to read attribute 'SizeC' of element 'Pixels' in OME XML");
    dimensions = static_cast<svec3>(intDim);

    // extract data type from 'Pixels' elem
    std::string pixelTypeStr;
    if (pixelsElem->QueryValueAttribute("PixelType", &pixelTypeStr) != TIXML_SUCCESS) {
        LWARNING("Failed to read attribute 'PixelType' of element 'Pixels' in OME XML. Trying attribute 'Type' instead...");
        if (pixelsElem->QueryValueAttribute("Type", &pixelTypeStr) != TIXML_SUCCESS) 
            throw tgt::Exception("Failed to read attribute 'PixelType'/'Type' of element 'Pixels' in OME XML");
    }
    dataType = pixelTypeStr; //< OME pixel type strings equal type strings used by the VolumeFactory
    if (dataType != "uint8" && dataType != "int8" && dataType != "uint16" && dataType != "int16" &&
        dataType != "uint32" && dataType != "int32" && dataType != "float" )
        throw tgt::Exception("Unknown/unsupported PixelType: " + dataType);

    // extract spacing from 'Pixels' elem
    if (pixelsElem->QueryFloatAttribute("PhysicalSizeX", &spacing.x) != TIXML_SUCCESS)
        throw tgt::Exception("Failed to read attribute 'PhysicalSizeX' of element 'Pixels' in OME XML");
    if (pixelsElem->QueryFloatAttribute("PhysicalSizeY", &spacing.y) != TIXML_SUCCESS)
        throw tgt::Exception("Failed to read attribute 'PhysicalSizeY' of element 'Pixels' in OME XML");
    if (pixelsElem->QueryFloatAttribute("PhysicalSizeZ", &spacing.z) != TIXML_SUCCESS)
        throw tgt::Exception("Failed to read attribute 'PhysicalSizeZ' of element 'Pixels' in OME XML");

    // collect 'TiffData' nodes
    std::vector<const TiXmlElement*> tiffdataElems;
    try {
        tiffdataElems = getXMLElementList(pixelsNode, "", "TiffData");
    }
    catch (tgt::Exception& e) {
        throw ("Failed to retrieve 'TiffData' nodes from OME XML: " + std::string(e.what()));
    }

    // extract file information from 'TiffData' nodes
    files.clear();
    std::string currentUUID;
    try {
        for (size_t i=0; i<tiffdataElems.size(); i++) {
            tgtAssert(tiffdataElems.at(i), "elem is null");
            const TiXmlElement* tiffDataElem = tiffdataElems.at(i);

            // extract UUID element and string (mandatory)
            const TiXmlElement* uuidElem = getXMLElement(tiffDataElem, "UUID"); 
            tgtAssert(uuidElem, "elem is null"); //< exception expected
            std::string uuid = uuidElem->GetText();
            if (uuid.empty())
                throw tgt::Exception("'UUID' element has no text");
            if (uuid == currentUUID) //< file already processed
                continue;
            else
                currentUUID = uuid;

            // extract filename (mandatory) from UUID elem
            const char* c_filename = uuidElem->Attribute("FileName");
            if (!c_filename)
                throw tgt::Exception("failed to read attribute 'FileName' from 'UUID' node");
            std::string filename(c_filename);
            if (filename.empty())
                throw tgt::Exception("'FileName' attribute of 'UUID' node is empty");
            filename = tgt::FileSystem::cleanupPath(path + "/" + filename);

            // extract FirstZ, FirstC, FirstT attributes from TiffData elem (optional)
            int firstZ = 0;
            int firstC = 0;
            int firstT = 0;
            tiffDataElem->QueryIntAttribute("FirstZ", &firstZ);
            tiffDataElem->QueryIntAttribute("FirstC", &firstC);
            tiffDataElem->QueryIntAttribute("FirstT", &firstT);

            // create file descriptor from extracted information
            files.push_back(OMETiffFile(filename, 0, firstZ, firstT, firstC));
        }
    }
    catch (tgt::Exception& e) {
        throw tgt::Exception("Failed to collect file information from 'TiffData' nodes in OME XML: " + std::string(e.what()));
    }
}

void OMETiffVolumeReader::determineDirectoryCount(std::vector<OMETiffFile>& files, size_t& totalNumberOfSlices) 
    throw (tgt::Exception) 
{

    // determine number of directories for each TIFF file
    totalNumberOfSlices = 0;
    for (size_t i=0; i<files.size(); i++) {
        OMETiffFile& file = files.at(i);
        file.numDirectories_ = 0;
        TIFF* tiffFile = TIFFOpen(file.filename_.c_str(), "r");
        if (tiffFile) {
            do {
                file.numDirectories_++;
            } while (TIFFReadDirectory(tiffFile));
            TIFFClose(tiffFile);
            if (file.numDirectories_ == 0)
                throw tgt::Exception("No directories found in TIFF file '" + file.filename_ + "'");
            totalNumberOfSlices += file.numDirectories_;
        }
        else
            throw tgt::Exception("Failed to open TIFF file '" + file.filename_ + "'");
    }
}

const TiXmlNode* OMETiffVolumeReader::getXMLNode(const TiXmlNode* parent, const std::string& path) const 
    throw (tgt::Exception) 
{
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
            throw tgt::Exception("Path '" + subPath  + "' does not exist beneath node '" + parent->Value() + "'");
        currentNode = childNode;
    }

    tgtAssert(currentNode, "node is null");
    return currentNode;
}

const TiXmlElement* OMETiffVolumeReader::getXMLElement(const TiXmlNode* parent, const std::string& path) const 
    throw (tgt::Exception) 
{
    // retrieve node
    const TiXmlNode* xmlNode = getXMLNode(parent, path);
    tgtAssert(xmlNode, "null pointer returned"); //< exception expected

    // convert to element
    const TiXmlElement* xmlElem = xmlNode->ToElement();
    if (!xmlElem)
        throw tgt::Exception("Element '" + path + "' beneath parent node '" + parent->Value() + "' does not exist");
    return xmlElem;
}

std::vector<const TiXmlNode*> OMETiffVolumeReader::getXMLNodeList(const TiXmlNode* parent,
        const std::string& path, const std::string& nodeName) const 
        throw (tgt::Exception)  
{
    tgtAssert(parent, "parent node is null");
    tgtAssert(!nodeName.empty(), "node name is empty");

    // get parent node
    const TiXmlNode* nodeListParent = 0;
    if (path.empty())
        nodeListParent = parent;
    else
        nodeListParent = getXMLNode(parent, path);
    tgtAssert(nodeListParent, "parent node null"); //< otherwise exception expected

    // iterate over children and collect all nodes with matching name
    std::vector<const TiXmlNode*> result;
    const TiXmlNode* childNode = 0;
    while ((childNode = nodeListParent->IterateChildren(childNode)) != 0) {
        if (childNode->Value() == nodeName)
            result.push_back(childNode);
    }
    return result;
}

std::vector<const TiXmlElement*> OMETiffVolumeReader::getXMLElementList(const TiXmlNode* parent, const std::string& path, const std::string& nodeName) const 
    throw (tgt::Exception) 
{
    tgtAssert(parent, "parent node is null");
    tgtAssert(!nodeName.empty(), "node name is empty");

    std::vector<const TiXmlNode*> nodeList = getXMLNodeList(parent, path, nodeName);
    std::vector<const TiXmlElement*> elemList;
    for (size_t i=0; i<nodeList.size(); i++) {
        const TiXmlElement* nodeElem = nodeList.at(i)->ToElement();
        if (!nodeElem)
            throw tgt::Exception("Child node '" + path + "/" + nodeName + "' of parent node '" + parent->Value() + "' is not an element");
        elemList.push_back(nodeElem);
    }
    tgtAssert(elemList.size() == nodeList.size(), "list size mis-match");

    return elemList;
}


/* Previous read slice method using Devil (not used anymore)
 
void OMETiffVolumeReader::readTiffSliceFileIL(const std::string& filename, const std::string& dataType, const tgt::svec3& volumeDim, void* destBuffer) const 
    throw (tgt::Exception)
{
    tgtAssert(destBuffer, "no dest buffer");

    // determine devil data type
    ILenum ilDataType;
    if (dataType == "uint8")
        ilDataType = IL_UNSIGNED_BYTE;
    else if (dataType == "uint16")
        ilDataType = IL_UNSIGNED_SHORT;
    else if (dataType == "uint32")
        ilDataType = IL_UNSIGNED_INT;
    else if (dataType == "int8")
        ilDataType = IL_BYTE;
    else if (dataType == "int16")
        ilDataType = IL_SHORT;
    else if (dataType == "int32")
        ilDataType = IL_INT;
    else if (dataType == "float")
        ilDataType = IL_FLOAT;
    else {
        tgtAssert(false, "unknown data type"); //< should have been checked before
        throw tgt::Exception("Unknown data type: " + dataType);   
    }

    // open image
    ILuint ImageName;
    ilGenImages(1, &ImageName);
    ilBindImage(ImageName);
    if (!ilLoad(IL_TIF, filename.c_str())) {
        int errorCode = ilGetError();
        ilDeleteImages(1, &ImageName);
        throw tgt::CorruptedFileException("Failed to open tiff file (error code=" + itos(errorCode) + ")", filename);
    }

    // check image properties
    if (ilGetInteger(IL_IMAGE_FORMAT) != IL_LUMINANCE) {
        ilDeleteImages(1, &ImageName);
        throw tgt::CorruptedFileException("Expected tiff image format 'IL_LUMINANCE'", filename);
    }
    if (ilGetInteger(IL_IMAGE_TYPE) != ilDataType) {
        ilDeleteImages(1, &ImageName);
        throw tgt::CorruptedFileException("Data type of tiff file does not match data type specified in OME XML", filename);
    }
    if (ilGetInteger(IL_IMAGE_WIDTH) != volumeDim.x) {
        ilDeleteImages(1, &ImageName);
        throw tgt::CorruptedFileException("Image width of tiff file does not match image width specified in OME XML", filename);
    }
    if (ilGetInteger(IL_IMAGE_HEIGHT) != volumeDim.y) {
        ilDeleteImages(1, &ImageName);
        throw tgt::CorruptedFileException("Image height of tiff file does not match image width specified in OME XML", filename);
    }
    if (ilGetInteger(IL_IMAGE_DEPTH) != 1) {
        ilDeleteImages(1, &ImageName);
        throw tgt::CorruptedFileException("Image depth of tiff file is greater 1: " + ilGetInteger(IL_IMAGE_DEPTH), filename);
    }

    // copy image data to corresponding position within target data set
    if (!ilCopyPixels(0, 0, 0, static_cast<ILuint>(volumeDim.x), static_cast<ILuint>(volumeDim.y), 1, IL_LUMINANCE, ilDataType, destBuffer)) {
        ilDeleteImages(1, &ImageName);
        throw tgt::CorruptedFileException("Failed to copy pixel data: " + ilGetError(), filename);
    }

    ilDeleteImages(1, &ImageName);
} */

} // namespace voreen
