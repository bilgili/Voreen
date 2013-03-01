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

#ifndef PROPERTYTIMELINEVIEW_H
#define PROPERTYTIMELINEVIEW_H

#include <QWidget>
#include <QGraphicsView>

class QGraphicsScene;
class CurrentFrameGraphicsItem;

namespace voreen {

class PropertyTimelineView : public QGraphicsView {
    Q_OBJECT
public:
   PropertyTimelineView(QGraphicsScene*, QWidget* = 0);
   static float zoom_;

public slots:
    void sceneOrder(QMatrix);
    void setCurrentFrame(int);
    void scrollBarOrder(int);

protected:
    void contextMenuEvent(QContextMenuEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);

    static int currentFrame_;
    QGraphicsScene* scene_;
    bool currentFrameChange_;

signals:
    void addKeyframe(QPointF);
    void clearTimeline();
    void removeTimeline();
    void sceneRequest(QMatrix);
    void scrollBarRequest(int);
    void noItemPressed(bool);
    void snapshot(int, bool);
    void frameChange(int);
    void mousePressedAt(QPointF, const QGraphicsItem*);
};

} // namespace voreen

#endif

