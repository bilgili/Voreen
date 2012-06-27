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

#include "voreen/core/volume/volumehandle.h"

#include "voreen/core/volume/volumeset.h"
#include "voreen/core/volume/volumeseries.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/io/volumeserializer.h"

#include "tgt/filesystem.h"

using std::string;

#include "voreen/core/volume/bricking/brickedvolumegl.h"

namespace voreen {

const std::string VolumeHandle::loggerCat_("Voreen.VolumeHandle");
const std::string VolumeHandle::XmlElementName = "VolumeHandle";
unsigned int VolumeHandle::nextObjectID_ = 0;

const std::string VolumeHandle::Origin::XmlElementName = "Origin";
std::string VolumeHandle::Origin::basePath_ = "";

bool VolumeHandle::Origin::operator==(const Origin& rhs) const {
    return (filename == rhs.filename && seriesname == rhs.seriesname && timestep == rhs.timestep);
}

bool VolumeHandle::Origin::operator<(const Origin& rhs) const {
    return (filename < rhs.filename);
}

TiXmlElement* VolumeHandle::Origin::serializeToXml() const {
    TiXmlElement* originElem = new TiXmlElement(XmlElementName);

    if (!basePath_.empty()) {
        string protocol;
        string path = filename;
        string inzip;

        // Handle "zip://..."
        string::size_type pos = filename.find("://");
        if (pos != string::npos) {
            protocol = filename.substr(0, pos + 3);
            path = filename.substr(pos + 3);

            // Extract part after the .zip file
            //TODO: not robust, doesn't handle uppercase file names
            string::size_type zippos = path.find(".zip/");
            if (zippos != string::npos) {
                inzip = path.substr(zippos + 4);
                path = path.substr(0, zippos + 4);
            }
        }

        originElem->SetAttribute("filename", protocol + tgt::FileSystem::relativePath(path, basePath_) + inzip);
    } else {
        originElem->SetAttribute("filename", filename);
    }
    originElem->SetAttribute("seriesname", seriesname);
    originElem->SetDoubleAttribute("timestep", timestep);
    return originElem;
}

void VolumeHandle::Origin::updateFromXml(TiXmlElement* elem) {
    errors_.clear();

    if (!(elem->Attribute("filename") &&
          elem->Attribute("seriesname") &&
          elem->QueryFloatAttribute("timestep", &timestep) == TIXML_SUCCESS))
        throw XmlAttributeException("Origin remains unknown"); // TODO Better Exception
    
    filename = elem->Attribute("filename");

    // replace backslashes
    string::size_type pos = filename.find("\\");
    while (pos != string::npos) {
        filename[pos] = '/';
        pos = filename.find("\\"); 
    } 
    
    if (!basePath_.empty()) {
        string protocol;
        string path = filename;

        // Handle "zip://..."
        pos = filename.find("://");
        if (pos != string::npos) {
            protocol = filename.substr(0, pos + 3);
            path = filename.substr(pos + 3);
        }        

        // build new path only if this is not an absolute path
        if (path.find("/") != 0 && path.find("\\") != 0 && path.find(":") != 1)
            filename = protocol + tgt::FileSystem::absolutePath(basePath_ + "/" + path);    
    }
    
    seriesname = elem->Attribute("seriesname");
}

VolumeHandle::VolumeHandle(Volume* const volume, const float time)
    : volume_(volume),
      time_(time),
      hardwareVolumeMask_(VolumeHandle::HARDWARE_VOLUME_NONE),
      parentSeries_(0),
      objectID_(++nextObjectID_),
      largeVolumeManager_(0)
{
#ifndef VRN_NO_OPENGL
    volumeGL_ = 0;
#endif

#ifdef VRN_MODULE_CUDA
    volumeCUDA_ = 0;
#endif
}

VolumeHandle::VolumeHandle(const VolumeHandle& handle) : Serializable() {
    if (handle != *this) {
        CopyValuesFromVolumeHandle(&handle);
    }
}

void VolumeHandle::CopyValuesFromVolumeHandle(const VolumeHandle* handle) {
    volume_ = handle->getVolume();
    time_ = handle->getTimestep();
    parentSeries_ = 0;
    hardwareVolumeMask_ = handle->getHardwareVolumeMask();
    objectID_ = ++nextObjectID_;
#ifndef VRN_NO_OPENGL
    volumeGL_ = handle->volumeGL_;
#endif

#ifdef VRN_MODULE_CUDA
    volumeCUDA_ = handle->volumeCUDA_;
#endif
}

VolumeHandle::~VolumeHandle() {
    freeHardwareVolumes();
    delete volume_;
    delete largeVolumeManager_;
}

bool VolumeHandle::operator<(const VolumeHandle& handle) const {
    return (time_ < handle.time_);
}

bool VolumeHandle::operator==(const VolumeHandle& handle) const {
    return (time_ == handle.time_);
}

bool VolumeHandle::isIdentical(const VolumeHandle& handle) const {
    return (objectID_ == handle.getObjectID());
}

bool VolumeHandle::isIdentical(VolumeHandle* const handle) const {
    if (handle == 0)
        return false;

    return (objectID_ == handle->getObjectID());
}

VolumeHandle::operator float() const {
    return time_;
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
        generateHardwareVolumes(hardwareVolumeMask_);
    }
}

float VolumeHandle::getTimestep() const {
    return time_;
}

