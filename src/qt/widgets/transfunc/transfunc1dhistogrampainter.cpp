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

#include "voreen/qt/widgets/transfunc/transfunc1dhistogrampainter.h"
#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"
#include "voreen/core/datastructures/volume/histogram.h"

#include "tgt/shadermanager.h"
#include "tgt/gpucapabilities.h"
#include "tgt/glcanvas.h"

namespace voreen {

TransFunc1DHistogramPainter::TransFunc1DHistogramPainter(tgt::GLCanvas* canvas)
    : tgt::Painter(canvas)
    , tf_(0)
    , histogram_(0)
    , logarithmic_(true)
    , viewLeft_(0.0f)
    , viewRight_(1.0f)
{
}

TransFunc1DHistogramPainter::~TransFunc1DHistogramPainter() {
}

void TransFunc1DHistogramPainter::setTransFunc(TransFunc* tf) {
    if (tf_ != tf) {
        tf_ = tf;
        resetZoom();
    }

    if(tf) {
        if(tf->getDomain().x < viewLeft_)
            viewLeft_ = tf->getDomain().x;

        if(tf->getDomain().y > viewRight_)
            viewRight_ = tf->getDomain().y;
    }

}

void TransFunc1DHistogramPainter::setHistogram(const VolumeHistogramIntensity* histogram) {
    histogram_ = histogram;
}

void TransFunc1DHistogramPainter::paint() {
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
    if(!tf_)
        return;

    tgt::vec2 dom = tf_->getDomain();

    glScalef(1.0f / (viewRight_ - viewLeft_), 1.0f, 1.0f);
    glTranslatef(-viewLeft_, 0.0f, 0.0f);

    // paint transfer function
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

    // Quad left of domain:
    glTexCoord1f(0.f);
    glVertex3f(viewLeft_, 0.f, -0.5f);
    glVertex3f(dom.x, 0.f, -0.5f);
    glVertex3f(dom.x, 1.f, -0.5f);
    glVertex3f(viewLeft_, 1.f, -0.5f);

    // Inside domain:
    glTexCoord1f(0.f);
    glVertex3f(dom.x, 0.f, -0.5f);
    glTexCoord1f(1.f);
    glVertex3f(dom.y, 0.f, -0.5f);
    glVertex3f(dom.y, 1.f, -0.5f);
    glTexCoord1f(0.f);
    glVertex3f(dom.x, 1.f, -0.5f);

    // Quad right of domain:
    glTexCoord1f(1.f);
    glVertex3f(dom.y, 0.f, -0.5f);
    glVertex3f(viewRight_, 0.f, -0.5f);
    glVertex3f(viewRight_, 1.f, -0.5f);
    glVertex3f(dom.y, 1.f, -0.5f);

    glEnd();

    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_1D);
    glEnable(GL_DEPTH_TEST);

    if(!histogram_) {
        glLoadIdentity();
        return;
    }
    // paint histogram
    const Histogram1D& h = histogram_->getHistogram();
    glDisable(GL_DEPTH_TEST);
    int nBuckets = (int)h.getNumBuckets();

    glColor3f(1.f, 0.f, 0.f);
    float bucketSize = (h.getMaxValue() - h.getMinValue()) / nBuckets;
    float offset = h.getMinValue();
    float max = h.getMaxBucket();
    if(max > 0) {
        if(logarithmic_)
            max = logf(max);
        glBegin(GL_QUADS);
        for (int i = 0 ; i < nBuckets ; ++i) {
            float y = 0.0f;
            int bucket = h.getBucket(i);
            if(bucket > 0) {
                if(logarithmic_) {
                    y = logf(static_cast<float>(bucket)) / max;
                }
                else {
                    y = bucket / max;
                }
            }

            glVertex3f(offset + (i * bucketSize), 0.0f,  -0.5f);  // Bottom Left
            glVertex3f(offset + ((i+1) * bucketSize), 0.0f,  -0.5f);  // Bottom Right
            glVertex3f(offset + ((i+1) * bucketSize), y,  -0.5f);  // Top Right
            glVertex3f(offset + (i * bucketSize), y,  -0.5f);  // Top Left
        }
        glEnd();
        glEnable(GL_DEPTH_TEST);
    }

    glLoadIdentity();
}

void TransFunc1DHistogramPainter::initialize() {
    getCanvas()->getGLFocus();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.f, 1.f, 0.f, 1.f, -2.f, 1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void TransFunc1DHistogramPainter::sizeChanged(const tgt::ivec2& size) {
    getCanvas()->getGLFocus();

    glViewport(0, 0, size.x, size.y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.f, 1.f, 0.f, 1.f, -2.f, 1.f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void TransFunc1DHistogramPainter::zoomIn() {
    if(tf_) {
        float domCenter = (tf_->getDomain().x + tf_->getDomain().y) * 0.5f;
        float domSize = (tf_->getDomain().y - tf_->getDomain().x);
        float viewSize = viewRight_ - viewLeft_;
        viewSize *= 0.5f;
        if(viewSize < domSize)
            viewSize = domSize;

        viewLeft_ = domCenter - (viewSize * 0.5f);
        viewRight_ = domCenter + (viewSize * 0.5f);
    }
}

void TransFunc1DHistogramPainter::zoomOut() {
    if(tf_) {
        float domCenter = (tf_->getDomain().x + tf_->getDomain().y) * 0.5f;
        float domSize = (tf_->getDomain().y - tf_->getDomain().x);
        float viewSize = viewRight_ - viewLeft_;
        float viewCenter = domCenter;

        viewSize *= 2.0f;
        if(viewSize > domSize)
            viewCenter = domCenter;

        viewLeft_ = viewCenter - (viewSize * 0.5f);
        viewRight_ = viewCenter + (viewSize * 0.5f);
    }
}

void TransFunc1DHistogramPainter::resetZoom() {
    if(tf_) {
        viewLeft_ = tf_->getDomain().x;
        viewRight_ = tf_->getDomain().y;
    }
}

float TransFunc1DHistogramPainter::getViewLeft() const {
    return viewLeft_;
}

float TransFunc1DHistogramPainter::getViewRight() const {
    return viewRight_;
}

} // namespace voreen
