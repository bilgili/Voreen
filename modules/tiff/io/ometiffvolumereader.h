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

#ifndef VRN_OMEVOLUMEREADER_H
#define VRN_OMEVOLUMEREADER_H

#include "voreen/core/io/volumereader.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"

#include <tiffio.h>
#include <IL/il.h>

class TiXmlDocument;

namespace voreen {

class IOProgress;

/**
 * Reader for microscopy data stored in OME-TIFF format.
 *
 * @note Requires the tiff library.
 */
class OMETiffVolumeReader : public VolumeReader {
public:

    enum OMEDataType {
        OME_INT8,
        OME_INT16,
        OME_INT32,
        OME_UINT8,
        OME_UINT16,
        OME_UINT32,
        OME_FLOAT,
        OME_UNKNOWN
    };

    OMETiffVolumeReader(ProgressBar* progress = 0);
    ~OMETiffVolumeReader() {}
    virtual VolumeReader* create(ProgressBar* progress = 0) const;

    virtual std::string getClassName() const   { return "OMETiffVolumeReader"; }
    virtual std::string getFormatDescription() const { return "OME-TIFF microscopy data"; }

    virtual VolumeList* read(const std::string& url)
        throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc);

private:
    void extractMetaData(TIFF* tiffFile, tgt::ivec3& dimensions, int& sizeC, int& sizeT, OMEDataType& dataType,
        tgt::vec3& spacing, std::vector<std::string>& filenames) const
        throw (tgt::CorruptedFileException);

    template <class T>
    void readTiffSlice(VolumeAtomic<T>* destVolume, ILint ilDataType, const std::string& filename, size_t sliceID) const
        throw (tgt::CorruptedFileException);

    // XPath-like XML helper functions
    const TiXmlNode* getXMLNode(const TiXmlNode* parent, const std::string& path) const;
    std::vector<const TiXmlNode*> getXMLNodeList(const TiXmlNode* parent, const std::string& path, const std::string& nodeName) const;

    OMEDataType omeDataTypeFromStr(const std::string& pixelTypeStr) const;
    std::string omeDataTypeToStr(OMEDataType type) const;

    static const std::string loggerCat_;
};

// --------------------
// template definitions

template <class T>
void voreen::OMETiffVolumeReader::readTiffSlice(VolumeAtomic<T>* destVolume, ILint ilDataType,
        const std::string& filename, size_t sliceID) const throw (tgt::CorruptedFileException) {

    tgtAssert(destVolume, "no dest volume");
    tgtAssert(sliceID < destVolume->getDimensions().z, "invalid slice id");

    ILuint ImageName;
    ilGenImages(1, &ImageName);
    ilBindImage(ImageName);

    // open image
    if (!ilLoad(IL_TIF, filename.c_str())) {
        int errorCode = ilGetError();
        ilDeleteImages(1, &ImageName);
        throw tgt::CorruptedFileException("Failed to open tiff file (error code=" + itos(errorCode) + ")", filename);
    }

    tgt::svec2 sliceDim = destVolume->getDimensions().xy();

    // check image properties
    if (ilGetInteger(IL_IMAGE_FORMAT) != IL_LUMINANCE) {
        ilDeleteImages(1, &ImageName);
        throw tgt::CorruptedFileException("Expected tiff image format 'IL_LUMINANCE'", filename);
    }
    if (ilGetInteger(IL_IMAGE_TYPE) != ilDataType) {
        ilDeleteImages(1, &ImageName);
        throw tgt::CorruptedFileException("Data type of tiff file does not match data type specified in OME XML", filename);
    }
    if (ilGetInteger(IL_IMAGE_WIDTH) != sliceDim.x) {
        ilDeleteImages(1, &ImageName);
        throw tgt::CorruptedFileException("Image width of tiff file does not match image width specified in OME XML", filename);
    }
    if (ilGetInteger(IL_IMAGE_HEIGHT) != sliceDim.y) {
        ilDeleteImages(1, &ImageName);
        throw tgt::CorruptedFileException("Image height of tiff file does not match image width specified in OME XML", filename);
    }
    if (ilGetInteger(IL_IMAGE_DEPTH) != 1) {
        ilDeleteImages(1, &ImageName);
        throw tgt::CorruptedFileException("Image depth of tiff file is greater 1: " + ilGetInteger(IL_IMAGE_DEPTH), filename);
    }

    // copy image data to corresponding position within target data set
    size_t offset = tgt::hmul(sliceDim) * sliceID /* * destVolume->getBytesPerVoxel()*/;
    if (!ilCopyPixels(0, 0, 0, static_cast<ILuint>(sliceDim.x), static_cast<ILuint>(sliceDim.y), 1, IL_LUMINANCE, ilDataType, destVolume->voxel() + offset)) {
        ilDeleteImages(1, &ImageName);
        throw tgt::CorruptedFileException("Failed to copy pixel data: " + ilGetError(), filename);
    }

    ilDeleteImages(1, &ImageName);
}


} // namespace voreen

#endif // VRN_OMEVOLUMEREADER_H
