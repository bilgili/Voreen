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

#ifndef VRN_VOLUMESETCONTAINER_H
#define VRN_VOLUMESETCONTAINER_H

#include "voreen/core/volume/volumeset.h"
#include "voreen/core/volume/observer.h"

#include "voreen/core/xml/serializable.h"

#include <vector>
#include <set>
#include <string>

namespace voreen {

class VolumeSetContainer : public Observable, public Serializable {
public:
    static const Identifier msgUpdateVolumeSetContainer_;
    static const Identifier msgSetVolumeSetContainer_;

    /** Holds the name of the xml element used when serializing the object */
    static const std::string XmlElementName;

    VolumeSetContainer();

    /**
     * Dtor causing all contained VolumeSets to be deleted. The deletion
     * of VolumeSets causes finally everthing to become deleted, including Volume
     * pointers.
     */
    ~VolumeSetContainer();

    /**
     * Add the given VolumeSet pointer to the container if it is
     * not already contained.
     *
     * @param   volset  The VolumeSet pointer which is about to be inserted
     * @return  true if the insertion was successful, or false if the insertion
     *          failed, e.g. because the VolumeSet already exisits in the container.
     */
    bool addVolumeSet(VolumeSet* volset);

    /**
     * Searches the container for a VolumeSet matching the
     * name of the given VolumeSet in volsetWanted.
     * If no such volume exists, 0 is returned.
     *
     * @param   volsetWanted    The VolumeSet you want to find within the container
     * @return  0 if no such VolumeSet can be found or the pointer to the wanted VolumeSet.
     */
    VolumeSet* findVolumeSet(VolumeSet* const volsetWanted);

    /**
     * Searches the container for a VolumeSet matching the name If no such VolumSet exists, 0 is
     * returned.
     *
     * @param   name    The name of the VolumeSet you want to find within the container
     * @return  0 if no such VolumeSet can be found or the pointer to the wanted VolumeSet.
     */
    VolumeSet* findVolumeSet(const std::string& name);

    /**
     * Removes the given VolumeSet from container <b>WITHOUT</b> deleting it, but
     * only if it is contained.
     * If the VolumeSet is not contained, it will not be removed and NULL will be
     * returned. Otherwise the removed VolumeSet is returned.
     */
    VolumeSet* removeVolumeSet(VolumeSet* const volset);

    /**
     * Removes the VolumeSet with the given name from container <b>WITHOUT</b>
     * deleting it, but only if it is contained.
     * If the VolumeSet is not contained, it will not be removed and NULL will be
     * returned. Otherwise the removed VolumeSet is returned.
     */
    VolumeSet* removeVolumeSet(const std::string& name);

    /**
     * Removes the given VolumeSet from container and deletes it, but
     * only if it is contained.
     * If the VolumeSet is not contained, it will not be deleted and "false"
     * will be returned. Otherwise "true" is returned.
     */
    bool deleteVolumeSet(VolumeSet* const volset);

    /**
     * Removes the VolumeSet with the given name from container and deletes
     * it, but only if it is contained.
     * If the VolumeSet is not contained, it will not be deleted and "false"
     * will be returned. Otherwise "true" is returned.
     */
    bool deleteVolumeSet(const std::string& name);

    /**
     * Returns the names of all VolumeSets contained in this VolumeSetContainer
     */
    std::vector<std::string> getVolumeSetNames() const;

    /**
     * Returns the VolumeSet (a std::set with a special comparator) containing
     * all VolumeSets.
     */
    const VolumeSet::VolumeSetSet& getVolumeSets() const;

    /**
     * Clears the entire container. All containing
     * VolumeSets will be deleted and the memory will be
     * freed.
     */
    void clear();

    virtual void notifyObservers() const;

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

protected:
    VolumeSet::VolumeSetSet volumesets_;
};

typedef TemplateMessage<VolumeSetContainer*> VolumeSetContainerMsg;

}   // namespace

#endif
