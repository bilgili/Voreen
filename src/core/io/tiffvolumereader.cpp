/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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
#include "voreen/core/io/ioprogress.h"

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

TiffVolumeReader::TiffVolumeReader(IOProgress* progress) : VolumeReader(progress)
{
    name_ = "Tiff Stack Reader";
    extensions_.push_back("tiff");
    extensions_.push_back("tif");
}

VolumeSet* TiffVolumeReader::read(const std::string &fileName)
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
            std::istringstream stream(static_cast<char*>(data));
            TextFileReader reader(&stream);
            reader.setSeparators("=");
            LDEBUG(static_cast<char*>(data));
            string type;
            std::istringstream args;
            while (reader.getNextLine(type, args, false)) {
                LDEBUG(type << ": " << args.str());
                if (type == "Band") {
                    args >> band;
                    LINFO("Band: " << band);
                }
                else if (type == "Z") {
                    args >> slices;
                    LINFO("Slices: " << slices);
                    dimensions.z = slices;
                }
                else {
                    // Parse lines of type <type> <value> and log results
                    // Later on these data should be filled into the metadata structure of a volume
                    int value;
                    int pos = type.size() - 1;
                    while (isdigit(type[pos]) && pos > 0)
                        --pos;
                    type = type.substr(0, pos+1);
                    std::stringstream valueStr(type.substr(pos+1, type.size()-1));
                    valueStr >> value;
                    LDEBUG("Type: " << type << " with value: " << value);
                }
            }
        }
        else
            LERROR("Error");
        TIFFClose(tif);
        LINFO("depth: " << depth << " bps: " << bps);
    }
    else {
        LERROR("Failed to open tiffstack");
        return 0;
    }

    LINFO("stacking " << dimensions.z*band << " images with dimensions (" << dimensions.x
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
    if (tif) {
        int* minValue = new int[band];
        int* maxValue = new int[band];
        for (int i=0; i<band; ++i) {
            minValue[i] = 65536;
            maxValue[i] = 0;
        }
        //(TIFFReadDirectory(tif));
        uint32 width, height;
        uint16 depth_, bps_;

        if (getProgress())
            getProgress()->setNumSteps(dimensions.z*band);

        for (int i=0; i < dimensions.z*band; i++) {
            TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
            TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
            TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &depth_);
            TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps_);

            // if size or type of current image do not match skip the image..
            if ((dimensions.x != static_cast<int>(width)) || (dimensions.y != static_cast<int>(height)) || (bps != bps_) || (depth_ != depth)) {
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
                    if (static_cast<int>(result = TIFFReadEncodedStrip (tif, stripCount,
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
                for (int j=0; j<dimensions.x*dimensions.y; ++j) {
                    int value;
                    if (use8BitDataset)
                        value = buffer[j];
                    else
                        value = buffer[j*2] + 256*buffer[j*2+1];
                    if (minValue[currentBand] > value)
                        minValue[currentBand] = value;
                    if (maxValue[currentBand] < value)
                        maxValue[currentBand] = value;
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

            if (getProgress())
                getProgress()->set(i);
        }

        TIFFClose(tif);
        for (int i=0; i<band; ++i) {
            LINFO("Band " << i << ": min/max value: " << minValue[i] << "/" << maxValue[i]);
            if ( !use8BitDataset && maxValue[i] < 4096) {
                LINFO("Band " << i << ": Recognized 12 bit dataset.");
                targetDataset[i]->setBitsStored(12);
            }
        }
        delete[] minValue;
        delete[] maxValue;
    }
    else {
        LERROR("Failed to open tiffstack");
        return 0;
    }

    VolumeSet* volumeSet = new VolumeSet(tgt::FileSystem::fileName(fileName));
    VolumeSeries* volumeSeries = new VolumeSeries("unknown", Modality::MODALITY_UNKNOWN);
    volumeSet->addSeries(volumeSeries);
    for ( int i = 0; i < band; i++ ) {
        VolumeHandle* volumeHandle = new VolumeHandle(targetDataset[i], static_cast<float>(i));
        volumeHandle->setOrigin(fileName, "unknown", static_cast<float>(i));
        volumeSeries->addVolumeHandle(volumeHandle);
    }
    return volumeSet;
}

} // namespace voreen
