/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "voreen/qt/widgets/transfunc/transfuncintensitypainter.h"

#include "tgt/shadermanager.h"

namespace voreen {

TransFuncIntensityPainter::TransFuncIntensityPainter(tgt::GLCanvas* cv)
	: tgt::Painter(cv),
	tf_(0)
{
	startingPoint_ = 0.f;
	endingPoint_ = 1.f;

}

TransFuncIntensityPainter::~TransFuncIntensityPainter() {
}

void TransFuncIntensityPainter::setTextureCoords(float start, float end) {
	startingPoint_ = start;
	endingPoint_ = end;
}

void TransFuncIntensityPainter::setTransFunc(TransFuncIntensity* tf) {
    tf_ = tf;
}

void TransFuncIntensityPainter::paint() {
    if (GpuCaps.areShadersSupported())
        tgt::Shader::deactivate();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float inc = 1.0f / 10.0f;
    for (int i = 0 ; i < 10 ; ++i) {
        glBegin(GL_QUADS);
            // Front Face
            if (i % 2)
                glColor3f(0.6f,0.6f,0.6f);
            else
                glColor3f(1.0f,1.0f,1.0f);
            glVertex3f( i*inc, 0.0f,  -0.5f);  // Bottom Left Of The Texture and Quad
            glVertex3f( (i+1)*inc, 0.0f,  -0.5f);  // Bottom Right Of The Texture and Quad
            glVertex3f( (i+1)*inc, 0.5f,  -0.5f);  // Top Right Of The Texture and Quad
            glVertex3f( i*inc, 0.5f,  -0.5f);  // Top Left Of The Texture and Quad
        glEnd();
        glBegin(GL_QUADS);
            // Front Face
            if (i % 2)
                glColor3f(1.0,1.0,1.0);
            else
                glColor3f(0.6,0.6,0.6);
            glVertex3f( i*inc, 0.5f,  -0.5f);  // Bottom Left Of The Texture and Quad
            glVertex3f( (i+1)*inc, 0.5f,  -0.5f);  // Bottom Right Of The Texture and Quad
            glVertex3f( (i+1)*inc, 1.0f,  -0.5f);  // Top Right Of The Texture and Quad
            glVertex3f( i*inc, 1.0f,  -0.5f);  // Top Left Of The Texture and Quad
        glEnd();
    }

    if (tf_) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_1D);
        glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
        glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);
        tf_->bind();

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glBegin(GL_QUADS);
            // Front Face
            glColor4f(1.0,1.0,1.0,1.0);
            glTexCoord2f(startingPoint_, 0.0f); glVertex3f( 0.0f, 0.0f,  -0.5f);  // Bottom Left Of The Texture and Quad
            glTexCoord2f(endingPoint_, 0.0f); glVertex3f( 1.0f, 0.0f,  -0.5f);  // Bottom Right Of The Texture and Quad
            glTexCoord2f(endingPoint_, 1.0f); glVertex3f( 1.0f,  1.0f,  -0.5f);  // Top Right Of The Texture and Quad
            glTexCoord2f(startingPoint_, 1.0f); glVertex3f(0.0f,  1.0f,  -0.5f);  // Top Left Of The Texture and Quad

            glTexCoord1f(startingPoint_); glVertex3f( 0.0f, 0.0f,  -0.5f);  // Bottom Left Of The Texture and Quad
            glTexCoord1f(endingPoint_); glVertex3f( 1.0f, 0.0f,  -0.5f);  // Bottom Right Of The Texture and Quad
            glTexCoord1f(endingPoint_); glVertex3f( 1.0f,  1.0f,  -0.5f);  // Top Right Of The Texture and Quad
            glTexCoord1f(startingPoint_); glVertex3f(0.0f,  1.0f,  -0.5f);  // Top Left Of The Texture and Quad
        glEnd();
        glDisable(GL_TEXTURE_1D);
    }
    glDisable(GL_BLEND);
    //getCanvas()->swap();
}

void TransFuncIntensityPainter::initialize() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f,1.0,0.0,1.0f,-2.0f,1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void TransFuncIntensityPainter::sizeChanged(const tgt::ivec2& size) {
    glViewport(0, 0, size.x, size.y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f,1.0f,0.0f,1.0f,-2.0f,1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

} // namespace
