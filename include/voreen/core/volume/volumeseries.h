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

#ifndef VRN_VOLUMESERIES_H
#define VRN_VOLUMESERIES_H

#include <string>
#include <iostream>
#include <set>

#ifndef VRN_MODALITY_H
#include "voreen/core/volume/modality.h"
#endif

#ifndef VRN_VOLUMEHANDLE_H
#include "voreen/core/volume/volumehandle.h"
#endif

namespace voreen {

class VolumeSet;

/**
 * This class holds a series for one VolumeSet. A VolumeSet can have several VolumeSeries.
 * A VolumeSeries hold one or more VolumeHandles. The VolumeSeries can be considered as
 * the "real" (medical) modalities like PET, CT, MR, etc. but also hold "artificial modalilities"
 * like gradients, etc..
 * VolumeSeries are identified by the name and classified by a modality. Several VolumeSeries
 * of the same modality with different names can be contained within a VolumeSet.
 *
 * @author  Dirk Feldmann, July - September 2008
 */
class VolumeSeries
{
public:
    /**
     * Comparator structure for ensuring that the comparison of VolumeSeries used
     * by the std::set compares the dereferenced pointers and not the pointers 
     * themselves!
     */
    struct VolumeSeriesComparator
    {
        bool operator()(const VolumeSeries* const series1, const VolumeSeries* const series2) const {
            if( (series1 == 0) || (series2 == 0) )
                return false;
            return (*series1 < *series2);
        }
    };
    typedef std::set<VolumeSeries*, VolumeSeriesComparator> SeriesSet;

public:
    /**
     * Ctor.
     * @param   parentSet   VolumeSet this VolumeSeries object belongs to.
     *  This parameter should not be NULL. It will be changed on calling
     *  <code>VolumeSet::addSeries()</code> as a VolumeSeries can only be
     *  contained in one VolumeSet.
     * @param   name    The name of this VolumeSeries. Usually it is identical
     *  to the name of the modality and eventually expanded by a number counting
     *  the already contained VolumeSeries of the same modality within a certain
     *  VolumeSet.
     * @param   modality    The modality of this series.
     */
    VolumeSeries(VolumeSet* const parentSet, const std::string& name = "",
        const Modality& modality = Modality::MODALITY_UNKNOWN);

	VolumeSeries(const VolumeSeries& series);

    /**
     * Dtor causing all contained VolumeHandles and therefore all volume data contained
     * to become deleted.
     */
	~VolumeSeries();

    VolumeSeries& operator=(const VolumeSeries& m);

    /**
     * Comparison of VolumeSeries ist based on the lexicographic
     * comparison of their names (std::string objects).
     */
	bool operator==(const VolumeSeries& m) const;
    bool operator!=(const VolumeSeries& m) const;
    bool operator<(const VolumeSeries& m) const;

	friend std::ostream& operator<<(std::ostream& os, const VolumeSeries& m);

    /**
     * Return the name of the Series. A Series is identified by its name.
     */
	const std::string& getName() const;

    /**
     * Set the name for this object. This is required if a series with the
     * given name is already contained in a VolumeSet but insertion is enforced.
     */
    void setName(const std::string& name);

    /**
     * Returns the modality of this VolumeSeries object.
     */
    const Modality& getModality() const;

    /**
     * Set the modality for this VolumeSeries object. This is probably only
     * necessary if a VolumeSeries needs to be forced to become of the given
     * modality, e.g. PET or CT.
     */
    void setModality(const Modality& modality);

    /**
     * Return the VolumeSet this VolumeSeres object belongs to.
     */
    VolumeSet* getParentVolumeSet() const;

    /**
     * Sets the given VolumeSet as the new parent for this VolumeSeries. It is
     * usually only called from <code>VolumeSet::addSeries()</code>. Use this 
     * method carefully! 
     */
    void setParentVolumeSet(VolumeSet* const volumeSet);

    /**
     * Returns all VolumeHandles held by this Series.
     */
    const VolumeHandle::HandleSet& getVolumeHandles() const;

    /**
     * Returns the VolumeHandle* for the given <b>index</b>. The index is the 
     * position within the map.
     *
     * @return  NULL if no such handle exists or otherwise a pointer to the handle.
     *          That pointer could still be NULL!
     */
    VolumeHandle* getVolumeHandle(const int index) const;

    /**
     * Returns the number of of VolumeHandles held by this VolumeSeries object.
     */
    int getNumVolumeHandles() const;

    /**
     * Returns true if this object has the name of the
     * Modality::MODALITY_UNKNOWN or otherwise false.
     */
    bool isUnknown() const;
	
