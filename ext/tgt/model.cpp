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

#include "tgt/model.h"

namespace tgt {

Model::Model(bool visible , bool _static) : Renderable( Bounds(vec3(), vec3()), _static, visible) {};
Model::~Model() {
    // deleting meshes
    std::list<Mesh*>::iterator it;
    for( it = meshList_.begin(); it != meshList_.end(); it++ ){
        delete(*it);
    }

    // deleting materials
    std::set<Material*>::iterator iter;
    for( iter = materialSet_.begin(); iter != materialSet_.end(); iter++ ){
        delete(*iter);
    }
};

void Model::render() {
        std::list<Mesh*>::iterator it;
    for( it = meshList_.begin(); it != meshList_.end(); it++ ){
        (*it)->render();
    }
}

void Model::regMaterial ( Material* material ) {
    materialSet_.insert( material );
}

void Model::addMesh( Mesh* mesh ){
    meshList_.push_front( mesh );
}

bool Model::removeMesh( Mesh* mesh ) {
    std::list<Mesh*>::iterator it;
    for( it = meshList_.begin(); it != meshList_.end(); it++ ){
        if ( *it == mesh )
            meshList_.erase( it );
            return true;
    }
    return false;
};


} // namespace tgt
