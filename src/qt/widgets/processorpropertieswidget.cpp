/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "voreen/qt/widgets/processorpropertieswidget.h"
#include "voreen/qt/widgets/expandableheaderbutton.h"
#include "voreen/qt/widgets/qpropertywidget.h"
#include "voreen/core/vis/processors/processor.h"
#include <QVBoxLayout>

namespace voreen {

ProcessorPropertiesWidget::ProcessorPropertiesWidget(QWidget* parent, Processor* processor,
                                                     PropertyWidgetFactory* widgetFactory, bool expanded, 
                                                     bool userExpandable)
    : QWidget(parent)
    , propertyWidget_(0)
    , processor_(processor)
{
    setObjectName("ProcessorTitleWidget");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);

    setUpdatesEnabled(false);

    header_ = new ExpandableHeaderButton(processor->getName().c_str(), this, expanded, userExpandable);
    connect(header_, SIGNAL(toggled(bool)), this, SLOT(updateState()));
    connect(header_, SIGNAL(setLODHidden()), this, SLOT(setLODHidden()));
    connect(header_, SIGNAL(setLODVisible()), this, SLOT(setLODVisible()));
    header_->showLODControls();
    mainLayout->addWidget(header_);

    // property widget
    propertyWidget_ = new QWidget;
    QVBoxLayout* vbox = new QVBoxLayout(propertyWidget_);
    std::vector<Property*> propertyList = processor->getProperties();

    // create widget for every property and put them into a vertical layout
    for (std::vector<Property*>::iterator iter = propertyList.begin(); iter != propertyList.end(); ++iter) {
        QPropertyWidget* w = dynamic_cast<QPropertyWidget*>((*iter)->createAndAddWidget(widgetFactory));
        if (w != 0) {
            connect(w, SIGNAL(propertyChanged()), this, SIGNAL(propertyChanged()));
            widgets_.push_back(w);
            vbox->addWidget(w);
        }
    }

    mainLayout->addWidget(propertyWidget_);

    setUpdatesEnabled(true);

    updateState();
}

void ProcessorPropertiesWidget::setLevelOfDetail(Property::LODSetting lod) {
    const std::vector<Property*> propertyList = processor_->getProperties();
    for (size_t i = 0; i < propertyList.size(); ++i) {
        std::set<PropertyWidget*> set = propertyList[i]->getPropertyWidgets(); // This set usually contains only one property
        for (std::set<PropertyWidget*>::iterator iter = set.begin(); iter != set.end(); ++iter) {
            QPropertyWidget* wdt = dynamic_cast<QPropertyWidget*>(*iter);
            for (std::vector<QPropertyWidget*>::iterator innerIter = widgets_.begin(); innerIter != widgets_.end(); ++innerIter) {
                if (wdt == (*innerIter)) {
                    if (lod == Property::USER)
                        wdt->hideLODControls();
                    else
                        wdt->showLODControls();
                    wdt->setVisible((propertyList[i]->isVisible() && (propertyList[i]->getLevelOfDetail() <= lod)));
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
        headerVisible |= (propertyList[i]->isVisible() && (propertyList[i]->getLevelOfDetail() <= lod));
    }

    setVisible(headerVisible);
}

void ProcessorPropertiesWidget::updateState() {
    propertyWidget_->setVisible(header_->isExpanded());
}

void ProcessorPropertiesWidget::setLODHidden() {
    const std::vector<Property*> propertyList = processor_->getProperties();
    for (size_t i = 0; i < propertyList.size(); ++i) {
        std::set<PropertyWidget*> set = propertyList[i]->getPropertyWidgets(); // This set usually contains only one property
        for (std::set<PropertyWidget*>::iterator iter = set.begin(); iter != set.end(); ++iter) {
            QPropertyWidget* wdt = dynamic_cast<QPropertyWidget*>(*iter);
            for (std::vector<QPropertyWidget*>::iterator innerIter = widgets_.begin(); innerIter != widgets_.end(); ++innerIter) {
                if (wdt == (*innerIter)) {
                    wdt->setLevelOfDetail(Property::USER);
                }
            }
        }
    }
}

void ProcessorPropertiesWidget::setLODVisible() {
    const std::vector<Property*> propertyList = processor_->getProperties();
    for (size_t i = 0; i < propertyList.size(); ++i) {
        std::set<PropertyWidget*> set = propertyList[i]->getPropertyWidgets(); // This set usually contains only one property
        for (std::set<PropertyWidget*>::iterator iter = set.begin(); iter != set.end(); ++iter) {
            QPropertyWidget* wdt = dynamic_cast<QPropertyWidget*>(*iter);
            for (std::vector<QPropertyWidget*>::iterator innerIter = widgets_.begin(); innerIter != widgets_.end(); ++innerIter) {
                if (wdt == (*innerIter)) {
                    wdt->setLevelOfDetail(Property::DEVELOPER);
                }
            }
        }
    }    
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

} // namespace
