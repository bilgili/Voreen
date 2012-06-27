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

#include "voreen/qt/widgets/transfunc/transfuncmappingcanvas.h"

#include <QPainter>
#include <QColorDialog>
#include <QFileDialog>
#include <QtGui>

#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/transfunc/transfuncintensity.h"
#include "voreen/core/vis/transfunc/transfuncmappingkey.h"

#include "voreen/qt/widgets/transfunc/transfuncgradient.h"

namespace voreen {

using tgt::vec2;

namespace {

inline QColor Col2QColor(const tgt::col4& color) {
    return QColor(color.r, color.g, color.b); // ignore alpha
}

inline tgt::col4 QColor2Col(const QColor& color) {
    return tgt::col4(color.red(), color.green(), color.blue(), 255); // ignore alpha
}

} // namespace

const std::string TransFuncMappingCanvas::loggerCat_("voreen.qt.widgets.TransFuncMappingCanvas");

TransFuncMappingCanvas::TransFuncMappingCanvas(QWidget *parent, TransFuncIntensity* tf, TransFuncGradient* gradient,
                                               MessageReceiver* msgReceiver, bool noColor,
                                               bool rampMode, bool clipThresholds, QString xAxisText,
                                               QString yAxisText, QString transferFuncPath)
    : QWidget(parent),
      gradient_(gradient),
      tf_(tf),
      xAxisText_(xAxisText),
      yAxisText_(yAxisText),
      transferFuncPath_(transferFuncPath),
      msgReceiver_(msgReceiver),
      isChanged_(false),
      rampMode_(rampMode),
      clipThresholds_(clipThresholds),
      rampCenter_(0.f),
      rampWidth_(0.f),
      noColor_(noColor)
{
    histogram_ = 0;
    propertyKey_ = "caption";
    defaultValue_ = 1.0;
    caption_ = QString("caption");

    //this can cause problems in dockwidget!
	//setMinimumSize(128, 128);
	
    xRange_ = vec2(0.f, 1.f);
    yRange_ = vec2(0.f, 1.f);
    padding_ = 12;
    arrowLength_ = 10;
    arrowWidth_ = 3;
    pointSize_ = 10;
    selectedKey_ = 0;
    selectedLeftPart_ = true;
    splitFactor_ = 1.5;
    minCellSize_ = 8;
    gridSnap_ = false;
    curDataset_ = 0;
    showHistogram_ = false;

    // distribute messages via distributor, if no message receiver specified
    if (!msgReceiver_)
        msgReceiver_ = tgt::Singleton<MessageDistributor>::getPtr();

    setThreshold(0.0f,1.0f);
    minThresholdGap_ = 0.008f;
    dragging_ = false;
    dragLine_ = -1;

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    setFocus();
    setCursor(Qt::CrossCursor);
    if (gradient_)
        gradient_->setTransFunc(tf_);

    if (!noColor_) {
        QAction* cc = new QAction(tr("Change color of key"), this);
        contextMenu_.addAction(cc);
        connect(cc, SIGNAL(triggered()), this, SLOT(changeCurrentColor()));
    }

    splitMergeAction_ = new QAction(tr(""), this); // Text will be set later
    contextMenu_.addAction(splitMergeAction_);
    connect(splitMergeAction_, SIGNAL(triggered()), this, SLOT(splitMergeKeys()));

    zeroAction_ = new QAction("", this); // Text will be set later
    contextMenu_.addAction(zeroAction_);
    connect(zeroAction_, SIGNAL(triggered()), this, SLOT(zeroKey()));

    deleteAction_ = new QAction(tr("Delete this key"), this);
    contextMenu_.addAction(deleteAction_);
    connect(deleteAction_, SIGNAL(triggered()), this, SLOT(deleteKey()));

    connect(this, SIGNAL(changed()), this, SLOT(onChanged()));

    loadAction_ = new QAction(tr("Load transfer function..."), this);
    fileMenu_.addAction(loadAction_);
    connect(loadAction_, SIGNAL(triggered()), this, SLOT(readFromDisc()));

    saveAction_ = new QAction(tr("Save transfer function..."), this);
    fileMenu_.addAction(saveAction_);
    connect(saveAction_, SIGNAL(triggered()), this, SLOT(saveToDisc()));

    resetAction_ = new QAction(tr("Reset transfer function"), this);
    fileMenu_.addAction(resetAction_);
    connect(resetAction_, SIGNAL(triggered()), this, SLOT(setStandardFunc()));
}

void TransFuncMappingCanvas::paintEvent(QPaintEvent *event) {
    event->accept();
    QPainter paint(this);

    // put origin in lower lefthand corner
    QMatrix m;
    m.translate(0.0, static_cast<float>(height())-1);
    m.scale(1.0, -1.0);
    paint.setMatrix(m);

    paint.setMatrixEnabled(true);
    paint.setRenderHint(QPainter::Antialiasing, false);
    paint.setPen(Qt::NoPen);
    paint.setBrush(Qt::white);
    paint.drawRect(0, 0, width() - 1, height() - 1);

    // ----------------------------------------------

    // draw grid
    paint.setPen(QColor(220, 220, 220));
    paint.setRenderHint(QPainter::Antialiasing, false);

    vec2 pmin = snapToGrid(vec2(0.f, 0.f));
    vec2 pmax = snapToGrid(vec2(1.f, 1.f));

    for (float f=pmin.x; f<pmax.x+gridSpacing_.x*0.5; f+=gridSpacing_.x) {
        vec2 p = wtos(vec2(f, 0.0));
        vec2 a = wtos(vec2(0.0, 0.f));
        vec2 b = wtos(vec2(0.0, 1.f));
        paint.drawLine(QPointF(p.x, a.y),
                       QPointF(p.x, b.y));
    }

    for (float f=pmin.y; f<pmax.y+gridSpacing_.y*0.5; f+=gridSpacing_.y) {
        vec2 p = wtos(vec2(0.0, f));
        vec2 a = wtos(vec2(0.f, 0.0));
        vec2 b = wtos(vec2(1.f, 0.0));
        paint.drawLine(QPointF(a.x, p.y),
                       QPointF(b.x, p.y));
    }

    // ----------------------------------------------

    if (showHistogram_ && histogram_) {
        // draw histogram
        paint.setPen(Qt::NoPen);
        paint.setBrush(QColor(200, 0, 0, 120));
        //paint.setBrush(QColor(128, 0, 0, 64));
        paint.setRenderHint(QPainter::Antialiasing, true);

        int histogramWidth = histogram_->getBucketCount();

        QPointF *points = new QPointF[histogramWidth + 2];

        float max = 0.0;
        for (int i=0; i<histogramWidth; ++i)
            if (static_cast<float>(histogram_->getValue(i)) > max)
                max = static_cast<float>(histogram_->getValue(i));

        for (int x=0; x<histogramWidth; ++x) {
            float value = static_cast<float>(histogram_->getValue(x))/max;
            value = powf(value, 0.2f);
            vec2 p = wtos(vec2(static_cast<float>(x)/histogramWidth, value * (yRange_[1] - yRange_[0]) + yRange_[0]));
            points[x].rx() = p.x;
            points[x].ry() = p.y;
        }
        vec2 p;
        p = wtos(vec2(1.0, yRange_[0]));
        points[histogramWidth].rx() = p.x;
        points[histogramWidth].ry() = p.y;
        p = wtos(vec2(0.0, yRange_[0]));
        points[histogramWidth + 1].rx() = p.x;
        points[histogramWidth + 1].ry() = p.y;

        paint.drawPolygon(points, histogramWidth + 2);

        delete[] points;
    }

    // ----------------------------------------------

    // draw x and y axes
    paint.setRenderHint(QPainter::Antialiasing, true);
    paint.setPen(Qt::gray);
    paint.setBrush(Qt::gray);

    // draw axes independently from visible range
    float oldx0_ = xRange_[0];
    float oldx1_ = xRange_[1];
    xRange_[0] = 0.f;
    xRange_[1] = 1.f;

    vec2 origin = wtos(vec2(0.0f, 0.0f));
    origin.x = floor(origin.x) + 0.5f;
    origin.y = floor(origin.y) + 0.5f;

    paint.setRenderHint(QPainter::Antialiasing, true);

    paint.drawLine(QPointF(padding_, origin.y),
                   QPointF(width() - padding_, origin.y));

    paint.drawLine(QPointF(origin.x, padding_),
                   QPointF(origin.x, height() - padding_));

    QPointF arrow[3];
    arrow[0] = QPointF(origin.x, height() - padding_);
    arrow[1] = QPointF(origin.x + arrowWidth_, height() - padding_ - arrowLength_);
    arrow[2] = QPointF(origin.x - arrowWidth_, height() - padding_ - arrowLength_);

    paint.drawConvexPolygon(arrow, 3);

    arrow[0] = QPointF(width() - padding_, origin.y);
    arrow[1] = QPointF(width() - padding_ - arrowLength_, origin.y - arrowWidth_);
    arrow[2] = QPointF(width() - padding_ - arrowLength_, origin.y + arrowWidth_);

    paint.drawConvexPolygon(arrow, 3);

    paint.scale(-1, 1);
    paint.rotate(180);
    paint.drawText(static_cast<int>(width() - 6.2f * padding_), static_cast<int>(-1 * (origin.y - 0.8f * padding_)), xAxisText_);
    paint.drawText(static_cast<int>(1.6f * padding_), static_cast<int>(-1 * (height() - 1.85f * padding_)), yAxisText_);

    paint.rotate(180);
    paint.scale(-1, 1);

    xRange_[0] = oldx0_;
    xRange_[1] = oldx1_;

    // ----------------------------------------------

    // draw mapping function
    QPen pen = QPen(Qt::darkRed);
    pen.setWidthF(1.5);
    paint.setPen(pen);

    origin = wtos(vec2(0.f));

    if (tf_->isEmpty()) {
        vec2 a = wtos(vec2(0.f, 0.f));
        vec2 b = wtos(vec2(1.f, 0.f));

        paint.drawLine(QPointF(a.x, a.y), QPointF(b.x, b.y));

    }
    else {
        vec2 old;
        for (int i=0; i<tf_->getNumKeys(); ++i) {
            TransFuncMappingKey *key = tf_->getKey(i);
            vec2 p = wtos(vec2(key->getIntensity(), key->getColorL().a / 255.0));
            if (i == 0)  {
                if (tf_->getKey(0)->getIntensity() > 0.f)
                    paint.drawLine(QPointF(wtos(vec2(0.f, 0.0)).x, p.y),
                                   QPointF(p.x - 1.0, p.y));
            }
            else {
                paint.drawLine(QPointF(old.x + 1.0, old.y),
                               QPointF(p.x - 1.0, p.y));
            }
            old = p;
            if (key->isSplit())
                old = wtos(vec2(key->getIntensity(), key->getColorR().a / 255.0));
        }
        if ( tf_->getKey(tf_->getNumKeys()-1)->getIntensity() < 1.f ) {
            paint.drawLine(QPointF(old.x + 1.0, old.y),
                           QPointF(wtos(vec2(1.f, 0.0)).x, old.y));
        }

        /*
        for (int x=0; x<width(); x++) {
            float p = stow(vec2(x, 0.0)).x;
            float y = tf_->getMappingForValue(p);
            vec2 d = wtos(vec2(p, y));
            paint.drawLine(QPointF(d.x, d.y),
                           QPointF(d.x+1.0, d.y));
        }
        */

        paintKeys(paint);
    }

    // ----------------------------------------------

    // show threshold function
    paint.setPen(Qt::lightGray);
    paint.setBrush(Qt::Dense4Pattern);
    int tw = static_cast<int>(wtos(vec2(1.f, 1.f)).x - wtos(vec2(0.f, 0.f)).x);
    int th = static_cast<int>(wtos(vec2(1.f, 1.f)).y - wtos(vec2(0.f, 0.f)).y);

    if (thresholdL_ > 0.0f) {
        paint.drawRect(static_cast<int>(origin.x), static_cast<int>(origin.y), static_cast<int>(thresholdL_ * tw + 1), th);
    }
    if (thresholdU_ < 1.0f) {
        paint.drawRect(static_cast<int>(origin.x + floor(thresholdU_ * tw)),
                       static_cast<int>(origin.y), static_cast<int>((1 - thresholdU_) * tw + 1), th);
    }

    paint.setRenderHint(QPainter::Antialiasing, false);

    paint.setPen(Qt::lightGray);
    paint.setBrush(Qt::NoBrush);
    paint.drawRect(0, 0, width() - 1, height() - 1);

    if (gradient_)
        gradient_->update();
}

void TransFuncMappingCanvas::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
	    msgReceiver_->postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, true));
        isChanged_ = false;
    }

    event->accept();
    tgt::vec2 sHit = tgt::vec2(event->x(), static_cast<float>(height()) - event->y());
    tgt::vec2 hit = stow(sHit);

    dragLine_ = hitLine(vec2(event->x(), event->y()));
    if (dragLine_ >= 0 && event->modifiers() == Qt::ShiftModifier) {
        dragLineStartY_ = event->y();
        return;
    }

    // see if a key was selected
    selectedKey_ = 0;
    for (int i=0; i<tf_->getNumKeys(); ++i) {
        TransFuncMappingKey *key = tf_->getKey(i);
        tgt::vec2 sp = wtos(tgt::vec2(key->getIntensity(), key->getColorL().a / 255.0));
        tgt::vec2 spr = wtos(tgt::vec2(key->getIntensity(), key->getColorR().a / 255.0));
        if (key->isSplit()) {
            if (sHit.x > sp.x - splitFactor_ * pointSize_ && sHit.x <= sp.x &&
                sHit.y > sp.y - pointSize_ && sHit.y < sp.y + pointSize_)
            {
                selectedKey_ = key;
                selectedLeftPart_ = true;
            }
            if (sHit.x >= spr.x && sHit.x < spr.x + splitFactor_ * pointSize_ &&
                sHit.y > spr.y - pointSize_ && sHit.y < spr.y + pointSize_)
            {
                selectedKey_ = key;
                selectedLeftPart_ = false;
            }
        }
        else {
            if (sHit.x > sp.x - pointSize_ && sHit.x < sp.x + pointSize_ &&
                sHit.y > sp.y - pointSize_ && sHit.y < sp.y + pointSize_)
            {
                selectedKey_ = key;
                selectedLeftPart_ = false;
            }
        }
    }


    if (selectedKey_ == 0 && event->button() == Qt::RightButton) {
        handleContextMenu(event);
        return;
    }

    if (selectedKey_ != 0) {

        if (event->button() == Qt::LeftButton) {
            dragging_ = true;
            if (selectedKey_->isSplit() && !selectedLeftPart_)
                emit colorChanged( Col2QColor(selectedKey_->getColorR()) );
            else
                emit colorChanged( Col2QColor(selectedKey_->getColorL()) );
        }
        else if (event->button() == Qt::RightButton) {
            handleRightClick(event);
            return;
        }
        /*
        //FIXME: the following lines slow down the widget usage drastically
       if (selectedKey_->isSplit()) {
            if (selectedLeftPart_) {
                emit updateCoordinates(selectedKey_->getIntensity(),
                                       selectedKey_->getColorL().a / 255.0);
                emit exclusiveModeChangedWithDataSource(true, selectedKey_->
                                                        getIntensity() - 0.001);
            }
            else {
                emit updateCoordinates(selectedKey_->getIntensity(),
                                       selectedKey_->getColorR().a / 255.0);
                emit exclusiveModeChangedWithDataSource(true, selectedKey_->
                                                        getIntensity() + 0.001);
            }
        }
        else {
            emit updateCoordinates(selectedKey_->getIntensity(),
                                   selectedKey_->getColorL().a / 255.0);
            emit exclusiveModeChangedWithDataSource(true, selectedKey_->
                                                    getIntensity());
        }
        repaint();
        emit changed();
        */
        return;
    }

    // insert new key
    if (hit.x >= 0.f && hit.x <= 1.f && hit.y >= 0.f && hit.y <= 1.f &&
        event->button() == Qt::LeftButton)
    {
        insertNewKey(hit);
        dragging_ = true;
        dragLine_ = -1;
    }

    update();
}

void TransFuncMappingCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        dragging_ = false;
        dragLine_ = -1;
    }
    event->accept();
    emit clearCoordinates();
    emit exclusiveModeChangedWithDataSource(false, 0.0);

	if (event->button() == Qt::LeftButton) {
		msgReceiver_->postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false));
        if (isChanged_)
            msgReceiver_->postMessage(new Message(VoreenPainter::repaint_), VoreenPainter::visibleViews_);
        isChanged_ = false;
        repaint();
	}
}

void TransFuncMappingCanvas::mouseMoveEvent(QMouseEvent* event) {
    using tgt::vec2;

    event->accept();
    mousePos_ = event->pos();

    vec2 sHit = vec2(event->x(), static_cast<float>(height()) - event->y());
    vec2 hit = stow(sHit);
    if (gridSnap_)
        hit = snapToGrid(hit);

    if (!dragging_ && hitLine(vec2(event->x(), event->y())) >= 0 && event->modifiers() == Qt::ShiftModifier)
        setCursor(Qt::SizeVerCursor);
    else
        unsetCursor();

    if (dragLine_ >= 0) {
        float delta = dragLineStartY_ - event->y();
        for (int i=0; i < tf_->getNumKeys(); ++i) {
            TransFuncMappingKey *key = tf_->getKey(i);
            if (i == dragLine_) {
                float f = wtos(vec2(key->isSplit() ? key->getAlphaR() : key->getAlphaL())).y;
                f += delta;
                f = stow(vec2(f)).y;
                if (f < 0.f)
                    f = 0.f;
                if (f > 1.f)
                    f = 1.f;
                key->setAlphaR(f);

                tf_->updateKey(key);
            }
            else if (i == dragLine_ + 1) {
                float f = wtos(vec2(key->getAlphaL())).y;
                f += delta;
                f = stow(vec2(f)).y;
                if (f < 0.f)
                    f = 0.f;
                if (f > 1.f)
                    f = 1.f;
                key->setAlphaL(f);
                tf_->updateKey(key);
            }
        }

        repaint();
        emit changed();

        dragLineStartY_ = event->y();
        return;
    }

    if (!dragging_) return;

    // keep location within valid texture coord range
    if (hit.x < 0.f)
        hit.x = 0.f;
    if (hit.x > 1.f)
        hit.x = 1.f;
    if (hit.y < 0.f)
        hit.y = 0.f;
    if (hit.y > 1.f)
        hit.y = 1.f;

    if (selectedKey_ != 0) {
        if (rampMode_) {
            tgtAssert(tf_->getNumKeys() == 2, "more than two keys in ramp mode");
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
                    minX = rightKey->getIntensity() - rightKey->getColorL().a / 255.0;
                    hit.x = std::max(hit.x, minX);
                }
                // moving left upwards only allowed if at left border (ramp function)
                if (hit.x != 0.f) {
                    hit.y = 0.f;
                }

            } else {
                // obey ramp function restrictions:
                // right key has to stay right of right key
                hit.x = std::max<float>(hit.x, tf_->getKey(0)->getIntensity());
                // max width = 1.f, max center = 1.f
                float maxX = leftKey->getIntensity() + 1.f;
                float minY = std::max(2.f - maxX, 0.5f);
                hit.y = std::max(hit.y, minY);
                if (leftKey->getIntensity() == 0.f) {
                    float maxX = 1.f - leftKey->getColorL().a / 255.0;
                    hit.x = std::min(hit.x, maxX);
                }
                // moving right downwards only allowed if at right border (ramp function)
                if (hit.x != 1.f) {
                    hit.y = 1.f;
                }
            }
            selectedKey_->setIntensity(hit.x);
            selectedKey_->setAlphaL(hit.y);
            calcRampParamsFromKeys();
        }
        else {
            if (event->modifiers() != Qt::ShiftModifier)
	            selectedKey_->setIntensity(hit.x);
		    if (event->modifiers() != Qt::ControlModifier) {
			    if (selectedKey_->isSplit()) {
				    if (selectedLeftPart_)
                        selectedKey_->setAlphaL(hit.y);
				    else
                        selectedKey_->setAlphaR(hit.y);
			    }
                else
				    selectedKey_->setAlphaL(hit.y);
		    }
        }
        tf_->updateKey(selectedKey_);
        repaint();
        emit updateCoordinates(hit.x, hit.y);
        if (selectedKey_->isSplit()) {
            if (selectedLeftPart_) {
                emit exclusiveModeChangedWithDataSource(true, hit.x - 0.001);
            }
            else {
                emit exclusiveModeChangedWithDataSource(true, hit.x + 0.001);
            }
        }
        else {
            emit exclusiveModeChangedWithDataSource(true, hit.x);
        }
        emit changed();
    }
}

