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

#ifndef VRN_GEOMETRYCONTAINER_H
#define VRN_GEOMETRYCONTAINER_H

#include <map>
#include "voreen/core/geometry/geometry.h"
#include "voreen/core/vis/identifier.h"

namespace voreen {

class GeometryContainer {
    public:
        /** Adds the given Geometry pointer to the container's content, associates it with the given name
         * and ID if the insertion is successful.
         * If the given pointer is NULL, the ID or a non-empty name already contained, the insertion will
         * will fail.
         * If no name is given, the insertion will only fail if the ID already exists or the pointer is
         * NULL.
         *
         * @param   id  ID for the pointer. This ID should <b>ALWAYS</b> be obtained by a call to
         *              method <code>getNextID()</code>. Must be greater 0.
         * @param    geo    Pointer to a Geometry object which is desired to be inserted. Must not be NULL.
         * @param    name   Name for the Geometry pointer. This parameter may be an empty string but it is
         *                  strongly recommended to use it, e. g. with the same value as the corresponding 
         *                  value used for the GeometryPortMapping.
         * @return    returns false if the name for the object already exists, the pointer is NULL,
         *            or if the ID already exists within the container.
         *            If the given name ist non-empty but already associated to another Geometry pointer,
         *            false is also returned. Elsewise true is returned.
         */
        bool addGeometry(const int id, Geometry* geo, const Identifier& name);
        //int addGeometry(const Identifier& name, Geometry* geo);

        /** Returns the pointer to a Geometry object associated with the given name if it
         * exists. Otherwise a NULL pointer will be returned.
         * This method internally looks up the ID for the given name and calls the method
         * "GeometryContainer::getGeometry(const int id)".
         *
         * @param    name    Name to which the Geometry pointer has been mapped.
         * @return    Pointer to the Geometry object if the name is known in the map
         */
        Geometry* getGeometry(const Identifier& name);

        /** Returns the pointer to a Geometry object for the given id if it
         * exists. Otherwise a NULL pointer will be returned.
         *
         * @param    id    ID for the Geometry pointer.
         * @return    Pointer to the Geometry object if the ID is valid.
         */
        Geometry* getGeometry(const int id);

        /** Removes the Geometry pointer for the given name from the container
         * <b>WITHOUT</b> deleting it and returns the pointer if existing.
         * This method internally looks up the ID for the given name and calls the method
         * "GeometryContainer::removeGeometry(const int id)".
         * After being removed from the container, the returned pointer will lose its values 
         * for the attributes name_ and id_ as they have been set by the container.
         * Therefore the pointer may become added again to the container.
         *
         * @param    name    Name for the Geometry pointer which shall be removed
         * @return    Geometry pointer which has been removed or NULL if no pointer 
         *            exists for the given name.
         */
        Geometry* removeGeometry(const Identifier& name);

        /** Removes the Geometry pointer for the given ID from the container
         * <b>WITHOUT</b> deleting it and returns the pointer if existing.
         * After being removed from the container, the returned pointer will lose its values
         * for the attributes name_ and id_ as they have been set by the container.
         * Therefore the pointer may become added again to the container.
         *
         * @param    id    ID for the Geometry pointer which shall be removed
         * @return    Geometry pointer which has been removed or NULL if no pointer 
         *            exists for the given ID.
         */
        Geometry* removeGeometry(const int id);

        /** Deletes the Geometry pointer for the given name from the container
         * <b>WITH</b> deleting it. The return value is true, if the deletion
         * was successful or false elswise.
         *
         * This method internally looks up the ID for the given name and calls
         * the method for the int parameter.
         *
         * @param    name    name for the Geometry pointer which shall be removed
         * @return    true if a pointer is associated with the given name and the
         *            deletion is successful or false elsewise.
         */
        bool deleteGeometry(const Identifier& name);

        /** Deletes the Geometry pointer for the given ID from the container
         * <b>WITH</b> deleting it. The return value is true, if the deletion
         * was successful or false elswise.
         *
         * @param    id    ID for the Geometry pointer which shall be removed
         * @return    true if a pointer is associated with the given ID and the
         *            deletion is successful or false elsewise.
         */
        bool deleteGeometry(const int id);

