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

#include "tgt/matrixstack.h"
#include "tgt/glmath.h"

#define MAT_COMPAT_MODE

namespace tgt {

//------------------------------------------------------------------------------
// MatrixStack
//------------------------------------------------------------------------------

const std::string MatrixStack::loggerCat_("tgt.Matrix.Stack");

MatrixStack::MatrixStack() {
    currentStack_ = MODELVIEW;
    modelViewStack_.push(mat4::identity);
    projectionStack_.push(mat4::identity);
    textureStack_.push(mat4::identity);
}

MatrixStack::~MatrixStack() {}

void MatrixStack::loadMatrix(const mat4& m) {
    getStack().top() = m;
#ifdef MAT_COMPAT_MODE
    tgt::loadMatrix(getStack().top());
#endif
}

void MatrixStack::multMatrix(const mat4& m) {
    getStack().top() = getStack().top() * m;
#ifdef MAT_COMPAT_MODE
    tgt::multMatrix(m);
#endif
}

void MatrixStack::loadIdentity() {
    getStack().top() = mat4::identity;
#ifdef MAT_COMPAT_MODE
    tgt::loadMatrix(getStack().top());
#endif
}

void MatrixStack::pushMatrix() {
    getStack().push(getStack().top());
#ifdef MAT_COMPAT_MODE
    glPushMatrix();
#endif
}

void MatrixStack::popMatrix() {
    getStack().pop();
    if(getStack().empty())
        getStack().push(mat4::identity);
#ifdef MAT_COMPAT_MODE
    //tgt::loadMatrix(getStack().top());
    glPopMatrix();
#endif
}

void MatrixStack::matrixMode(StackMode c) {
    currentStack_ = c;
#ifdef MAT_COMPAT_MODE
    switch(currentStack_) {
        case MODELVIEW:
            glMatrixMode(GL_MODELVIEW);
            break;
        case PROJECTION:
            glMatrixMode(GL_PROJECTION);
            break;
        case TEXTURE:
            glMatrixMode(GL_TEXTURE);
            break;
    }
#endif
}

void MatrixStack::scale(const vec3& factors) {
    multMatrix(mat4::createScale(factors));
}

void MatrixStack::scale(float x, float y, float z) {
    multMatrix(mat4::createScale(tgt::vec3(x, y, z)));
}

void MatrixStack::translate(float x, float y, float z) {
    multMatrix(mat4::createTranslation(tgt::vec3(x, y, z)));
}

void MatrixStack::translate(const vec3& trans) {
    multMatrix(mat4::createTranslation(trans));
}

void MatrixStack::rotate(float angle, const vec3& axis) {
    multMatrix(mat4::createRotation(deg2rad(angle), axis));
}

void MatrixStack::rotate(float angle, float x, float y, float z) {
    multMatrix(mat4::createRotation(deg2rad(angle), tgt::vec3(x, y, z)));
}

} // namespace tgt
