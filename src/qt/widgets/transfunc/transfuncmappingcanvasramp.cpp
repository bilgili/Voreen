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

#include "voreen/qt/widgets/transfunc/transfuncmappingcanvasramp.h"

#include "voreen/core/datastructures/transfunc/transfuncmappingkey.h"
#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"

#include <QMouseEvent>

namespace voreen {

using tgt::vec2;

TransFuncMappingCanvasRamp::TransFuncMappingCanvasRamp(QWidget* parent, TransFunc1DKeys* tf, bool noColor,
                                               QString xAxisText,
                                               QString yAxisText)
    : TransFuncMappingCanvas(parent, tf, noColor, xAxisText, yAxisText)
    , rampCenter_(0.f)
    , rampWidth_(0.f)
{
    keyContextMenu_.clear();
    if (!noColor_) {
        QAction* cc = new QAction(tr("Change color of key"), this);
        connect(cc, SIGNAL(triggered()), this, SLOT(changeCurrentColor()));
        keyContextMenu_.addAction(cc);
    }
}

TransFuncMappingCanvasRamp::~TransFuncMappingCanvasRamp() {
}

//--------- methods for reacting on Qt events ---------//

void TransFuncMappingCanvasRamp::showKeyContextMenu(QMouseEvent* event) {
    keyContextMenu_.popup(event->globalPos());
}

void TransFuncMappingCanvasRamp::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit toggleInteractionMode(true);
    }

    event->accept();

    tgt::vec2 sHit = tgt::vec2(event->x(), static_cast<float>(height()) - event->y());
    tgt::vec2 hit = stow(sHit);

    // see if a key was selected
    selectedKey_ = 0;
    for (int i=0; i<tf_->getNumKeys(); ++i) {
        TransFuncMappingKey* key = tf_->getKey(i);
        tgt::vec2 sp = wtos(tgt::vec2(key->getIntensity(), key->getColorL().a / 255.f));
        //tgt::vec2 spr = wtos(tgt::vec2(key->getIntensity(), key->getColorR().a / 255.f));
        if (sHit.x > sp.x - pointSize_ && sHit.x < sp.x + pointSize_ &&
            sHit.y > sp.y - pointSize_ && sHit.y < sp.y + pointSize_)
        {
            selectedKey_ = key;
        }
    }

    if (event->button() == Qt::RightButton) {
        if (selectedKey_ == 0)
            showNoKeyContextMenu(event);
        else
            showKeyContextMenu(event);
        return;
    }

    if (selectedKey_ != 0 && event->button() == Qt::LeftButton) {
        dragging_ = true;
        //keep values within valid range
        hit = tgt::clamp(hit, 0.f, 1.f);
        updateCoordinates(event->pos(), hit);
        if (selectedKey_->isSplit() && !selectedLeftPart_)
            emit colorChanged(Col2QColor(selectedKey_->getColorR()));
        else
            emit colorChanged(Col2QColor(selectedKey_->getColorL()));
    }
}

