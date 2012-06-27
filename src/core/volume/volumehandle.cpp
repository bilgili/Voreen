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

#include "voreen/core/volume/volumeset.h"

namespace voreen {

VolumeHandle::VolumeHandle(const VolumeHandle& handle) {
    if (handle != *this) {
        hardwareVolumeMask_ = handle.getHardwareVolumeMask();
        volume_ = handle.getVolume();
        volumeGL_ = handle.getVolumeGL();
        time_ = handle.getTimestep();
        parentSeries_ = handle.getParentSeries();
    }
}

VolumeHandle::VolumeHandle(VolumeSeries* const parentSeries, Volume* const volume, const float time)
    : volume_(volume), 
      time_(time),
      parentSeries_(parentSeries), 
      hardwareVolumeMask_(VolumeHandle::HARDWARE_VOLUME_NONE)
{
#ifndef VRN_NO_OPENGL
    volumeGL_ = 0;
#endif
}

VolumeHandle::~VolumeHandle() {
    delete volume_;

#ifndef VRN_NO_OPENGL
    delete volumeGL_;
#endif
    // TODO: add deleting for attributes for CUDA volumes
}

bool VolumeHandle::operator<(const VolumeHandle& handle) const {
    return (time_ < handle.time_);
}

bool VolumeHandle::operator==(const VolumeHandle& handle) const {
    return (time_ == handle.time_);
}

VolumeHandle::operator float() const {
    return time_;
}

Volume* VolumeHandle::getVolume() const {
    return volume_;
}

void VolumeHandle::setVolume(Volume* const volume) {
    if( volume != volume_ ) {
        delete volume_;
        volume_ = volume;
        // as the volume has changed, the hardware volumes need
        // to be updated, too! So simply regenerate them.
        //
        generateHardwareVolumes(hardwareVolumeMask_);
    }
}

float VolumeHandle::getTimestep() const {
    return time_;
}

void VolumeHandle::setTimestep(const float timestep) {
    time_ = timestep;
}

VolumeSeries* VolumeHandle::getParentSeries() const {
    return parentSeries_;
}

void VolumeHandle::setParentSeries(VolumeSeries* const series) {
    parentSeries_ = series;
}

int VolumeHandle::getHardwareVolumeMask() const {
    return hardwareVolumeMask_;
}

void VolumeHandle::generateHardwareVolumes(int volumeMask) {
    hardwareVolumeMask_ = volumeMask;
#ifndef VRN_NO_OPENGL
    if ((hardwareVolumeMask_ & VolumeHandle::HARDWARE_VOLUME_GL) != 0) {
        delete volumeGL_;
        volumeGL_ = new VolumeGL(volume_);
    }
#endif
    // TODO: add support for CUDA in the same fashion as it is done for VolumeGL
    // above
}

bool VolumeHandle::hasVolumeGL() const {
#ifndef VRN_NO_OPENGL
    return ((hardwareVolumeMask_ & VolumeHandle::HARDWARE_VOLUME_GL) != 0);
#else
    return false;
#endif
}

bool VolumeHandle::hasVolumeCUDA() const {
    // TODO: adapt when implementing VolumeCUDA
    //
    return false;
}

Volume* VolumeHandle::getRelatedVolume(const Modality& modality) const
{
    if( (parentSeries_ == 0) )
        return 0;

    VolumeSet* set = parentSeries_->getParentVolumeSet();
    if( set != 0 ) {
        std::vector<VolumeSeries*> series = set->findSeries(modality);
        for( size_t i = 0; i < series.size(); i++ ) {
            if( series[i] != 0 ) {
                VolumeHandle* handle = series[i]->findVolumeHandle(time_);
                if( handle != 0 )
                    return handle->getVolume();
            }
        }
    }
    return 0;
}

#ifndef VRN_NO_OPENGL
VolumeGL* VolumeHandle::getRelatedVolumeGL(const Modality& modality) const {
    if( (parentSeries_ == 0) )
        return 0;

    VolumeSet* set = parentSeries_->getParentVolumeSet();
    if( set != 0 ) {
        std::vector<VolumeSeries*> series = set->findSeries(modality);
        for( size_t i = 0; i < series.size(); i++ ) {
            if( series[i] != 0 ) {
                VolumeHandle* handle = series[i]->findVolumeHandle(time_);
                if( handle != 0 )
                    return handle->getVolumeGL();
            }
        }
    }
    return 0;
}
#endif

} // namespace
