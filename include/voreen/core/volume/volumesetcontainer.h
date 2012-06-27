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

#ifndef VRN_VOLUMESETCONTAINER_H
#define VRN_VOLUMESETCONTAINER_H

#ifndef VRN_VOLUMESET_H
#include "voreen/core/volume/volumeset.h"
#endif

#include <vector>
#include <set>
#include <string>

namespace voreen
{

class VolumeSetContainer
{
public:
    static const Identifier msgUpdateVolumeSetContainer_;
    static const Identifier msgSetVolumeSetContainer_;

    VolumeSetContainer();

    /**
     * Dtor causing all contained VolumeSets to be deleted. The deletion
     * of VolumeSets causes finally everthing to become deleted, including Volume
     * pointers.
     */
    ~VolumeSetContainer();

    /**
     * Add the given VolumeSet pointer to the container if it is
     * not already contained. If it is already contained and the pointer
     * you want to be added to the container is not the contained one itself,
     * it will be deleted and replaced by the one from the VolumeContainer 
     * object.
     *
     * NOTE: you need not to delete the pointer when
     * you created the VolumeSet* by using new! The pointer
     * will be deleted on calling <code>clear()</code> or
     * within the dtor of this class!
     *
     * @param   volset  The VolumeSet pointer which is about to be inserted
     * @return  true if the insertion was successful, or false if the insertion
     *          failed, e.g. because the VolumeSet already exisits in the container.
     */
    bool addVolumeSet(VolumeSet*& volset);

    /**
     * Searches the container for a VolumeSet matching the
     * name of the given VolumeSet in volsetWanted.
     * If no such volume exists, 0 is returned.
     * If you created volsetWanted by using new, you have to
     * ensure its deletion. Normally you will pass an address of
     * a temporary VolumeSet object.
     *
     * @param   volsetWanted    The VolumeSet you want to find within the container
     * @return  0 if no such VolumeSet can be found or the pointer to the wanted VolumeSet.
     */
    VolumeSet* findVolumeSet(VolumeSet* const volsetWanted);

    /**
     * Searches the container for a VolumeSet matching the
     * name in the given std::string name.
     * If no such volume exists, 0 is returned.
     *
     * @param   name    The name of the VolumeSet you want to find within the container
     * @return  0 if no such VolumeSet can be found or the pointer to the wanted VolumeSet.
     */
    VolumeSet* findVolumeSet(const std::string& name);

    /**
     * Returns "true" if the given VolumeSet is contained within this container,
     * or "false" otherwise.
     */
    bool containsVolumeSet(VolumeSet* const volset);

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
    const VolumeSet::VSPSet& getVolumeSets() const;
    
    /**
     * Clears the entire container. All containing
     * VolumeSets will be deleted and the memeroy will be
     * freed.
     */
    void clear();

    /**
     * Method for repacking old VolumeContainer into VolumeSets
     * and inserting them into this container. Remove method as
     * soon as VolumeContainer is about to be eliminated.
     */
    //VolumeSet* insertContainer(VolumeContainer* container, bool replace = false);

protected:
    VolumeSet::VSPSet volumesets_;
};

typedef TemplateMessage<VolumeSetContainer*> VolumeSetContainerMsg;

}   // namespace

#endif
