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

#include "tgt/mesh.h"

#include "tgt/model.h"

namespace tgt {

Mesh::Mesh( Model* model,
            std::vector<tgt::vec3>* vertexList,
            std::vector<tgt::vec3>* normalList,
            std::vector<tgt::vec2>* texcoordList,
            bool visible,
            bool _static
          )
          : Renderable( Bounds(vec3(), vec3()), _static, visible),
            model_(model),
            vertexList_(vertexList),
            normalList_(normalList),
            texcoordList_(texcoordList)
        {};

Mesh::~Mesh(){
    delete( vertexList_ );
    delete( normalList_ );
    delete( texcoordList_ );
    
    std::list<Submesh*>::iterator it;
    for( it = submeshList_.begin(); it != submeshList_.end(); it++ ){
        delete(*it);
    }
};

void Mesh::addSubmesh( std::vector<uint16_t>* vertexIndices,
                        std::vector<uint16_t>* normalIndices,
                        std::vector<uint16_t>* textureIndices,
                        Material* material ) 
{
    Submesh* submesh = new Submesh( this, vertexIndices, normalIndices, textureIndices, material );
    submeshList_.push_front( submesh );
    
    // used to keep track of materials in the model object
    model_->regMaterial( material );
}

void Mesh::render() {
    std::list<Submesh*>::iterator it;
    for( it = submeshList_.begin(); it != submeshList_.end(); it++ ){
        (*it)->render();
    }
}

} // namespace tgt
