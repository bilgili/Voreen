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

#include "voreen/core/geometry/geometrycontainer.h"

using std::map;
using std::pair;

namespace voreen {

int GeometryContainer::nextID_ = 1;    // static member!

bool GeometryContainer::initGeometry(Geometry* geo, const Identifier& name, const int id) {
    if ( (geo->id_ == 0) ) {
        geo->name_ = name;
        geo->id_ = id;
        return true;
    }

    return false;
}

void GeometryContainer::resetGeometry(Geometry* geo) {
    geo->name_ = "";
    geo->id_ = 0;
}

// public methods
//
int GeometryContainer::getNextID() const {
    return nextID_;
}

bool GeometryContainer::addGeometry(const int id, Geometry* geo, const Identifier& name) {
    // Don't start working, when pointer is NULL
    //
    if ( (geo == 0) || (id <= 0) ) {
        return 0;
    }

    // Try to insert the name if it is non-empty.
    //
    pair< map<Identifier, int>::iterator, bool > ret;
    if ( name != "" ) {
        ret = names_.insert( pair<Identifier, int>(name, id) );
        if ( ret.second == false ) {
            return false;
        }
    }
    
    // If name and ID have been successfully linked, so they can be assigned
    // to Geometry object. If the assignment fails, the (name, id) entry
    // will be removed from the map again and the method will fail.
    //
    if ( initGeometry(geo, name, id) == false ) {
        if ( name != "" ) {
            names_.erase(ret.first);
        }
        return false;
    }
    
    pair< map<int, Geometry*>::iterator, bool > ret2;
    ret2 = geos_.insert( pair<int, Geometry*>(id, geo) );
    if ( ret2.second == false ) {
        resetGeometry(geo);
        return false;
    }

    // As the given ID has been used successfully by now, the next call
    // to getNextID() must return a new ID. Therefore nextID_ is incermented.
    //
    nextID_++;

    return true;
}

Geometry* GeometryContainer::getGeometry(const Identifier& name) {
    const int id = getGeometryID(name);
    if ( id == 0 ) {
        return 0;
    }

    map<int, Geometry*>::iterator itGeo = geos_.find(id);
    if ( itGeo == geos_.end() ) {
        return 0;
    }

    return itGeo->second;
}

Geometry* GeometryContainer::getGeometry(const int id) {
    map<int, Geometry*>::iterator itGeo = geos_.find(id);
    if ( itGeo == geos_.end() ) {
        return 0;
    }

    return itGeo->second;
}

Geometry* GeometryContainer::removeGeometry(const Identifier& name) {
    // try to find the ID for the given name first
    //
    map<Identifier, int>::iterator itID = names_.find(name);
    if ( itID == names_.end() ) {
        return 0;
    }

    // if ID for the name exists, call the method for the ID.
    // The name which has been found above will be searched again.
    //
    Geometry* geo = removeGeometry(itID->second);
    resetGeometry(geo);
    return geo;
}

Geometry* GeometryContainer::removeGeometry(const int id) {
    map<int, Geometry*>::iterator itGeo = geos_.find(id);
    if ( itGeo == geos_.end() ) {
        return 0;
    }

    // find name for the given ID and the matching iterator in the name map
    //
    const Identifier& strName = itGeo->second->getName();
    map<Identifier, int>::iterator itID = names_.find(strName);
    if ( itID == names_.end() ) {
        return 0;
    }
    
    // delete map entries
    //
    names_.erase(itID);
    geos_.erase(itGeo);

    // Geometry pointer will maybe re-inserted into the map =>
    // reset old name and old ID associated to it by addGeometry()-method
    //
    resetGeometry(itGeo->second);
    return itGeo->second;
}

bool GeometryContainer::deleteGeometry(const Identifier& name) {
    // try to find the ID for the given name first
    //
    map<Identifier, int>::iterator itID = names_.find(name);
    if ( itID == names_.end() ) {
        return false;
    }

    // if ID for the name exists, call the method for the ID
    //
    return deleteGeometry(itID->second);
}

bool GeometryContainer::deleteGeometry(const int id) {
    if ( id <= 0 ) {
        return false;
    }

    map<int, Geometry*>::iterator itGeo = geos_.find(id);
    if ( itGeo == geos_.end() ) {
        return false;
    }

    // Find name for the given ID and the matching iterator in the name map
    //
    const Identifier& strName = itGeo->second->getName();
    map<Identifier, int>::iterator itID = names_.find(strName);
    if ( itID == names_.end() ) {
        return false;
    }

    // delete the Geometry pointer first
    // (iterator will be invalid after erase() called on map)
    //
    delete itGeo->second;
    itGeo->second = 0;

    // delete map entries
    //
    names_.erase(itID);
    geos_.erase(itGeo);
    return true;
}

bool GeometryContainer::replaceGeometry(const Identifier& name, Geometry* geo) {
    if ( (geo == 0) || (name == "") ) {
        return false;
    }

    const int id = getGeometryID(name);
    return replaceGeometry(id, geo);
}

bool GeometryContainer::replaceGeometry(const int id, Geometry* geo) {
    if ( (geo == 0) || (id == 0) ) {
        return false;
    }
    
    map<int, Geometry*>::iterator itGeo = geos_.find(id);
    if ( itGeo == geos_.end() ) {
        return false;
    }
    
    Geometry* pOldGeo = itGeo->second;
    if ( initGeometry(geo, pOldGeo->getName(), id) == true ) {
        delete pOldGeo;
        pOldGeo = 0;
        itGeo->second = geo;

        // mark the concerned pointer as a pointer
        // which has been replaced.
        //
        // NOTE: this attribute can only be set as GeometryContainer 
        // class is friend to Geometry class! This construction was been
        // build in order to avoid taking care of a further map which
        // tracks replaced pointers.
        //
        itGeo->second->changed_ = true;
        return true;
    }

    return false;
}

Identifier GeometryContainer::getGeometryName(const int id) const {
    map<int, Geometry*>::const_iterator itGeo = geos_.find(id);
    if ( itGeo == geos_.end() ) {
        return "";
    }
    return itGeo->second->getName();
}

int GeometryContainer::getGeometryID(const Identifier& name) const {
    map<Identifier, int>::const_iterator itID = names_.find(name);
    if ( itID == names_.end() ) {
        return 0;
    }
    return itID->second;
}

bool GeometryContainer::containsGeometry(const Identifier& name) const {
    const int id = getGeometryID(name);
    return containsGeometry(id);
}

bool GeometryContainer::containsGeometry(const int id) const {
    map<int, Geometry*>::const_iterator itGeo = geos_.find(id);
    if ( itGeo != geos_.end() ) {
        return true;
    }

    return false;
}

void GeometryContainer::clear() {
    for ( map<int, Geometry*>::iterator itGeos = geos_.begin(); itGeos != geos_.end(); itGeos++ ) {
        resetGeometry(itGeos->second);
    }

    // Now clear all maps
    //
    geos_.clear();
    names_.clear();

    // The next valid ID is now 1 again. Therefore reset
    // the ID counter.
    //
    nextID_ = 1;
}

void GeometryContainer::clearDeleting() {
    for ( map<int, Geometry*>::iterator itGeos = geos_.begin(); itGeos != geos_.end(); itGeos++ ) {
        delete itGeos->second;
        itGeos->second = 0;
    }

    // Now clear all maps
    //
    geos_.clear();
    names_.clear();

    // The next valid ID is now 1 again. Therefore reset
    // the ID counter.
    //
    nextID_ = 1;
}

}    // namespace
