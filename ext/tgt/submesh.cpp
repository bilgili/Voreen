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

#include "tgt/submesh.h"
#include "tgt/mesh.h"

namespace tgt {

Submesh::Submesh(
        Mesh* mesh,
        std::vector<uint16_t>* vertexIndices,
        std::vector<uint16_t>* normalIndices,
        std::vector<uint16_t>* textureIndices,
        Material* material,
        bool visible,
        bool _static
         )
        : Renderable( Bounds(vec3(), vec3()), _static, visible),
          mesh_(mesh),
          vertexIndices_(vertexIndices),
          normalIndices_(normalIndices),
          textureIndices_(textureIndices),
          material_(material)
        {};

Submesh::~Submesh(){
    delete( textureIndices_ );
    delete( normalIndices_ );
    delete( vertexIndices_ );
    // materials are not uniqe and handled by the model class.
};

void Submesh::render() { 
    material_->activate();
    if ( material_->getTextureDiffuse() ) material_->getTextureDiffuse()->bind();


    glBegin (GL_TRIANGLES);
	for( int i = 0; i < (int) vertexIndices_->size(); i++) {
        
        // texture
        if ( textureIndices_ )
			glTexCoord2f (
                (*(mesh_->getTexcoordList()))[ (*textureIndices_)[i] ].x, 
				(*(mesh_->getTexcoordList()))[ (*textureIndices_)[i] ].y
            );
        
		// normals
		if ( normalIndices_ )
			glNormal3f (
                (*(mesh_->getNormalList()))[ (*normalIndices_)[i] ].x, 
				(*(mesh_->getNormalList()))[ (*normalIndices_)[i] ].y,
                (*(mesh_->getNormalList()))[ (*normalIndices_)[i] ].z
            );

		// vertices
        if ( vertexIndices_ )
            glVertex3f(
                (*(mesh_->getVertexList()))[ (*vertexIndices_)[i] ].x, 
                (*(mesh_->getVertexList()))[ (*vertexIndices_)[i] ].y,
                (*(mesh_->getVertexList()))[ (*vertexIndices_)[i] ].z
            );
	}
	glEnd();

};

} // namespace tgt
