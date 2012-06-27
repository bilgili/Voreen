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

#ifndef TGT_VERTEXARRAY_H
#define TGT_VERTEXARRAY_H

#include "tgt/tgt_gl.h"

#include <vector>
#include <cstring>

#include "tgt/config.h"
#include "tgt/vector.h"
#include "tgt/texturemanager.h"
#include "tgt/types.h"
#include "tgt/vertex.h"
#include "tgt/attribute.h"


//TODO42 Vererbung?

namespace tgt {

class VertexArray {
public:

    enum BufferUsage {
        STREAM_DRAW = GL_STREAM_DRAW_ARB,
        STREAM_READ = GL_STREAM_READ_ARB,
        STREAM_COPY = GL_STREAM_COPY_ARB,

        STATIC_DRAW = GL_STATIC_DRAW_ARB,
        STATIC_READ = GL_STATIC_READ_ARB,
        STATIC_COPY = GL_STATIC_COPY_ARB,

        DYNAMIC_DRAW = GL_DYNAMIC_DRAW_ARB,
        DYNAMIC_READ = GL_DYNAMIC_READ_ARB,
        DYNAMIC_COPY = GL_DYNAMIC_COPY_ARB
    };

    typedef void (*fnp_renderInBuffer) (void);

private:

    class VAGroup {
    public:

        VAGroup();

        // created during a setter
        fnp_RenderInBuffer      renderInBuffer_;
        Attribute*              attribute_;
        GLenum                  primitiveMode_;

        std::vector<uint8_t>    buffer_;

        size_t sizeOfVertex_;
        size_t size_;
        size_t offset_;

    };

    std::vector<VAGroup>    groups_;
    BufferUsage             usage_;
    GLuint                  bufferID_;

    /**
     * This method sends geometries to the graphic hardware
     * and frees the client memory
    */
    void lock();

public:

    /**
     * creates the object with numGroups groups
     * @param numGroups number of groups
     * @param usage Usage of the buffer object
    */
    explicit VertexArray(size_t numGroups, BufferUsage usage = STATIC_DRAW);

    /**
     * frees memory
    */
    ~VertexArray() {
        //Speicher auf der Grafikarte freigeben
        glDeleteBuffersARB(1, &bufferID_);
    }

    /**
     * Add vertices to inner buffer
     * @param vertices contains the vertices
    */
    template<class InterleavedType>
    void addVertices(size_t idx, const InterleavedType& vertices);

    VAGroup* getGroup(size_t idx) {
        tgtAssert(idx < groups_.size(), "group index out of Bounds");
        return &(groups_[idx]);
    }

    /**
     * renders this object
    */
    void render();

    void setGroupAttribs(
        size_t              idx,
        fnp_renderInBuffer  renderInBuffer,
        Attribute*          attribute,
        GLenum              primitiveMode) {

        groups_[idx].usage_ = usage;
    }
};

template<class InterleavedType>
void VertexArray::addVertices(size_t idx, const InterleavedType& vertices) {
    size_t oldSize = groups_[idx].buffer_.size();
    size_t sizeInBytes = vertices.size() * sizeof(typename InterleavedType::VertexFormat);

    groups_[idx].buffer_.resize(oldSize + sizeInBytes);

    memcpy(&groups_[idx].buffer_[oldSize], &vertices[0], sizeInBytes);

    groups_[idx].sizeOfVertex_  = sizeof(typename InterleavedType::VertexFormat);
    groups_[idx].primitiveMode_ = vertices.primitiveMode_;
}

}

#endif //TGT_VERTEXARRAY_H
