/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2005-2013 Visualization and Computer Graphics Group, *
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

#ifndef TGT_MATRIXSTACK_H
#define TGT_MATRIXSTACK_H

#include <stack>

#include "tgt/matrix.h"
#include "tgt/singleton.h"
#include "tgt/types.h"

namespace tgt {


class MatrixStack;
#ifdef DLL_TEMPLATE_INST
template class TGT_API Singleton<MatrixStack>;
#endif

/**
*   Texture Manager
*/
class TGT_API MatrixStack : public Singleton<MatrixStack> {

public:

    enum StackMode {
        MODELVIEW,
        PROJECTION,
        TEXTURE
    };

    /**
     *   Init texturemanager.
     */
    MatrixStack();
    virtual ~MatrixStack();

    void loadMatrix(const mat4& m);
    void multMatrix(const mat4& m);
    void loadIdentity();
    void pushMatrix();
    void popMatrix();
    void matrixMode(StackMode c);

    void scale(const vec3& factors);
    void scale(float x, float y, float z);

    void translate(const vec3& trans);
    void translate(float x, float y, float z);

    void rotate(float angle, const vec3& axis);
    void rotate(float angle, float x, float y, float z);

    mat4 getModelViewMatrix() const {
        return modelViewStack_.top();
    }
    mat4 getProjectionMatrix() const {
        return projectionStack_.top();
    }
    mat4 getTextureMatrix() const {
        return textureStack_.top();
    }

private:
    static const std::string loggerCat_;

    std::stack<mat4> modelViewStack_;
    std::stack<mat4> projectionStack_;
    std::stack<mat4> textureStack_;

    StackMode currentStack_;
    std::stack<mat4>& getStack() {
        switch(currentStack_) {
            case MODELVIEW:
                return modelViewStack_;
            case PROJECTION:
                return projectionStack_;
            case TEXTURE:
                return textureStack_;
            default:
                tgtAssert(false, "should not get here");
                return modelViewStack_;
        }
    }
};


} // namespace tgt

#define MatStack tgt::Singleton<tgt::MatrixStack>::getRef()

#endif //TGT_TEXTUREMANAGER_H
