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

#include "voreen/core/datastructures/volume/volumehandle.h"

#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/datastructures/volume/modality.h"

#include "tgt/filesystem.h"

using std::string;

#include "voreen/core/datastructures/volume/bricking/brickedvolumegl.h"

namespace voreen {

const std::string VolumeHandle::loggerCat_("voreen.VolumeHandle");
const std::string VolumeOrigin::loggerCat_("voreen.VolumeOrigin");

VolumeOrigin::VolumeOrigin()
    : url_("")
{}

VolumeOrigin::VolumeOrigin(const VolumeOrigin& rhs)
    : Serializable()
{
    clone(rhs);
}

VolumeOrigin::VolumeOrigin(const std::string& filepath)
    : url_(filepath)
{
    cleanupURL();
}

VolumeOrigin::VolumeOrigin(const std::string& protocol, const std::string& filepath, const std::string& searchString) {

    // TODO: validate parameters

    url_ = filepath;

    if (!protocol.empty())
        url_.insert(0, protocol + "://");

    if (!searchString.empty())
        url_.append("?" + searchString);

    cleanupURL();
}

void VolumeOrigin::serialize(XmlSerializer& s) const {
   std::string basePath = tgt::FileSystem::dirName(s.getDocumentPath());
   if (!basePath.empty()) {
        VolumeSerializerPopulator serializerPopulator;
        VolumeOrigin originConv;
        try {
            originConv = serializerPopulator.getVolumeSerializer()->convertOriginToRelativePath(*this, basePath);
        }
        catch (tgt::UnsupportedFormatException& e) {
            throw SerializationException(std::string(e.what()));
        }
        s.serialize("filename", originConv.getURL());
    }
    else {
        s.serialize("filename", getURL());
    }
}

void VolumeOrigin::deserialize(XmlDeserializer& s) {
    s.deserialize("filename", url_);

    std::string basePath = tgt::FileSystem::dirName(s.getDocumentPath());
    if (!basePath.empty()) {
        VolumeSerializerPopulator serializerPopulator;
        VolumeOrigin originConv;
        try {
            originConv = serializerPopulator.getVolumeSerializer()->convertOriginToAbsolutePath(*this, basePath);
        }
        catch (tgt::UnsupportedFormatException& e) {
            throw SerializationException(std::string(e.what()));
        }
        url_ = originConv.getURL();
    }

    cleanupURL();
}

VolumeOrigin::~VolumeOrigin() {
}

VolumeOrigin& VolumeOrigin::operator=(const VolumeOrigin& rhs) {
    clone(rhs);
    return *this;
}

bool VolumeOrigin::operator==(const VolumeOrigin& rhs) const {
    return (tgt::FileSystem::comparePaths(getPath(), rhs.getPath()));
}

const std::string& VolumeOrigin::getURL() const {
    return url_;
}

std::string VolumeOrigin::getPath() const {

    if (url_.empty()) {
        LWARNING("Empty origin URL");
        return "";
    }

    std::string fullPath; // path + searchstring

    // remove protocol portion
    string::size_type sep_pos = url_.find("://");
    if (sep_pos == std::string::npos) {
        // URL does not contain protocol specifier
        fullPath = url_;
    }
    else {
        // return substring after protocol separator
        if (url_.size() < sep_pos + 1) {
            LWARNING("Origin URL does not contain a path");
            return "";
        }
        fullPath = url_.substr(sep_pos + 3);
    }

    // remove search string
    sep_pos = fullPath.find("?");
    if (sep_pos == std::string::npos) {
        // URL does not contain search string
        return fullPath;
    }
    else {
        // return substring before search string separator
        return fullPath.substr(0, sep_pos);
    }

}

std::string VolumeOrigin::getFilename() const {

    if (url_.empty()) {
        LWARNING("Empty origin URL");
        return "";
    }

    return tgt::FileSystem::fileName(getPath());
}

std::string VolumeOrigin::getSearchString() const {

    if (url_.empty()) {
        LWARNING("Empty origin URL");
        return "";
    }

    // find search string separator
    string::size_type sep_pos = url_.find("?");
    if (sep_pos == std::string::npos) {
        // URL does not contain a search string
        return "";
    }
    else {
        // return substring after search string separator
        return url_.substr(sep_pos + 1);
    }

}

std::string VolumeOrigin::getProtocol() const {

    string::size_type sep_pos = url_.find("://");
    if (sep_pos == std::string::npos) {
        // URL does not contain protocol specifier
        return "";
    }
    else {
        // return substring before protocol separator
        return url_.substr(0, sep_pos);
    }

}

void VolumeOrigin::cleanupURL() {
    // replace backslashes
    string::size_type pos = url_.find("\\");
    while (pos != string::npos) {
        url_[pos] = '/';
        pos = url_.find("\\");
    }
}

void VolumeOrigin::addSearchParameter(const std::string& key, const std::string& value) {

    if (key.empty() || value.empty()) {
        LWARNING("Search key or value empty.");
        return;
    }

    if (url_.empty()) {
        LWARNING("Empty origin URL");
        return;
    }

    // start search string, if not present
    if (getSearchString().empty())
        url_.append("?");
    else
        url_.append("&");

    // append search parameter
    url_.append(key);
    url_.append("=");
    url_.append(value);

}

std::string VolumeOrigin::getSearchParameter(std::string key) const {

    std::string searchString = getSearchString();
    if (searchString.empty())
        return "";

    // insert parameter separator at front of search string (eases the following search operation)
    searchString.insert(0, "&");

    // find key
    key.insert(0, "&");
    key.append("=");
    string::size_type param_pos = searchString.find(key);
    if (param_pos == std::string::npos) {
        // not found
        return "";
    }
    else {
        // found, detect position of next parameter in search string
        string::size_type next_pos = searchString.find("&", param_pos + key.size());
        if (next_pos == std::string::npos) {
            // parameter is the last one in search string
            return searchString.substr(param_pos + key.size());
        }
        else {
            // return substring till next parameter
            return searchString.substr(param_pos + key.size(), next_pos - param_pos - key.size());
        }

    }
}

// ----------------------------------------------------------------------------

VolumeHandle::VolumeHandle(Volume* const volume, const float time)
    : volume_(volume)
    , time_(time)
    , modality_(Modality::MODALITY_UNKNOWN)
    , hardwareVolumeMask_(VolumeHandle::HARDWARE_VOLUME_NONE)
    , largeVolumeManager_(0)
{
#ifndef VRN_NO_OPENGL
    volumeGL_ = 0;
#endif

#ifdef VRN_MODULE_CUDA
    volumeCUDA_ = 0;
#endif
}

VolumeHandle::VolumeHandle()
    : volume_(0)
    , time_(0.0f)
    , modality_(Modality::MODALITY_UNKNOWN)
    , hardwareVolumeMask_(VolumeHandle::HARDWARE_VOLUME_NONE)
    , largeVolumeManager_(0)
{
#ifndef VRN_NO_OPENGL
    volumeGL_ = 0;
#endif

#ifdef VRN_MODULE_CUDA
    volumeCUDA_ = 0;
#endif
}

VolumeHandle::~VolumeHandle() {
    notifyDelete();
    freeHardwareVolumes();
    delete volume_;
    delete largeVolumeManager_;
}

void VolumeHandle::releaseVolumes() {

    volume_ = 0;

#ifndef VRN_NO_OPENGL
    volumeGL_ = 0;
    hardwareVolumeMask_ &= ~VolumeHandle::HARDWARE_VOLUME_GL; // remove bit
#endif

#ifdef VRN_MODULE_CUDA
    volumeCUDA_ = 0;
    hardwareVolumeMask_ &= ~VolumeHandle::HARDWARE_VOLUME_CUDA; // remove bit
#endif

}


Volume* VolumeHandle::getVolume() const {
    return volume_;
}

void VolumeHandle::setVolume(Volume* const volume) {
    if (volume != volume_) {
        delete volume_;
        volume_ = volume;

        // As the volume has changed, the hardware volumes need
        // to be updated, too! So simply regenerate them.
        if (volume_)
            generateHardwareVolumes(hardwareVolumeMask_);
    }
}

float VolumeHandle::getTimestep() const {
    return time_;
}

void VolumeHandle::setTimestep(float timestep) {
    time_ = timestep;
}

bool VolumeHandle::reloadVolume() {

    // try to load volume from origin
    VolumeSerializerPopulator populator;
    VolumeHandle* handle = 0;
    try {
        handle = populator.getVolumeSerializer()->load(origin_);
    }
    catch (tgt::FileException& e) {
        LWARNING(e.what());
    }
    catch (std::bad_alloc&) {
        LWARNING("std::Error BAD ALLOCATION");
    }

    if (!handle || !handle->getVolume()) {
        delete handle;
        return false;
    }

    // free dependent volumes
    freeHardwareVolumes();

    // assigns new volume, but save temporal reference to former one
    Volume* volumeTemp = volume_;
    volume_ = handle->getVolume();
    handle->releaseVolumes();
    delete handle;

    // regenerate dependent volumes
    generateHardwareVolumes(hardwareVolumeMask_);

    // inform observers
    notifyReload();

    // now delete former volume object
    delete volumeTemp;

    return true;
}

int VolumeHandle::getHardwareVolumeMask() const {
    return hardwareVolumeMask_;
}

bool VolumeHandle::hasHardwareVolumes(int volumeMask) const {
    return (hardwareVolumeMask_ & volumeMask);
}

void VolumeHandle::generateHardwareVolumes(int volumeMask) {
    tgtAssert(volume_, "No volume");

#ifndef VRN_NO_OPENGL
    if (volumeMask & VolumeHandle::HARDWARE_VOLUME_GL) {
        LDEBUG("generate GL hardware volume for " << origin_.getURL());
        delete volumeGL_;
        LGL_ERROR;

        BrickedVolume* brickedVolume = dynamic_cast<BrickedVolume*>(volume_);
        if (!tgt::hand(tgt::greaterThan(volume_->getDimensions(), tgt::ivec3(1))) && !brickedVolume) {
            LERROR("Unable to create OpenGL texture: Volume must have a size greater one in all dimensions, got: " << volume_->getDimensions());
        } else {
            if (brickedVolume)
                volumeGL_ = new BrickedVolumeGL(brickedVolume);
            else
                volumeGL_ = new VolumeGL(volume_);

            hardwareVolumeMask_ |= VolumeHandle::HARDWARE_VOLUME_GL; // set bit
            LGL_ERROR;
        }
    }
#endif

#ifdef VRN_MODULE_CUDA
    if (volumeMask & VolumeHandle::HARDWARE_VOLUME_CUDA) {
        LDEBUG("generate CUDA hardware volume");
        delete volumeCUDA_;
        volumeCUDA_ = new VolumeCUDA(volume_);
        hardwareVolumeMask_ |= VolumeHandle::HARDWARE_VOLUME_CUDA; // set bit
    }
#endif
}

void VolumeHandle::freeHardwareVolumes(int volumeMask) {
#ifndef VRN_NO_OPENGL
    if (volumeMask & VolumeHandle::HARDWARE_VOLUME_GL) {
        if (volumeGL_) {
            delete volumeGL_;
            volumeGL_ = 0;
            LGL_ERROR;
        }
        hardwareVolumeMask_ &= ~VolumeHandle::HARDWARE_VOLUME_GL; // remove bit
    }
#endif

#ifdef VRN_MODULE_CUDA
    if (volumeMask & VolumeHandle::HARDWARE_VOLUME_CUDA) {
        delete volumeCUDA_;
        volumeCUDA_ = 0;
        hardwareVolumeMask_ &= ~VolumeHandle::HARDWARE_VOLUME_CUDA; // remove bit
    }
#endif
}

#ifndef VRN_NO_OPENGL

VolumeGL* VolumeHandle::getVolumeGL() {
    if (volumeGL_ == 0)
        generateHardwareVolumes(VolumeHandle::HARDWARE_VOLUME_GL);

    return volumeGL_;
}

#endif // VRN_NO_OPENGL

#ifdef VRN_MODULE_CUDA

VolumeCUDA* VolumeHandle::getVolumeCUDA() {
    if (volumeCUDA_ == 0)
        generateHardwareVolumes(VolumeHandle::HARDWARE_VOLUME_CUDA);

    return volumeCUDA_;
}

#endif // VRN_MODULE_CUDA

const VolumeOrigin& VolumeHandle::getOrigin() const {
    return origin_;
}

void VolumeHandle::setOrigin(const VolumeOrigin& origin) {
    origin_ = origin;
}

void VolumeHandle::serialize(XmlSerializer& s) const {
    s.serialize("modality", getModality().getName());
    s.serialize("timestep", getTimestep());
    s.serialize("Origin", origin_);
}

void VolumeHandle::deserialize(XmlDeserializer& s) {
    // deserialize modality
    try {
        std::string modality;
        s.deserialize("modality", modality);
        setModality(Modality(modality));
    }
    catch (XmlSerializationNoSuchDataException&) {
        // No modality found in VolumeHandle element...
        s.removeLastError();
    }

    // deserialize timestep
    float timestep;
    s.deserialize("timestep", timestep);
    setTimestep(timestep);

    // deserialize Origin
    try {
        s.deserialize("Origin", origin_);
    }
    catch (SerializationException& e) {
        s.addError(std::string(e.what()));
        LWARNING(std::string(e.what()));
        return;
    }

    // load volume from origin
    VolumeSerializerPopulator populator;
    VolumeHandle* handle = 0;
    try {
        handle = populator.getVolumeSerializer()->load(origin_);
        // copy over loaded volume from temporary handle and free it
        if (handle) {
            setVolume(handle->getVolume());
            handle->releaseVolumes();

            // We take control of the LargeVolumeManager if there is one
            if (handle->getLargeVolumeManager()) {
                setLargeVolumeManager(handle->getLargeVolumeManager());
                handle->setLargeVolumeManager(0);
                getLargeVolumeManager()->setVolumeHandle(this);
            }

            delete handle;
        }
    }
    catch (tgt::FileException& e) {
        s.addError(e);
        LWARNING(e.what());
    }
    catch (std::bad_alloc& /*e*/) {
        s.addError("Unable to load " + origin_.getPath() + ": bad allocation");
        LWARNING("bad allocation when reading " << origin_.getPath() + " (URL: " + origin_.getURL() + ")");
    }
}

LargeVolumeManager* VolumeHandle::getLargeVolumeManager() {
    return largeVolumeManager_;
}

void VolumeHandle::setLargeVolumeManager(LargeVolumeManager* largeVolumeManager) {
    largeVolumeManager_ = largeVolumeManager;
}

void VolumeHandle::setModality(Modality modality) {
    modality_ = modality;
}

Modality VolumeHandle::getModality() const {
    return modality_;
}

void VolumeHandle::notifyDelete() {
    std::vector<VolumeHandleObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->volumeHandleDelete(this);
}

void VolumeHandle::notifyReload() {
    std::vector<VolumeHandleObserver*> observers = getObservers();
    for (size_t i=0; i<observers.size(); ++i)
        observers[i]->volumeChange(this);
}

} // namespace
