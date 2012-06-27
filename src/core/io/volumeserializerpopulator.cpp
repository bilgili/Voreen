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

#include "voreen/core/io/volumeserializerpopulator.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/voreenmodule.h"
#include "voreen/core/io/volumeserializer.h"

#include "voreen/core/io/datvolumereader.h"
#include "voreen/core/io/datvolumewriter.h"
#include "voreen/core/io/rawvolumereader.h"
#include "voreen/core/io/brickedvolumereader.h"
#include "voreen/core/io/progressbar.h"

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

    if (VoreenApplication::app()) {
        // create progress bar
        if (showProgress) {
            progressDialog_ = VoreenApplication::app()->createProgressDialog();
            if (progressDialog_) {
                progressDialog_->setTitle("Loading volume");
                progressDialog_->setMessage("Loading volume ...");
            }
        }

        // retrieve volume readers/writers from modules
        std::vector<VoreenModule*> modules = VoreenApplication::app()->getModules();
        for (size_t i=0; i<modules.size(); i++) {
            for (size_t j=0; j<modules.at(i)->getVolumeReaders().size(); j++) {
                readers_.push_back(modules.at(i)->getVolumeReaders().at(j)->create(progressDialog_));
            }
            for (size_t j=0; j<modules.at(i)->getVolumeWriters().size(); j++) {
                writers_.push_back(modules.at(i)->getVolumeWriters().at(j)->create(progressDialog_));
            }
        }
    }
    else {
        LWARNINGC("voreen.VolumeSerializerPopulator", "VoreenApplication not instantiated");
    }

    readers_.push_back(new DatVolumeReader(progressDialog_));
    readers_.push_back(new RawVolumeReader(progressDialog_));
    readers_.push_back(new BrickedVolumeReader(progressDialog_) );

#ifdef VRN_WITH_TIFF
    readers_.push_back(new TiffVolumeReader(progressDialog_));
#endif

#ifdef VRN_WITH_ZLIB
    readers_.push_back(new ZipVolumeReader(this, progressDialog_));
#endif

    // populate array with all known VolumeReaders
    // --> if an FormatClashException occurs here it is an error in this method
    for (size_t i = 0; i < readers_.size(); ++i) {
        try {
            vs_->registerReader(readers_[i]);
        }
        catch (const FormatClashException& e) {
            LWARNINGC("voreen.VolumeSerializerPopulator", e.what());
        }
    }

    // populate array with all known VolumeWriters
    // --> if an FormatClashException occurs here it is an error in this method
    writers_.push_back(new DatVolumeWriter());

    for (size_t i = 0; i < writers_.size(); ++i) {
        try {
            vs_->registerWriter(writers_[i]);
        }
        catch (const FormatClashException& e) {
            LWARNINGC("voreen.VolumeSerializerPopulator", e.what());
        }
    }
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
