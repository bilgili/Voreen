/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#ifndef TGT_MODEL_H
#define TGT_MODEL_H

#include <list>
#include <vector>
#include <set>

#include "tgt/config.h"
#include "tgt/mesh.h"
#include "tgt/material.h"
#include "tgt/renderable.h"

namespace tgt {

/** 
    Model class. Used to keep track of materials and holds the meshes of the model.
*/
class Model : public Renderable{

friend class Mesh;

protected:
    std::list<tgt::Mesh*> meshList_;
    
    /// Filled by mesh constructor. Holds all materials used by model.
    /// Used to delete materials properly.
    std::set< Material* > materialSet_;

    /// This should be called by the mesh constructor.
    virtual void regMaterial( Material* material );
    
public:
    /** A contructor. */
    Model(bool visible = true, bool _static = true);
    /** A destructor. Deletes all registered Materials. */
    virtual ~Model();
    
    /** render the model. */
    virtual void render();
    virtual void init() {};
    virtual void deinit() {};
    
    /** Adds a mesh to the model. */
    virtual void addMesh( Mesh* mesh );
    
    // do we need this? how do we handle materials?
    /** Removes mesh from model. */
    virtual bool removeMesh( Mesh* mesh );
    
    /** Returns all the Meshes of the model. */
    virtual std::list<tgt::Mesh*> getMeshes() { return meshList_; };
};

} // namespace tgt
#endif // TGT_MODEL_H
