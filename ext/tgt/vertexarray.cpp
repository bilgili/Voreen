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

#include "vertexarray.h"

namespace tgt {

VertexArray::VAGroup::VAGroup()
  : offset_(0),
    size_(0),
    primitiveMode_(GL_TRIANGLES),
    attribute_(0)
{}

VertexArray::VertexArray(size_t numGroups)
  : groups_(numGroups),
    wireframe_(false),
    asNPR_(false),
    isLocked_(false),
    bufferID_(0)
{}

//Vielleicht kann man hier auch mit glBufferSubData rumhantieren
//anstatt ein neues Array anzulegen
void VertexArray::lock(BufferUsage usage) {
    tgtAssert(!isLocked_, "VertexArray wurde bereits gelockt.");

    std::vector<uint8_t> buffer;
    size_t totalSize = 0;

    for (size_t i = 0; i < groups_.size(); ++i) {
        buffer.insert(buffer.end(), groups_[i].buffer_.begin(), groups_[i].buffer_.end());
        if (groups_[i].sizeOfVertex_)
            groups_[i].size_ = groups_[i].buffer_.size() / groups_[i].sizeOfVertex_;
        else
            groups_[i].size_ = 0;
        groups_[i].offset_ = totalSize;
        totalSize += groups_[i].size_;
    }

    //Und auf der Grafikkarte speichern
    glGenBuffersARB(1, &bufferID_);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, bufferID_);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, buffer.size(), &buffer[0], usage);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    //Den gesamten Speicher freigeben.
    //Alle Geometrien liegen nun auf der Grafikkarte
    buffer.clear();

    isLocked_ = true;
}

void VertexArray::render() {
    if (!isLocked_)
        lock();

    //TODO42 GLEW einbinden
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, bufferID_);

    if (wireframe_)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    for (size_t idx = 0; idx < groups_.size(); ++idx) {
        if(groups_[idx].attribute_)
            groups_[idx].attribute_->init();

        glInterleavedArrays(GL_T2F_N3F_V3F, 0, 0);
        glDrawArrays(groups_[idx].primitiveMode_, groups_[idx].offset_, groups_[idx].size_);

        if(groups_[idx].attribute_)
            groups_[idx].attribute_->deinit();
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);
    //Bindung ans Buffer Object lösen:
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

/*
    if(asNPR_) {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, lineBufferID_);
        glColor3f(0.0f, 0.0f, 0.0f);

        glInterleavedArrays(GL_V3F, 0, 0);
        for (size_t idx = 0; idx < groups_.size(); ++idx)
            glDrawArrays(GL_LINES, groups_[idx].lineOffset_, groups_[idx].lineSize_);

        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }
*/
}

} // namespace