    /**
     * Add the given VolumeHandle to the VolumeSeries object if
     * it does not already exist and returns true then.
     * If the handle is NULL or already contained, false will be returned.
     *
     * If the handle is already contained in the VolumeSeries, it will not be
     * inserted and if it is not identical (on pointer comparison) to the one
     * contained, the given one will be deleted and replaced by the contained
     * VolumeHandle, unless the parameter forceInsertion is set to "true".
     * If so, the insertion should never fail as the given handle's timestep
     * will be set to the currently greatest contained timestep + 1.0f.
     *
     * Besides, the parent VolumeSeries of the given handle is set to this.
     *
     * @param   handle The VolumeHandle to be inserted.
     * @param   forceInsertion  Indicates whether insertion may fail or not.
     *  If it is set to "true", the timestep of the handle will be eventually
     *  adapted to become the greatest and the insertion should not fail.
     * @return  "true" if insertion is successful, "false" otherwise. If the
     *  parameter forceInsertion is set to "true", this method should always
     *  return "true".
     */
    bool addVolumeHandle(VolumeHandle*& handle, const bool forceInsertion = true);

    /**
     * Returns the VolumeHandle* for the given timestep. A VolumeHandle
     * is identified by the timestep associated to it.
     *
     * @return  NULL if no such handle exists or otherwise a pointer to the handle.
     *          That pointer could still be NULL!
     */
    VolumeHandle* findVolumeHandle(VolumeHandle* const handle) const;

    /**
     * Returns the VolumeHandle* for the given timestep. A VolumeHandle
     * is identified by the timestep associated to it.
     *
     * @return  NULL if no such handle exists or otherwise a pointer to the handle.
     *          That pointer could still be NULL!
     */
    VolumeHandle* findVolumeHandle(const float timestep) const;

    /**
     * If the given VolumeHandle is contained in this VolumeSeries, it will
     * be removed from it and returned. Otherwise NULL will be returned.
     *
     * <b>NOTE: The VolumeHandle will NOT be deleted! Use <code>deleteVolumeHandle()</code>
     * to remove AND delete the VolumeHandle.</b>
     */
    VolumeHandle* removeVolumeHandle(VolumeHandle* const handle);

    /**
     * If a a VolumeHandle of the given timestep is contained in this VolumeSeries, it will
     * be removed from it and returned. Otherwise NULL will be returned.
     *
     * <b>NOTE: The VolumeHandle will NOT be deleted! Use <code>deleteVolumeHandle()</code>
     * to remove AND delete the VolumeHandle.</b>
     */
    VolumeHandle* removeVolumeHandle(const float timestep);

    /**
     * If the given VolumeHandle is contained in this VolumeSeries, it will
     * be removed from it, deleted and "true" will be returned. Otherwise 
     * "false" will be returned.
     *
     * <b>NOTE: The VolumeHandle will be deleted! Use <code>removeVolumeHandle()</code>
     * to remove the VolumeHandle only.</b>
     */
    bool deleteVolumeHandle(VolumeHandle* const handle);

    /**
     * If a VolumeHandle of the given timestep is contained in this VolumeSeries, 
     * it will be removed from it, deleted and "true" will be returned. Otherwise 
     * "false" will be returned.
     *
     * <b>NOTE: The VolumeHandle will be deleted! Use <code>removeVolumeHandle()</code>
     * to remove the VolumeHandle only.</b>
     */
    bool deleteVolumeHandle(const float timestep);

    /**
     * This message is sent to VolumeSelectionProcessor object so that they can
     * update their properties on chanding the content of this VolumeSeries.
     */
    static const Identifier msgUpdateTimesteps_;

protected:
    std::string name_;  /** name of this series */
    Modality modality_; /** the modality of this series */
    VolumeHandle::HandleSet handles_;   /** all VolumeHandles stored in this series */
    float maximumTimestep_; /** currently greatest timestep from all VolumeHandles contained */
    VolumeSet* parentVolumeSet_; /** the VolumeSet this series is contained in */

private:
    /**
     * Sets the attribute maximumTimestep_ to a valid value.
     */
    void adjustMaximumTimestep();

    /**
     * Copy the VolumeSeries. This methode is called from copy
     * ctor and assigment operator.
     */
    void clone(const VolumeSeries& series);

    /**
     * Deletes the given VolumeHandles held by this VolumeSeries. Call
     * this method on finally deleting VolumeSeries objects. Usually there
     * will be no need to call this method, as it called by the VolumeSet
     * object's dtor holding this VolumeSeries.
     */
    void freeHandles();
};

inline std::ostream& operator<<(std::ostream& os, const VolumeSeries& m) {
    return os << m.name_;
}

} // namespace voreen

#endif // VRN_MODALITY_H
