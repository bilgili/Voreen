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

#ifndef TGT_SUBMESH_H
#define TGT_SUBMESH_H

#include <vector>

#include "tgt/config.h"
#include "tgt/vector.h"
#include "tgt/material.h"
#include "tgt/texture.h"
#include "tgt/renderable.h"
#include "tgt/logmanager.h"

namespace tgt {

class Texture;
class Material;
class Mesh;

/** 
    While the mesh holds all the geometric data the submesh stores the indices for rendering
    as well as its material. 
*/
class Submesh : public Renderable {
protected:
    Mesh* mesh_;
    /** Vertex indices. */
    std::vector<uint16_t>* vertexIndices_;
    /** Normal indices. */
    std::vector<uint16_t>* normalIndices_;
    /** Texture indices. */
    std::vector<uint16_t>* textureIndices_;
    
    /** The material bound to this submesh */
    Material* material_;
    
public:
    /** A constructor. */
    Submesh( Mesh* mesh,
             std::vector<uint16_t>* vertexIndices,
             std::vector<uint16_t>* normalIndices,
             std::vector<uint16_t>* textureIndices = NULL,
             Material* material = NULL,
             bool visible = true, 
             bool _static = true    
             );
    /** A destructor. */
    virtual ~Submesh();
    
    /** Render the submesh. Should be reworked.*/
    virtual void render();
    virtual void init() {};
    virtual void deinit() {};
    
};

}  // namespace tgt
#endif // TGT_SUBMESH_H
