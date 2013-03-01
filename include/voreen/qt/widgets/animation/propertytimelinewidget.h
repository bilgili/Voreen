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

#ifndef PROPERTYTIMELINEWIDGET_H
#define PROPERTYTIMELINEWIDGET_H

#include <QComboBox>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPolygonItem>
#include <QHBoxLayout>

#include <QMenu>
#include <QRadioButton>
#include <QPolygonF>
#include <QPushButton>
#include <QWidget>

#include "propertytimelineview.h"
#include "currentframegraphicsitem.h"
#include "keyframegraphicsitem.h"
#include "voreen/core/properties/propertyowner.h"
#include "voreen/core/animation/propertykeyvalue.h"
#include "voreen/core/animation/propertytimeline.h"
#include "voreen/core/animation/templatepropertytimeline.h"
#include "voreen/core/animation/timelineobserver.h"

namespace voreen {

class QPropertyWidget;
class KeyframeGraphicsItem;

/**
* Graphical representation of a PropertyTimeline in the animationcore.
* The propertyTimlineWidget gives all the basic functionality for
* manipulating the Animation.
* However for every property type you need to reimplement type
* specific actions, like propertycloning, in the TemplatePropertyTimelineWidgets
*/
class PropertyTimelineWidget : public QWidget, public TimelineObserver, public PropertyOwner {
    Q_OBJECT
public:
    PropertyTimelineWidget(std::string, PropertyTimeline*, QWidget* = 0);

    PropertyTimelineView* getPropertyTimelineView();
    virtual QPropertyWidget* getWidget();
    /// This is the implementation of the virtual function of the timelineobserver in the core section
    virtual void timelineChanged() {}

     virtual std::string getClassName() const { return "PropertyTimelineWidget"; }
     virtual VoreenModule* create() const { return 0; }

public slots:
    void setFps(int);
    void setDuration(int);
    /// de- or activates the timeline for rendering of propertychanges
    void activateTimeline(bool);

protected:
    void initConnections();

    /// Holds some Keyframerelated SubWidgets, like the propertyWidget and interpolationselector_ Widgets
    QWidget* KeyframeWidget_;
    /// Smothness group for button and text element
    QGraphicsItemGroup* smoothGroup_;
    bool smoothness_;
    QGraphicsEllipseItem* smoothItem_;
    QGraphicsSimpleTextItem* smoothTextItem_;
    void changeSmoothnessColor();
    QHBoxLayout* mainLayout_;
    QPropertyWidget* propertyWidget_;

    PropertyTimeline* propertyTimeline_;

    int duration_;
    int fps_;
    QGraphicsPixmapItem* inInterpolationSelector_;
    QGraphicsPixmapItem* outInterpolationSelector_;
    QMenu* inInterpolationMenu_;
    QMenu* outInterpolationMenu_;

    QGraphicsScene* propertyTimelineScene_;
    PropertyTimelineView* propertyTimelineView_;
    CurrentFrameGraphicsItem* currentFrameGraphicsItem_;
    std::vector<KeyframeGraphicsItem*> keyframeGraphicsItems_;
    void keyPressEvent(QKeyEvent*);

    /**
    * Delegate Keyframe Addings to the corresponding function in the templated class
    * We are in need of this because Q_OBJECTS can't be templated, but we need signal/slots
    */
    virtual void addTemplateKeyframeCore(KeyframeGraphicsItem*, QPointF) {}

    /**
    * Delegate Keyframe Addings to the corresponding function in the templated class
    * We are in need of this because Q_OBJECTS can't be templated, but we need signal/slots
    */
    virtual void addTemplateKeyframeScene(KeyframeGraphicsItem*, PropertyKeyValueBase*) {}

    /**
    * Delegate Keyframeupdates to the corresponding function in the templated class
    * We are in need of this because Q_OBJECTS can't be templated, but we need signal/slots
    */
    virtual void templateItemClicked(KeyframeGraphicsItem*) {}

    /**
    * Delegate Keyframeupdates to the corresponding function in the templated class
    * We are in need of this because Q_OBJECTS can't be templated, but we need signal/slots
    */
    virtual void updateTemplateKeyframe() {}

