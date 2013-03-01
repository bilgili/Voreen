/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "voreen/qt/widgets/transfunc/transfunc1dkeyspainter.h"
#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"

#include "tgt/shadermanager.h"
#include "tgt/gpucapabilities.h"
#include "tgt/glcanvas.h"

namespace voreen {

TransFunc1DKeysPainter::TransFunc1DKeysPainter(tgt::GLCanvas* canvas)
    : tgt::Painter(canvas)
    , tf_(0)
{
}

TransFunc1DKeysPainter::~TransFunc1DKeysPainter() {
}

void TransFunc1DKeysPainter::setTransFunc(TransFunc1DKeys* tf) {
    tf_ = tf;
}

void TransFunc1DKeysPainter::paint() {
    // ensure that no shader is active while painting
    if (GpuCaps.areShadersSupported())
        tgt::Shader::deactivate();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float inc = 0.1f;
    // paint checkerboard
    for (int i = 0 ; i < 10 ; ++i) {
        glBegin(GL_QUADS);
            // Front Face
            if (i % 2)
                glColor3f(0.6f, 0.6f, 0.6f);
            else
                glColor3f(1.f, 1.f, 1.f);
            glVertex3f( i      * inc, 0.0f,  -0.5f);  // Bottom Left
            glVertex3f((i + 1) * inc, 0.0f,  -0.5f);  // Bottom Right
            glVertex3f((i + 1) * inc, 0.5f,  -0.5f);  // Top Right
            glVertex3f( i      * inc, 0.5f,  -0.5f);  // Top Left
        glEnd();
        glBegin(GL_QUADS);
            // Front Face
            if (i % 2)
                glColor3f(1.f, 1.f, 1.f);
            else
                glColor3f(0.6f, 0.6f, 0.6f);
            glVertex3f( i      * inc, 0.5f,  -0.5f);  // Bottom Left
            glVertex3f((i + 1) * inc, 0.5f,  -0.5f);  // Bottom Right
            glVertex3f((i + 1) * inc, 1.0f,  -0.5f);  // Top Right
            glVertex3f( i      * inc, 1.0f,  -0.5f);  // Top Left
        glEnd();
    }

    // paint transfer function
    if (tf_) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_1D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
        glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
        tf_->bind();

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glBegin(GL_QUADS);
            glColor4f(1.f, 1.f, 1.f, 1.f);
            // Bottom Left
            glTexCoord1f(0.f);
            glVertex3f(0.f, 0.f, -0.5f);

            // Bottom Right
            glTexCoord1f(1.f);
            glVertex3f(1.f, 0.f, -0.5f);

            // Top Right
            glTexCoord1f(1.f);
            glVertex3f(1.f, 1.f, -0.5f);

            // Top Left
            glTexCoord1f(0.f);
            glVertex3f(0.f, 1.f, -0.5f);

        glEnd();

        glBlendFunc(GL_ONE, GL_ZERO);
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_1D);
        glEnable(GL_DEPTH_TEST);
    }
}

void TransFunc1DKeysPainter::initialize() {
    getCanvas()->getGLFocus();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.f, 1.f, 0.f, 1.f, -2.f, 1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void TransFunc1DKeysPainter::sizeChanged(const tgt::ivec2& size) {
    getCanvas()->getGLFocus();

    glViewport(0, 0, size.x, size.y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.f, 1.f, 0.f, 1.f, -2.f, 1.f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

} // namespace voreen