bool VolumeHandle::setTimestep(const float timestep) {
    if (timestep != time_) {
        // If the timestep is not already contained in
        // the parent VolumeSeries it can be used for
        // this VolumeHandle. Otherwise it cannot be used.
        if (parentSeries_ != 0 && parentSeries_->findVolumeHandle(timestep) == 0) {
            time_ = timestep;
            parentSeries_->timestepChanged(this);
            return true;
        }
        if (parentSeries_ == 0) {
            time_ = timestep;
            return true;
        }
    }
    return false;
}

VolumeSeries* VolumeHandle::getParentSeries() const {
    return parentSeries_;
}

void VolumeHandle::setParentSeries(VolumeSeries* const series) {
    parentSeries_ = series;
}

unsigned int VolumeHandle::getObjectID() const {
    return objectID_;
}

int VolumeHandle::getHardwareVolumeMask() const {
    return hardwareVolumeMask_;
}

bool VolumeHandle::hasHardwareVolumes(int volumeMask) const {
    return (hardwareVolumeMask_ & volumeMask);
}

void VolumeHandle::generateHardwareVolumes(int volumeMask) {
    std::string filename = (volume_->meta().getFileName().empty() ? "unknown" : volume_->meta().getFileName());

#ifndef VRN_NO_OPENGL
    if (volumeMask & VolumeHandle::HARDWARE_VOLUME_GL) {
        LDEBUG("generate GL hardware volume for " << filename);
        delete volumeGL_;

		BrickedVolume* brickedVolume = dynamic_cast<BrickedVolume*>(volume_);
		if (brickedVolume) {
			volumeGL_ = new BrickedVolumeGL(brickedVolume);
		} else {
			volumeGL_ = new VolumeGL(volume_);
		}

        hardwareVolumeMask_ |= VolumeHandle::HARDWARE_VOLUME_GL; // set bit
    }
#endif

#ifdef VRN_MODULE_CUDA
    if (volumeMask & VolumeHandle::HARDWARE_VOLUME_CUDA) {
        LDEBUG("generate CUDA hardware volume for " << filename);
        delete volumeCUDA_;
        volumeCUDA_ = new VolumeCUDA(volume_);
        hardwareVolumeMask_ |= VolumeHandle::HARDWARE_VOLUME_CUDA; // set bit
    }
#endif
}

void VolumeHandle::freeHardwareVolumes(int volumeMask) {
#ifndef VRN_NO_OPENGL
    if (volumeMask & VolumeHandle::HARDWARE_VOLUME_GL) {
        delete volumeGL_;
        volumeGL_ = 0;
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

VolumeHandle* VolumeHandle::getRelatedVolumeHandle(const Modality& modality) const {
    if (parentSeries_ == 0)
        return 0;

    VolumeSet* set = parentSeries_->getParentSet();
    if (set != 0) {
        std::vector<VolumeSeries*> series = set->findSeries(modality);
        for (size_t i = 0; i < series.size(); ++i) {
            if (series[i] != parentSeries_) {
                VolumeHandle* handle = series[i]->findVolumeHandle(time_);
                if (handle)
                    return handle;
            }
        }
    }
    return 0;
}

Volume* VolumeHandle::getRelatedVolume(const Modality& modality) const {
    VolumeHandle* handle = getRelatedVolumeHandle(modality);
    if (handle)
        return handle->getVolume();
    else
        return 0;
}

#ifndef VRN_NO_OPENGL

VolumeGL* VolumeHandle::getRelatedVolumeGL(const Modality& modality) {
    VolumeHandle* handle = getRelatedVolumeHandle(modality);
    if (handle)
        return handle->getVolumeGL();
    else
        return 0;
}

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

const VolumeHandle::Origin& VolumeHandle::getOrigin() const {
    return origin_;
}

void VolumeHandle::setOrigin(const std::string& filename, const std::string& seriesname, const float& timestep) {
    origin_.filename = filename;
    origin_.seriesname = seriesname;
    origin_.timestep = timestep;
}

void VolumeHandle::setOrigin(const VolumeHandle::Origin& origin) {
    origin_ = origin;
}

TiXmlElement* VolumeHandle::serializeToXml() const {
    TiXmlElement* handleElem = new TiXmlElement(getXmlElementName());
    // Serialize Data TODO remove filename
    //handleElem->SetAttribute("filename", getFileName());
    handleElem->SetDoubleAttribute("timestep", getTimestep());
    // Serialize Origin
    handleElem->LinkEndChild(origin_.serializeToXml());
    return handleElem;
}

void VolumeHandle::updateFromXml(TiXmlElement* elem) {
    errors_.clear();
    // deserialize VolumeHandle TODO remove filename
    float timestep;
    if (!(elem->QueryFloatAttribute("timestep", &timestep) == TIXML_SUCCESS))
        throw XmlAttributeException("Attributes missing on VolumeHandle element"); // TODO Better Exception
    setTimestep(timestep);
    // deserialize Origin
    origin_.updateFromXml(elem->FirstChildElement(Origin::XmlElementName));

    VolumeSerializerPopulator populator;
    VolumeHandle* handle = populator.getVolumeSerializer()->loadFromOrigin(origin_);
    
    CopyValuesFromVolumeHandle(handle);
}

std::string VolumeHandle::getFileNameFromXml(TiXmlElement* elem) {
    Origin origin;
    origin.updateFromXml(elem->FirstChildElement(Origin::XmlElementName));
    return origin.filename;
}

LargeVolumeManager* VolumeHandle::getLargeVolumeManager() {
	return largeVolumeManager_;
}

void VolumeHandle::setLargeVolumeManager(LargeVolumeManager* largeVolumeManager) {
	largeVolumeManager_ = largeVolumeManager;
}

} // namespace
