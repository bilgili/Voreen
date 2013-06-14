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

#include "voreen/qt/widgets/customlabel.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/voreenmodule.h"
#include "voreen/core/interaction/interactionhandler.h"
#include "voreen/core/processors/processor.h"
#include "voreen/core/ports/port.h"

#include "voreen/qt/widgets/expandableheaderbutton.h"
#include "voreen/qt/widgets/property/lightpropertywidget.h"
#include "voreen/qt/widgets/property/processorpropertieswidget.h"
#include "voreen/qt/widgets/property/propertyvectorwidget.h"
#include "voreen/qt/widgets/property/qpropertywidget.h"
#include "voreen/qt/widgets/property/grouppropertywidget.h"
#include "voreen/core/ports/renderport.h" //for size properties

#include <QVBoxLayout>
#include <QGridLayout>

namespace voreen {

ProcessorPropertiesWidget::ProcessorPropertiesWidget(QWidget* parent, const Processor* processor,
                                                     bool expanded, bool userExpandable)
    : QWidget(parent)
    , ProcessorObserver()
    , propertyWidget_(0)
    , processor_(processor)
    , expanded_(expanded)
    , userExpandable_(userExpandable)
    , widgetInstantiationState_(NONE)
{
    tgtAssert(processor_, "null pointer passed");

    setObjectName("ProcessorTitleWidget");
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(0, 0, 0, 0);
    mainLayout_->setSpacing(0);
    mainLayout_->setMargin(0);

    setUpdatesEnabled(false);

    header_ = new ExpandableHeaderButton(processor_->getGuiName().c_str(), this,
            expanded_, userExpandable_);
    connect(header_, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    connect(header_, SIGNAL(toggled(bool)), this, SLOT(widgetInstantiation()));
    connect(header_, SIGNAL(setLODHidden()), this, SLOT(setLODHidden()));
    connect(header_, SIGNAL(setLODVisible()), this, SLOT(setLODVisible()));
    header_->showLODControls();
    mainLayout_->addWidget(header_);

    static_cast<Observable<ProcessorObserver>*>(const_cast<Processor*>(processor_))->addObserver(this);
}

void ProcessorPropertiesWidget::setVisibilityOfGroups() {
    for(std::map<std::string, GroupPropertyWidget*>::iterator it = propertyGroupsMap_.begin(); it != propertyGroupsMap_.end(); ++it) {
        std::string groupID = it->first;
        GroupPropertyWidget* groupWidget = it->second;
        tgtAssert(groupWidget, "null pointer in propertyGroupsMap_");
        Property* property = groupWidget->getProperty();
        tgtAssert(property, "group widget has no property");
        PropertyOwner* owner = property->getOwner();
        tgtAssert(owner, "property has no owner");
        groupWidget->setVisible(owner->isPropertyGroupVisible(groupID));
    }
}

void ProcessorPropertiesWidget::setLevelOfDetail(Property::LODSetting lod) {

    std::vector<Property*> propertyList = processor_->getProperties();
    for (size_t i=0; i<processor_->getPorts().size(); i++) {
        std::vector<Property*> portProps = processor_->getPorts().at(i)->getProperties();
        propertyList.insert(propertyList.end(), portProps.begin(), portProps.end());
    }

    // update property widgets visibility and LOD controls
    for (size_t i = 0; i < propertyList.size(); ++i) {
        std::set<PropertyWidget*> set = propertyList[i]->getPropertyWidgets(); // This set usually contains only one property
        for (std::set<PropertyWidget*>::iterator iter = set.begin(); iter
                != set.end(); ++iter) {
            QPropertyWidget* wdt = dynamic_cast<QPropertyWidget*> (*iter);
            for (std::vector<QPropertyWidget*>::iterator innerIter =
                    widgets_.begin(); innerIter != widgets_.end(); ++innerIter) {
                if (wdt == (*innerIter)) {
                    if (lod == Property::USER)
                        wdt->hideLODControls();
                    //FIXME: do this only when lod controls are not disabled in propertylistwidget
                    // else
                    //     wdt->showLODControls();
                    wdt->setVisible((propertyList[i]->isVisible()
                            && (propertyList[i]->getLevelOfDetail() <= lod)));
                    wdt->showNameLabel(propertyList[i]->isVisible()
                            && (propertyList[i]->getLevelOfDetail() <= lod));

                }
            }
        }
    }

    if (lod == Property::USER)
        header_->hideLODControls();
    else
        header_->showLODControls();

    bool headerVisible = false;

    for (size_t i = 0; i < propertyList.size(); ++i) {
        headerVisible |= (propertyList[i]->isVisible()
                && (propertyList[i]->getLevelOfDetail() <= lod));
    }

    std::map<std::string, GroupPropertyWidget*>::iterator it;
    for(it = propertyGroupsMap_.begin(); it != propertyGroupsMap_.end(); it++) {
        GroupPropertyWidget* gpw = it->second;

        if(gpw) {
            if (lod == Property::USER) {
                if(gpw->isAnyPropertyVisible(lod))
                    gpw->setVisible(true);
                else
                    gpw->setVisible(false);
            }
            else
                gpw->setVisible(true);
        }
    }


    setVisible(headerVisible);
}

void ProcessorPropertiesWidget::showHeader(bool visible) {
    if (visible)
        header_->showLODControls();
    else
        header_->hideLODControls();
}

void ProcessorPropertiesWidget::updateState() {
    if(propertyWidget_!=0) {
        propertyWidget_->setVisible(header_->isExpanded());
    }
    updateGeometry();  // prevent flicker when hiding property widgets
}

void ProcessorPropertiesWidget::setLODHidden() {

    std::vector<Property*> propertyList = processor_->getProperties();
    for (size_t i=0; i<processor_->getPorts().size(); i++) {
        std::vector<Property*> portProps = processor_->getPorts().at(i)->getProperties();
        propertyList.insert(propertyList.end(), portProps.begin(), portProps.end());
    }

    // update property widgets LOD level
    for (size_t i = 0; i < propertyList.size(); ++i) {
        std::set<PropertyWidget*> set = propertyList[i]->getPropertyWidgets(); // This set usually contains only one property
        for (std::set<PropertyWidget*>::iterator iter = set.begin(); iter
                != set.end(); ++iter) {
            QPropertyWidget* wdt = dynamic_cast<QPropertyWidget*> (*iter);
            for (std::vector<QPropertyWidget*>::iterator innerIter =
                    widgets_.begin(); innerIter != widgets_.end(); ++innerIter) {
                if (wdt == (*innerIter)) {
                    wdt->setLevelOfDetail(Property::DEVELOPER);
                }
            }
        }
    }
    header_->hideLODControls();
}

void ProcessorPropertiesWidget::setLODVisible() {

    std::vector<Property*> propertyList = processor_->getProperties();
    for (size_t i=0; i<processor_->getPorts().size(); i++) {
        std::vector<Property*> portProps = processor_->getPorts().at(i)->getProperties();
        propertyList.insert(propertyList.end(), portProps.begin(), portProps.end());
    }

    // update property widgets LOD level
    for (size_t i = 0; i < propertyList.size(); ++i) {
        std::set<PropertyWidget*> set = propertyList[i]->getPropertyWidgets(); // This set usually contains only one property
        for (std::set<PropertyWidget*>::iterator iter = set.begin(); iter
                != set.end(); ++iter) {
            QPropertyWidget* wdt = dynamic_cast<QPropertyWidget*> (*iter);
            for (std::vector<QPropertyWidget*>::iterator innerIter =
                    widgets_.begin(); innerIter != widgets_.end(); ++innerIter) {
                if (wdt == (*innerIter)) {
                    wdt->setLevelOfDetail(Property::USER);
                }
            }
        }
    }
    header_->showLODControls();
}

bool ProcessorPropertiesWidget::isExpanded() const {
    return header_->isExpanded();
}

void ProcessorPropertiesWidget::setExpanded(bool expanded) {
    header_->setExpanded(expanded);
    updateState();
}

void ProcessorPropertiesWidget::toggleExpansionState() {
    header_->setExpanded(!header_->isExpanded());
    updateState();
}

bool ProcessorPropertiesWidget::isUserExpandable() const {
    return header_->userExpandable();
}

void ProcessorPropertiesWidget::setUserExpandable(bool expandable) {
    header_->setUserExpandable(expandable);
    updateState();
}

void ProcessorPropertiesWidget::updateHeaderTitle() {
    header_->updateNameLabel(processor_->getGuiName());
}

void ProcessorPropertiesWidget::propertyModified() {
    emit modified();
}

void ProcessorPropertiesWidget::widgetInstantiation() {
    instantiateWidgets();
}

void ProcessorPropertiesWidget::instantiateWidgets() {
    setUpdatesEnabled(false);
    if (!VoreenApplication::app()) {
        LERRORC("voreen.qt.ProcessorPropertiesWidget", "VoreenApplication not instantiated");
        return;
    }
    if (widgetInstantiationState_ == NONE) {
        propertyWidget_ = new QWidget;
        QGridLayout* gridLayout = new QGridLayout(propertyWidget_);
        gridLayout->setContentsMargins(3, 4, 0, 2);
        gridLayout->setSpacing(2);
        gridLayout->setColumnStretch(0, 1);
        gridLayout->setColumnStretch(1, 2);
        gridLayout->setEnabled(false);
        std::vector<Property*> propertyList = processor_->getProperties();
        for (size_t i=0; i<processor_->getPorts().size(); i++) {
            std::vector<Property*> portProps = processor_->getPorts().at(i)->getProperties();
            propertyList.insert(propertyList.end(), portProps.begin(), portProps.end());
        }

        // create widget for every property and put them into a vertical layout
        int rows = 0;

        // create non-lazy widgets (such as widgets with editor windows)
        for (std::vector<Property*>::iterator iter = propertyList.begin(); iter != propertyList.end(); ++iter) {
            Property* prop = *iter;
            if (!VoreenApplication::app()->lazyInstantiation(prop)) {
                PropertyWidget* propWidget = VoreenApplication::app()->createPropertyWidget(prop);
                if (propWidget)
                    prop->addWidget(propWidget);

                QPropertyWidget* w = dynamic_cast<QPropertyWidget*>(propWidget);
                if (w != 0) {
                    widgets_.push_back(w);
                    connect(w, SIGNAL(modified()), this, SLOT(propertyModified()));
                    connect(w,SIGNAL(visibilityChanged()),this,SLOT(setVisibilityOfGroups()));
                    // we are dealing with a propertygroup
                    if (prop->getGroupID() != "") {
                        std::map<std::string, GroupPropertyWidget*>::iterator it = propertyGroupsMap_.find(prop->getGroupID());
                        // the group does not exist
                        if (it == propertyGroupsMap_.end()) {
                            std::string guiName = prop->getOwner()->getPropertyGroupGuiName(prop->getGroupID());
                            propertyGroupsMap_[prop->getGroupID()] = new GroupPropertyWidget(prop, false, guiName, this);
                            //prop->getOwner()->setPropertyGroupWidget(prop->getGroupID(), propertyGroupsMap_[prop->getGroupID()]);
                            // the group is not visible
                            if(!prop->getOwner()->isPropertyGroupVisible(prop->getGroupID())) {
                                propertyGroupsMap_[prop->getGroupID()]->setVisible(false);
                            }
                        }
                        propertyGroupsMap_[prop->getGroupID()]->addWidget(w, w->getNameLabel(), QString::fromStdString(w->getPropertyGuiName()));
                        if(w->getNameLabel())
                            w->getNameLabel()->setMinimumWidth(60);
                        gridLayout->addWidget(propertyGroupsMap_[prop->getGroupID()], rows, 0, 1, 2);
                        propertyGroupsMap_[prop->getGroupID()]->setVisible(prop->getOwner()->isPropertyGroupVisible(prop->getGroupID()));
                    }
                    else {
                        CustomLabel* nameLabel = w->getNameLabel();
                        if(nameLabel) {
                            gridLayout->addWidget(w, rows, 1, 1, 1);
                            gridLayout->addWidget(nameLabel, rows, 0, 1, 1);
                        }
                        else {
                            gridLayout->addWidget(w, rows, 0, 1, 2);
                        }
                    }
                }
            }
            ++rows;
        }
        gridLayout->setEnabled(true);

        mainLayout_->addWidget(propertyWidget_);
        setUpdatesEnabled(true);
        updateState();
        widgetInstantiationState_ = ONLY_NONLAZY;
    }
    else if(widgetInstantiationState_ == ONLY_NONLAZY) {
        // now, all remaining lazy instantiation widgets must be created
        QGridLayout* gridLayout = dynamic_cast<QGridLayout*>(propertyWidget_->layout());
        std::vector<Property*> propertyList = processor_->getProperties();
        for (size_t i=0; i<processor_->getPorts().size(); i++) {
            std::vector<Property*> portProps = processor_->getPorts().at(i)->getProperties();
            propertyList.insert(propertyList.end(), portProps.begin(), portProps.end());
        }

        // create widget for every property and put them into a vertical layout
        int rows = 0;
        for (std::vector<Property*>::iterator iter = propertyList.begin(); iter != propertyList.end(); ++iter) {
            Property* prop = *iter;
            //if (dynamic_cast<RenderSizeOriginProperty*>(prop) || dynamic_cast<RenderSizeReceiveProperty*>(prop)) continue;
            if (VoreenApplication::app()->lazyInstantiation(prop)) {
                PropertyWidget* propWidget = VoreenApplication::app()->createPropertyWidget(prop);
                if (propWidget)
                    prop->addWidget(propWidget);
                QPropertyWidget* w = dynamic_cast<QPropertyWidget*>(propWidget);
                if (w != 0) {
                    widgets_.push_back(w);
                    connect(w, SIGNAL(modified()), this, SLOT(propertyModified()));
                    connect(w,SIGNAL(visibilityChanged()),this,SLOT(setVisibilityOfGroups()));
                    // we are dealing with a propertygroup
                    if (prop->getGroupID() != "") {
                        std::map<std::string, GroupPropertyWidget*>::iterator it = propertyGroupsMap_.find(prop->getGroupID());
                        // the group does not exist
                        if (it == propertyGroupsMap_.end()) {
                            std::string guiName = prop->getOwner()->getPropertyGroupGuiName(prop->getGroupID());
                            propertyGroupsMap_[prop->getGroupID()] = new GroupPropertyWidget(prop, false, guiName, this);
                            //prop->getOwner()->setPropertyGroupWidget(prop->getGroupID(), propertyGroupsMap_[prop->getGroupID()]);
                            // the group is not visible
                            if(!prop->getOwner()->isPropertyGroupVisible(prop->getGroupID())) {
                                propertyGroupsMap_[prop->getGroupID()]->setVisible(false);
                            }
                        }
                        propertyGroupsMap_[prop->getGroupID()]->addWidget(w, w->getNameLabel(), QString::fromStdString(w->getPropertyGuiName()));
                        if(w->getNameLabel())
                            w->getNameLabel()->setMinimumWidth(60);
                        gridLayout->addWidget(propertyGroupsMap_[prop->getGroupID()], rows, 0, 1, 2);
                        propertyGroupsMap_[prop->getGroupID()]->setVisible(prop->getOwner()->isPropertyGroupVisible(prop->getGroupID()));
                    }
                    else {
                        CustomLabel* nameLabel = w->getNameLabel();

                        if(dynamic_cast<LightPropertyWidget*>(w)) {     // HACK: this prevents a cut off gui element e.g. seen in the clipping plane widget
                            gridLayout->setRowStretch(rows, 1);
                        }
                        if(!dynamic_cast<PropertyVectorWidget*>(w)) {
                            if (nameLabel) {
                                gridLayout->addWidget(nameLabel, rows, 0, 1, 1);
                                gridLayout->addWidget(w, rows, 1, 1, 1);
                            }
                            else {
                                gridLayout->addWidget(w, rows, 0, 1, 2);
                            }
                        }
                        else {
                            if (nameLabel) {
                                gridLayout->addWidget(nameLabel, rows, 0, 1, 1);
                                ++rows;
                                gridLayout->addWidget(w, rows, 1, 1, 1);
                            }
                            else {
                                gridLayout->addWidget(w, rows, 0, 1, 2);
                            }
                            //gridLayout->addWidget(nameLabel, rows, 0, 1, 1);
                            //++rows;
                            //gridLayout->addWidget(w, rows, 0, 1, 2);
                        }
                    }

                }
            }
            ++rows;
        }

        setUpdatesEnabled(true);
        //updateState();
        widgetInstantiationState_ = ALL;
    }
    setUpdatesEnabled(true);
}

void ProcessorPropertiesWidget::showEvent(QShowEvent* event) {
    instantiateWidgets();
    QWidget::showEvent(event);
}

void ProcessorPropertiesWidget::processorWidgetCreated(const Processor*) {}
void ProcessorPropertiesWidget::processorWidgetDeleted(const Processor*) {}

void ProcessorPropertiesWidget::portsChanged(const Processor*) {
}

void ProcessorPropertiesWidget::propertiesChanged(const PropertyOwner*) {

    if (!VoreenApplication::app()) {
        LERRORC("voreen.qt.ProcessorPropertiesWidget", "VoreenApplication not instantiated");
        return;
    }

    if(!propertyWidget_) {
        LERRORC("voreen.qt.ProcessorPropertiesWidget", "Property widget not instantiated");
        return;
    }

    QGridLayout* gridLayout = dynamic_cast<QGridLayout*>(propertyWidget_->layout());
    std::vector<Property*> propertyList = processor_->getProperties();
    for (size_t i=0; i<processor_->getPorts().size(); i++) {
        std::vector<Property*> portProps = processor_->getPorts().at(i)->getProperties();
        propertyList.insert(propertyList.end(), portProps.begin(), portProps.end());
    }
    for (unsigned int i=0; i<propertyList.size(); i++) {
        Property* prop = propertyList.at(i);
        const std::set<PropertyWidget*> propWidgets = prop->getPropertyWidgets();
        if (propWidgets.empty()) {
            PropertyWidget* propWidget = VoreenApplication::app()->createPropertyWidget(prop);
            if (propWidget)
                prop->addWidget(propWidget);
            QPropertyWidget* w = dynamic_cast<QPropertyWidget*>(propWidget);
            if (w) {
                widgets_.push_back(w);
                connect(w, SIGNAL(modified()), this, SLOT(propertyModified()));
                CustomLabel* nameLabel = w->getNameLabel();
                int curRow = gridLayout->rowCount();
                if(nameLabel) {
                    gridLayout->addWidget(nameLabel, curRow, 0, 1, 1);
                    gridLayout->addWidget(w, curRow, 1, 1, 1);
                }
                else {
                    gridLayout->addWidget(w, curRow, 0, 1, 2);
                }
            }
        }
    }
}

} // namespace
