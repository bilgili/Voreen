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

#include "voreen/qt/widgets/customlabel.h"

#include "voreen/qt/widgets/expandableheaderbutton.h"
#include "voreen/qt/widgets/property/lightpropertywidget.h"
#include "voreen/qt/widgets/property/processorpropertieswidget.h"
#include "voreen/qt/widgets/property/propertyvectorwidget.h"
#include "voreen/qt/widgets/property/qpropertywidget.h"
#include "voreen/qt/widgets/property/qpropertywidgetfactory.h"
#include "voreen/qt/widgets/property/grouppropertywidget.h"
#include "voreen/qt/widgets/property/transfuncpropertywidget.h"
#include "voreen/qt/widgets/property/volumecollectionpropertywidget.h"
#include "voreen/qt/widgets/property/volumehandlepropertywidget.h"

#include "voreen/core/interaction/interactionhandler.h"
#include "voreen/core/processors/processor.h"
#include "voreen/core/datastructures/volume/volumecontainer.h"

#include <QVBoxLayout>
#include <QGridLayout>

namespace voreen {

ProcessorPropertiesWidget::ProcessorPropertiesWidget(QWidget* parent, const Processor* processor,
                                                     bool expanded, bool userExpandable)
    : QWidget(parent)
    , propertyWidget_(0)
    , processor_(processor)
    , volumeContainer_(0)
    , expanded_(expanded)
    , userExpandable_(userExpandable)
    , widgetFactory_(0)
    , widgetInstantiationState_(NONE)
{

    setObjectName("ProcessorTitleWidget");
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(0, 0, 0, 0);
    mainLayout_->setSpacing(0);
    mainLayout_->setMargin(0);

    setUpdatesEnabled(false);

    header_ = new ExpandableHeaderButton(processor_->getName().c_str(), this,
            expanded_, userExpandable_);
    connect(header_, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    connect(header_, SIGNAL(setLODHidden()), this, SLOT(setLODHidden()));
    connect(header_, SIGNAL(setLODVisible()), this, SLOT(setLODVisible()));
    header_->showLODControls();
    mainLayout_->addWidget(header_);
}

void ProcessorPropertiesWidget::setLevelOfDetail(Property::LODSetting lod) {

    std::vector<Property*> propertyList(processor_->getProperties());

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

    setVisible(headerVisible);
}

void ProcessorPropertiesWidget::showHeader(bool visible) {
    if (visible)
        header_->showLODControls();
    else
        header_->hideLODControls();
}

void ProcessorPropertiesWidget::setVolumeContainer(VolumeContainer* volCon) {
    volumeContainer_ = volCon;
}

void ProcessorPropertiesWidget::updateState() {
    if(propertyWidget_!=0)
        propertyWidget_->setVisible(header_->isExpanded());
    updateGeometry();  // prevent flicker when hiding property widgets
}

void ProcessorPropertiesWidget::setLODHidden() {

    std::vector<Property*> propertyList(processor_->getProperties());
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

    std::vector<Property*> propertyList(processor_->getProperties());

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
    header_->updateNameLabel(processor_->getName());
}

void ProcessorPropertiesWidget::propertyModified() {
    emit modified();
}

void ProcessorPropertiesWidget::instantiateWidgets() {
setUpdatesEnabled(false);
if(widgetInstantiationState_ == NONE) {
    propertyWidget_ = new QWidget;
    QGridLayout* gridLayout = new QGridLayout(propertyWidget_);
    gridLayout->setContentsMargins(3, 4, 0, 2);
    gridLayout->setSpacing(2);
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 2);
    gridLayout->setEnabled(false);
    std::vector<Property*> propertyList = processor_->getProperties();

    // create widget for every property and put them into a vertical layout
    int rows = 0;
    widgetFactory_ = new QPropertyWidgetFactory();
    for (std::vector<Property*>::iterator iter = propertyList.begin(); iter
            != propertyList.end(); ++iter) {
                if (dynamic_cast<TransFuncProperty*>(*iter)) {
                    QPropertyWidget* w =
                            dynamic_cast<QPropertyWidget*>((*iter)->createAndAddWidget(widgetFactory_));
                    if (w != 0) {
                        widgets_.push_back(w);
                        connect(w, SIGNAL(modified()), this, SLOT(propertyModified()));
                        QLabel* nameLabel = const_cast<QLabel*>(w->getNameLabel());
                        gridLayout->addWidget(w, rows, 1, 1, 1);
                        gridLayout->addWidget(nameLabel, rows, 0, 1, 1);
                    }
                }
                ++rows;
    }
    gridLayout->setEnabled(true);

    delete widgetFactory_;
    widgetFactory_ = 0;
    mainLayout_->addWidget(propertyWidget_);
    setUpdatesEnabled(true);
    updateState();
    widgetInstantiationState_ = ONLY_TF;
}
else if(widgetInstantiationState_ == ONLY_TF) {
    QGridLayout* gridLayout = dynamic_cast<QGridLayout*>(propertyWidget_->layout());
    std::vector<Property*> propertyList = processor_->getProperties();

    // create widget for every property and put them into a vertical layout
    int rows = 0;
    widgetFactory_ = new QPropertyWidgetFactory();
    for (std::vector<Property*>::iterator iter = propertyList.begin(); iter
            != propertyList.end(); ++iter) {
                if(!dynamic_cast<TransFuncProperty*>(*iter)) {
                    QPropertyWidget* w =
                            dynamic_cast<QPropertyWidget*>((*iter)->createAndAddWidget(widgetFactory_));
                    if (w != 0) {
                        widgets_.push_back(w);
                        connect(w, SIGNAL(modified()), this, SLOT(propertyModified()));
                        // we are dealing with a propertygroup
                        if((*iter)->getGroupID() != "") {
                            std::map<std::string, GroupPropertyWidget*>::iterator it = propertyGroupsMap_.find((*iter)->getGroupID());
                            // the group does not exist
                            if(it == propertyGroupsMap_.end()) {
                                std::string guiName = (*iter)->getOwner()->getPropertyGroupGuiName((*iter)->getGroupID());
                                propertyGroupsMap_[(*iter)->getGroupID()] = new GroupPropertyWidget((*iter), false, guiName, this);
                                (*iter)->getOwner()->setPropertyGroupWidget((*iter)->getGroupID(), propertyGroupsMap_[(*iter)->getGroupID()]);
                                // the group is not visible
                                if(!(*iter)->getOwner()->isPropertyGroupVisible((*iter)->getGroupID())) {
                                    propertyGroupsMap_[(*iter)->getGroupID()]->setVisible(false);
                                }
                            }
                            propertyGroupsMap_[(*iter)->getGroupID()]->addWidget(w, const_cast<QLabel*>(w->getNameLabel()), QString::fromStdString(w->getPropertyGuiName()));
                            const_cast<QLabel*>(w->getNameLabel())->setMinimumWidth(60);
                            gridLayout->addWidget(propertyGroupsMap_[(*iter)->getGroupID()], rows, 0, 1, 2);
                            propertyGroupsMap_[(*iter)->getGroupID()]->setVisible((*iter)->getOwner()->isPropertyGroupVisible((*iter)->getGroupID()));
                        }
                        else {
                            QLabel* nameLabel = const_cast<QLabel*>(w->getNameLabel());

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
                                gridLayout->addWidget(nameLabel, rows, 0, 1, 1);
                                ++rows;
                                gridLayout->addWidget(w, rows, 0, 1, 2);

                            }

                            if (dynamic_cast<VolumeHandlePropertyWidget*>(w)){
                                if(volumeContainer_)
                                    static_cast<VolumeHandlePropertyWidget*>(w)->setVolumeContainer(volumeContainer_);
                            }
                            else if (dynamic_cast<VolumeCollectionPropertyWidget*>(w)){
                                if(volumeContainer_)
                                    static_cast<VolumeCollectionPropertyWidget*>(w)->setVolumeContainer(volumeContainer_);
                            }
                    }

                    }
            }
            ++rows;
        }

        delete widgetFactory_;
        widgetFactory_ = 0;
        setUpdatesEnabled(true);
        updateState();
        widgetInstantiationState_ = ALL;
    }
    setUpdatesEnabled(true);
}

void ProcessorPropertiesWidget::showEvent(QShowEvent* event) {
    instantiateWidgets();
    QWidget::showEvent(event);
}

} // namespace