void TransFuncMappingCanvas::mouseDoubleClickEvent(QMouseEvent *event) {
    event->accept();
    changeCurrentColor();
}

void TransFuncMappingCanvas::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Shift && underMouse() && !dragging_ && hitLine(vec2(mousePos_.x(), mousePos_.y())) >= 0)
        setCursor(Qt::SizeVerCursor);
}

void TransFuncMappingCanvas::keyReleaseEvent(QKeyEvent* /*event*/) {
    unsetCursor();
}

void TransFuncMappingCanvas::paintKeys(QPainter& paint) {
    for (int i=0; i<tf_->getNumKeys(); ++i) {
        TransFuncMappingKey *key = tf_->getKey(i);
        tgt::vec2 p = wtos(tgt::vec2(key->getIntensity(), key->getColorL().a / 255.0));
        int props;
        if (key->isSplit()) {
            props = MARKER_LEFT;
            if (key == selectedKey_ && selectedLeftPart_)
                props |= MARKER_SELECTED;

            drawMarker(paint, key->getColorL(), p, props);

            p = wtos(tgt::vec2(key->getIntensity(), key->getColorR().a / 255.0));
            props = MARKER_RIGHT;
            if (key == selectedKey_ && !selectedLeftPart_)
                props |= MARKER_SELECTED;

            drawMarker(paint, key->getColorR(), p, props);
        }
        else {
            props = MARKER_NORMAL;
            if (key == selectedKey_)
                props |= MARKER_SELECTED;
            drawMarker(paint, key->getColorL(), p, props);
        }
    }
//     gradient_->repaint();
}

