/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "voreen/qt/widgets/transfunc/transfunc2dprimitivespainter.h"

#include "voreen/core/datastructures/transfunc/transfunc2dprimitives.h"
#include "voreen/core/datastructures/transfunc/transfuncprimitive.h"
#include "voreen/core/datastructures/volume/histogram.h"
#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorresample.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatorgradient.h"

#include "tgt/qt/qtcanvas.h"
#include "tgt/glcanvas.h"
#include "tgt/texture.h"
#include "tgt/shadermanager.h"
#include "tgt/gpucapabilities.h"

#include <QApplication>
#include <QColorDialog>
#include <QCursor>
#include <QPoint>

namespace voreen {

TransFunc2DPrimitivesPainter::TransFunc2DPrimitivesPainter(tgt::GLCanvas* canvas)
    : QObject()
    , tgt::Painter(canvas)
    , tgt::EventListener()
    , tf_(0)
    , selectedPrimitive_(0)
    , showHistogram_(false)
    , showGrid_(false)
    , histogramLogarithmic_(true)
    , histogramBrightness_(1.f)
    , volume_(0)
    , histogram_(0)
    , histogramTex_(0)
    , dragging_(false)
    , scaleFactor_(1.f)
{
    canvas->getEventHandler()->addListenerToBack(this);
}

TransFunc2DPrimitivesPainter::~TransFunc2DPrimitivesPainter() {
    delete histogram_;
    delete histogramTex_;
}

void TransFunc2DPrimitivesPainter::mousePressEvent(tgt::MouseEvent* event) {
    event->accept();
    tgt::ivec2 size = getCanvas()->getSize();
    tgt::vec2 pos = tgt::vec2(    static_cast<float>(event->x()) / size.x,
                              1.f-static_cast<float>(event->y()) / size.y);

    TransFuncPrimitive* p = getPrimitiveUnderMouse(pos);

    if (p)
        selectPrimitive(p, pos);
    else
        deselectPrimitive();

    mouseCoord_ = pos;

    getCanvas()->update();
}

void TransFunc2DPrimitivesPainter::mouseMoveEvent(tgt::MouseEvent* event) {
    event->accept();
    tgt::ivec2 size = getCanvas()->getSize();
    tgt::vec2 pos = tgt::vec2(    static_cast<float>(event->x()) / size.x,
                              1.f-static_cast<float>(event->y()) / size.y);

    tgt::vec2 offset = pos - mouseCoord_;

    if (selectedPrimitive_) {
        if (!dragging_) {
            emit toggleInteractionMode(true);
            dragging_ = true;
        }
        // test whether the movement is correct
        /*bool inside = */selectedPrimitive_->move(offset);
        // one or more control points are moved outside of the canvas
        // -> do not allow movement and reset mouse cursor to position before movement
        //if (!inside) {
            //tgt::QtCanvas* canvas = static_cast<tgt::QtCanvas*>(getCanvas());
            //QPoint p(static_cast<int>(mouseCoord_.x * size.x),
                     //static_cast<int>((1.f - mouseCoord_.y) * size.y));
            //p = canvas->mapToGlobal(p);
            //QCursor::setPos(p);
        //}
        //else {
            mouseCoord_ = pos;
            updateTF();
        //}
    }

    getCanvas()->update();
}

void TransFunc2DPrimitivesPainter::mouseReleaseEvent(tgt::MouseEvent* event) {
    event->accept();
    if (selectedPrimitive_ && dragging_) {
        dragging_ = false;
        emit toggleInteractionMode(false);
    }
}

void TransFunc2DPrimitivesPainter::mouseDoubleClickEvent(tgt::MouseEvent* event) {
    event->accept();
    // mousepress event was send right before doubleClick event
    // -> primitive already selected
    colorizePrimitive();
}

void TransFunc2DPrimitivesPainter::wheelEvent(tgt::MouseEvent* event) {
    event->accept();

    int offset = 0;
    if (event->button() == tgt::MouseEvent::MOUSE_WHEEL_DOWN)
        offset = -10;
    else if (event->button() == tgt::MouseEvent::MOUSE_WHEEL_UP)
        offset = 10;

    if (selectedPrimitive_) {
        int trans = selectedPrimitive_->getColor().a;
        if ((trans + offset) > 255)
            trans = 255;
        else if (trans + offset < 0)
            trans = 0;
        else
            trans += offset;

        transparencyChanged(trans);

        emit setTransparencySlider(trans);
    }
}

void TransFunc2DPrimitivesPainter::selectPrimitive(TransFuncPrimitive* p, tgt::vec2 pos) {
    if (selectedPrimitive_)
        selectedPrimitive_->deselect();

    selectedPrimitive_ = p;
    selectedPrimitive_->select(pos);

    // inform widget about selection
    emit primitiveSelected();
}

void TransFunc2DPrimitivesPainter::deselectPrimitive() {
    if (selectedPrimitive_) {
        selectedPrimitive_->deselect();
        selectedPrimitive_ = 0;
    }

    // inform editorwidget about deselection
    emit primitiveDeselected();
}

TransFuncPrimitive* TransFunc2DPrimitivesPainter::getPrimitiveUnderMouse(tgt::vec2 pos) {
    TransFuncPrimitive* clicked = 0;

    getCanvas()->getGLFocus();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_BLEND);
    // paint all primitives with special color
    // (first component is id and both other components are 123)
    tf_->paintForSelection();

