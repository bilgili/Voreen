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

#ifndef VRN_VOLUMESERIES_H
#define VRN_VOLUMESERIES_H

#include <string>
#include <iostream>
#include <set>

#include "voreen/core/volume/modality.h"
#include "voreen/core/volume/volumehandle.h"
#include "voreen/core/xml/serializable.h"

namespace voreen {

class VolumeSet;

/**
 * Holds a series of VolumeHandles for a VolumeSet. A VolumeSet can have several VolumeSeries.
 * The VolumeSeries can be considered as the "real" (medical) modalities like PET, CT, or MR,
 * but also hold "artificial modalities" like gradients.
 */
class VolumeSeries : public Serializable {
public:
    friend class VolumeSet; // for setParentSet()

    /**
     * Comparator structure for ensuring that the comparison of VolumeSeries used
     * by the std::set compares the dereferenced pointers and not the pointers
     * themselves!
     */
    struct VolumeSeriesComparator {
        bool operator()(const VolumeSeries* const series1, const VolumeSeries* const series2) const {
            if (series1 == 0 || series2 == 0)
                return false;
            else
                return (*series1 < *series2);
        }
    };
    typedef std::set<VolumeSeries*, VolumeSeriesComparator> SeriesSet;

    /**
     * Constructor.
     * @param   name    The name of this VolumeSeries. Usually it is identical
     *  to the name of the modality and eventually expanded by a number counting
     *  the already contained VolumeSeries of the same modality within a certain
     *  VolumeSet.
     * @param   modality    The modality of this series.
     */
    VolumeSeries(const std::string& name = "unknown",
                 const Modality& modality = Modality::MODALITY_UNKNOWN);

    VolumeSeries(const VolumeSeries& series);

    /**
     * Deletes all VolumeHandles in this VolumeSeries.
     */
    ~VolumeSeries();

    VolumeSeries& operator=(const VolumeSeries& m);

    /**
     * Comparison of VolumeSeries ist based on the lexicographic
     * comparison of their names.
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
     * The name can only be set if the parent VolumeSet (if existing) contains
     * no other VolumeSeries with the same name as the given one. If it does, the
     * method will fail and return "false".
     * If setting the name succeeds, "true" is returned.
     * If there is no parent VolumeSet, setting the name always succeeds.
     */
    bool setName(const std::string& name);

    /**
     * Returns the name of the series followed by the name of the modality in
     * brackets. Example: "unknown 2 (unknown)".
     */
    std::string getLabel() const;

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
    VolumeSet* getParentSet() const;

    /**
     * Returns all VolumeHandles held by this Series.
     */
    const VolumeHandle::HandleSet& getVolumeHandles() const;

    /**
     * Returns the VolumeHandle for the given <b>index</b>. The index is the
     * position within the map.
     *
     * @return  0 if no such handle exists or otherwise a pointer to the handle.
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
     * Add the given VolumeHandle to the VolumeSeries.
     *
     * Depending on the state of the parameter forceInsertion the timestep of the VolumeHandle
     * will increased to prevent collisions with existing VolumeHandles in this VolumeSeries.
     *
     * The parent VolumeSeries of the given handle is set to this VolumeSeries.
     *
     * @param handle The VolumeHandle to be inserted.
     * @param forceInsertion  Indicates whether insertion may fail or not.
     *  If it is set to "true", the timestep of the handle will be
     *  set to the new maximum and the insertion should not fail.
     * @return "true" if insertion is successful, "false" otherwise. If the
     *  parameter forceInsertion is set to "true", this method should always
     *  return "true".
     */
    bool addVolumeHandle(VolumeHandle* handle, const bool forceInsertion = true);

    /**
     * Returns the given VolumeHandle if is already stored in this VolumeSeries and
     * 0 otherwise.
     */
    VolumeHandle* findVolumeHandle(VolumeHandle* const handle) const;

    /**
     * Returns the VolumeHandle for the given timestep or 0 if none exists.
     * A VolumeHandle is identified by the timestep associated to it.
     */
    VolumeHandle* findVolumeHandle(const float timestep) const;

    /**
     * If the given VolumeHandle is contained in this VolumeSeries, it will
     * be removed from it and returned. Otherwise 0 will be returned.
     *
     * Note: The VolumeHandle will not be deleted! Use <code>deleteVolumeHandle()</code>
     * to remove and delete the VolumeHandle.
     */
    VolumeHandle* removeVolumeHandle(VolumeHandle* const handle);

    /**
     * If a a VolumeHandle of the given timestep is contained in this VolumeSeries, it will
     * be removed from it and returned. Otherwise 0 will be returned.
     *
     * Note: The VolumeHandle will not be deleted! Use <code>deleteVolumeHandle()</code>
     * to remove and delete the VolumeHandle.
     */
    VolumeHandle* removeVolumeHandle(const float timestep);

    /**
     * If the given VolumeHandle is contained in this VolumeSeries, it will
     * be removed from it, deleted and "true" will be returned. Otherwise
     * "false" will be returned.
     *
     * Note: The VolumeHandle will be deleted! Use <code>removeVolumeHandle()</code>
     * to remove the VolumeHandle only.
     */
    bool deleteVolumeHandle(VolumeHandle* const handle);

    /**
     * If a VolumeHandle of the given timestep is contained in this VolumeSeries,
     * it will be removed from it, deleted and "true" will be returned. Otherwise
     * "false" will be returned.
     *
     * Note: The VolumeHandle will be deleted! Use <code>removeVolumeHandle()</code>
     * to remove the VolumeHandle only.
     */
    bool deleteVolumeHandle(const float timestep);

    /**
     * Indicates that the given VolumeHandle has changed its timestep.
     * Calls this method only if necessary. It only adjusts the maximum
     * timestep and notifies its observers about the event.
     */
    void timestepChanged(VolumeHandle* handle);

    /**
     * Returns the name of the xml element used when serializing the object
     */
    virtual std::string getXmlElementName() const { return XmlElementName; }

    /**
     * Serializes the object to XML.
     */
    virtual TiXmlElement* serializeToXml() const;

    /**
     * Updates the object from XML.
     */
    virtual void updateFromXml(TiXmlElement* elem);

    static std::set<std::string> getFileNamesFromXml(TiXmlElement* elem);

    /**
     * This message is sent to VolumeSelectionProcessor object so that they can
     * update their properties on chanding the content of this VolumeSeries.
     */
    static const Identifier msgUpdateTimesteps_;

    /** Holds the name of the xml element used when serializing the object */
    static const std::string XmlElementName;

protected:
    std::string name_;                  ///< name of this series
    Modality modality_;                 ///< the modality of this series
    VolumeHandle::HandleSet handles_;   ///< all VolumeHandles stored in this series
    float maximumTimestep_;             ///< currently greatest timestep from all VolumeHandles contained
    VolumeSet* parentSet_;              ///< the VolumeSet this series is contained in

private:
    /**
     * Sets the given VolumeSet as the new parent for this VolumeSeries. It is
     * usually only called from <code>VolumeSet::addSeries()</code>.
     */
    void setParentSet(VolumeSet* const volumeSet);

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

    void notifyObservers();
};

inline std::ostream& operator<<(std::ostream& os, const VolumeSeries& m) {
    return os << m.name_;
}

} // namespace voreen

#endif // VRN_MODALITY_H