void TransFuncMappingCanvas::insertNewKey(tgt::vec2& hit) {
    // don't insert new keys in ramp mode
    if (rampMode_)
        return;

    if (hit.x < 0.f)
        hit.x = 0.f;
    if (hit.y < 0.f)
        hit.y = 0.f;
    if (hit.x > 1.f)
        hit.x = 1.f;
    if (hit.y > 1.f)
        hit.y = 1.f;

    if (gridSnap_)
        hit = snapToGrid(hit);

    TransFuncMappingKey *key = new TransFuncMappingKey(hit.x, QColor2Col(Qt::lightGray));

//     key->setIntensity(hit.x);
//     key->getColorL().a = (hit.y);
//     key->setSplit(false);
    key->setAlphaL(hit.y);
    tf_->addKey(key);
    TransFuncMappingKey* leftKey = getOtherKey(key, true);
    TransFuncMappingKey* rightKey = getOtherKey(key, false);

    // interpolate color of inserted key from neighbouring keys
    // (weighted by distance)
    tgtAssert( tf_->getNumKeys() >= 2, "Less than two transfer mapping keys!");
    tgt::col4 keyColor;
    if (!leftKey)
        keyColor = rightKey->getColorL();
    else if (!rightKey)
        keyColor = leftKey->getColorR();
    else {
        float leftSource = leftKey->getIntensity();
        float rightSource = rightKey->getIntensity();
        float distSource = rightSource - leftSource;
        tgt::vec4 leftColor = static_cast<tgt::vec4>(leftKey->getColorR());
        tgt::vec4 rightColor = static_cast<tgt::vec4>(rightKey->getColorL());

        keyColor = static_cast<tgt::col4>(
            leftColor* ( (distSource-(hit.x-leftSource))/distSource  ) +
            rightColor*( (distSource-(rightSource-hit.x))/distSource ) );
    }
    key->setColorL( keyColor );

    selectedKey_ = key;

    repaint();
    emit updateCoordinates(hit.x, hit.y);
    emit exclusiveModeChangedWithDataSource(true, hit.x);
    emit colorChanged( Col2QColor(keyColor) );
    emit changed();
}