void TransFuncMappingCanvasRamp::mouseMoveEvent(QMouseEvent* event) {
    unsetCursor();
    event->accept();
    mousePos_ = event->pos();

    vec2 sHit = vec2(event->x(), static_cast<float>(height()) - event->y());
    vec2 hit = stow(sHit);

    // return when no key was inserted or selected
    if (!dragging_)
        return;

    // keep location within valid texture coord range
    hit = tgt::clamp(hit, 0.f, 1.f);

    if (selectedKey_ != 0) {
        TransFuncMappingKey* leftKey = tf_->getKey(0);
        TransFuncMappingKey* rightKey = tf_->getKey(1);
        if (selectedKey_ == leftKey) {
            // obey ramp function restrictions:
            // left key has to stay left of right key
            hit.x = std::min<float>(hit.x, rightKey->getIntensity());
            // max width = 1.f, min center = 0.f
            float minX = rightKey->getIntensity() - 1.f;
            float maxY = std::min(-minX, 0.5f);
            hit.y = std::min(hit.y, maxY);
            if (rightKey->getIntensity() == 1.f) {
                minX = rightKey->getIntensity() - rightKey->getColorL().a / 255.f;
                hit.x = std::max(hit.x, minX);
            }
            // moving left upwards only allowed if at left border (ramp function)
            if (hit.x != 0.f)
                hit.y = 0.f;
        }
        else {
            // obey ramp function restrictions:
            // right key has to stay right of right key
            hit.x = std::max<float>(hit.x, leftKey->getIntensity());
            // max width = 1.f, max center = 1.f
            float maxX = leftKey->getIntensity() + 1.f;
            float minY = std::max(2.f - maxX, 0.5f);
            hit.y = std::max(hit.y, minY);
            if (leftKey->getIntensity() == 0.f) {
                float maxX = 1.f - leftKey->getColorL().a / 255.f;
                hit.x = std::min(hit.x, maxX);
            }
            // moving right downwards only allowed if at right border (ramp function)
            if (hit.x != 1.f)
                hit.y = 1.f;
        }
        selectedKey_->setIntensity(hit.x);
        selectedKey_->setAlphaL(hit.y);
        calcRampParameterFromKeys();
        updateCoordinates(event->pos(), vec2(hit.x, selectedKey_->getAlphaL()));
        repaint();
        emit changed();
    }
}

void TransFuncMappingCanvasRamp::mouseReleaseEvent(QMouseEvent* event) {
    event->accept();
    if (event->button() == Qt::LeftButton) {
        dragging_ = false;
        hideCoordinates();
        update();
        emit toggleInteractionMode(false);
    }
}

void TransFuncMappingCanvasRamp::getRampParameter(float &rampCenter, float &rampWidth) {
    rampCenter = rampCenter_;
    rampWidth = rampWidth_;
}

void TransFuncMappingCanvasRamp::setRampParameter(float center, float width) {
    rampCenter_ = center;
    rampWidth_ = width;
    calcKeysFromRampParameter();
    repaint();
    emit changed();
}

void TransFuncMappingCanvasRamp::calcKeysFromRampParameter() {
    TransFuncMappingKey* key0 = tf_->getKey(0);
    TransFuncMappingKey* key1 = tf_->getKey(1);
    key0->setIntensity(rampCenter_ - rampWidth_/2.f);
    key0->getColorL().a = 0;
    key1->setIntensity(rampCenter_ + rampWidth_/2.f);
    key1->getColorL().a = 255;
    if (key0->getIntensity() < 0.f) {
        key0->getColorL().a = static_cast<unsigned char>(255.f * (-key0->getIntensity()*1.f/(key1->getIntensity()-key0->getIntensity())));
        key0->setIntensity(0.f);
    }
    if (key1->getIntensity() > 1.f) {
        key1->getColorL().a = static_cast<unsigned char>(255.f * ((1.f-key0->getIntensity())*1.f/(key1->getIntensity()-key0->getIntensity())));
        key1->setIntensity(1.f);
    }
    tf_->updateKey(key0);
    calcRampParameterFromKeys();
}

void TransFuncMappingCanvasRamp::calcRampParameterFromKeys() {
    TransFuncMappingKey* key0 = tf_->getKey(0);
    TransFuncMappingKey* key1 = tf_->getKey(1);
    float left = key0->getIntensity();
    float right = key1->getIntensity();
    if (left == 0.f) {
        left = - key0->getColorL().a / 255.f * (key1->getIntensity() - key0->getIntensity()) /
            (key1->getColorL().a / 255.f - key0->getColorL().a / 255.f);
    }
    if (right == 1.f) {
        right = 1.f + (1.f - key1->getColorL().a / 255.f) * (key1->getIntensity() - key0->getIntensity()) /
            (key1->getColorL().a / 255.f - key0->getColorL().a / 255.f);
    }
    rampCenter_ = (right + left) / 2.f;
    rampWidth_ = right - left;
}

} // namespace voreen
