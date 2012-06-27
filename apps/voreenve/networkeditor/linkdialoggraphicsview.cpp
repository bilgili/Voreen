/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "linkdialoggraphicsview.h"

#include <QAction>
#include <QKeyEvent>

namespace voreen {
LinkDialogGraphicsView::LinkDialogGraphicsView(QWidget* parent)
    : QGraphicsView(parent)
{
    setMinimumSize(700, 480);
    setMaximumHeight(480);
    setScene(new QGraphicsScene);
    setMouseTracking(true);
    setBackgroundBrush(QBrush(Qt::darkGray));
    setRenderHint(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QAction* deleteAction = new QAction(QIcon(":/voreenve/icons/eraser.png"), tr("Delete"), this);
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteAction()));
    contextMenu_.addAction(deleteAction);
}

void LinkDialogGraphicsView::deleteAction() {
    emit deleteActionTriggered(selectedArrow_);
}

void LinkDialogGraphicsView::keyPressEvent(QKeyEvent* event) {
    QGraphicsView::keyPressEvent(event);

    if (!event->isAccepted() && (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace))
        emit deleteActionTriggered(0);
}

void LinkDialogGraphicsView::contextMenuEvent(QContextMenuEvent* event) {
    selectedArrow_ = itemAt(event->pos());
    if (selectedArrow_)
        contextMenu_.popup(event->globalPos());
}

} // namespace
