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

#include "voreen/qt/widgets/animation/templatepropertytimelinewidget.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/voreenmodule.h"

#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/buttonproperty.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/fontproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/matrixproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/propertyvector.h"
#include "voreen/core/properties/shaderproperty.h"
#include "voreen/core/properties/stringproperty.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/volumeurllistproperty.h"
#include "voreen/core/properties/volumeurlproperty.h"
#include "voreen/core/properties/link/propertylink.h"

#include "voreen/core/animation/interpolationfunction.h"
#include "voreen/core/animation/interpolationfunctionfactory.h"

#include "voreen/core/properties/propertywidget.h"
#include "voreen/qt/widgets/property/qpropertywidget.h"
#include "voreen/qt/widgets/property/buttonpropertywidget.h"
#include "voreen/qt/widgets/property/colorpropertywidget.h"
#include "voreen/qt/widgets/animation/currentframegraphicsitem.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/qt/widgets/animation/keyframegraphicsitem.h"
#include "voreen/qt/widgets/animation/propertytimelinewidget.h"
#include "voreen/qt/widgets/property/qpropertywidget.h"
#include "voreen/qt/widgets/property/shaderpropertywidget.h"
#include "voreen/qt/widgets/property/stringpropertywidget.h"

#include "tgt/camera.h"
#include "tgt/vector.h"

#include <QContextMenuEvent>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QLabel>
#include <QMenu>
#include <QPen>
#include <QVBoxLayout>

#include <cmath>
#include <iostream>

using tgt::Camera;

namespace {

voreen::QPropertyWidget* createQPropertyWidget(voreen::Property* prop) {

    voreen::PropertyWidget* propWidget = voreen::VoreenApplication::app()->createPropertyWidget(prop);
    //if (propWidget)
    //  prop->addWidget(propWidget);
    voreen::QPropertyWidget* qPropWidget = dynamic_cast<voreen::QPropertyWidget*>(propWidget);
    return qPropWidget;
}

} // namespace anonymous

