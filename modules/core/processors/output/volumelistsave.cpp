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

#include "volumelistsave.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/utils/stringutils.h"
#include "tgt/filesystem.h"

namespace voreen {

const std::string VolumeListSave::loggerCat_("voreen.core.VolumeListSave");

VolumeListSave::VolumeListSave()
    : VolumeProcessor()
    , inport_(Port::INPORT, "volumehandle.input", "VolumeList Input", false)
    , outputDirectory_("outputDirectory", "Directory", "Select directory...",
        "", "", FileDialogProperty::DIRECTORY)
    , baseName_("basename", "Basename")
    , saveButton_("save", "Save")
    , continousSave_("continousSave", "Save continuously", false)
    , volumeSerializerPopulator_(0)
{
    addPort(inport_);

    addProperty(outputDirectory_);
    addProperty(baseName_);

    saveButton_.onChange(CallMemberAction<VolumeListSave>(this, &VolumeListSave::saveVolumeList));
    addProperty(saveButton_);
    addProperty(continousSave_);
}

VolumeListSave::~VolumeListSave() {
    delete volumeSerializerPopulator_;
}

Processor* VolumeListSave::create() const {
    return new VolumeListSave();
}

void VolumeListSave::initialize() throw (tgt::Exception) {

    VolumeProcessor::initialize();

    tgtAssert(!volumeSerializerPopulator_, "serializer populator already created");
    volumeSerializerPopulator_ = new VolumeSerializerPopulator();
}

void VolumeListSave::deinitialize() throw (tgt::Exception) {
    delete volumeSerializerPopulator_;
    volumeSerializerPopulator_ = 0;

    VolumeProcessor::deinitialize();
}

void VolumeListSave::process() {
    if (inport_.hasChanged() && continousSave_.get())
        saveVolumeList();
}

void VolumeListSave::saveVolumeList() {
    if (!isInitialized())
        return;

    tgtAssert(volumeSerializerPopulator_, "no populator");

    if (!inport_.hasData() || inport_.getData()->empty())
        return;
    if (outputDirectory_.get() == "")
        return;

    std::string directory = outputDirectory_.get();
    const VolumeSerializer* serializer = volumeSerializerPopulator_->getVolumeSerializer();
    const VolumeList* inputCollection = inport_.getData();
    tgtAssert(inputCollection, "no collection");
    const std::string maxNr = itos(inputCollection->size() - 1);
    for (size_t i=0; i<inputCollection->size(); i++) {
        std::string volFilename;

        std::string nrSuffix = itos(i);
        nrSuffix = std::string(maxNr.length()-nrSuffix.length(), '0') + nrSuffix;
        if (baseName_.get() != "")
            volFilename = baseName_.get() + nrSuffix;
        else if (dynamic_cast<Volume*>(inputCollection->at(i))) {
            volFilename = tgt::FileSystem::fileName(static_cast<Volume*>(inputCollection->at(i))->getOrigin().getFilename());
            if (volFilename == "")
                volFilename = "volume" + nrSuffix;
        }

        tgtAssert(volFilename != "", "no volFilename");
        if (tgt::FileSystem::fileExtension(volFilename) == "")
            volFilename += ".vvd";
        std::string outputFilename = tgt::FileSystem::cleanupPath(directory + "/" + volFilename);

        try {
            serializer->write(outputFilename, inputCollection->at(i));
        }
        catch (tgt::FileException e) {
            LERROR("Failed to save volume to file '" << outputFilename << "': " << e.what());
        }

    }
}


}   // namespace
