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

#ifndef TEMPLATEPROPERTYTIMELINEWIDGET_H
#define TEMPLATEPROPERTYTIMELINEWIDGET_H

#include "voreen/qt/widgets/animation/propertytimelinewidget.h"

namespace voreen {

class QPropertyWidget;

template <class T> class TemplatePropertyTimelineWidget : public PropertyTimelineWidget {
public:
    TemplatePropertyTimelineWidget(std::string, TemplatePropertyTimeline<T>*, int , QWidget* = 0);

    QPropertyWidget* getWidget();

    /// This is the implementation of the virtual function of the timelineobserver in the core section
    void timelineChanged();

protected:

    /// is used when a complete animation is loaded so that saved Keyframes are automatically added
    void generateKeyframeItems();

    /// generate Tooltips for all Items
    void generateToolTips();


    /// adds a Keyframe to the Animation core
    void addTemplateKeyframeCore(KeyframeGraphicsItem*, QPointF);

    /// adds a Keyframe to the propertytimelinescene
    void addTemplateKeyframeScene(KeyframeGraphicsItem*, PropertyKeyValueBase*);

    /// when an item is clicked show the appropriate propertywidgets and Interpolationselectors for Keyframemanipulation
    void templateItemClicked(KeyframeGraphicsItem*);

    /// update the Value of the selected Keyframe
    void updateTemplateKeyframe();

    /// update the position of a moved Keyframe
    void updateTemplateKeyframePosition(float, KeyframeGraphicsItem*);

    /// shift a keyframe
    void shiftTemplateKeyframePosition(float, KeyframeGraphicsItem*);

    /// plots the function
    void visualize();

    /// sets the outinterpolation
    void setTemplateOutInterpolation(QAction*);

    /// sets the in interpolation
    void setTemplateInInterpolation(QAction*);

    void activateTemplateTimeline(bool);

    /**
    * makes an incomplete copy of the given property. In the given scenario we only need the according value
    * so no linking information or other stuff is copied
    */
    TemplateProperty<T>* copyProperty(const Property*);

    /// links GraphicsItem with the corresponding KeyValues of Type <T>
    std::map<KeyframeGraphicsItem*, const PropertyKeyValue<T>*> keyframes_;

    /// links the string with the corresponding InterpolationFunction
    std::map<QAction*, InterpolationFunction<T>*> QActionInterpolationFunctionMap_;

    /**
    * holds a copy of the property to get the corresponding widget
    * for manipulation purpose that needn't affect the network
    */
    TemplateProperty<T>* property_;


    /// Pointer to the coressponding TemplatePropertyTimeline for Manipulation purposes
    TemplatePropertyTimeline<T>* templatePropertyTimeline_;


    /// This is the Polygon for the Interpolationvisualisation (only <float> and <int> Timelines use it actually)
    QGraphicsPathItem* plotItem_;

    /// delete a Keyframe
    void deleteTemplateKeyframe();

    /// populates the Interpolationfunctionmenus with categories and the function names
    void populateInterpolationMenus();

    /// indicates interaction, mainly used to determine the correct time for undo steps
    bool interaction_;

    void templateSnapshot(int, bool);

    void setTemplateSmooth(bool);

};

} // namespace voreen

#endif

