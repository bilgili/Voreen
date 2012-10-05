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

#ifndef VRN_SPLITGRAPHICSVIEW_H
#define VRN_SPLITGRAPHICSVIEW_H

#include <QGraphicsView>

class QWheelEvent;
class QMouseEvent;
class QKeyEvent;

namespace voreen {

    class PropertyGraphicsItem;
    class PropertyListGraphicsItem;
    class PropertyLinkArrowGraphicsItem;

class SplitGraphicsView : public QGraphicsView {
Q_OBJECT;
public:
    enum SplitPosition{
        UNKNOWN,
        LEFT,
        CENTER,
        RIGHT
    };

public:
    //constructor and destructor
    SplitGraphicsView(QWidget* parent = 0);
    ~SplitGraphicsView();

    //---------------------------------------------------------------------------------------------------------------
    //                  splitgraphicsview functions                                                                  
    //---------------------------------------------------------------------------------------------------------------
    SplitGraphicsView* getBuddyView() const;
    void setBuddyView(SplitGraphicsView* view);
    SplitPosition getSplitPosition() const;
    void setSplitPosition(SplitPosition sp);
    PropertyListGraphicsItem* getPropertyList() const;
    void setPropertyList(PropertyListGraphicsItem* list);
    void updateViewport();

    //current draged arrow
    PropertyLinkArrowGraphicsItem* getCurrentDragedArrow();
    void initCurrentDragedArrow(PropertyGraphicsItem* item, QPointF mousePos);
    void deleteCurrentDragedArrow();
signals:
    void linkProperties(PropertyGraphicsItem* src, PropertyGraphicsItem* dst);
public slots:
    void updateBuddySelection();

private:
    //member
    SplitGraphicsView* buddyView_;
    SplitPosition splitPosition_;
    PropertyListGraphicsItem* list_;

    PropertyLinkArrowGraphicsItem* currentDragedArrow_;

    //---------------------------------------------------------------------------------------------------------------
    //                  events                                                                                       
    //---------------------------------------------------------------------------------------------------------------
protected:
    //wheel
    void wheelEvent(QWheelEvent* event);
    //mouse
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    //key
    void keyPressEvent(QKeyEvent* event);
};

} //namespace voreen

#endif // VRN_SPLITGRAPHICSVIEW_H