    /**
    * Delegate Keyframepositionupdates to the corresponding function in the templated class
    * We are in need of this because Q_OBJECTS can't be templated, but we need signal/slots
    */
    virtual void updateTemplateKeyframePosition(float, KeyframeGraphicsItem*) {}

    virtual void shiftTemplateKeyframePosition(float, KeyframeGraphicsItem*) {}

    /**
    * Delegate out interpolationfunction changes to the corresponding function in the templated class
    * We are in need of this because Q_OBJECTS can't be templated, but we need signal/slots
    */
    virtual void setTemplateOutInterpolation(QAction*) {}

    /**
    * Delegate in interpolationfunction changes  to the corresponding function in the templated class
    * We are in need of this because Q_OBJECTS can't be templated, but we need signal/slots
    */
    virtual void setTemplateInInterpolation(QAction*) {}

    /**
    * Delegate Keyframe deletion to the corresponding function in the templated class
    * We are in need of this because Q_OBJECTS can't be templated, but we need signal/slots
    */
    virtual void deleteTemplateKeyframe() {}

    /**
    * Delegate CameraSnapshots to the corresponding function in the templated class
    * We are in need of this because Q_OBJECTS can't be templated, but we need signal/slots
    */
    virtual void templateSnapshot(int, bool) {}

    /**
    * Delegate Timelineactivations to the corresponding function in the templated class
    * We are in need of this because Q_OBJECTS can't be templated, but we need signal/slots
    */
    virtual void activateTemplateTimeline(bool) {}

    QString getTimeString(int frame);

    /**
    * Holds a reference to the currently selected KeyframeGraphicsItem
    * This is needed to identify the corresponding KeyValue in the core on Value changes
    */
    KeyframeGraphicsItem* currentItem_;

    /**
    * This is a status bool to determine that an Action altering the Core TemplatePropertyTimeline
    * has its origin in the gui this should only be set to true if it can be guaranteed that
    * afterwards the observers of a templatepropertytimeline get a notification via timelinechanged methods
    * if this isn't assured it can and may lead to ugly and hard to debug sideeffects. So if at any point the
    * gui does't respond to an altered Timeline this is the first thing to check.
    */
    bool timelineChange_;

    QPushButton* activateTimelineButton_;

    /**
    * container for showing status information of a keyvalue besides tooltips
    */
    QWidget* currentFrameWidget_;
    QLabel* currentFrameLabel_;
    QHBoxLayout* currentFrameLayout_;

    void resizeEvent(QResizeEvent*);

    virtual void emitKeyframeAdded();

    virtual void setTemplateSmooth(bool);

    void moveFrameHUD(int);
    void showFrameHUD(bool);

    QGraphicsTextItem* currentFrameCounter_;
protected slots:

    /// Adds a Keyframe at given point in Animation Core
    KeyframeGraphicsItem* addKeyframeCore(QPointF);
    /// Adds a Keyframe at given point in propertyTimelineScene
    KeyframeGraphicsItem* addKeyframeScene(PropertyKeyValueBase*);

    void clearTimeline();
    void removeTimeline();

    /// Sets the current Frame
    void setCurrentFrame(int);
    ///Invoken when an item is clicked
    void itemClicked(KeyframeGraphicsItem*);
    ///Invoken when an item is released
    void itemReleased(KeyframeGraphicsItem*, bool shift);
    /// Item is moving
    void itemMoving(KeyframeGraphicsItem* kfgi);

    /// plots the interpolation for int and float
    virtual void visualize() {}
    /// shows or hides the PropertyWidgets
    void disablePropertyWidget(bool);
    ///This method is used to update the KeyValues coresponding to a Keyframe
    void updateKeyframe();
    ///Slot for showing the InterpolationMenu
    void showInInterpolationMenu();
    ///Slot for showing the InterpolationMenu
    void showOutInterpolationMenu();
    /// takes a snapshot of the current camera position and rotation
    void snapshot(int, bool);
    /// sets the smooth parameter of the currently selected keyvalue
    void setSmooth(bool);

    void interpolationSelectorPressed(QPointF, const QGraphicsItem*);

signals:
    void renderAt(float);
    void viewResizeSignal(int);
    void keyframeAdded();
    void keyframeChanged();
    void removeTimeline(Property* prop);
};

} // namespace voreen

#endif
