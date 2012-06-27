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

#include "voreen/core/io/volumeserializerpopulator.h"

#include "voreen/core/io/datvolumereader.h"
#include "voreen/core/io/datvolumewriter.h"
#include "voreen/core/io/nrrdvolumereader.h"
#include "voreen/core/io/nrrdvolumewriter.h"
#include "voreen/core/io/philipsusvolumereader.h"
#include "voreen/core/io/quadhidacvolumereader.h"
#include "voreen/core/io/rawvolumereader.h"
#include "voreen/core/io/tuvvolumereader.h"
#include "voreen/core/io/interfilevolumereader.h"
#include "voreen/core/io/brickedvolumereader.h"
#include "voreen/core/io/multivolumereader.h"

#include "voreen/core/io/volumeserializer.h"

#include "voreen/core/application.h"

#ifdef VRN_MODULE_FLOWREEN
#include "voreen/modules/flowreen/flowreader.h"
#endif

#ifdef VRN_WITH_DCMTK
    #include "voreen/core/io/dicomvolumereader.h"
#endif

#ifdef VRN_WITH_MATLAB
    #include "voreen/core/io/matvolumereader.h"
#endif

#ifdef VRN_WITH_PVM
    #include "voreen/core/io/pvmvolumereader.h"
#endif

#ifdef VRN_WITH_TIFF
    #include "voreen/core/io/tiffvolumereader.h"
#endif

#ifdef VRN_WITH_ZLIB
    #include "voreen/core/io/zipvolumereader.h"
#endif


namespace voreen {

VolumeSerializerPopulator::VolumeSerializerPopulator(bool showProgress)
  : vs_(new VolumeSerializer()),
    progressDialog_(0)
{

    if (showProgress) {
        progressDialog_ = VoreenApplication::app()->createProgressDialog();
        if(progressDialog_) {
            progressDialog_->setTitle("Loading volume");
            progressDialog_->setMessage("Loading volume ...");
        }
    }

    /*
        populate array with all known VolumeReaders
        --> if an FormatClashException occurs here it is an error in this method
    */
    readers_.push_back(new DatVolumeReader(progressDialog_));
    readers_.push_back(new NrrdVolumeReader());
    readers_.push_back(new QuadHidacVolumeReader());
    readers_.push_back(new RawVolumeReader(progressDialog_));
    readers_.push_back(new TUVVolumeReader());
    readers_.push_back(new InterfileVolumeReader());
    readers_.push_back(new BrickedVolumeReader(progressDialog_) );
    readers_.push_back(new MultiVolumeReader(this, progressDialog_));

#ifdef VRN_WITH_DCMTK
    readers_.push_back(new DicomVolumeReader(progressDialog_));
#else
    readers_.push_back(new PhilipsUSVolumeReader());
#endif

#ifdef VRN_MODULE_FLOWREEN
    readers_.push_back(new FlowReader(progressDialog_));
#endif

#ifdef VRN_WITH_MATLAB
    readers_.push_back(new MatVolumeReader());
#endif

#ifdef VRN_WITH_PVM
    readers_.push_back(new PVMVolumeReader(progressDialog_));
#endif

#ifdef VRN_WITH_TIFF
    readers_.push_back(new TiffVolumeReader(progressDialog_));
#endif

#ifdef VRN_WITH_ZLIB
    readers_.push_back(new ZipVolumeReader(this, progressDialog_));
#endif

    for (size_t i = 0; i < readers_.size(); ++i)
        vs_->registerReader(readers_[i]);

    /*
        populate array with all known VolumeWriters
        --> if an FormatClashException occurs here it is an error in this method
    */
    writers_.push_back(new DatVolumeWriter());
    writers_.push_back(new NrrdVolumeWriter());

    for (size_t i = 0; i < writers_.size(); ++i)
        vs_->registerWriter(writers_[i]);
}

VolumeSerializerPopulator::~VolumeSerializerPopulator() {
    delete vs_;

    for (size_t i = 0; i < readers_.size(); ++i)
        if (readers_[i] && !readers_[i]->isPersistent())
            delete readers_[i];

    for (size_t i = 0; i < writers_.size(); ++i)
        delete writers_[i];

    if (progressDialog_)
        progressDialog_->hide();
    delete progressDialog_;
}

const VolumeSerializer* VolumeSerializerPopulator::getVolumeSerializer() const {
    return vs_;
}

std::vector<std::string> VolumeSerializerPopulator::getSupportedReadExtensions() const {

    std::vector<std::string> extensions;
    for (size_t i=0; i<readers_.size(); ++i) {
        for (size_t j=0; j<readers_[i]->getExtensions().size(); ++j) {
            if (std::find(extensions.begin(), extensions.end(), readers_[i]->getExtensions()[j]) == extensions.end())
                extensions.push_back(readers_[i]->getExtensions()[j]);
        }
    }

    return extensions;
}

std::vector<std::string> VolumeSerializerPopulator::getSupportedWriteExtensions() const {

    std::vector<std::string> extensions;
    for (size_t i=0; i<writers_.size(); ++i) {
        for (size_t j=0; j<writers_[i]->getExtensions().size(); ++j) {
            if (std::find(extensions.begin(), extensions.end(), writers_[i]->getExtensions()[j]) == extensions.end())
                extensions.push_back(writers_[i]->getExtensions()[j]);
        }
    }

    return extensions;
}

} // namespace voreen