void TransFuncMappingCanvas::handleRightClick(QMouseEvent* event) {
    Q_ASSERT(selectedKey_ != 0);

    //
    // Set context-dependent text for menu items
    //

    // no context menu in ramp mode
    if (rampMode_)
        return;

    // Split/merge
    splitMergeAction_->setText((selectedKey_->isSplit() ? tr("Merge this key") : tr("Split this key")));

    // Zero/unzero
    QString zeroText;
    zeroText = tr("Zero ");

    if (selectedLeftPart_)
        zeroText += tr("to the left");
    else
        zeroText += tr("to the right");

    zeroAction_->setText(zeroText);

    // allow deletion of keys only if there are more than two keys
    deleteAction_->setEnabled( tf_->getNumKeys() > 2);

    contextMenu_.popup(event->globalPos());
}

void TransFuncMappingCanvas::handleContextMenu(QMouseEvent *event) {
    fileMenu_.popup(event->globalPos());
}

void TransFuncMappingCanvas::dataSourceChanged(Volume* newDataset) {
    curDataset_ = newDataset;
    showHistogram_ = false;
    if (histogram_)
        delete histogram_;
    histogram_ = 0;
    //toggleShowHistogram(true);
    repaint();
}

void TransFuncMappingCanvas::setThreshold(float l, float u) {
    thresholdL_=l;
    thresholdU_=u;
    if (thresholdU_-thresholdL_ < minThresholdGap_) {
        if (thresholdL_ < 1.f-minThresholdGap_)
            thresholdU_ = thresholdL_ + minThresholdGap_;
        else
            thresholdL_ = thresholdU_ - minThresholdGap_;

    }

    if (clipThresholds_)
        xRange_ = vec2(thresholdL_, thresholdU_);

    update();
}

void TransFuncMappingCanvas::setRampMode(bool on) {
    if ( on && rampMode_ == false) {
        setStandardFunc();
        setRampParams(0.5f, 1.f);
        update();
    }
    rampMode_ = on;
}

void TransFuncMappingCanvas::setRampParams(float rampCenter, float rampWidth) {

    tgtAssert(rampCenter >= 0.f && rampCenter <= 1.f, "ramp center out of range [0;1]");
    tgtAssert(rampWidth >= 0.f && rampWidth <= 1.f, "ramp width out of range [0;1]");

    rampCenter_ = rampCenter;
    rampWidth_ = rampWidth;

    if (rampMode_) {
        calcKeysFromRampParams();
        repaint();
        emit changed();
    }
}

