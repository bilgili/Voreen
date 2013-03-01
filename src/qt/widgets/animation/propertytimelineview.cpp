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
#include "voreen/qt/widgets/animation/currentframegraphicsitem.h"
#include "voreen/qt/widgets/animation/keyframegraphicsitem.h"
#include "voreen/qt/widgets/animation/propertytimelinewidget.h"

#include <QContextMenuEvent>
#include <QVBoxLayout>
#include <QGraphicsProxyWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMenu>
#include <QScrollBar>
#include <cmath>
#include <iostream>


namespace voreen {

float PropertyTimelineView::zoom_ = 1;
int PropertyTimelineView::currentFrame_ = 0;


PropertyTimelineView::PropertyTimelineView(QGraphicsScene* qgs, QWidget* parent)
    : QGraphicsView(qgs, parent)
    , scene_(qgs)
    , currentFrameChange_(false)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setAlignment(Qt::AlignLeft);
    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SIGNAL(scrollBarRequest(int)));
}

void PropertyTimelineView::contextMenuEvent(QContextMenuEvent* e) {
    QMenu* addFrameMenu = new QMenu(this);
    addFrameMenu->setStyleSheet("background:white");

    QAction addFrameAction(tr("Add key frame"), this);
    QAction snapshotAction(tr("Take snapshot"), this);
    QAction clearTimelineAction(tr("Clear timeline"), this);
    QAction removeTimelineAction(tr("Remove timeline"), this);

    addFrameMenu->addAction(&addFrameAction);
    addFrameMenu->addAction(&snapshotAction);
    addFrameMenu->addSeparator();
    addFrameMenu->addAction(&clearTimelineAction);
    addFrameMenu->addAction(&removeTimelineAction);

    QAction* action = addFrameMenu->exec(mapToGlobal(e->pos()));
    if(action == &addFrameAction)
        emit addKeyframe(mapToScene(e->pos()));
    else if(action == &snapshotAction)
        emit snapshot(static_cast<int>(mapToScene(e->pos()).x()), true);
    else if(action == &clearTimelineAction)
        emit clearTimeline();
    else if(action == &removeTimelineAction)
        emit removeTimeline();
}

void PropertyTimelineView::mousePressEvent(QMouseEvent* event) {
    if(dynamic_cast<CurrentFrameGraphicsItem*>(itemAt(event->pos()))) {
        currentFrameChange_ = true;
        QGraphicsView::mousePressEvent(event);
    }
    else if(!dynamic_cast<KeyframeGraphicsItem*>(itemAt(event->pos())) &&
        !dynamic_cast<QGraphicsPixmapItem*>(itemAt(event->pos())) &&
        !dynamic_cast<QGraphicsEllipseItem*>(itemAt(event->pos()))) {
            emit noItemPressed(true);
            QGraphicsView::mousePressEvent(event);
    }
    else if(dynamic_cast<KeyframeGraphicsItem*>(itemAt(event->pos()))) {
        QGraphicsView::mousePressEvent(event);
    }
    else if(itemAt(event->pos())) {
        emit mousePressedAt(event->pos(), itemAt(event->pos()));
        //mouseReleaseEvent(new QMouseEvent(QEvent::MouseButtonRelease, event->pos(), Qt::LeftButton, Qt::RightButton, Qt::NoModifier));
    }
    else {
        QGraphicsView::mousePressEvent(event);
    }

}

void PropertyTimelineView::mouseMoveEvent(QMouseEvent* event) {
    if(currentFrameChange_) {
        if(mapToScene(event->pos()).x() < 0)
            emit frameChange(0);
        else if(mapToScene(event->pos()).x() > (int)AnimationEditor::getDuration())
            emit frameChange(static_cast<int>(AnimationEditor::getDuration()));
        else
            emit frameChange(static_cast<int>(mapToScene(event->pos()).x()));
    }
    QGraphicsView::mouseMoveEvent(event);
}

void PropertyTimelineView::mouseReleaseEvent(QMouseEvent* event) {
    currentFrameChange_ = false;
    QGraphicsView::mouseReleaseEvent(event);
}

void PropertyTimelineView::sceneOrder(QMatrix geometry) {    // this maybe useful for viewport propagation
    setMatrix(geometry);
    repaint();
}

void PropertyTimelineView::setCurrentFrame(int frame) {
    currentFrame_ = frame;
}

void PropertyTimelineView::scrollBarOrder(int value) {
    horizontalScrollBar()->setValue(value);
}

} //namespace voreen