    glPopMatrix();
    glPopAttrib();

    // read pixels at clicked position
    GLubyte pixel[3];
    glReadPixels(static_cast<int>(getCanvas()->getWidth()*pos.x),
                 static_cast<int>(getCanvas()->getHeight()*pos.y), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &pixel);

    if ((pixel[1] == 123) && (pixel[2] == 123))
        clicked = tf_->getPrimitive(pixel[0]);

    // no primitive clicked but the user could have clicked on a control point
    if (!clicked)
        clicked = tf_->getPrimitiveForClickedControlPoint(pos);

    return clicked;
}


void TransFunc2DPrimitivesPainter::addQuadPrimitive() {
    // create new primitive
    TransFuncPrimitive* p = new TransFuncQuad(tgt::vec2(0.5f), 0.2f, tgt::col4(128), scaleFactor_);

    // add primitive to transfer function
    tf_->addPrimitive(p);

    // update texture of transfer function
    updateTF();

    // select primitive and update widgets
    selectPrimitive(p);

    // repaint canvas
    getCanvas()->update();
}

void TransFunc2DPrimitivesPainter::addBananaPrimitive() {
    // create new primitive
    TransFuncPrimitive* p = new TransFuncBanana(tgt::vec2(0.f), tgt::vec2(0.3f, 0.4f),
                                                tgt::vec2(0.34f, 0.2f), tgt::vec2(0.5f, 0.f),
                                                tgt::col4(128), scaleFactor_);

    // add primitive to transfer function
    tf_->addPrimitive(p);

    // update texture of transfer function
    updateTF();

    // select primitive and update widgets
    selectPrimitive(p);

    // repaint canvas
    getCanvas()->update();
}

void TransFunc2DPrimitivesPainter::deletePrimitive() {
    if (selectedPrimitive_) {
        tf_->removePrimitive(selectedPrimitive_);
        selectedPrimitive_ = 0;
        updateTF();
        emit primitiveDeselected();

        getCanvas()->update();
    }
}

void TransFunc2DPrimitivesPainter::colorizePrimitive() {
    if (selectedPrimitive_) {
        tgt::col4 primitiveColor = selectedPrimitive_->getColor();
        // set start color to current color of the primitive
        QColor qcolor(primitiveColor.r, primitiveColor.g, primitiveColor.b, primitiveColor.a);
        qcolor = QColorDialog::getRgba(qcolor.rgba());
        if (qcolor.isValid()) {
            primitiveColor.r = qcolor.red();
            primitiveColor.g = qcolor.green();
            primitiveColor.b = qcolor.blue();
            primitiveColor.a = qcolor.alpha();
            selectedPrimitive_->setColor(primitiveColor);

            // set transparency slider to choosen value
            emit setTransparencySlider(qcolor.alpha());

            updateTF();

            getCanvas()->update();
        }
    }
}