void TransFuncMappingCanvas::getRampParams(float &rampCenter, float &rampWidth) {
    rampCenter = rampCenter_;
    rampWidth = rampWidth_;
}

void TransFuncMappingCanvas::readFromDisc(QString fileName) {
    clean();
    QFile file(fileName);
    bool res = file.open(QIODevice::ReadOnly);
    if (res) {
        QDataStream in(&file);
        qint32 size;
        in >> size;
        LDEBUG("loadingsize("<< sizeof(size)<<"): " << size);
        for (int i=0;i<size;i++) {
            qint32 inten;
            qint32 trans;
            float intensity;
            float transparency;
            QColor color;
            bool split;
            in >> inten;
            intensity = static_cast<float>(inten) / 255.f;
            in >> trans;
            transparency = static_cast<float>(trans) / 255.f;
            in >> color;
            in >> split;
            TransFuncMappingKey* key = new TransFuncMappingKey(intensity, QColor2Col(color) );
            key->setAlphaL(transparency);
            key->setSplit(split);
            if (split) {
                qint32 spTrans;
                float splitTransparency;
                QColor altColor;
                in >> altColor;
                in >> spTrans;
                splitTransparency = static_cast<float>(spTrans) / 255.f;
                key->setColorR( QColor2Col(altColor) );
                key->setAlphaR(splitTransparency);
            }
            tf_->addKey(key);
        }
        file.close();
    }
    else {
        std::cerr << "Could not open file, using standard transfer-function..." << std::endl;
        setStandardFunc();
    }
    tf_->updateTexture();
    emit changed();
}

void TransFuncMappingCanvas::saveToDisc(QString fileName) {
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);

	out << (qint32)tf_->getNumKeys();
    for (int i=0; i<tf_->getNumKeys(); ++i) {
        out << static_cast<qint32>(tf_->getKey(i)->getIntensity() * 255.f);
		out << static_cast<qint32>(tf_->getKey(i)->getAlphaL() * 255.f);
        out << Col2QColor(tf_->getKey(i)->getColorL());
        out << tf_->getKey(i)->isSplit();
        if (tf_->getKey(i)->isSplit()) {
            out << Col2QColor(tf_->getKey(i)->getColorR());
            out << (qint32)(tf_->getKey(i)->getAlphaR() * 255.f);
        }
    }
    file.close();
}

void TransFuncMappingCanvas::clean() {
    tf_->clearKeys();
    selectedKey_ = 0;
    update();
}

QSize TransFuncMappingCanvas::minimumSizeHint () const {
    return QSize(50,20);
}

QSize TransFuncMappingCanvas::sizeHint () const {
    return QSize(200, 100);
}

QSizePolicy TransFuncMappingCanvas::sizePolicy () const {
    return QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void TransFuncMappingCanvas::setTransFunc(TransFuncIntensity* tf) {
    tf_ = tf;
    selectedKey_ = 0;
    repaint();
}

void TransFuncMappingCanvas::setXAxisText(const std::string& text) {
    xAxisText_ = QString(text.c_str());
}

void TransFuncMappingCanvas::setYAxisText(const std::string& text) {
    yAxisText_ = QString(text.c_str());
}

void TransFuncMappingCanvas::splitMergeKeys() {
    if (!selectedKey_)
        return;

    selectedKey_->setSplit(!selectedKey_->isSplit());
    emit changed();
    repaint();
}

void TransFuncMappingCanvas::unselectKey() {

    selectedKey_ = 0;
    repaint();
};

void TransFuncMappingCanvas::deleteKey() {
    if (!selectedKey_)
        return;

    tf_->removeKey(selectedKey_);
    selectedKey_ = 0;

    repaint();
    emit changed();
}

void TransFuncMappingCanvas::changeCurrentColor(const QColor& c) {
    tgt::col4 tgtcolor = QColor2Col(c);

    if (!selectedKey_ || rampMode_)
        return;
    if (c.isValid()) {
        bool change = true;
        if (selectedKey_->isSplit() && !selectedLeftPart_) {
            tgtcolor.a = selectedKey_->getColorR().a;
            if (selectedKey_->getColorR() != tgtcolor)
                selectedKey_->setColorR(tgtcolor);
            else
                change = false;
        }
        else {
            tgtcolor.a = selectedKey_->getColorL().a;
            if (selectedKey_->getColorL() != tgtcolor)
                selectedKey_->setColorL(tgtcolor);
            else
                change = false;
        }

        if (change) {
            emit changed();
            emit colorChanged(c);
            repaint();
        }
    }
}

void TransFuncMappingCanvas::changeCurrentColor() {
    if (!selectedKey_ || rampMode_ || noColor_)
        return;

    msgReceiver_->postMessage(new BoolMsg(VoreenPainter::switchCoarseness_, false));
    QColor oldColor;
    if (selectedKey_->isSplit() && !selectedLeftPart_)
        oldColor = Col2QColor( selectedKey_->getColorR() );
    else 
        oldColor = Col2QColor( selectedKey_->getColorL() );
    
    QColor newColor = QColorDialog::getColor(oldColor, 0);
    if (newColor.isValid())
        changeCurrentColor(newColor);
}

void TransFuncMappingCanvas::zeroKey() {
    if (!selectedKey_)
        return;

    TransFuncMappingKey* otherKey = getOtherKey(selectedKey_, selectedLeftPart_);
    if (otherKey) {
        if (!otherKey->isSplit())
            otherKey->setSplit(true);
        if (selectedLeftPart_)
            otherKey->setAlphaR(0.0);
        else
            otherKey->setAlphaL(0.0);
    }

    if (!selectedKey_->isSplit())
        selectedKey_->setSplit(true);

    if (selectedLeftPart_)
        selectedKey_->setAlphaL(0.0);
    else
        selectedKey_->setAlphaR(0.0);

    emit changed();
    repaint();
}

void TransFuncMappingCanvas::gradAlphaKey() {
    if (!selectedKey_)
        return;

    TransFuncMappingKey* otherKey = getOtherKey(selectedKey_, selectedLeftPart_);
    if (otherKey) {
        if (!otherKey->isSplit())
            otherKey->setSplit(true);
    }

    if (!selectedKey_->isSplit())
        selectedKey_->setSplit(true);

    emit changed();
    repaint();
}

void TransFuncMappingCanvas::readFromDisc() {
    QFileDialog fileDialog(this,
        tr("Choose a transfer function to open"),
        transferFuncPath_,
        #ifdef VRN_WITH_DEVIL
            tr("Transfer Function (*.tf *.tfi *.lut *.table *.plist *.bmp *.png *.jpg *.gif *.tga *.tif)"));
        #else
            tr("Transfer Function (*.tf *.tfi *.lut *.table *.plist)"));
        #endif

    if ( fileDialog.exec() && !fileDialog.selectedFiles().empty() ) {

        QString fileName = fileDialog.selectedFiles().at(0);

        if (!fileName.isEmpty()) {
            if (fileName.endsWith(".tf", Qt::CaseInsensitive))
                readFromDisc(fileName);
            else
                tf_->load(fileName.toStdString());

            // update dialog path
            transferFuncPath_ = fileDialog.directory().path();

            repaint();
            emit changed();
        }
    }
}