namespace voreen {

template <class T>
TemplatePropertyTimelineWidget<T>::TemplatePropertyTimelineWidget(std::string name, TemplatePropertyTimeline<T>* propTimeline, int frame, QWidget* parent)
        : PropertyTimelineWidget(name, propTimeline, parent)
        , templatePropertyTimeline_(propTimeline)
        , interaction_(false)
{
    timelineChange_ = false;
    if (dynamic_cast<TemplatePropertyTimelineWidget<tgt::vec4>*>(this) || dynamic_cast<TemplatePropertyTimelineWidget<tgt::ivec4>*>(this)
        || dynamic_cast<TemplatePropertyTimelineWidget<tgt::vec3>*>(this) || dynamic_cast<TemplatePropertyTimelineWidget<tgt::vec3>*>(this)) {
        if (!dynamic_cast<ColorPropertyWidget*>(getWidget()))
            setFixedHeight(100);
    }

    currentFrameGraphicsItem_->setPos(QPoint(frame, static_cast<int>(currentFrameGraphicsItem_->pos().y())));
    currentItem_ = 0;
    timelineChange_ = true;
    propertyWidget_ = 0;
    QPropertyWidget* propWidget = 0;
    propWidget = getWidget();

    templatePropertyTimeline_->addObserver(this);

    inInterpolationMenu_ = new QMenu();
    outInterpolationMenu_ = new QMenu();

    populateInterpolationMenus();
    inInterpolationSelector_->show();
    outInterpolationSelector_->show();

    KeyframeWidget_ = new QWidget(this);
    KeyframeWidget_->setMinimumWidth(80);
    if (propWidget != 0) {
        propertyWidget_ = propWidget;
        QHBoxLayout* KeyframeLayout = new QHBoxLayout(KeyframeWidget_);
        KeyframeLayout->addWidget(propWidget);
    }
    else {
        propWidget = new StringPropertyWidget(new StringProperty("id", "", ""), this);
        propWidget->setEnabled(false);
        propWidget->setMaximumHeight(0);
        QHBoxLayout* KeyframeLayout = new QHBoxLayout(KeyframeWidget_);
        KeyframeLayout->addWidget(propWidget);
    }
    mainLayout_->addWidget(KeyframeWidget_);
    KeyframeWidget_->show();

    QPen* pen = new QPen();

    pen->setStyle(Qt::SolidLine);
    pen->setWidth(2);
    pen->setBrush(QColor(200,100,100,150));

    plotItem_ = new QGraphicsPathItem();
    plotItem_->setPen(*pen);
    propertyTimelineScene_->addItem(plotItem_);

    initConnections();
    generateKeyframeItems();

    QLinearGradient gradient(0, 0, 0, 40);
    gradient.setSpread(QGradient::PadSpread);
    gradient.setColorAt(0, QColor(255,255,255,0));
    gradient.setColorAt(0.5, QColor(150,150,150,100));
    gradient.setColorAt(1, QColor(50,0,50,100));
    propertyTimelineScene_->setBackgroundBrush(gradient);

    visualize();
}

template <class T>
void TemplatePropertyTimelineWidget<T>::populateInterpolationMenus() {
    InterpolationFunctionFactory* iff = new InterpolationFunctionFactory();
    std::vector<InterpolationFunction<T>*> listOfFunctions = iff->getListOfFunctions<T>();
    typename std::vector<InterpolationFunction<T>*>::iterator it;
    std::map<std::string, QMenu*> categories;
    it = listOfFunctions.begin();
    while(it != listOfFunctions.end()) {
        std::string id = (*it)->getCategory();
        if (categories.find(id) == categories.end()) {
            QMenu* categoryMenu = new QMenu(QString::fromStdString(id), this);
            categories[id] = categoryMenu;
            inInterpolationMenu_->addMenu(categoryMenu);
            outInterpolationMenu_->addMenu(categoryMenu);
        }
        it++;
    }

    it = listOfFunctions.begin();
    while(it != listOfFunctions.end()) {
        QMenu* categoryMenu = (*categories.find((*it)->getCategory())).second;
        QAction* action  = new QAction(QString::fromStdString((*it)->getGuiName()), this);
        categoryMenu->addAction(action);
        QActionInterpolationFunctionMap_[action] = (*it)->create();
        it++;
    }
    delete iff;
}

template <class T>
void TemplatePropertyTimelineWidget<T>::generateKeyframeItems() {
    typename std::vector<KeyframeGraphicsItem*>::iterator it2 = keyframeGraphicsItems_.begin();
    while(it2 != keyframeGraphicsItems_.end()) {
        propertyTimelineScene_->removeItem((*it2));
        it2++;
    }

    keyframeGraphicsItems_.clear();
    keyframes_.clear();

    //new generation
    std::map<float, PropertyKeyValue<T>*> initialKeyValues = templatePropertyTimeline_->getKeyValues();
    typename std::map<float, PropertyKeyValue<T>*>::iterator it =  initialKeyValues.begin();
    while(it != initialKeyValues.end()) {
        KeyframeGraphicsItem* kfgi = addKeyframeScene((*it).second);
        keyframes_[kfgi] = (*it).second;
        it++;
    }
    disablePropertyWidget(true);
    timelineChange_ = false;
}


template <class T>
void TemplatePropertyTimelineWidget<T>::timelineChanged() {
    if (timelineChange_ == true) {
        timelineChange_ = false;
    }
    else {
        generateKeyframeItems();
        inInterpolationSelector_->hide();
        outInterpolationSelector_->hide();
        disablePropertyWidget(true);
    }
    visualize();
}

template <class T>
void TemplatePropertyTimelineWidget<T>::addTemplateKeyframeScene(KeyframeGraphicsItem* kfgi, PropertyKeyValueBase* keyVal) {
    if (dynamic_cast<PropertyKeyValue<T>*>(keyVal)) {
        PropertyKeyValue<T>* key = dynamic_cast<PropertyKeyValue<T>*>(keyVal);
        if (!templatePropertyTimeline_->getKeyValues().empty()) {
            keyframes_[kfgi] = key;
        }
    }
}

template <class T>
void TemplatePropertyTimelineWidget<T>::addTemplateKeyframeCore(KeyframeGraphicsItem* kfgi, QPointF pos) {
    timelineChange_ = true;
    float position = (float)pos.x()/30.0f;

    const PropertyKeyValue<T>* kValue =  templatePropertyTimeline_->newKeyValue(position);
    timelineChange_ = false;

    if (!templatePropertyTimeline_->getKeyValues().empty()) {
        keyframes_[kfgi] = kValue;
        generateToolTips();
    }
}

template <>
void TemplatePropertyTimelineWidget<TransFunc*>::templateItemClicked(KeyframeGraphicsItem* kfgi) {

    if (keyframes_.find(kfgi) == keyframes_.end()) {
        LERROR("KeyframeGraphicsItem not found");
        return;
    }
    if (!keyframes_[kfgi]->getValue()) {
        LERROR("KeyframeGraphicsItem contains no value");
        return;
    }

    property_->set(keyframes_[kfgi]->getValue()->clone());
    propertyWidget_->updateFromProperty();
    property_->updateWidgets();
    currentItem_ = kfgi;
    smoothGroup_->hide();
    if (keyframes_[currentItem_]->getForegoingInterpolationFunction() != 0) {
        inInterpolationSelector_->setToolTip(QString::fromStdString(keyframes_[kfgi]->getForegoingInterpolationFunction()->getGuiName()));
        if (dynamic_cast<const MultiPointInterpolationFunction<TransFunc*>*>(keyframes_[currentItem_]->getForegoingInterpolationFunction())) {
            smoothGroup_->show();
        }
        smoothness_ = keyframes_[currentItem_]->getSmooth();
        changeSmoothnessColor();
    }
    if (keyframes_[currentItem_]->getFollowingInterpolationFunction() != 0) {
        outInterpolationSelector_->setToolTip(QString::fromStdString(keyframes_[kfgi]->getFollowingInterpolationFunction()->getGuiName()));
        if (dynamic_cast<const MultiPointInterpolationFunction<TransFunc*>*>(keyframes_[currentItem_]->getForegoingInterpolationFunction())) {
            smoothGroup_->show();
        }
        smoothness_ = keyframes_[currentItem_]->getSmooth();
        changeSmoothnessColor();
    }
}

template <>
void TemplatePropertyTimelineWidget<Camera>::templateItemClicked(KeyframeGraphicsItem* kfgi) {

    if (keyframes_.find(kfgi) == keyframes_.end()) {
        LERROR("KeyframeGraphicsItem not found");
        return;
    }
    //if (!keyframes_[kfgi]->getValue()) {
        //LERROR("KeyframeGraphicsItem contains no value");
        //return;
    //}

    currentItem_ = kfgi;
    if (keyframes_[currentItem_]->getForegoingInterpolationFunction() != 0) {
        inInterpolationSelector_->setToolTip(QString::fromStdString(keyframes_[kfgi]->getForegoingInterpolationFunction()->getGuiName()));
        if (dynamic_cast<const MultiPointInterpolationFunction<Camera>*>(keyframes_[currentItem_]->getForegoingInterpolationFunction())) {
            smoothGroup_->show();
        }
        smoothness_ = keyframes_[currentItem_]->getSmooth();
        changeSmoothnessColor();
    }
    if (keyframes_[currentItem_]->getFollowingInterpolationFunction() != 0) {
        outInterpolationSelector_->setToolTip(QString::fromStdString(keyframes_[kfgi]->getFollowingInterpolationFunction()->getGuiName()));
        if (dynamic_cast<const MultiPointInterpolationFunction<Camera>*>(keyframes_[currentItem_]->getForegoingInterpolationFunction())) {
            smoothGroup_->show();
        }
        smoothness_ = keyframes_[currentItem_]->getSmooth();
        changeSmoothnessColor();
    }
}

template <class T>
void TemplatePropertyTimelineWidget<T>::templateItemClicked(KeyframeGraphicsItem* kfgi) {

    if (keyframes_.find(kfgi) == keyframes_.end()) {
        LERROR("KeyframeGraphicsItem not found");
        return;
    }

    property_->set(keyframes_[kfgi]->getValue());
    propertyWidget_->updateFromProperty();
    property_->updateWidgets();
    currentItem_ = kfgi;
    smoothGroup_->hide();
    if (keyframes_[currentItem_]->getForegoingInterpolationFunction() != 0) {
        inInterpolationSelector_->setToolTip(QString::fromStdString(keyframes_[kfgi]->getForegoingInterpolationFunction()->getGuiName()));
        if (dynamic_cast<const MultiPointInterpolationFunction<T>*>(keyframes_[currentItem_]->getForegoingInterpolationFunction())) {
            smoothGroup_->show();
        }
        smoothness_ = keyframes_[currentItem_]->getSmooth();
        changeSmoothnessColor();
    }
    if (keyframes_[currentItem_]->getFollowingInterpolationFunction() != 0) {
        outInterpolationSelector_->setToolTip(QString::fromStdString(keyframes_[kfgi]->getFollowingInterpolationFunction()->getGuiName()));
        if (dynamic_cast<const MultiPointInterpolationFunction<T>*>(keyframes_[currentItem_]->getForegoingInterpolationFunction())) {
            smoothGroup_->show();
        }
        smoothness_ = keyframes_[currentItem_]->getSmooth();
        changeSmoothnessColor();
    }
}

template <class T>
void TemplatePropertyTimelineWidget<T>::updateTemplateKeyframe() {
    if (property_ != 0 && property_->getOwner() == 0) {
        templatePropertyTimeline_->setNewUndoState();   // this is for properties without interaction support (e.g. options, bool, ...)
        emitKeyframeAdded();
    }
    if (!interaction_) {
        interaction_ = true;
        templatePropertyTimeline_->setNewUndoState();   // the interaction starts here so we want an undo state
    }
    if (!interactionMode()) {
        interaction_ = false;   // interaction stops
        emitKeyframeAdded();
    }
    timelineChange_ = true;

    if (currentItem_!=0) {
        const PropertyKeyValue<T>* keyValue1 = keyframes_[currentItem_];
        PropertyKeyValue<T>* keyValue = const_cast<PropertyKeyValue<T>*>(keyValue1);
        if (property_ != 0 && keyValue != 0) {
            templatePropertyTimeline_->changeValueOfKeyValue(property_->get(),keyValue);
            emit(renderAt(keyValue->getTime()));
            visualize();
        }
    }

    timelineChange_ = false;
    generateToolTips();
}

template <class T>
void TemplatePropertyTimelineWidget<T>::updateTemplateKeyframePosition(float x, KeyframeGraphicsItem* kfgi) {
    timelineChange_ = true;
    templatePropertyTimeline_->setNewUndoState();
    PropertyKeyValue<T>* keyValue = const_cast<PropertyKeyValue<T>*>(keyframes_[kfgi]);
    if (x > 0) {
        int test = templatePropertyTimeline_->changeTimeOfKeyValue(x, keyValue);
        if (test == KV_MOVED_TO_DIFFERENT_INTERVAL || test == KV_EXISTS_AT_NEW_TIME) {
            timelineChange_ = false;
            generateKeyframeItems();
        }
    }
    else {
        templatePropertyTimeline_->changeTimeOfKeyValue(0, keyValue);
    }
    visualize();
    emitKeyframeAdded();
    timelineChange_ = false;
}

template <class T>
void TemplatePropertyTimelineWidget<T>::shiftTemplateKeyframePosition(float x, KeyframeGraphicsItem* kfgi) {
    timelineChange_ = true;
    templatePropertyTimeline_->setNewUndoState();
    PropertyKeyValue<T>* keyValue = const_cast<PropertyKeyValue<T>*>(keyframes_[kfgi]);
    if (x > 0) {
        int test = templatePropertyTimeline_->shiftKeyValue(x, keyValue);
        generateKeyframeItems();
    }
    else {
        templatePropertyTimeline_->changeTimeOfKeyValue(0, keyValue);
    }
    visualize();
    emitKeyframeAdded();
    timelineChange_ = false;
}

template <class T>
void TemplatePropertyTimelineWidget<T>::deleteTemplateKeyframe() {
    timelineChange_ = false;        // in most cases the deletion of a keyvalue will change other values
    if (currentItem_ != 0) {
        templatePropertyTimeline_->deleteKeyValue(keyframes_[currentItem_]);    // after this command the generate keyframe method will be invoken so there is no need to remove the item from the scene
        currentItem_ = 0;
        disablePropertyWidget(false);
        emitKeyframeAdded();
        inInterpolationSelector_->hide();
        outInterpolationSelector_->hide();
    }
    timelineChange_ = false;
}

template <class T>
void TemplatePropertyTimelineWidget<T>::generateToolTips() {
    std::vector<KeyframeGraphicsItem*>::iterator it = keyframeGraphicsItems_.begin();
    std::string itemToolTip = "";

    while(it != keyframeGraphicsItems_.end()) {

        KeyframeGraphicsItem* item = (*it);
        std::ostringstream out;
        if (keyframes_.find(item) != keyframes_.end()) {
            out << keyframes_[item]->getTime();
            itemToolTip = "Time: "+out.str();
            if (keyframes_[item]->getForegoingInterpolationFunction() != 0) {
                itemToolTip += "<br>Foregoing Interpolation Function: "+keyframes_[item]->getForegoingInterpolationFunction()->getGuiName();
            }
            if (keyframes_[item]->getFollowingInterpolationFunction() != 0) {
                itemToolTip += "<br>Following Interpolation Function: "+keyframes_[item]->getFollowingInterpolationFunction()->getGuiName();
            }
            item->setToolTip(QString::fromStdString(itemToolTip));
        }
        ++it;
    }
}

template <class T>
void TemplatePropertyTimelineWidget<T>::activateTemplateTimeline(bool active) {
    templatePropertyTimeline_->setActiveOnRendering(active);
    if (active) {
        QLinearGradient gradient(0, 0, 0, 40);
        gradient.setSpread(QGradient::PadSpread);
        gradient.setColorAt(0, QColor(255,255,255,0));
        gradient.setColorAt(0.5, QColor(150,150,150,100));
        gradient.setColorAt(1, QColor(50,0,50,100));
        propertyTimelineScene_->setBackgroundBrush(gradient);
        propertyTimelineView_->setEnabled(true);
    }
    else {
        QLinearGradient gradient(0, 0, 0, 40);
        gradient.setSpread(QGradient::PadSpread);
        gradient.setColorAt(0, QColor(255,255,255,200));
        gradient.setColorAt(0.5, QColor(255,150,150,200));
        gradient.setColorAt(0, QColor(255,100,100,200));
        propertyTimelineScene_->setBackgroundBrush(gradient);
        propertyTimelineView_->setEnabled(false);

    }
}

template <class T>
void TemplatePropertyTimelineWidget<T>::setTemplateOutInterpolation(QAction* action) {
    timelineChange_ = true;
    PropertyKeyValue<T>* keyValue = const_cast<PropertyKeyValue<T>*>(keyframes_[currentItem_]);
    templatePropertyTimeline_->setInterpolationFunctionAfter(QActionInterpolationFunctionMap_[action]->create(), keyValue);
    updateKeyframe();
    timelineChange_ = false;
    if (dynamic_cast<MultiPointInterpolationFunction<T>*>(QActionInterpolationFunctionMap_[action])) {
            smoothGroup_->show();
            smoothness_ = keyframes_[currentItem_]->getSmooth();
            changeSmoothnessColor();
    }
}

template <class T>
void TemplatePropertyTimelineWidget<T>::setTemplateInInterpolation(QAction* action) {
    timelineChange_ = true;
    PropertyKeyValue<T>* keyValue = const_cast<PropertyKeyValue<T>*>(keyframes_[currentItem_]);
    templatePropertyTimeline_->setInterpolationFunctionBefore(QActionInterpolationFunctionMap_[action]->create(), keyValue);
    updateKeyframe();
    timelineChange_ = false;
    if (dynamic_cast<MultiPointInterpolationFunction<T>*>(QActionInterpolationFunctionMap_[action])) {
            smoothGroup_->show();
            smoothness_ = keyframes_[currentItem_]->getSmooth();
            changeSmoothnessColor();
    }
}

template<>
void TemplatePropertyTimelineWidget<float>::visualize() {
    //QPoint left = propertyTimelineView_->mapFromScene(QPoint(0,0));
    QPainterPath path;                                      // TODO: don't plot the the whole duration but only the the viewport width
    float max = dynamic_cast<FloatProperty*>(property_)->getMaxValue();
    float min = dynamic_cast<FloatProperty*>(property_)->getMinValue();
    float stretch = 30.0f/(max+min);    // todo: take abs? 20 is height of timelinescene

    int step = static_cast<int>(ceilf((float)duration_/5000.0f));
    for(int i = 0; i < duration_; i+=step) {
        float y = templatePropertyTimeline_->getPropertyAt((float)i/30.0f);
        path.lineTo(i, 25 - stretch * y);
    }
    plotItem_->setPath(path);
}

template<>
void TemplatePropertyTimelineWidget<int>::visualize() {
    QPainterPath path;
    float max = dynamic_cast<IntProperty*>(property_)->getMaxValue();
    float min = dynamic_cast<IntProperty*>(property_)->getMinValue();
    float stretch = 30.0f/(max+min);    // todo: take abs? 20 is height of timelinescene
    int step = static_cast<int>(ceilf((float)duration_/5000.0f));
    for(int i = 0; i < duration_; i+=step) {
        int y = templatePropertyTimeline_->getPropertyAt((float)i/30.0f);
        path.lineTo(i, 25 - stretch * y);
    }
    plotItem_->setPath(path);
}

template<>
void TemplatePropertyTimelineWidget<bool>::visualize() {
    QPainterPath path;
    int step = static_cast<int>(ceilf((float)duration_/5000.0f));
    for(int i = 0; i < duration_; i+=step) {
        bool y = templatePropertyTimeline_->getPropertyAt((float)i/30.0f);
        path.lineTo(i, 25 -  25* y);
    }
    plotItem_->setPath(path);
}

template<>
void TemplatePropertyTimelineWidget<tgt::vec4>::visualize() {
    if (dynamic_cast<FloatVec4Property*>(property_) && property_->getViews() == Property::COLOR) {
        QLinearGradient gradient(0, 0, duration_, 0);
        gradient.setSpread(QGradient::PadSpread);
        std::map<float, PropertyKeyValue<tgt::vec4>*> initialKeyValues = dynamic_cast<TemplatePropertyTimeline<tgt::vec4>*>(templatePropertyTimeline_)->getKeyValues();
        std::map<float, PropertyKeyValue<tgt::vec4>*>::iterator it =  initialKeyValues.begin();
        while(it != initialKeyValues.end()) {
            float position = (float)(*it).first / duration_ * fps_;     // TODO: this is only for linear interpolation

            QColor color(static_cast<int>((*it).second->getValue().x * 255),
                         static_cast<int>((*it).second->getValue().y * 255),
                         static_cast<int>((*it).second->getValue().z * 255),
                         255);
            gradient.setColorAt(position, color);
            it++;
        }
        propertyTimelineScene_->setBackgroundBrush(gradient);
    }
}

template<class T>
void TemplatePropertyTimelineWidget<T>::visualize() {}


template <class T>
void TemplatePropertyTimelineWidget<T>::templateSnapshot(int pos, bool force) {
    templatePropertyTimeline_->setNewUndoState();
    templatePropertyTimeline_->setCurrentSettingAsKeyvalue(pos/30.0f, force);
}


template<>
TemplateProperty<bool>* TemplatePropertyTimelineWidget<bool>::copyProperty(const Property* tempProperty) {
    Property* prop = const_cast<Property*>(tempProperty);

    if (dynamic_cast<BoolProperty*>(prop) != 0) {
        BoolProperty* retProperty = dynamic_cast<BoolProperty*>(prop);
        return new BoolProperty(retProperty->getID(), retProperty->getGuiName(), retProperty->get(), retProperty->getInvalidationLevel());
    }
    else
        return 0;
}

template<>
TemplateProperty<Camera>* TemplatePropertyTimelineWidget<Camera>::copyProperty(const Property* /*tempProperty*/) {
    return 0;
}

template<>
TemplateProperty<float>* TemplatePropertyTimelineWidget<float>::copyProperty(const Property* tempProperty) {
    Property* prop = const_cast<Property*>(tempProperty);
    if (dynamic_cast<FloatProperty*>(prop)) {
        FloatProperty* retProperty = dynamic_cast<FloatProperty*>(prop);
        FloatProperty* ret =new FloatProperty(retProperty->getID(), retProperty->getGuiName(), retProperty->get(), retProperty->getMinValue(),
        retProperty->getMaxValue(), retProperty->getInvalidationLevel());
        ret->setOwner(this);
        ret->setStepping(retProperty->getStepping());
        return ret;
    }
    else
        return 0;
}

template<>
TemplateProperty<int>* TemplatePropertyTimelineWidget<int>::copyProperty(const Property* tempProperty) {
    Property* prop = const_cast<Property*>(tempProperty);
    if (dynamic_cast<IntProperty*>(prop)) {
        IntProperty* retProperty = dynamic_cast<IntProperty*>(prop);
        IntProperty* ret = new IntProperty(retProperty->getID(), retProperty->getGuiName(), retProperty->get(), retProperty->getMinValue(),
        retProperty->getMaxValue(), retProperty->getInvalidationLevel());
        ret->setStepping(retProperty->getStepping());
        ret->setOwner(this);
        return ret;
    }
    else
        return 0;
}

template<>
TemplateProperty<tgt::vec2>* TemplatePropertyTimelineWidget<tgt::vec2>::copyProperty(const Property* tempProperty) {
    Property* prop = const_cast<Property*>(tempProperty);
    if (dynamic_cast<FloatVec2Property*>(prop)) {
        FloatVec2Property* retProperty = dynamic_cast<FloatVec2Property*>(prop);
        FloatVec2Property* ret = new FloatVec2Property(retProperty->getID(), retProperty->getGuiName(), retProperty->get(),
        retProperty->getMinValue(), retProperty->getMaxValue(), retProperty->getInvalidationLevel());
        ret->setOwner(this);
        return ret;
    }
    else
        return 0;
}

template<>
TemplateProperty<tgt::vec3>* TemplatePropertyTimelineWidget<tgt::vec3>::copyProperty(const Property* tempProperty) {
    Property* prop = const_cast<Property*>(tempProperty);
    if (dynamic_cast<FloatVec3Property*>(prop)) {
        FloatVec3Property* retProperty = dynamic_cast<FloatVec3Property*>(prop);
        FloatVec3Property* ret = new FloatVec3Property(retProperty->getID(), retProperty->getGuiName(), retProperty->get(),
        retProperty->getMinValue(), retProperty->getMaxValue(), retProperty->getInvalidationLevel());
        ret->setOwner(this);
        return ret;
    }
    else
        return 0;
}

template<>
TemplateProperty<tgt::vec4>* TemplatePropertyTimelineWidget<tgt::vec4>::copyProperty(const Property* tempProperty) {
    Property* prop = const_cast<Property*>(tempProperty);
    if (dynamic_cast<FloatVec4Property*>(prop)) {
        FloatVec4Property* retProperty = dynamic_cast<FloatVec4Property*>(prop);
        FloatVec4Property* ret =new FloatVec4Property(retProperty->getID(), retProperty->getGuiName(), retProperty->get(), retProperty->getMinValue(),
        retProperty->getMaxValue(), retProperty->getInvalidationLevel());
        ret->setOwner(this);
        return ret;
    }
    else
        return 0;

}

template<>
TemplateProperty<tgt::ivec2>* TemplatePropertyTimelineWidget<tgt::ivec2>::copyProperty(const Property* tempProperty) {
    Property* prop = const_cast<Property*>(tempProperty);
    if (dynamic_cast<IntVec2Property*>(prop)) {
        IntVec2Property* retProperty = dynamic_cast<IntVec2Property*>(prop);
        IntVec2Property* ret = new IntVec2Property(retProperty->getID(), retProperty->getGuiName(), retProperty->get(), retProperty->getMinValue(),
        retProperty->getMaxValue(), retProperty->getInvalidationLevel());
        ret->setOwner(this);
        return ret;
    }
    else
        return 0;
}

template<>
TemplateProperty<tgt::ivec3>* TemplatePropertyTimelineWidget<tgt::ivec3>::copyProperty(const Property* tempProperty) {
    Property* prop = const_cast<Property*>(tempProperty);
    if (dynamic_cast<IntVec3Property*>(prop)) {
        IntVec3Property* retProperty = dynamic_cast<IntVec3Property*>(prop);
        IntVec3Property* ret = new IntVec3Property(retProperty->getID(), retProperty->getGuiName(), retProperty->get(), retProperty->getMinValue(),
        retProperty->getMaxValue(), retProperty->getInvalidationLevel());
        ret->setOwner(this);
        return ret;
    }
    else
        return 0;
}

template<>
TemplateProperty<tgt::ivec4>* TemplatePropertyTimelineWidget<tgt::ivec4>::copyProperty(const Property* tempProperty) {
    Property* prop = const_cast<Property*>(tempProperty);
    if (dynamic_cast<IntVec4Property*>(prop)) {
        IntVec4Property* retProperty = dynamic_cast<IntVec4Property*>(prop);
        IntVec4Property* ret = new IntVec4Property(retProperty->getID(), retProperty->getGuiName(), retProperty->get(), retProperty->getMinValue(),
        retProperty->getMaxValue(), retProperty->getInvalidationLevel());
        ret->setOwner(this);
        return ret;
    }
    else
        return 0;
}

template<>
TemplateProperty<ShaderSource>* TemplatePropertyTimelineWidget<ShaderSource>::copyProperty(const Property* tempProperty) {
    Property* prop = const_cast<Property*>(tempProperty);
    if (dynamic_cast<ShaderProperty*>(prop)) {
        ShaderProperty* retProperty = dynamic_cast<ShaderProperty*>(prop);
        ShaderSource shaderSource = retProperty->get();
        ShaderProperty* sp = new ShaderProperty(retProperty->getID(), retProperty->getGuiName(),
                shaderSource.originalFragmentFilename_,
                shaderSource.originalVertexFilename_,
                shaderSource.originalGeometryFilename_);
        sp->setFragmentSource(shaderSource.fragmentSource_);
        sp->setVertexSource(shaderSource.vertexSource_);
        sp->setGeometrySource(shaderSource.geometrySource_);
        if(shaderSource.fragmentIsExternal_)
            sp->setFragmentFilename(shaderSource.externalFragmentFilename_);
        if(shaderSource.vertexIsExternal_)
            sp->setFragmentFilename(shaderSource.externalVertexFilename_);
        if(shaderSource.geometryIsExternal_)
            sp->setFragmentFilename(shaderSource.externalGeometryFilename_);
        sp->setOwner(this);
        return sp;
    }
    else
        return 0;
    return 0;
}

template<>
TemplateProperty<std::string>* TemplatePropertyTimelineWidget<std::string>::copyProperty(const Property* tempProperty) {
    Property* prop = const_cast<Property*>(tempProperty);
    if (dynamic_cast<FileDialogProperty*>(prop)) {    // FileDialogProperty
        FileDialogProperty* fdp = dynamic_cast<FileDialogProperty*>(prop);
        return new FileDialogProperty(fdp->getID(), "", "", "");// the property has no appropriate information at this point resulting in crashes on dereferentiation
    }
    else if (dynamic_cast<StringOptionProperty*>(prop)) {
        StringOptionProperty* returnProperty = new StringOptionProperty(prop->getID(), prop->getGuiName(), prop->getInvalidationLevel());
        std::vector<std::string> descriptions = dynamic_cast<StringOptionProperty*>(prop)->getDescriptions();
        std::string key;
        std::stringstream ss;
            for( unsigned int i = 0; i<descriptions.size();i++){
                ss << i;
                key = ss.str();
                returnProperty->addOption(key, descriptions[i]);

            }
        return returnProperty;
    }
    else if (dynamic_cast<StringProperty*>(prop)) {
        StringProperty* retProperty = dynamic_cast<StringProperty*>(prop);
        return new StringProperty(retProperty->getID(), retProperty->getGuiName(), retProperty->get(), retProperty->getInvalidationLevel());
    }
    else
        return 0;
}

template<>
TemplateProperty<TransFunc*>* TemplatePropertyTimelineWidget<TransFunc*>::copyProperty(const Property* tempProperty) {
    Property* prop = const_cast<Property*>(tempProperty);
    if (dynamic_cast<TransFuncProperty*>(prop)) {
        TransFuncProperty* retProperty = dynamic_cast<TransFuncProperty*>(prop);
        TransFuncProperty* ret = new TransFuncProperty(retProperty->getID(), retProperty->getGuiName(), retProperty->getInvalidationLevel());
        ret->setOwner(this);
        return ret;
    }
    else
        return 0;
}

/*template<>
TemplateProperty<VolumeCollection*>* TemplatePropertyTimelineWidget<VolumeCollection*>::copyProperty(const Property* tempProperty) {
    Property* prop = const_cast<Property*>(tempProperty);
    if (dynamic_cast<VolumeCollectionProperty*>(prop)) {
        VolumeCollectionProperty* retProperty = dynamic_cast<VolumeCollectionProperty*>(prop);
        return new VolumeCollectionProperty(retProperty->getID(), retProperty->getGuiName(), retProperty->get(), retProperty->getInvalidationLevel());
    }
    else
        return 0;
} */

/*template<>
TemplateProperty<Volume*>* TemplatePropertyTimelineWidget<Volume*>::copyProperty(const Property* tempProperty) {
    Property* prop = const_cast<Property*>(tempProperty);
    if (dynamic_cast<VolumeHandleProperty*>(prop)) {
        VolumeHandleProperty* retProperty = dynamic_cast<VolumeHandleProperty*>(prop);
        return new VolumeHandleProperty(retProperty->getID(), retProperty->getGuiName(), retProperty->get(), retProperty->getInvalidationLevel());
    }
    else
        return 0;
}*/

template <class T>
QPropertyWidget* TemplatePropertyTimelineWidget<T>::getWidget() {

    if (!VoreenApplication::app()) {
        LERRORC("voreen.qt.TemplatePropertyTimelineWidget", "VoreenApplication not instantiated");
        return 0;
    }

    property_ = copyProperty(templatePropertyTimeline_->getCorrespondingProperty());
    if (property_ != 0) {
        // start casting
        if (dynamic_cast<BoolProperty*>(property_) != 0) {
            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget)
                retWidget->hideLODControls();
            return retWidget;
        }
        else if (dynamic_cast<TemplateProperty<Camera>*>(property_)) {
            return new StringPropertyWidget(new StringProperty("","",""), this);
        }
        else if (dynamic_cast<FloatProperty*>(property_)) {
            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget)
                retWidget->hideLODControls();
            return retWidget;
        }
        else if (dynamic_cast<IntProperty*>(property_)) {
            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget)
                retWidget->hideLODControls();
            return retWidget;
        }
        else if (dynamic_cast<ShaderProperty*>(property_)) {
            /*QPropertyWidget* retWidget = static_cast<QPropertyWidget*>(qpwf->createWidget(dynamic_cast<ShaderProperty*>(property_)));
            retWidget->hideLODControls();
            retWidget->updateFromProperty();
            retWidget->update();
            delete qpwf;
            return retWidget; */

            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget) {
                retWidget->hideLODControls();
                retWidget->updateFromProperty();
                retWidget->update();
            }
            return retWidget;
        }
        else if (dynamic_cast<StringOptionProperty*>(property_)) {
            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget)
                retWidget->hideLODControls();
            return retWidget;
        }
        else if (dynamic_cast<OptionProperty<std::string>*>(property_)) {
            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget)
                retWidget->hideLODControls();
            return retWidget;
        }
        else if (dynamic_cast<FileDialogProperty*>(property_)) {
            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget)
                retWidget->hideLODControls();
            return retWidget;
        }
        else if (dynamic_cast<StringProperty*>(property_)) {
            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget)
                retWidget->hideLODControls();
            return retWidget;
        }
        else if (dynamic_cast<StringOptionProperty*>(property_)) {
            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget)
                retWidget->hideLODControls();
            return retWidget;
        }
        else if (dynamic_cast<TransFuncProperty*>(property_)) {
            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget)
                retWidget->hideLODControls();
            return retWidget;
        }
        //VectorProperties
        else if (dynamic_cast<FloatVec2Property*>(property_)) {
            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget)
                retWidget->hideLODControls();
            return retWidget;
        }
        else if (dynamic_cast<FloatVec3Property*>(property_)) {
            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget)
                retWidget->hideLODControls();
             return retWidget;
        }
        else if (dynamic_cast<FloatVec4Property*>(property_)) {
            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget)
                retWidget->hideLODControls();
            return retWidget;
        }
        else if (dynamic_cast<IntVec2Property*>(property_)) {
            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget)
                retWidget->hideLODControls();
            return retWidget;
        }
        else if (dynamic_cast<IntVec3Property*>(property_)) {
            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget)
                retWidget->hideLODControls();
            return retWidget;
        }
        else if (dynamic_cast<IntVec4Property*>(property_)) {
            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget)
                retWidget->hideLODControls();
            return retWidget;
        }

        else if (dynamic_cast<VolumeURLListProperty*>(property_)) {
            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget)
                retWidget->hideLODControls();
            return retWidget;
        }
        else if (dynamic_cast<VolumeURLProperty*>(property_)) {
            QPropertyWidget* retWidget = createQPropertyWidget(property_);
            if (retWidget)
                retWidget->hideLODControls();
            return retWidget;
        }
    }

    return 0;
}
template <class T>
void TemplatePropertyTimelineWidget<T>::setTemplateSmooth(bool smooth) {
    timelineChange_ = true;
    if (keyframes_.find(currentItem_) != keyframes_.end())
        templatePropertyTimeline_->changeSmoothnessOfKeyValue(smooth, keyframes_[currentItem_]);

}

template class TemplatePropertyTimelineWidget<float>;
template class TemplatePropertyTimelineWidget<int>;
template class TemplatePropertyTimelineWidget<bool>;
template class TemplatePropertyTimelineWidget<tgt::ivec2>;
template class TemplatePropertyTimelineWidget<tgt::ivec3>;
template class TemplatePropertyTimelineWidget<tgt::ivec4>;
template class TemplatePropertyTimelineWidget<tgt::vec2>;
template class TemplatePropertyTimelineWidget<tgt::vec3>;
template class TemplatePropertyTimelineWidget<tgt::vec4>;
template class TemplatePropertyTimelineWidget<tgt::Camera>;
template class TemplatePropertyTimelineWidget<std::string>;
template class TemplatePropertyTimelineWidget<ShaderSource>;
template class TemplatePropertyTimelineWidget<TransFunc*>;
//template class TemplatePropertyTimelineWidget<VolumeCollection*>;
//template class TemplatePropertyTimelineWidget<Volume*>;

} //namespace voreen

