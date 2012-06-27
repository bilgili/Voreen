/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

namespace voreen {

const std::string VolumeHandle::loggerCat_("Voreen.VolumeHandle");
unsigned int VolumeHandle::nextObjectID_ = 0;

VolumeHandle::VolumeHandle(VolumeSeries* const parentSeries, Volume* const volume, const float time)
    : volume_(volume), 
      time_(time),
      hardwareVolumeMask_(VolumeHandle::HARDWARE_VOLUME_NONE),
      parentSeries_(parentSeries), 
      objectID_(++nextObjectID_)
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
        volume_ = handle.getVolume();
        time_ = handle.getTimestep();
        parentSeries_ = handle.getParentSeries();
        hardwareVolumeMask_ = handle.getHardwareVolumeMask();
        objectID_ = ++nextObjectID_;
#ifndef VRN_NO_OPENGL
        volumeGL_ = handle.volumeGL_;
#endif

#ifdef VRN_MODULE_CUDA
        volumeCUDA_ = handle.volumeCUDA_;
#endif
    }
}

VolumeHandle::~VolumeHandle() {
    freeHardwareVolumes();
    delete volume_;
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
    if( handle == 0 )
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
        volumeGL_ = new VolumeGL(volume_);
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

Volume* VolumeHandle::getRelatedVolume(const Modality& modality) const {
    if (parentSeries_ == 0)
        return 0;

    VolumeSet* set = parentSeries_->getParentVolumeSet();
    if (set != 0) {
        std::vector<VolumeSeries*> series = set->findSeries(modality);
        for (size_t i = 0; i < series.size(); ++i) {
            if (series[i] != 0 && series[i] != parentSeries_) {
                VolumeHandle* handle = series[i]->findVolumeHandle(time_);
                if (handle != 0)
                    return handle->getVolume();
            }
        }
    }
    return 0;
}

#ifndef VRN_NO_OPENGL

VolumeGL* VolumeHandle::getRelatedVolumeGL(const Modality& modality) {
    if (parentSeries_ == 0)
        return 0;

    VolumeSet* set = parentSeries_->getParentVolumeSet();
    if (set != 0) {
        std::vector<VolumeSeries*> series = set->findSeries(modality);
        for (size_t i = 0; i < series.size(); ++i) {
            if (series[i] != 0 && series[i] != parentSeries_) {
                VolumeHandle* handle = series[i]->findVolumeHandle(time_);
                if (handle != 0)
                    return handle->getVolumeGL();
            }
        }
    }
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

void VolumeHandle::setOrigin(const std::string& filename, const std::string& seriesname, const float& timestep) {
    origin_.filename = filename;
    origin_.seriesname = seriesname;
    origin_.timestep = timestep;
}

void VolumeHandle::setOrigin(const VolumeHandle::Origin& origin) {
    origin_ = origin;
}

const VolumeHandle::Origin& VolumeHandle::getOrigin() const {
    return origin_;
}

const std::string VolumeHandle::Origin::XmlElementName = "Origin";

TiXmlElement* VolumeHandle::Origin::serializeToXml() const {
    serializableSanityChecks();
    TiXmlElement* originElem = new TiXmlElement(XmlElementName);
    
    originElem->SetAttribute("filename", filename);
    originElem->SetAttribute("seriesname", seriesname);
    originElem->SetDoubleAttribute("timestep", timestep);
    return originElem;
}

void VolumeHandle::Origin::updateFromXml(TiXmlElement* elem) {
    errors_.clear();
    serializableSanityChecks(elem);

    if (!(elem->Attribute("filename") &&
          elem->Attribute("seriesname") &&
          elem->QueryFloatAttribute("timestep", &timestep) == TIXML_SUCCESS))
        throw XmlAttributeException("Origin remains unknown"); // TODO Better Exception
    filename = elem->Attribute("filename");
    seriesname = elem->Attribute("seriesname");
}

const std::string VolumeHandle::XmlElementName = "VolumeHandle";

TiXmlElement* VolumeHandle::serializeToXml() const {
    serializableSanityChecks();
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
    serializableSanityChecks(elem);
    // deserialize VolumeHandle TODO remove filename
    float timestep;
    if (!(elem->QueryFloatAttribute("timestep", &timestep) == TIXML_SUCCESS))
        throw XmlAttributeException("Attributes missing on VolumeHandle element"); // TODO Better Exception
    setTimestep(timestep);
    // deserialize Origin
    origin_.updateFromXml(elem->FirstChildElement(Origin::XmlElementName));
}

void VolumeHandle::updateFromXml(TiXmlElement* elem, std::map<VolumeHandle::Origin, std::pair<Volume*, bool> >& volumeMap) {
    updateFromXml(elem);
    std::map<VolumeHandle::Origin, std::pair<Volume*, bool> >::iterator it = volumeMap.find(origin_);
    if (it != volumeMap.end()) {
        setVolume(it->second.first);
        it->second.second = true; // We used the volume - now the map knows that.
    }
    else throw SerializerException("Origin of this handle not found");// TODO better exception
}

std::string VolumeHandle::getFileNameFromXml(TiXmlElement* elem) {
    Origin origin;
    origin.updateFromXml(elem->FirstChildElement(Origin::XmlElementName));
    return origin.filename;
}

} // namespace
