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

#include "voreen/modules/base/processors/volume/volumesave.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/volumeserializerpopulator.h"

namespace voreen {

VolumeSave::VolumeSave()
    : VolumeProcessor(),
    inport_(Port::INPORT, "volumehandle.input", false, Processor::INVALID_RESULT),
    filename_("outputfilename", "File", "Select file...",
             "", "*.dat", FileDialogProperty::SAVE_FILE, Processor::VALID),
    continousSave_("continousSave", "Save continously", false, Processor::VALID),
    saveButton_("save", "Save"),
    volLoadPop_(0),
    serializer_(0)
{

    filename_.onChange(CallMemberAction<VolumeSave>(this, &VolumeSave::saveVolume));
    addProperty(filename_);

    addProperty(continousSave_);

    saveButton_.onChange(CallMemberAction<VolumeSave>(this, &VolumeSave::saveVolume));
    addProperty(saveButton_);

    addPort(inport_);
}

VolumeSave::~VolumeSave() {
    delete volLoadPop_;
    //serializer will be deleted by populator
}

Processor* VolumeSave::create() const {
    return new VolumeSave();
}

std::string VolumeSave::getProcessorInfo() const {
    return std::string("Saves a volume to disk.");
}

void VolumeSave::initialize() throw (VoreenException) {

    VolumeProcessor::initialize();

    if(!volLoadPop_)
        volLoadPop_ = new VolumeSerializerPopulator();
    if(!serializer_)
        serializer_ = volLoadPop_->getVolumeSerializer();

    initialized_ = true;
}

void VolumeSave::process() {
    if (inport_.hasChanged() && continousSave_.get())
        saveVolume();
}

void VolumeSave::saveVolume() {
    if(!inport_.getData() || !serializer_ || (filename_.get() == ""))
        return;

    serializer_->save(filename_.get(), inport_.getData());
}

bool VolumeSave::isEndProcessor() const {
    return true;
}

}   // namespace