void TransFuncMappingCanvas::saveToDisc() {
    QFileDialog fileDialog(this);
    fileDialog.setDefaultSuffix(tr("tfi"));
    fileDialog.setWindowTitle(tr("Choose a filename to save transfer function"));
    fileDialog.setDirectory(transferFuncPath_);
    QStringList filters;
    filters << tr("Transfer function (*.tfi)")
            << tr("Transfer function image (*.png)");
    fileDialog.setFilters(filters);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList fileList;

    if (fileDialog.exec()) {
        fileList = fileDialog.selectedFiles();
        if (fileDialog.selectedFilter() == filters[0])
            tf_->save(fileList.at(0).toStdString());
        else
            tf_->savePNG(fileList.at(0).toStdString(), 512);
        
        // update dialog path
        transferFuncPath_ = fileDialog.directory().path();
    }
}

void TransFuncMappingCanvas::setStandardFunc() {
    tf_->createStdFunc();
    selectedKey_ = 0;
    rampCenter_ = 0.5f;
    rampWidth_ = 1.f;
    emit changed();
    repaint();
}

void TransFuncMappingCanvas::toggleClipThresholds(bool enabled) {
    clipThresholds_ = enabled;
    if (clipThresholds_)
        xRange_ = vec2(thresholdL_, thresholdU_);
    else
        xRange_ = vec2(0.f, 1.f);

    update();
}

void TransFuncMappingCanvas::toggleGridSnap(bool enabled) {
    gridSnap_ = enabled;
}

void TransFuncMappingCanvas::toggleShowHistogram(bool enabled) {
    if (!curDataset_)
        return;

    int histogramWidth = 256;
    
    int bits = curDataset_->getBitsStored();
    switch (bits) {
    case 8:
        histogramWidth = 256;
        break;
    case 12:
        histogramWidth = 512;
        break;
    case 16:
        histogramWidth = 512;
        break;
    case 32:
        histogramWidth = 256;
        break;
    }

    if (enabled && !showHistogram_) {
        if (histogram_)
            showHistogram_ = true;
        else if (!histogram_ && curDataset_) {
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            histogram_ = new HistogramIntensity(curDataset_, histogramWidth);
            QApplication::restoreOverrideCursor();
            showHistogram_ = enabled;
        }
        else
            showHistogram_ = false;
    }
    else
        showHistogram_ = enabled;
    repaint();
}

void TransFuncMappingCanvas::onChanged() {
    isChanged_ = true;
}

TransFuncMappingKey* TransFuncMappingCanvas::getOtherKey(TransFuncMappingKey* selectedKey, bool selectedLeftPart) {
    TransFuncMappingKey* otherKey = 0;
    for (int i=0; i < tf_->getNumKeys(); ++i) {
        if ((selectedLeftPart && i < tf_->getNumKeys() - 1 && tf_->getKey(i + 1) == selectedKey) ||
            (!selectedLeftPart && i > 0 && tf_->getKey(i - 1) == selectedKey))
        {
            otherKey = tf_->getKey(i);
        }
    }
    return otherKey;
}

void TransFuncMappingCanvas::calcKeysFromRampParams() {
    tgtAssert(tf_->getNumKeys() == 2, "more than two keys in ramp mode");
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
    tf_->updateKey(key1);
    calcRampParamsFromKeys();
}

void TransFuncMappingCanvas::calcRampParamsFromKeys() {
    tgtAssert(tf_->getNumKeys() == 2, "more than two keys in ramp mode");
    TransFuncMappingKey* key0 = tf_->getKey(0);
    TransFuncMappingKey* key1 = tf_->getKey(1);
    float left = key0->getIntensity();
    float right = key1->getIntensity();
    if (left == 0.f) {
        left = - key0->getColorL().a / 255.0 * (key1->getIntensity() - key0->getIntensity()) /
            (key1->getColorL().a / 255.0 - key0->getColorL().a / 255.0);
    }
    if (right == 1.f) {
        right = 1.f + (1.f - key1->getColorL().a / 255.0) * (key1->getIntensity() - key0->getIntensity()) /
            (key1->getColorL().a / 255.0 - key0->getColorL().a / 255.0);
    }
    rampCenter_ = (right + left) / 2.f;
    rampWidth_ = right - left;
}

