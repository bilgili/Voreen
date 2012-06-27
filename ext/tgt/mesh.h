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

#ifndef TGT_MESH_H
#define TGT_MESH_H

#include <vector>
#include <list>

#include "tgt/config.h"
#include "tgt/vector.h"
#include "tgt/submesh.h"
#include "tgt/renderable.h"

namespace tgt {

class Texture;
class Material;
class Model;

/** 
    This is the mesh class. It mainly stores the geometrical data while 
    indices an materials are stored in submeshes. 
*/
class Mesh : public Renderable{
protected:
    /// Only needed for material deleting.
    Model* model_;
    
    /** Vertex data. */
    std::vector< tgt::vec3 >* vertexList_;
    /** Normal data. */
    std::vector< tgt::vec3 >* normalList_;
    /** Texture coordinates data. */
    std::vector< tgt::vec2 >* texcoordList_;
    /** List of all submeshes belonging to this mesh. */
    std::list< Submesh* > submeshList_;
    
public:
    /** A constructor. */
    Mesh( Model* model,
          std::vector< tgt::vec3 >* vertexList,
          std::vector< tgt::vec3 >* normalList,
          std::vector< tgt::vec2 >* texcoordList,
         bool visible = true, 
         bool _static = true 
        );
    
    /** A destructor */
    virtual ~Mesh();
    
    /** Creates new Submesh and registers its material in the model. */
    virtual void addSubmesh(std::vector<uint16_t>* vertexIndices,
                            std::vector<uint16_t>* normalIndices,
                            std::vector<uint16_t>* textureIndices = NULL,
                            Material* material = NULL );
    /** Draws all Submeshes. */
    virtual void render();
    
    virtual void init() {};
    virtual void deinit() {};


    virtual std::vector< tgt::vec3 >* getVertexList() { return vertexList_; }
    virtual std::vector< tgt::vec3 >* getNormalList() { return normalList_; }
    virtual std::vector< tgt::vec2 >* getTexcoordList() { return texcoordList_; }
};

}  // namespace tgt
#endif // TGT_MESH_H
