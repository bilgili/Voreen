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

#include "voreen/modules/base/processors/datasource/imagesequencesource.h"

#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/ports/allports.h"
#include "voreen/core/voreenapplication.h"
#include "voreen/core/datastructures/imagesequence.h"
#include "voreen/core/io/ioprogress.h"

#include "tgt/filesystem.h"
#include "tgt/gpucapabilities.h"

namespace voreen {

const std::string ImageSequenceSource::loggerCat_("voreen.ImageSequenceSource");

ImageSequenceSource::ImageSequenceSource()
    : RenderProcessor(),
      outport_(Port::OUTPORT, "imagesequence.out"),
      imageDirectory_("imageDirectory","Image Directory", "Select Image Directory",
          "", "", FileDialogProperty::DIRECTORY),
      textureFiltering_("textureFiltering", "Enable Texture Filtering", true),
      uploadTextureData_("uploadTextures", "Upload Textures", true),
      reloadSequence_("reloadSequence", "Reload"),
      clearSequence_("clearSequence", "Clear Sequence"),
      numImages_("numImages", "Num Images", 0, 0, 1000, false, VALID),
      imageSequence_(0),
      currentDir_(""),
      sequenceOwner_(false),
      forceReload_(false)
{
    addPort(outport_);

    textureFiltering_.onChange(CallMemberAction<ImageSequenceSource>(this, &ImageSequenceSource::forceReload));
    uploadTextureData_.onChange(CallMemberAction<ImageSequenceSource>(this, &ImageSequenceSource::forceReload));
    reloadSequence_.onClick(CallMemberAction<ImageSequenceSource>(this, &ImageSequenceSource::forceReload));
    clearSequence_.onClick(CallMemberAction<ImageSequenceSource>(this, &ImageSequenceSource::unsetDirectoryName));
    numImages_.setWidgetsEnabled(false);

    addProperty(imageDirectory_);
    addProperty(textureFiltering_);
    addProperty(uploadTextureData_);
    addProperty(reloadSequence_);
    addProperty(clearSequence_);
    addProperty(numImages_);
}

ImageSequenceSource::~ImageSequenceSource() {
}

Processor* ImageSequenceSource::create() const {
    return new ImageSequenceSource();
}

std::string ImageSequenceSource::getProcessorInfo() const {
    return "Loads all images from the specified directory and puts them out as image sequence "
           "containing one OpenGL texture per image.";
}

void ImageSequenceSource::process() {
    if (forceReload_) {
        loadImageSequence(imageDirectory_.get());
    }
}

void ImageSequenceSource::initialize() throw (VoreenException) {

    RenderProcessor::initialize();

    imageSequence_ = new ImageSequence();
    sequenceOwner_ = true;
    initialized_ = true;

    if (!imageDirectory_.get().empty()) {
        //currentDir_ = imageDirectory_.get();
        loadImageSequence(imageDirectory_.get());
    }
    else {
        numImages_.set(0);
    }
}

void ImageSequenceSource::deinitialize() throw (VoreenException) {
    outport_.setData(0);
    clearSequence();
    delete imageSequence_;
    imageSequence_ = 0;

    RenderProcessor::deinitialize();
}

void ImageSequenceSource::invalidate(int inv) {

    RenderProcessor::invalidate(inv);

    if (inv == Processor::VALID)
        return;

    if (!isInitialized())
        return;

    if (imageDirectory_.get() != currentDir_) {
        forceReload_ = true;
    }
}

void ImageSequenceSource::loadImageSequence(const std::string& d)
    throw (tgt::FileException, std::bad_alloc) {

    if (!isInitialized()) {
        LERROR("loadImageSequence(): not initialized");
        return;
    }

    // important: d might be cleared by clearSequence
    std::string dir(d);

    forceReload_ = false;

    if (!imageSequence_) {
        LERROR("No image sequence present");
        return;
    }

    // clear current sequence
    clearSequence();

    if (dir.empty())
        return;

    LINFO("Loading images from directory " << dir << " ...");
    currentDir_ = dir;
    imageDirectory_.set(dir);

    // load images as textures and collect them in an image sequence
    std::vector<std::string> filenames = tgt::FileSystem::readDirectory(dir, true, false);

    // create progress bar
    IOProgress* progressDialog = 0;
    if (!filenames.empty()) {
        progressDialog = VoreenApplication::app()->createProgressDialog();
        progressDialog->setTitle("Loading Images");
        progressDialog->setTotalSteps(filenames.size());
        progressDialog->show();
        progressDialog->setProgress(0);
        progressDialog->forceUpdate();
    }

    tgt::Texture::Filter filterMode = textureFiltering_.get() ? tgt::Texture::LINEAR : tgt::Texture::NEAREST;
    for (size_t i=0; i<filenames.size(); ++i) {
        if (progressDialog) {
            progressDialog->setMessage("Loading " + filenames[i] + " ...");
            progressDialog->setProgress(i);
        }
        tgt::Texture* texture = TexMgr.load(dir + "/" + filenames[i], filterMode,
            false, false, true, false, !GpuCaps.isNpotSupported());
        if (texture)
            imageSequence_->add(texture);
        else
            LWARNING("Failed to load image: " << filenames[i]);
    }
    LGL_ERROR;

    // upload texture data
    if (uploadTextureData_.get()) {
        LINFO("Uploading texture data ...");
        for (unsigned int i=0; i<imageSequence_->size(); i++)
            imageSequence_->at(i)->uploadTexture();
        LGL_ERROR;
    }

    // clear progress
    if (progressDialog) {
        progressDialog->hide();
        delete progressDialog;
        progressDialog = 0;
    }

    // output sequence
    outport_.setData(imageSequence_);

    LINFO("Loaded " << imageSequence_->size() << " images.");
    numImages_.set(imageSequence_->size());
    LGL_ERROR;
}


void ImageSequenceSource::reloadImageSequence() {
    if (!currentDir_.empty())
        forceReload();
}

void ImageSequenceSource::clearSequence() {

    if (!isInitialized()) {
        LERROR("loadImageSequence(): not initialized");
        return;
    }

    if (sequenceOwner_ && imageSequence_) {
        if (!imageSequence_->empty()) {
            LINFO("Clearing sequence");
            for (size_t i=0; i<imageSequence_->size(); ++i) {
                delete imageSequence_->at(i);
            }
            imageSequence_->clear();
            LGL_ERROR;
        }
    }
    else {
        imageSequence_ = new ImageSequence();
        sequenceOwner_ = true;
    }
    currentDir_ = "";
    imageDirectory_.set("");
    numImages_.set(0);
}

void ImageSequenceSource::setImageSequence(ImageSequence* sequence) {

    if (!isInitialized()) {
        LERROR("loadImageSequence(): not initialized");
        return;
    }

    clearSequence();  // now owner of the sequence -> delete it before assigning
    delete imageSequence_;

    imageSequence_ = sequence;
    sequenceOwner_ = false;

    outport_.setData(imageSequence_);
    outport_.invalidate();
    invalidate();

    numImages_.set(sequence ? sequence->size() : 0);
}

const ImageSequence* ImageSequenceSource::getImageSequence() const {
    return imageSequence_;
}

void ImageSequenceSource::forceReload() {
    forceReload_ = true;
    invalidate();
}

void ImageSequenceSource::unsetDirectoryName() {
    imageDirectory_.set("");
}

} // namespace