int TransFuncMappingCanvas::hitLine(const tgt::vec2& p) {
    int hit = -1;
    vec2 sHit = vec2(p.x, static_cast<float>(height()) - p.y);
    vec2 old;
    for (int i=0; i < tf_->getNumKeys(); ++i) {
        TransFuncMappingKey *key = tf_->getKey(i);
        vec2 p = wtos(vec2(key->getIntensity(), key->getColorL().a / 255.0));
//         if (i == 0) {
//             paint.drawLine(QPointF(wtos(vec2(x0_, 0.0)).x, p.y),
//                            QPointF(p.x - 1.0, p.y));
//         } else {
//             paint.drawLine(QPointF(old.x + 1.0, old.y),
//                            QPointF(p.x - 1.0, p.y));
//         }
        if (i > 0) {
            vec2 p1 = vec2(old.x + 1.0, old.y);
            vec2 p2 = vec2(p.x - 1.0, p.y);
            float s = (p2.y - p1.y) / (p2.x - p1.x);
            float a = p1.y + (sHit.x - p1.x) * s;
            if (sHit.x >= p1.x+10 && sHit.x <= p2.x-10 && abs(static_cast<int>(sHit.y) - static_cast<int>(a)) < 5) {
                hit = i - 1;
            }
        }

        old = p;
        if (key->isSplit())
            old = wtos(vec2(key->getIntensity(), key->getColorR().a / 255.0));
    }
//         paint.drawLine(QPointF(old.x + 1.0, old.y),
//                        QPointF(wtos(vec2(x1_, 0.0)).x, old.y));
    return hit;
}

tgt::vec2 TransFuncMappingCanvas::wtos(tgt::vec2 p) {
    float sx, sy;
    sx = (p.x - xRange_[0]) / (xRange_[1] - xRange_[0]) * (static_cast<float>(width()) - 2 * padding_ - 1.5 * arrowLength_) + padding_;
    sy = (p.y - yRange_[0]) / (yRange_[1] - yRange_[0]) * (static_cast<float>(height()) - 2 * padding_ - 1.5 * arrowLength_) + padding_;
    return tgt::vec2(sx, sy);
}

tgt::vec2 TransFuncMappingCanvas::stow(tgt::vec2 p) {
    float wx, wy;
    wx = (p.x - padding_) / (static_cast<float>(width()) - 2 * padding_ - 1.5 * arrowLength_) * (xRange_[1] - xRange_[0]) + xRange_[0];
    wy = (p.y - padding_) / (static_cast<float>(height()) - 2 * padding_ - 1.5 * arrowLength_) * (yRange_[1] - yRange_[0]) + yRange_[0];
    return tgt::vec2(wx, wy);
}

tgt::vec2 TransFuncMappingCanvas::snapToGrid(tgt::vec2 p) {
    tgt::vec2 n;
    n.x = tgt::round(p.x / gridSpacing_.x) * gridSpacing_.x;
    n.y = tgt::round(p.y / gridSpacing_.y) * gridSpacing_.y;
    return n;
}

void TransFuncMappingCanvas::resizeEvent(QResizeEvent* /*event*/) {
    gridSpacing_ = vec2(1.0, 1.0);
    // refine gridSpacing_ as good as possible
    float factor[2] = {0.1f, 0.2f};
    for (int k=0; k<2; ++k) {
        for (int component=0; component<2; ++component) {
            vec2 cellSize = wtos(gridSpacing_) - wtos(vec2(0.0, 0.0));
            cellSize[component] *= factor[k];
            while (cellSize[component] > minCellSize_) {
                gridSpacing_[component] *= factor[k];
                cellSize[component] *= factor[k];
            }
            cellSize[component] /= factor[k];
        }
    }
}

void TransFuncMappingCanvas::drawMarker(QPainter& paint, const tgt::col4& tgtcolor, const tgt::vec2& p, int props) {
    QColor color = Col2QColor(tgtcolor);

    if (props & MARKER_ALPHA)
        paint.setBrush(QBrush(Qt::red, Qt::Dense4Pattern));
    else if (noColor_)
        paint.setBrush(Qt::transparent);
    else
        paint.setBrush(color);

    QPen pen(QBrush((props & MARKER_ZEROED) ? Qt::black : Qt::darkGray), Qt::SolidLine);
    if (props & MARKER_SELECTED)
        pen.setWidth(3);
    paint.setPen(pen);

    if (props & MARKER_LEFT) {
        paint.drawPie(QRectF(p.x - splitFactor_ * pointSize_/2, p.y - pointSize_/2,
                             splitFactor_ * pointSize_, pointSize_),
                      90 * 16, 180 * 16);
    }
    else if (props & MARKER_RIGHT) {
        paint.drawPie(QRectF(p.x - splitFactor_ * pointSize_/2, p.y - pointSize_/2,
                             splitFactor_ * pointSize_, pointSize_),
                      270 * 16, 180 * 16);
    }
    else {
        paint.drawEllipse(QRectF(p.x - pointSize_/2, p.y - pointSize_/2,
                                 pointSize_, pointSize_));
    }
}

} // namespace voreen
