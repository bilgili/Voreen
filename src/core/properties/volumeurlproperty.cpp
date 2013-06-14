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

#include "voreen/core/properties/volumeurlproperty.h"

#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumelist.h"
#include "voreen/core/properties/volumeinfoproperty.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/progressbar.h"
#include "voreen/core/voreenapplication.h"

#include "tgt/filesystem.h"

namespace voreen {

const std::string VolumeURLProperty::loggerCat_("voreen.VolumeURLProperty");

VolumeURLProperty::VolumeURLProperty(const std::string& id, const std::string& guiText,
                    const std::string& url, int invalidationLevel)
    : StringProperty(id, guiText, url, invalidationLevel)
    , volume_(0)
    , volumeOwner_(false)
    , infoProp_(0)
    , progressBar_(0)
{}

VolumeURLProperty::VolumeURLProperty()
    : StringProperty("", "", "", Processor::INVALID_RESULT)
    , volume_(0)
    , volumeOwner_(false)
    , progressBar_(0)
{}

Property* VolumeURLProperty::create() const {
    return new VolumeURLProperty();
}

void VolumeURLProperty::deinitialize() throw (tgt::Exception) {
    std::string curURL = get();
    setVolume(0); //< also deletes the volume, if property is owner
    delete progressBar_;
    progressBar_ = 0;

    StringProperty::deinitialize();

    // restore URL (has been cleared by setVolume(0)
    set(curURL);
}

void VolumeURLProperty::set(const std::string& url) {
    if (getVolume() && getVolume()->getOrigin().getURL() != url)
        setVolume(0);
    StringProperty::set(url);
}

void VolumeURLProperty::setURL(const std::string& url) {
    set(url);
}

std::string VolumeURLProperty::getURL() const {
    return get();
}

void VolumeURLProperty::setVolume(VolumeBase* handle, bool owner) {

    if (handle != volume_)
        clear();

    volume_ = handle;
    volumeOwner_ = owner;

    if(infoProp_)
        infoProp_->setVolume(handle);

    set(handle ? handle->getOrigin().getURL() : "");
}

VolumeBase* VolumeURLProperty::getVolume() const {
    return volume_;
}

void VolumeURLProperty::loadVolume() throw (tgt::FileException, std::bad_alloc){

    std::string url = get();
    if (url.empty()) {
        LWARNING("loadVolume(): empty URL");
        return;
    }

    ProgressBar* progressBar = getProgressBar();
    if (progressBar) {
        progressBar->setTitle("Loading volume");
        progressBar->setMessage("Loading volume ...");
    }
    VolumeSerializerPopulator serializerPopulator(progressBar);
    VolumeList* volumeList = serializerPopulator.getVolumeSerializer()->read(url);

    if (volumeList && !volumeList->empty()) {
        VolumeBase* handle = volumeList->first();
        tgtAssert(handle, "No handle");
        setVolume(static_cast<Volume*>(handle));

        // delete superfluous volumes
        if (volumeList->size() > 1) {
            LWARNING("More than one volume loaded from file: " + url + ". Discarding surplus volumes!");
                for (size_t i=1; i<volumeList->size(); i++)
                    delete volumeList->at(i);
        }

        // property does take ownership of loaded handles
        volumeOwner_ = true;
    }

    delete volumeList;
}

void VolumeURLProperty::clear() {
    if (volumeOwner_)
        delete volume_;
    volume_ = 0;
    volumeOwner_ = false;
    if(infoProp_)
        infoProp_->setVolume(0);
    StringProperty::set("");
}

void VolumeURLProperty::addInfoProperty(VolumeInfoProperty* pointer) {
    infoProp_ = pointer;
}

void VolumeURLProperty::serialize(XmlSerializer& s) const {
    Property::serialize(s);

    std::string relativeURL;
    if (!value_.empty()) {
        std::string basePath = tgt::FileSystem::dirName(s.getDocumentPath());
        relativeURL = VolumeURL::convertURLToRelativePath(value_, basePath);
    }
    s.serialize("url", relativeURL);
}

void VolumeURLProperty::deserialize(XmlDeserializer& s) {
    std::string relativeURL;
    s.deserialize("url", relativeURL);
    if (relativeURL.empty())
        value_ = "";
    else {
        std::string basePath = tgt::FileSystem::dirName(s.getDocumentPath());
        value_ = VolumeURL::convertURLToAbsolutePath(relativeURL, basePath);
    }

    Property::deserialize(s);
}

ProgressBar* VolumeURLProperty::getProgressBar() {
    if (!progressBar_)
        progressBar_ = VoreenApplication::app()->createProgressDialog();
    return progressBar_;
}

} // namespace voreen