        /** Replaces the Geometry pointer associated with the given name by the given
         * pointer. Therefore, the previous ID and the name are preserved. The name
         * must exists in the container and it must not be empty. Also the new pointer
         * must not be NULL.
         *
         * This method internally looks up the ID associated with the name and calls
         * the method for the integer parameter.
         *
         * @param    name    Name of the Geometry pointer which is about to be replaced. Must be non-empty.
         * @pram    pGeo    pointer to the Geometry object which shall replace the previos. Must be not NULL.
         * @return    Returns true if the parameters are valid and the replacement was successfull.
         */
        bool replaceGeometry(const Identifier& name, Geometry* geo);

        /** Replaces the Geometry pointer associated with the given ID by the given
         * pointer. Therefore, the previous ID and the name are preserved. The ID
         * must exists in the container and it must not be 0. Also the new pointer
         * must not be NULL.
         *
         * @param    id    ID of the Geometry pointer which is about to be replaced. Must be non-empty.
         * @pram    pGeo    pointer to the Geometry object which shall replace the previos. Must be not NULL.
         * @return    Returns true if the parameters are valid and the replacement was successfull.
         *
         */
        bool replaceGeometry(const int id, Geometry* geo);

        /** Returns the ID from the Geometry pointer for the given name or
         * 0 if no pointer is associated with that name.
         * @param    name    Name to find the ID for.
         * @return    ID of the pointer associated with name or 0 if no
         *            pointer is associated with that name.
         */
        int getGeometryID(const Identifier& name) const;

        /** Returns the name from the Geometry pointer for the given ID or
         * an empty string if no pointer is associated with that ID.
         * @param    ID    ID to find the name for.
         * @return    Name of the pointer associated with id or "" if no
         *            pointer is associated with that ID.
         */
        Identifier getGeometryName(const int id) const;

        /** Checks whether a Geometry pointer is associated with the given
         * name within the container.
         * @param    name    Name for the pointer to check wheter contained or not.
         * @return    true if Name is contained or false elsewise.
         */
        bool containsGeometry(const Identifier& name) const;
        
        /** Checks whether a Geometry pointer is associated with the given
         * ID within the container.
         * @param    id    ID for the pointer to check wheter contained or not.
         * @return    true if ID is contained or false elsewise.
         */
        bool containsGeometry(const int id) const;
    
        /** Return the next available ID which may become associated with an
         * Geometry pointer.
         * @return	next available ID for a Geometry pointer
         */
        int getNextID() const;

        /** Removes <b>ALL</b> Geometry pointers from the container by removing their 
         * associated names and IDs but <b>DOES NOT</b> call <code>delete</code> on 
         * the pointers! It performs a total reset to initial state incl. the ID counter:
         * after a call to clear(), the next call to getNextID() will then return 1
         * again, because all the previously associated IDs are invalid afterwards!
         */
        void clear();

        /** Similar to clear() but this method calls delete on every pointer
         * and frees the memory.
         */
        void clearDeleting();

    protected:
        std::map<Identifier, int> names_;    // maps names to IDs
        std::map<int, Geometry*> geos_;        // maps IDs to Geometry pointers

    private:

        /** Performs one-time initialization for Geometry objects. That means, the given name
         * for the pointer storing the geometry and the generated ID will become associated
         * to the Geometry object and therefore stored in the object.
         * Once initialized, the object cannot be added to the container again, unless 
         * resetGeometry() is called.
         * This method is designed to be used only in method addGeometry().
         */
        bool initGeometry(Geometry* geo, const Identifier& name, const int id);

        /** Performs the "reset" for the attributes of the given Geometry pointer:
         * name_ will be set to "" and id_ will be set to 0, so that the pointer
         * can become re-entered into the container again.
         * This method is designed for usage within the method removeGeometry()
         * which removes the Geometry pointer from the container but does not
         * delete it!
         */
        void resetGeometry(Geometry* geo);

        static int nextID_;
};

}    // namespace

#endif  //VRN_GEOMETRYCONTAINER_H
