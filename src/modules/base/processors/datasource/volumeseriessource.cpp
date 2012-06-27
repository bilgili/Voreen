/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/modules/base/processors/datasource/volumeseriessource.h"

#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/processors/processorwidgetfactory.h"
#include "voreen/core/ports/allports.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/textfilereader.h"
#include "voreen/core/voreenapplication.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"

namespace voreen {

const std::string VolumeSeriesSource::loggerCat_("voreen.VolumeSeriesSource");

VolumeSeriesSource::VolumeSeriesSource()
    : Processor(),
      volumeHandle_(0),
      filename_("seriesfile", "Volume Series File", "Select Volume Series File",
                VoreenApplication::app()->getVolumePath(), "Volume Series Files (*.sdat)",
                FileDialogProperty::OPEN_FILE, Processor::INVALID_RESULT),
      step_("step", "Time Step", 0, 0, 1000),
      outport_(Port::OUTPORT, "volumehandle.volumehandle", 0),
      needUpload_(false)
{
    addProperty(filename_);
    filename_.onChange(CallMemberAction<VolumeSeriesSource>(this, &VolumeSeriesSource::openSeries));

    addProperty(step_);
    step_.setTracking(false);
    step_.onChange(CallMemberAction<VolumeSeriesSource>(this, &VolumeSeriesSource::loadStep));

    addPort(outport_);
}

std::string VolumeSeriesSource::getProcessorInfo() const {
    return  "Supplies a single volume out of a series of (time-varying) volume files. "
        "The series is defined in a .sdat file, which is like a .dat file, just with multiple "
        "'ObjectFileName:' entries, each corresponding to a single volume file. "
        "In contrast to the VolumeCollectionSource, the VolumeSeriesSource is intended to be used "
        "for large data series and therefore holds only the current time step in memory.<br/>"
        "See volumeseries workspace archive on www.voreen.org for an example.";
}

Processor* VolumeSeriesSource::create() const {
    return new VolumeSeriesSource();
}

void VolumeSeriesSource::process() {
    if (needUpload_) {
        VolumeTexture* tex = const_cast<VolumeTexture*>(volumeHandle_.getVolumeGL()->getTexture());
        Volume* vol = volumeHandle_.getVolume();
        if (vol) {
            tex->setPixelData(reinterpret_cast<GLubyte*>(vol->getData()));
            tex->uploadTexture();
            tex->setPixelData(0); // so that tex dtor will not try to free it
        }
        needUpload_ = false;
    }
}

void VolumeSeriesSource::initialize() throw (VoreenException) {
    Processor::initialize();
    outport_.setData(&volumeHandle_);
    loadStep();
}

void VolumeSeriesSource::loadStep() {
    Volume* v = volumeHandle_.getVolume();
    if (!v)
        return;

    int step = step_.get();
    std::string filename;

    if (files_.size() > 0 && step < static_cast<int>(files_.size()))
        filename = files_[step];
    else
        return;

    std::ifstream f(filename.c_str(), std::ios_base::binary);
    if (!f) {
        LERROR("Could not open file: " << filename);
        return;
    }

    // read the volume as a raw blob, should be fast
    LINFO("Loading raw file " << filename);
    f.read(reinterpret_cast<char*>(v->getData()), v->getNumBytes());
    if (!f.good()) {
        LERROR("Reading from file failed: " << filename);
        return;
    }

    // Special handling for float volumes: normalize values to [0.0; 1.0]
    VolumeFloat* vf = dynamic_cast<VolumeFloat*>(v);
    if (vf && spreadMin_ != spreadMax_) {
        const size_t n = vf->getNumVoxels();

        // use spread values if available
        if (spreadMin_ != spreadMax_) {
            const float d = spreadMax_ - spreadMin_;
            float* voxel = vf->voxel();
            for (size_t i = 0; i < n; ++i)
                voxel[i] = (voxel[i] - spreadMin_) / d;
        } else {
            LINFO("Normalizing float data to [0.0; 1.0]. "
                  << "This might not be what you want, better define 'Spread: <min> <max>' in the .sdat file.");
            const float d = vf->max() - vf->min();
            const float p = vf->min();
            float* voxel = vf->voxel();
            for (size_t i = 0; i < n; ++i)
                voxel[i] = (voxel[i] - p) / d;
        }
        vf->invalidate();
    }

    needUpload_ = true;
    invalidate();
}

void VolumeSeriesSource::openSeries() {
    try {
        TextFileReader reader(filename_.get());
        if (!reader)
            throw tgt::FileNotFoundException("Reading sdat file failed", filename_.get());

        std::string type;
        std::istringstream args;
        std::string mode, format;
        tgt::ivec3 resolution(0);
        tgt::vec3 sliceThickness(1.f);
        spreadMin_ = 0.f;
        spreadMax_ = 0.f;

        files_.clear();
        std::string blockfile;

        while (reader.getNextLine(type, args, false)) {
            if (type == "ObjectFileName:") {
                std::string s;
                args >> s;
                LDEBUG(type << " " << s);
                files_.push_back(tgt::FileSystem::dirName(filename_.get()) + "/" + s);
            } else if (type == "Resolution:") {
                args >> resolution[0];
                args >> resolution[1];
                args >> resolution[2];
                LDEBUG(type << " " << resolution[0] << " x " <<
                       resolution[1] << " x " << resolution[2]);
            } else if (type == "SliceThickness:") {
                args >> sliceThickness[0] >> sliceThickness[1] >> sliceThickness[2];
                LDEBUG(type << " " << sliceThickness[0] << " "
                       << sliceThickness[1] << " " << sliceThickness[2]);
            } else if (type == "Format:") {
                args >> format;
                LDEBUG(type << " " << format);
            } else if (type == "Spread:") {
                args >> spreadMin_ >> spreadMax_;
                LDEBUG(type << " " << spreadMin_ << " " << spreadMax_);
            } else {
                LWARNING("Unknown type: " << type);
            }

            if (args.fail()) {
                LERROR("Format error");
            }
        }

        volumeHandle_.freeHardwareVolumes();

        if (format == "UCHAR") {
            VolumeUInt8* vol8 = new VolumeUInt8(resolution, sliceThickness);
            volumeHandle_.setVolume(vol8);
        }
        else if (format == "USHORT") {
            VolumeUInt16* vol16 = new VolumeUInt16(resolution, sliceThickness);
            volumeHandle_.setVolume(vol16);
        }
        else if (format == "FLOAT") {
            VolumeFloat* vf = new VolumeFloat(resolution, sliceThickness);
            volumeHandle_.setVolume(vf);
        }
        else {
            LERROR("Unsupported format: " << format);
            volumeHandle_.setVolume(0);
            return;
        }

        if (step_.get() >= static_cast<int>(files_.size()))
            step_.set(0);
        step_.setMaxValue(files_.size() - 1);
    }
    catch (tgt::FileException) {
        LERROR("Loading failed");
    }
    catch (std::bad_alloc) {
        LERROR("Out of memory");
    }

    loadStep();
}

} // namespace
