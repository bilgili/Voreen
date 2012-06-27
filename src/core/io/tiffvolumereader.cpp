/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/io/tiffvolumereader.h"
#include "voreen/core/io/textfilereader.h"

#include <fstream>
#include <iostream>
#include <assert.h>
#include <tiffio.h>

#include "tgt/exception.h"
#include "tgt/vector.h"

using std::string;
using tgt::vec3;
using tgt::ivec3;
using tgt::Texture;

namespace voreen {

const std::string TiffVolumeReader::loggerCat_ = "voreen.io.VolumeReader.tiff";

VolumeSet* TiffVolumeReader::read(const std::string &fileName, bool generateVolumeGL)
    throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{
    ivec3 dimensions;
    int band = 1;

    LINFO(fileName);

    TIFF* tif = TIFFOpen(fileName.c_str(), "r");
    if (tif) {
        int dircount = 0;
        do {
            dircount++;
        } while (TIFFReadDirectory(tif));
        LDEBUG(dircount << " directories found");
        dimensions.z = dircount;
        TIFFClose(tif);
    }
    else {
        LERROR("Failed to open tiffstack");
        return 0;
    }

    uint16 depth, bps;
    tif = TIFFOpen(fileName.c_str(), "r");

    if (tif) {
		//TIFFReadDirectory(tif));
        uint32 width, height;

        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &depth);
        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);
        dimensions.x = width;
        dimensions.y = height;

        uint16 count;
        void *data;
        int slices;
        if (TIFFGetField(tif, 33471, &count, &data)) {
            std::istringstream stream((char*)data);
            TextFileReader reader(&stream);
            reader.setSeparators("=");
            LDEBUG((char*)data);
            string type;
            std::istringstream args;
            while (reader.getNextLine(type, args, false)) {
                LDEBUG(type << ": " << args.str());
                if (type == "Band") {
                    args >> band;
                    LDEBUG("Band: " << band);
                }
                else if (type == "Z") {
                    args >> slices;
                    LDEBUG("Slices: " << slices);
                    dimensions.z = slices;
                }
                else {
                    // FIXME: No idea what this is good for
                    int value;
                    int pos = type.size() - 1;
                    while (isdigit(type[pos]) && pos > 0)
                        --pos;
                    type = type.substr(0, pos+1);
                    std::stringstream valueStr(type.substr(pos+1, type.size()-1));
                    valueStr >> value;
                }
            }
        }
        else
            LERROR("Error");
        TIFFClose(tif);
        LDEBUG("depth: " << depth << " bps: " << bps);
    }
    else
    {
        LERROR("Failed to open tiffstack");
        return 0;
    }

    LDEBUG("stacking " << dimensions.z*band << " images with dimensions (" << dimensions.x
          << ", " << dimensions.y << ") into " << band << " datasets.");
    std::vector<Volume*> targetDataset;
    std::vector<uint8_t*> scalars8;
    std::vector<uint16_t*> scalars16;
    bool use8BitDataset;
    if (bps == 8) {
        use8BitDataset = true;
    }

    else {
        if (bps != 16)
            LWARNING("Couldn't recognize bit depth. Trying 16 bit...");
        use8BitDataset = false;
    }

    if (use8BitDataset) {
        for (int i=0; i<band; ++i) {
            targetDataset.push_back(new VolumeUInt8(dimensions));
            scalars8.push_back(reinterpret_cast<uint8_t*>(((VolumeUInt8*)targetDataset[i])->voxel()));
        }
    }
    else {
        for (int i=0; i<band; ++i) {
            targetDataset.push_back(new VolumeUInt16(dimensions));
            scalars16.push_back(reinterpret_cast<uint16_t*>(((VolumeUInt16*)targetDataset[i])->voxel()));
        }
    }

    tif = TIFFOpen(fileName.c_str(), "r");
    int minValue = 65536;
    int maxValue = 0;
    if (tif) {
		//(TIFFReadDirectory(tif));
        uint32 width, height;
        uint16 depth_, bps_;

        for(int i=0; i < dimensions.z*band; i++) {
            TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
            TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
            TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &depth_);
            TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps_);

            // if size or type of current image do not match skip the image..
            if((dimensions.x != (int)width) || (dimensions.y != (int)height) || (bps != bps_) || (depth_ != depth)) {
                LWARNING("Images dimensions of " << i << ". image do not match!");
            }
            else {
                int currentBand = i % band;
                // Read in the possibly multiple strips
                int stripMax, stripCount;
                tsize_t stripSize;
                unsigned long imageOffset, result;
                unsigned long bufferSize;
                stripSize = TIFFStripSize (tif);
                stripMax = TIFFNumberOfStrips (tif);

                imageOffset = 0;

                bufferSize = stripMax * stripSize;
                uint8_t* buffer = new uint8_t[bufferSize];

                for (stripCount = 0; stripCount < stripMax; stripCount++) {
                    if((int)(result = TIFFReadEncodedStrip (tif, stripCount,
                                    buffer + imageOffset,
                                    stripSize)) == -1){
                        LERROR("Read error on input strip number " << stripCount);
                        delete[] buffer;
                        for (int i=0; i<band; ++i)
                            delete targetDataset[i];
                        TIFFClose(tif);
                        return 0;

                    }
                    imageOffset += result;
                }
                for (int i=0; i<dimensions.x*dimensions.y; ++i) {
                    int value;
                    if (use8BitDataset)
                        value = buffer[i];
                    else
                        value = buffer[i*2] + 256*buffer[i*2+1];
                    if (minValue > value)
                        minValue = value;
                    if (maxValue < value)
                        maxValue = value;
                }
                if (use8BitDataset) {
                    memcpy(scalars8[currentBand], buffer, dimensions.x * dimensions.y);
                    scalars8[currentBand] += dimensions.x * dimensions.y;
                }
                else {
                    memcpy(scalars16[currentBand], buffer, dimensions.x * dimensions.y * 2);
                    scalars16[currentBand] += dimensions.x * dimensions.y;
                }
                delete[] buffer;
            }
            TIFFReadDirectory(tif);
        }

        TIFFClose(tif);
        LINFO("min/max value: " << minValue << "/" << maxValue);
		//if ( !use8BitDataset && maxValue < 4096) {
			//LINFO("Recognized 12 bit dataset.");
			//for (int i=0; i<band; ++i)
				//targetDataset[i]->setBitsStored(12);
		//}
    }
    else
    {
        LERROR("Failed to open tiffstack");
        return 0;
    }

    VolumeSet* volumeSet = new VolumeSet(fileName);
    VolumeSeries* volumeSeries = new VolumeSeries(volumeSet, "unknown", Modality::MODALITY_UNKNOWN);
    volumeSet->addSeries(volumeSeries);
    for( int i = 0; i < band; i++ ) {   
        VolumeHandle* volumeHandle = new VolumeHandle(volumeSeries, targetDataset[i], static_cast<float>(i));
        volumeSeries->addVolumeHandle(volumeHandle);
        if( generateVolumeGL == true )
            volumeHandle->generateHardwareVolumes(VolumeHandle::HARDWARE_VOLUME_GL);
    }
    return volumeSet;
}

} // namespace voreen
