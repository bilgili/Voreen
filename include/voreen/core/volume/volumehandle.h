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

#ifndef VRN_VOLUMEHANDLE_H
#define VRN_VOLUMEHANDLE_H

#include "voreen/core/volume/volume.h"

#ifndef VRN_NO_OPENGL
#include "voreen/core/volume/volumegl.h"
#endif

#include <set>

namespace voreen {

class VolumeSeries;

/**
 * Class for handling different types and needs for volumes. Besides this
 * class holds the current timestep of the slected volume in its VolumeSeries.
 * The class is designed for being the only class which has to take care of
 * what kind of hardware volumes (VolumeGL, VolumeCUDA, etc.) are used.
 *
 * @author  Dirk Feldmann, July - September 2008
 */
class VolumeHandle {
public:
    /**
     * All types of available hardware specializations.
     * This enum is used to create a mask by OR-ing the values.
     */
    enum HardwareVolumes {
        HARDWARE_VOLUME_NONE = 0, 
        HARDWARE_VOLUME_GL = 1, 
        HARDWARE_VOLUME_CUDA = 2
    };

    /**
     * Comparator structure for ensuring that the comparison of VolumeHandles used
     * by the std::set compares the dereferenced pointers and not the pointers 
     * themselves!
     */
    struct VolumeHandleComparator {
        bool operator()(const VolumeHandle* const handle1, const VolumeHandle* const handle2) const {
            if ((handle1 == 0) || (handle2 == 0))
                return false;
            else
                return (*handle1 < *handle2);
        }
    };
    typedef std::set<VolumeHandle*, VolumeHandleComparator> HandleSet;

    /**
     * Ctor. NOTE: no hardware specific volume data like VolumeGL are
     * created. If you desire to use hardware specific volume data / textures,
     * call <code>generateHardwareVolumes()</code> on the object created by this
     * ctor.
     * 
     * @param   volume  The volume data for this VolumeHandle.
     * @param   time    The timestep fot this VolumeHandle.
     * @param   parentSeries    The VolumeSeries containing this VolumeHandle. Should be
     *  non-NULL to prevent unpredictable results.
     */
    VolumeHandle(VolumeSeries* const parentSeries, Volume* const volume, const float time);
    VolumeHandle(const VolumeHandle& handle);

    /**
     * Dtor deleting all Volume pointers and the hardware specific ones if they had been
     * generated.
     */
    ~VolumeHandle();

    /**
     * Comparison of the handles is defined as the comparison
     * of the timesteps held by the handles.
     */
    bool operator<(const VolumeHandle& handle) const;
    bool operator==(const VolumeHandle& handle) const;

    /**
     * Operator for convenience: a handle can be casted into
     * its timestep held.
     */
    operator float() const;

    /**
     * Returns the generic (Voreen) Volume.
     */
    Volume* getVolume() const;

    /**
     * (Re)Sets the volume for this handle and deletes the previous one.
     * Usually there should be no need for using this method as the volume
     * is initialized within the ctor, but some VolumeReaders need to modify
     * the read data.
     */
    void setVolume(Volume* const volume);

    /**
     * Returns the associated Timestep for the selected volume from the VolumeSeries
     * containing the object of this class.
     */
    float getTimestep() const;

    /**
     * Sets the Timestep for this VolumeHandle. This is only need on insertion into
     * VolumeSeries if a VolumeHandle is already contained within the series.
     */
    void setTimestep(const float timestep);

    /**
     * Returns the VolumeSeries* to which this VolumeHandle belongs to.
     */
    VolumeSeries* getParentSeries() const;

    /**
     * Sets the given Series as the new parent Series for this VolumeHandle.
     * This method is usually only called by class VolumeSeries on inserting
     * a VolumeHandle from <code>VolumeSeries::addVolumeHandle()</code>
     * so use with caution in order to prevent unpredictable behavior.
     */
    void setParentSeries(VolumeSeries* const series);

    /**
     * Returns the mask indicating what hardware volume are currently used
     * by this handle.
     * The used types can be extracted by AND-ing the mask with the desired bit(s)
     * like usual.
     */
    int getHardwareVolumeMask() const;

    /**
     * Causes the handle to generate specialized hardware volumes defined
     * by the given mask.
     *
     * <b>NOTE: the generation can only succeed if the nescessary compiler defines
     * are activated.</b>
     *
     * @param   volumeMask  OR-junction of HardwareVolumes enum determining
     *                      what kinds of hardware volumes to create additionally
     *                      to the one in volume_
     */
    void generateHardwareVolumes(int volumeMask);

    /**
     * Returns true if the handle holds a VolumeGL*, otherwise
     * false will be returned.
     */
    bool hasVolumeGL() const;

    /**
     * Returns true if the handle holds a VolumeCUDA*, otherwise
     * false will be returned.
     */
    bool hasVolumeCUDA() const;

    /**
     * Returns the first pointer to a Volume object of the given
     * modality and the same timestep as this VolumeHandle from all
     * VolumeSeries know to its parent VolumeSet or NULL if no 
     * such series, timestep or volume exist.
     * This is required for finding other series (former "modalities")
     * of the same scan at the same timestep.
     */
    Volume* getRelatedVolume(const Modality& modality) const;

#ifndef VRN_NO_OPENGL
    /**
     * Returns the first pointer to a VolumeGL object of the given
     * modality and the same timestep as this VolumeHandle from all
     * VolumeSeries know to its parent VolumeSet or NULL if no 
     * such series, timestep or volume exist.
     * This is required for finding other series (former "modalities")
     * of the same scan at the same timestep.
     */
    VolumeGL* getRelatedVolumeGL(const Modality& modality) const;

    VolumeGL* getVolumeGL() const { return volumeGL_; }
    void setVolumeGL(VolumeGL* const volumeGL) { volumeGL_ = volumeGL; }
#endif
    // TODO: add methods for CUDA support

protected:
    Volume* volume_;
    float time_;
    int hardwareVolumeMask_;
    VolumeSeries* parentSeries_;    /** VolumeSeries containing this object */

#ifndef VRN_NO_OPENGL
    VolumeGL* volumeGL_;
#endif
    // TODO: add attribute for CUDA
};

} // namespace 

#endif // VRN_VOLUMEHANDLE_H