void TransFunc2DPrimitivesPainter::fuzzinessChanged(int fuzzi) {
    if (selectedPrimitive_)
        selectedPrimitive_->setFuzziness(fuzzi/100.f);

    updateTF();

    getCanvas()->update();
}

void TransFunc2DPrimitivesPainter::transparencyChanged(int trans) {
    if (selectedPrimitive_) {
        tgt::col4 color = selectedPrimitive_->getColor();
        color.a = trans;
        selectedPrimitive_->setColor(color);
    }

    updateTF();

    getCanvas()->update();
}

void TransFunc2DPrimitivesPainter::resetTransferFunction() {
    selectedPrimitive_ = 0;
    tf_->clear();

    updateTF();

    getCanvas()->update();
}

void TransFunc2DPrimitivesPainter::paint() {
    if (GpuCaps.areShadersSupported())
        tgt::Shader::deactivate();

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDisable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    if (showHistogram_ && histogramTex_) {
        glEnable(GL_TEXTURE_2D);
        histogramTex_->bind();
        glBegin(GL_QUADS);
            // Front Face
            glColor3f(1.f, 1.f, 1.f);
            glTexCoord2f(0.f, 0.f); glVertex3f(0.f, 0.f, -0.5f);  // Bottom Left Of The Texture and Quad
            glTexCoord2f(1.f, 0.f); glVertex3f(1.f, 0.f, -0.5f);  // Bottom Right Of The Texture and Quad
            glTexCoord2f(1.f, 1.f); glVertex3f(1.f, 1.f, -0.5f);  // Top Right Of The Texture and Quad
            glTexCoord2f(0.f, 1.f); glVertex3f(0.f, 1.f, -0.5f);  // Top Left Of The Texture and Quad
        glEnd();
        histogramTex_->disable();
        glDisable(GL_TEXTURE_2D);
    }

    if (showGrid_) {
        glBegin(GL_LINES);
        glColor3f(0.7f, 0.7f, 0.7f);
        for (int i = 0; i < 10; ++i) {
            glVertex3f(0.f, i * 0.1f, -0.5f);
            glVertex3f(1.f, i * 0.1f, -0.5f);

            glVertex3f(i * 0.1f,  0.f, -0.5f);
            glVertex3f(i * 0.1f,  1.f, -0.5f);
        }
        glEnd();
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    tf_->paintInEditor();
    
    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void TransFunc2DPrimitivesPainter::initialize() {
    getCanvas()->getGLFocus();

    GLint buffer;
    glGetIntegerv(GL_DRAW_BUFFER, &buffer);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.f, 1.f, 0.f, 1.f, -2.f, 1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDrawBuffer(buffer);
}

void TransFunc2DPrimitivesPainter::updateTF() {
    tf_->invalidateTexture();
    emit repaintSignal();
}

void TransFunc2DPrimitivesPainter::sizeChanged(const tgt::ivec2& size) {
    getCanvas()->getGLFocus();

    glViewport(0, 0, size.x, size.y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.f, 1.f, 0.f, 1.f, -2.f, 1.f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

const TransFuncPrimitive* TransFunc2DPrimitivesPainter::getSelectedPrimitive() {
    return selectedPrimitive_;
}

void TransFunc2DPrimitivesPainter::setTransFunc(TransFunc2DPrimitives* tf) {
    selectedPrimitive_ = 0;
    tf_ = tf;

    if (tf_) {
        updateTF();
        getCanvas()->update();
    }
}

void TransFunc2DPrimitivesPainter::toggleHistogramLogarithmic(int state) {
    histogramLogarithmic_ = static_cast<bool>(state);

    updateHistogramTexture();

    getCanvas()->update();
}

void TransFunc2DPrimitivesPainter::histogramBrightnessChanged(int brightness) {
    histogramBrightness_ = brightness / 100.f;

    updateHistogramTexture();

    getCanvas()->update();
}

void TransFunc2DPrimitivesPainter::setHistogramVisible(bool v) {
    showHistogram_ = v;
    if (showHistogram_ && !histogram_) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        createHistogram();
        createHistogramTexture();
        QApplication::restoreOverrideCursor();

        // update texture of transfer function because the scaling factor changed
        updateTF();
    }
}

void TransFunc2DPrimitivesPainter::createHistogram() {
    const VolumeBase* gradientVolume = 0;
    const VolumeBase* intensityVolume = 0;

    int bucketsg;
    int bucketsi;
    size_t numChannels = volume_->getNumChannels();
    const VolumeRAM* vol = volume_->getRepresentation<VolumeRAM>();
    if (numChannels == 4) {
        // gradients are already stored in the volume
        gradientVolume = volume_;
        if ((vol->getBytesPerVoxel() / numChannels) > 1)
            bucketsg = 512;
        else
            bucketsg = 256;
        bucketsi = bucketsg;
    }
    else {
        // calculate 8 bit gradients
        if (tgt::max(volume_->getDimensions()) <= 128) {
            intensityVolume = volume_;
        } else {
            // HACK!
            tgt::ivec3 newDims = tgt::ivec3(tgt::vec3(volume_->getDimensions()) / (tgt::max(volume_->getDimensions()) / 128.f));
            intensityVolume = VolumeOperatorResample::APPLY_OP(volume_, newDims, VolumeRAM::LINEAR);
        }
        VolumeOperatorGradient volOpGr;
        gradientVolume = volOpGr.apply<uint8_t>(intensityVolume,VolumeOperatorGradient::VOG_CENTRAL_DIFFERENCE);

        bucketsg = 256;
        if ((vol->getBytesPerVoxel() / numChannels) > 1)
            bucketsi = 512;
        else
            bucketsi = 256;
    }
    // create histogram with scaling to the maximum gradient length in the dataset
    histogram_ = new VolumeHistogramIntensityGradient(gradientVolume, intensityVolume, bucketsi, bucketsg, true);
    scaleFactor_ = histogram_->getScaleFactor();
    tf_->setScaleFactor(scaleFactor_);

    if (numChannels != 4)
        delete gradientVolume;

    if (intensityVolume != volume_)
        delete intensityVolume;
}

void TransFunc2DPrimitivesPainter::createHistogramTexture() {
    tgt::ivec3 dims = tgt::ivec3(static_cast<int>(histogram_->getBucketCountIntensity()),
        static_cast<int>(histogram_->getBucketCountGradient()), 1);

    delete histogramTex_;
    histogramTex_ = new tgt::Texture(dims, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, tgt::Texture::LINEAR);

    updateHistogramTexture();
}

void TransFunc2DPrimitivesPainter::updateHistogramTexture() {
    for (int i = 0; i < histogramTex_->getHeight(); ++i) {
        for (int j = 0; j < histogramTex_->getWidth(); ++j) {
            int value;
            if (histogramLogarithmic_)
                value = tgt::iround(histogram_->getLogNormalized(j, i)*255.f*histogramBrightness_);
            else
                value = tgt::iround(histogram_->getNormalized(j, i)*255.f*histogramBrightness_);
            if (value > 255)
                value = 255;
            else if (value < 0)
                value = 0;
            histogramTex_->texel<uint8_t>(j, i) = static_cast<uint8_t>(value);
        }
    }
    histogramTex_->uploadTexture();
}

void TransFunc2DPrimitivesPainter::volumeChanged(const VolumeBase* newVolume) {
    volume_ = newVolume;

    histogramBrightness_ = 1.f;
    showHistogram_ = false;
    scaleFactor_ = 1.f;
    if (tf_)
        tf_->setScaleFactor(scaleFactor_);

    delete histogram_;
    delete histogramTex_;
    histogram_ = 0;
    histogramTex_ = 0;
}

void TransFunc2DPrimitivesPainter::toggleShowGrid(bool v) {
    showGrid_ = v;
}

} // namespace voreen
