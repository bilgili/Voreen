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

#ifndef VRN_VOLUMESET_H
#define VRN_VOLUMESET_H

#include <vector>
#include <map>

#include "voreen/core/vis/message.h"
#include "voreen/core/volume/volumeseries.h"
#include "voreen/core/xml/serializable.h"

namespace voreen {

class VolumeSetContainer;

/**
 * Contains VolumeSeries that are identified by their name and classified by a modality.
 * Several VolumeSeries of the same modality with different names can be contained within a
 * VolumeSet.
 */
class VolumeSet : public Serializable {
public:
    friend class VolumeSetContainer; // for setParentContainer()

    /**
     * Struct used as a comparator for std::set. The set stores pointers, but the comparsion
     * has to be done on the objects, not on the pointers!
     */
    struct VolumeSetComparator {
        bool operator()(const VolumeSet* const set1, const VolumeSet* const set2) const {
            if (set1 == 0 || set2 == 0)
                return false;
            else
                return (*set1 < *set2);
        }
    };

    //TODO: stupid name but anything else would be illogical. joerg
    typedef std::set<VolumeSet*, VolumeSetComparator> VolumeSetSet;

    /**
     * A VolumeSet is identified by the given name which is usually the file name.
     */
    VolumeSet(const std::string& filename = "");

    /**
     * Dtor causing all contained VolumeSeries to become deleted. The deletion of
     * VolumeSeries causes finally everything to become deleted, including Volume
     * pointers.
     */
    ~VolumeSet();

    /**
     * VolumeSets are defined to be equal when the
     * name_ attributes are identical. The comparison is
     * done lexicographically.
     */
    bool operator<(const VolumeSet& volset) const;
    bool operator==(const VolumeSet& volset) const;

    /**
     * Returns the name of the VolumeSet. Usually this is the file name.
     */
    const std::string& getName() const;

    void setName(const std::string& name);

    const VolumeSetContainer* getParentContainer() const;

    /**
     * Returns the first Volume from the first handle in the first series being available.
     */
    Volume* getFirstVolume() const;

    /**
     * Returns a std::vector containing all VolumeHandle
     * pointers from all VolumeSeries within this VolumeSet.
     * The pointers contained in the vector are all non-null. Otherwise
     * they would not have been added.
     */
    std::vector<VolumeHandle*> getAllVolumeHandles() const;

    /**
     * Force all Series in this VolumeSet to become of the given type.
     */
    void forceModality(const Modality& modality);

    /**
     * Adds the given VolumeSeries to the VolumeSet.
     *
     * Depending on the state of the parameter forceInsertion the name of the VolumeSeries
     * will be changed by appending the number of contained series with the same modality,
     * e.g., "unknown" will become "unknown 2".
     *
     * The parent VolumeSet of the given series is set to this VolumeSet.
     *
     * @param series  The VolumeSeries to be inserted
     * @param forceInsertion  Indicates whether the insertion may not fail
     *  and therefore eventually the name of the given VolumeSeries may be
     *  altered.
     * @return "true" if the insertion succeeds, "false" otherwise. If the
     * parameter forceInsertion is set to "true", this method should always
     * return "true".
     */
    bool addSeries(VolumeSeries* series, const bool forceInsertion = true);

    /**
     * Returns the VolumeSeries stored in this VolumeSet matching the given
     * name if such an object exists. Otherwise 0 is retuned.
     */
    VolumeSeries* findSeries(const std::string& seriesName);

    /**
     * Returns the given VolumeSeries if it is already stored in this VolumeSet or 0 otherwise.
     */
    VolumeSeries* findSeries(VolumeSeries* const series);

    /**
     * Returns all VolumeSeries from this VolumeSet having the same modality
     * as the given one. For a certain modality, several series with different
     * names may exist (e.g. "PET", "PET1").
     */
    std::vector<VolumeSeries*> findSeries(const Modality& modality) const;

    /**
     * If the given VolumeSeries exists within this VolumeSet, it is removed
     * and returned, but it will not be deleted. Otherwise 0 is returned.
     */
    VolumeSeries* removeSeries(VolumeSeries* const series);

    /**
     * If a VolumeSeries with the given name exists within this VolumeSet, it
     * is removed and returned, but it will not be deleted. Otherwise 0 is
     * returned.
     */
    VolumeSeries* removeSeries(const std::string& name);

    /**
     * If VolumeSeries with the given modality are stored within this VolumeSet,
     * they will be removed and returned, but not deleted. Otherwise, the returned
     * vector will be empty.
     */
    std::vector<VolumeSeries*> removeSeries(const Modality& modality);

    /**
     * If the given VolumeSeries is stored within this VolumeSet, it will
     * will be removed from it and deleted, and "true" will be returned.
     * Otherwise "false" is returned.
     */
    bool deleteSeries(VolumeSeries* const series);

    /**
     * If a VolumeSeries with the given name is stored within this VolumeSet,
     * it will will be removed from it and deleted, and "true" will be returned.
     * Otherwise "false" is returned.
     */
    bool deleteSeries(const std::string& name);

    /**
     * Removes and deletes all VolumeSeries with the given modality from this
     * VolumeSet if existing. If at least one VolumeSeries has been deleted,
     * "true" is returned. Otherwise "false" will be returned.
     */
    bool deleteSeries(const Modality& modality);

    /**
     * Returns all VolumeSeries stored in this VolumeSet.
     */
    const VolumeSeries::SeriesSet& getSeries() const;

    /**
     * Returns the names of all VolumeSeries stored in this VolumeSet.
     */
    std::vector<std::string> getSeriesNames() const;

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

    /**
     * TODO docs
     */
    static std::set<std::string> getFileNamesFromXml(TiXmlElement* elem);

    /**
     * This message is sent to VolumeSetSourceProcessor in order to
     * indicate changes on the VolumeSet, so that the processors
     * can update their properties. The Processors therefor must be
     * be registred at the MessageDistributor what is usually done
     * within their ctors.
     */
    static const Identifier msgUpdateVolumeSeries_;

    /** Holds the name of the xml element used when serializing the object */
    static const std::string XmlElementName;

protected:
    void setParentContainer(VolumeSetContainer* const parent);
    void notifyObservers();

    VolumeSeries::SeriesSet series_;    /** The VolumeSeries stored in this VolumeSet */
    std::string name_;  /** The name of the VolumeSet (usually the file name incl. path) */
    std::map<std::string, int> modalityCounter_; /** counts the different modalities. */
    VolumeSetContainer* parentContainer_; /** The VolumeSetContainer which is parent of this object */
};

typedef TemplateMessage<VolumeSet*> VolumeSetPtrMsg;

} // namespace

#endif // VRN_VOLUMESET_H
