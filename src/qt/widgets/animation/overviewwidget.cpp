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

#include "voreen/qt/widgets/animation/animationeditor.h"
#include "voreen/qt/widgets/animation/animationexportwidget.h"
#include "voreen/qt/widgets/animation/currentframegraphicsitem.h"
#include "voreen/qt/widgets/animation/overviewwidget.h"
#include "voreen/core/ports/renderport.h"

#include <QContextMenuEvent>
#include <QDialog>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QImage>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <cmath>
#include <iostream>
#include <sstream>

namespace voreen {

#define PREVIEW_SIZE 40

OverviewView::OverviewView(QGraphicsScene* qgs, QWidget* parent)
    : QGraphicsView(qgs, parent)
    , barMovement_(false)
    , scene_(qgs)
    , slide_(false)
    , offset_(0)
{
    currentFrameGraphicsItem_ = new CurrentFrameGraphicsItem(true, true);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setCacheMode(CacheBackground);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setAlignment(Qt::AlignLeft);
    scene_->setBackgroundBrush(Qt::blue);
    QLinearGradient gradient(0, 0, 0, 80);
    gradient.setSpread(QGradient::PadSpread);
    gradient.setColorAt(0, QColor(255,255,255,0));
    gradient.setColorAt(0.5, QColor(150,150,150,200));
    gradient.setColorAt(1, QColor(50,0,50,100));
    scene_->setBackgroundBrush(gradient);
    scene_->addItem(currentFrameGraphicsItem_);
}

void OverviewView::setBar(QGraphicsRectItem* bar) {
    highlightBar_ = bar;
}

void OverviewView::contextMenuEvent(QContextMenuEvent* e) {
    emit contextMenuRequest(e->pos());
}

void OverviewView::setCurrentFrame(int frame) {
    currentFrame_ = frame;
    currentFrameGraphicsItem_->setPos(frame, 20);
}

void OverviewView::mousePressEvent(QMouseEvent* e) {
    if (scene_->itemAt(mapToScene(e->pos()).x(), mapToScene(e->pos()).y())
        && scene_->itemAt(mapToScene(e->pos()).x(), mapToScene(e->pos()).y())->boundingRect() == highlightBar_->boundingRect()) {
        barMovement_ = true;
        relativeBarPosition_ = e->pos().x();
    }
    else if (e->button() == Qt::LeftButton) {
        QScrollBar* scroll = horizontalScrollBar();
        if (e->x()+ scroll->value() >= 0 && e->x()+ scroll->value() <= AnimationEditor::getDuration()) {
            currentFrameGraphicsItem_->setPos(e->x() + scroll->value(), 20);
            slide_ = true;
            emit currentFrameChanged(e->x() + scroll->value());
        }
    }
    QGraphicsView::mousePressEvent(e);
}

void OverviewView::mouseMoveEvent(QMouseEvent* e) {
    QScrollBar* scroll = horizontalScrollBar();
    if (barMovement_) {
        emit barMovement(offset_ - 1 * (relativeBarPosition_ - e->pos().x()));
    }
    else if (slide_ && e->x()+ scroll->value() >= 0 && e->x() + scroll->value() <= AnimationEditor::getDuration()) {
        currentFrameGraphicsItem_->setPos(e->x() + scroll->value(), 20);
        emit currentFrameChanged(e->x() + scroll->value());
    }
    else if (slide_ && e->x()+ scroll->value() > AnimationEditor::getDuration()) {
        currentFrameGraphicsItem_->setPos(AnimationEditor::getDuration(), 20);
        emit currentFrameChanged(static_cast<int>(AnimationEditor::getDuration()));
    }
    else if (slide_ && e->x()+ scroll->value() < 0) {
        currentFrameGraphicsItem_->setPos(0, 20);
        emit currentFrameChanged(0);
    }
}

void OverviewView::mouseReleaseEvent(QMouseEvent* /*e*/) {
    slide_ = false;
    barMovement_ = false;
    offset_ = static_cast<int>(highlightBar_->boundingRect().x());
}

void OverviewView::offsetCorrection(int x) {
    if (!barMovement_)
        offset_ = x;
}

// ---------------------------------------------------------------------------

OverviewWidget::OverviewWidget(QWidget* parent, NetworkEvaluator* networkEval)
    : QWidget(parent)
    , duration_(1200)
    , zoom_(1)
    , viewportWidth_(500)
    , scrollBarPosition_(0)
    , canvasRenderer_(0)
    , networkEvaluator_(networkEval)
    , currentFrame_(0)
    , fps_(30)
    , autoPreview_(false)
{
    canvasRenderer_ = 0;
    previews_.clear();
    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    mainLayout->setMargin(1);
    mainLayout->setSpacing(1);
    setFixedHeight(70 + PREVIEW_SIZE);

    overviewScene_ = new QGraphicsScene(this);

    overviewView_ = new OverviewView(overviewScene_, this);

    highlightBar_ = new QGraphicsRectItem();
    overviewView_->setBar(highlightBar_);
    highlightBar_->setZValue(2);
    highlightBar_->setPen(QPen(Qt::DotLine));
    QLinearGradient gradient1(0,0,0, PREVIEW_SIZE);
    gradient1.setSpread(QGradient::ReflectSpread);
    gradient1.setColorAt(0.0, QColor(20, 100 ,100, 30));
    gradient1.setColorAt(1.0, QColor(80, 100 ,100, 30));
    QBrush brush(gradient1);
    highlightBar_->setBrush(gradient1);
    overviewScene_->addItem(highlightBar_);
    highlight_ = new QGraphicsRectItem();
    highlight_->setPen(QPen(Qt::DotLine));
    QLinearGradient gradient(0,0,0, PREVIEW_SIZE );
    gradient.setSpread(QGradient::ReflectSpread);
    gradient.setColorAt(0.0, QColor(120, 100 ,100, 30));
    gradient.setColorAt(1.0, QColor(100, 100 ,100, 30));
    highlight_->setBrush(gradient);
    overviewScene_->addItem(highlight_);

    overviewView_->setStyleSheet("background:transparent");
    connect(overviewView_, SIGNAL(currentFrameChanged(int)), this, SIGNAL(currentFrameChanged(int)));
    connect(overviewView_, SIGNAL(contextMenuRequest(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
    connect(overviewView_, SIGNAL(barMovement(int)), this, SIGNAL(barMovement(int)));
    connect(this, SIGNAL(offsetCorrection(int)), overviewView_, SLOT(offsetCorrection(int)));

    overviewView_->setGeometry(QRect(0, 0, duration_, 50));
    overviewView_->setSceneRect(QRect(0, 0, duration_, 50));
    overviewView_->setCurrentFrame(0);

    mainLayout->addWidget(overviewView_);
    QLabel* nameLabel = new QLabel(this);
    nameLabel->lower();
    nameLabel->move(8, 45);
    nameLabel->setText(QString::fromStdString("Overview"));

    if (canvasRenderer_) {
        const std::vector<Port*> port = canvasRenderer_->getInports();
        if (port.size() == 1) {
            if(dynamic_cast<RenderPort*>(port.at(0)) != 0)
                renderPort_ = dynamic_cast<RenderPort*>(port.at(0));
        }
    }
    setStandardRenderport();
}

int OverviewWidget::getCurrentFrame() {
    return currentFrame_;
}

void OverviewWidget::autoPreview(bool autoPreview) {
    autoPreview_ = autoPreview;
    renderPreviews();
}

void OverviewWidget::reset() {
    overviewView_->setCurrentFrame(0);
    setFps(30);
    scrollBarOrder(0);
}

void OverviewWidget::contextMenuRequest(QPoint pos) {
    QMenu* addFrameMenu = new QMenu(this);
    //addFrameMenu->setStyleSheet("background:white");
    QAction snapshotAction(tr("Take Snapshot"), this);
    QAction regeneratePreviews(tr("Regenerate Previews"), this);

    addFrameMenu->addAction(&snapshotAction);
    //addFrameMenu->addSeparator();
    addFrameMenu->addAction(&regeneratePreviews);

    QMenu* canvasRendererMenu = new QMenu("Canvasrenderer", this);
    const ProcessorNetwork* network = networkEvaluator_->getProcessorNetwork();
    const std::vector<CanvasRenderer*>& canvasRenderer = network->getProcessorsByType<CanvasRenderer>();
    std::map<QAction*, CanvasRenderer*> menuMap;

    for (size_t i = 0; i < canvasRenderer.size(); ++i) {
        QString canvasRendererName = QString::fromStdString(canvasRenderer.at(i)->getID());
        QAction* action = new QAction(canvasRendererName, canvasRendererMenu);
        canvasRendererMenu->addAction(action);
        menuMap[action] = canvasRenderer.at(i);

    }

    addFrameMenu->addMenu(canvasRendererMenu);
    QAction* action = addFrameMenu->exec(QCursor::pos());
    if (action == &snapshotAction) {
        recordAt(static_cast<int>(overviewView_->mapToScene(pos).x()));
        if(autoPreview_)
            renderPreviews();
    }
    else if (action == &regeneratePreviews) {
        updatePreviews();
    }
    else if (action != 0){
        canvasRenderer_ = menuMap[action];
        const std::vector<Port*> port = canvasRenderer_->getInports();
        if (port.size() == 1) {
             if(dynamic_cast<RenderPort*>(port.at(0)) != 0)
                 renderPort_ = dynamic_cast<RenderPort*>(port.at(0));
         }
        if(autoPreview_)
            renderPreviews();
    }
}

void OverviewWidget::setStandardRenderport() {
    const ProcessorNetwork* network = networkEvaluator_->getProcessorNetwork();
    if(network != 0) {
        const std::vector<CanvasRenderer*>& canvasRenderer = network->getProcessorsByType<CanvasRenderer>();
        if (!canvasRenderer.empty()) {
            canvasRenderer_ = *canvasRenderer.begin();
            const std::vector<Port*> port = canvasRenderer_->getInports();
            if (port.size() == 1) {
                if (dynamic_cast<RenderPort*>(port.at(0)) != 0)
                    renderPort_ = dynamic_cast<RenderPort*>(port.at(0));
            }
        }
        else
            renderPort_ = 0;
    }
}

void OverviewWidget::selectCanvasRenderer() {
   QMenu* menu = new QMenu(this);
   const ProcessorNetwork* network = networkEvaluator_->getProcessorNetwork();
   const std::vector<CanvasRenderer*>& canvasRenderer = network->getProcessorsByType<CanvasRenderer>();
   std::map<QAction*, CanvasRenderer*> menuMap;

   for (size_t i = 0; i < canvasRenderer.size(); ++i) {
       QString canvasRendererName = QString::fromStdString(canvasRenderer.at(i)->getID());
       QAction* action = new QAction(canvasRendererName, menu);
       menu->addAction(action);
       menuMap[action] = canvasRenderer.at(i);
   }
   QAction* result = menu->exec(QCursor::pos());
   if (result != 0) {
       canvasRenderer_ = menuMap[result];
       const std::vector<Port*> port = canvasRenderer_->getInports();
       if (port.size() == 1) {
            if (dynamic_cast<RenderPort*>(port.at(0)) != 0)
                renderPort_ = dynamic_cast<RenderPort*>(port.at(0));
        }
   }
   delete menu;
   if(autoPreview_)
       renderPreviews();
}

void OverviewWidget::sceneOrder(QMatrix matrix) {
    matrix_ = matrix;
    zoom_ = static_cast<int>(matrix.m11());
}

void OverviewWidget::scrollBarOrder(int scrollBarPosition) {
    if (zoom_ != 0) {
        scrollBarPosition_ = scrollBarPosition;
        highlight_->setRect(scrollBarPosition_/zoom_, -15, (viewportWidth_) / zoom_ , 85);
        highlightBar_->setRect(scrollBarPosition_/zoom_, -15, (viewportWidth_) / zoom_ , 15);
        emit offsetCorrection(static_cast<int>(highlightBar_->boundingRect().x()));
    }
}

void OverviewWidget::updatePreviews() {
    bool temp = autoPreview_;
    autoPreview_ = true;
    renderPreviews();
    autoPreview_ = temp;
}

void OverviewWidget::renderPreviews() {
    if(autoPreview_) {
        if (renderPort_) {
            int currentFrame = currentFrame_;
            if (!previews_.empty()) {
                for (size_t vs = 0; vs < previews_.size(); vs++) {
                    overviewScene_->removeItem(previews_.at(vs));
                }
            }
            previews_.clear();
            int quadSize = PREVIEW_SIZE;
            int stepLength = duration_ / quadSize;
            if (stepLength < quadSize)
                stepLength = quadSize;
            // save the current settings
            tgt::ivec2 size = renderPort_->getSize();
            renderPort_->requestSize(tgt::ivec2(quadSize,quadSize));
            QPixmap previewPixmap;
            QImage* preview = 0;
            for (int i = 0; i < duration_ - 1; i += stepLength) {
                preview = new QImage(QSize(quadSize, quadSize), QImage::Format_ARGB32);
                emit currentFrameChanged(i);
                networkEvaluator_->process();
                tgt::col4* buffer = renderPort_->readColorBuffer<uint8_t>(); // TODO: catch exceptions
                for (int x = quadSize-1; x >= 0; x-=1) {
                    for (int y = 0; y < quadSize; y+=1) {
                        QColor color(buffer->r, buffer->g, buffer->b, 255);
                        QRgb rgba = color.rgba();
                        preview->setPixel(y,x, rgba);
                        buffer++;
                    }
                }
                const QImage img = preview->copy(0,0, quadSize,quadSize);

                previewPixmap = QPixmap::fromImage(img);
                QGraphicsPixmapItem* pixmapItem = overviewScene_->addPixmap(previewPixmap);
                previews_.push_back(pixmapItem);
                pixmapItem->moveBy(i, 26);
                pixmapItem->setZValue(-1);
            }
            delete preview;
            renderPort_->requestSize(size);
            setCurrentFrame(currentFrame);
            emit currentFrameChanged(currentFrame);
        }
    }

}

void OverviewWidget::setCurrentFrame(int currentFrame) {
    currentFrame_ = currentFrame;
    overviewView_->setCurrentFrame(currentFrame);
}

void OverviewWidget::setFps(int fps) {
    QList<QGraphicsItem*> items = overviewScene_->items();
    for (int i = 0; i < items.size(); ++i) {
        if (!(dynamic_cast<CurrentFrameGraphicsItem*>(items.at(i)) || dynamic_cast<QGraphicsRectItem*>(items.at(i))))
            overviewScene_->removeItem(items.at(i));
    }
    fps_ = fps;
    int secs = 0;
    for (int x = 0; x < duration_ + fps; x+=fps) {
        if (x % (2*fps) == 0) {
            overviewScene_->addLine(x, 15, x, 35);
            QGraphicsTextItem* textItem = overviewScene_->addText(QString::fromStdString(getTimeString(secs)));
            textItem->moveBy(x-14, -5);

        }
        else if (x % fps == 0) {
            overviewScene_->addLine(x, 20, x, 30);
        }
        secs+=1;
    }
    overviewView_->setCurrentFrame(0);
    overviewScene_->addRect(0, 25, duration_, 1);
    overviewView_->setSceneRect(-2,-2, duration_, 60);


    highlight_->setRect(scrollBarPosition_/zoom_, -15, (viewportWidth_ - 60) / zoom_ , 85);
    highlight_->show();

    highlightBar_->setRect(scrollBarPosition_/zoom_, -15, (viewportWidth_ - 60) / zoom_ , 15);
    highlightBar_->show();

    previews_.clear();
}

void OverviewWidget::updateViewportRect(int viewportWidth) {
    viewportWidth_ = viewportWidth;
    highlight_->setRect(scrollBarPosition_/zoom_, -15, (viewportWidth_) / zoom_ , 80);
    highlightBar_->setRect(scrollBarPosition_/zoom_, -15, (viewportWidth_) / zoom_ , 15);
}

void OverviewWidget::setDuration(int duration) {
    duration_ = duration;
    setFps(fps_);
}

std::string OverviewWidget::getTimeString(int sec) {
    std::stringstream out;
    if (sec < 60) {
        out << "00:";
        if (sec < 10)
            out << "0" << sec;
        else
            out << sec;
    }
    else if (sec < 600) {
        int min = 0;
        while (sec >= 60) {
            sec -= 60;
            min += 1;
        }
        out << "0" << min;
        if (sec < 10)
            out << ":0" << sec;
        else
            out << ":" << sec;
    }
    else {
        int min = 0;
        while (sec > 60) {
            sec -= 60;
            min += 1;
        }
        if (sec < 10)
            out << min << ":0" << sec;
        else
            out << min << ":" << sec;
    }
    return out.str();
}

} //namespace voreen
