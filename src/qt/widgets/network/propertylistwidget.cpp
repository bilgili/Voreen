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

#include "voreen/qt/widgets/network/propertylistwidget.h"

#include "voreen/qt/widgets/compactproperty/compactpropertywidgetfactory.h"
#include "voreen/qt/widgets/qpropertywidget.h"
#include "voreen/qt/widgets/processorpropertieswidget.h"

#include <QVBoxLayout>

namespace voreen {

PropertyListWidget::PropertyListWidget(QWidget* parent, ProcessorNetwork* processorNet, PropertyListWidget::WidgetMode mode, Property::LODSetting lod)
    : QScrollArea(parent)
    , processorNetwork_(processorNet)
    , widgetMode_(mode)
    , levelOfDetail_(lod)
    , containerWidget_(0)
    , lastSelectedProcessor_(0)
{

    if (processorNetwork_)
        processorNetwork_->addObserver(this);

    setMinimumWidth(330);
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    createWidgets();
    updateState();
}

PropertyListWidget::~PropertyListWidget() {
    
    // delete all widgets
    for (std::map<Processor*, ProcessorPropertiesWidget*>::iterator it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it)
        delete it->second;

    // deregister from network
    if (processorNetwork_)
        processorNetwork_->removeObserver(this);

    processorWidgetMap_.clear();
}

void PropertyListWidget::setNetwork(ProcessorNetwork* network) {

    // update network and register as observer
    processorNetwork_ = network;
    if (processorNetwork_)
        processorNetwork_->addObserver(this);

    // update the widget's state
    networkChanged();
}

void PropertyListWidget::networkChanged() {
    clear();
    createWidgets();
    updateState();
}

void PropertyListWidget::processorAdded(Processor* processor) {
    CompactPropertyWidgetFactory* factory = new CompactPropertyWidgetFactory;

    // generate header widget for the new processor and insert it into the widget map
    ProcessorPropertiesWidget* headerWidget = new ProcessorPropertiesWidget(this, processor, factory, false, widgetMode_ == LIST);
    headerWidget->setVisible(widgetMode_ == LIST);
    connect(headerWidget, SIGNAL(propertyChanged()), this, SIGNAL(repaintSignal()));
    processorWidgetMap_.insert(std::make_pair(processor, headerWidget));

    // insert header widget before stretch (last item in the layout)
    QVBoxLayout* mainLayout = dynamic_cast<QVBoxLayout*>(widget()->layout());
    if (mainLayout)
        mainLayout->insertWidget(mainLayout->count()-1, headerWidget);

    delete factory;
}


void PropertyListWidget::processorRemoved(Processor* processor){
    // delete processor's header widget and remove it from the map
    std::map<Processor*, ProcessorPropertiesWidget*>::iterator it = processorWidgetMap_.find(processor);
    if (it != processorWidgetMap_.end()) {
        if (widget() && widget()->layout())
            widget()->layout()->removeWidget(it->second);
        delete it->second;
        processorWidgetMap_.erase(it);
    }
}

void PropertyListWidget::processorSelected(Processor* processor) {
    if (processor) {
        // expand property widget of selected processor
        std::map<Processor*, ProcessorPropertiesWidget*>::iterator it = processorWidgetMap_.find(processor);
        if (it == processorWidgetMap_.end())
            return;

        if (widgetMode_ == LIST) {
            it->second->setExpanded(true);
        }
        else {
            ProcessorPropertiesWidget* headerWidget = it->second;
            for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it) {
                it->second->setVisible(it->second == headerWidget);          
                it->second->setExpanded(it->second == headerWidget);          
            }
        }
        lastSelectedProcessor_ = processor;
    }
}

void PropertyListWidget::clear() {   
    QVBoxLayout* containerLayout = 0;
    if (containerWidget_)
        containerLayout = dynamic_cast<QVBoxLayout*>(containerWidget_->layout());

    setUpdatesEnabled(false);

    //delete all widgets and remove them from container layout
    std::map<Processor*, ProcessorPropertiesWidget*>::iterator it;
    for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it) {
        if (containerLayout)
            containerLayout->removeWidget(it->second);
        delete it->second;
    }

    // remove remaining layout items from container layout
    if (containerLayout)
        while (containerLayout->count() > 0)
            containerLayout->removeItem(containerLayout->itemAt(0));
    
    setUpdatesEnabled(true);

    processorWidgetMap_.clear();
    lastSelectedProcessor_ = 0;
}

void PropertyListWidget::createWidgets() { 
    if (!processorNetwork_)
        return;

    // generate container for header widgets
    QVBoxLayout* containerLayout = 0;
    if (!containerWidget_) {
        containerWidget_ = new QWidget;
        containerLayout = new QVBoxLayout(containerWidget_);
        setWidget(containerWidget_);
    }
    else {
        containerLayout = dynamic_cast<QVBoxLayout*>(containerWidget_->layout());
    }

    if (!containerLayout)
        return;

    // generate processor property widgets: each processor property contains a processor's properties
    // along with a expansion header
    CompactPropertyWidgetFactory* factory = new CompactPropertyWidgetFactory;
    setUpdatesEnabled(false);
    for (int i=0; i<processorNetwork_->getNumProcessors(); ++i) {
        Processor* proc = processorNetwork_->getProcessors().at(i);
        ProcessorPropertiesWidget* headerWidget = new ProcessorPropertiesWidget(this, proc, factory, false, true);
        connect(headerWidget, SIGNAL(propertyChanged()), this, SIGNAL(repaintSignal()));
        //connect(graphicsProcessorNet_->processorItems[i], SIGNAL(processorNameChanged()), headerWidget, SLOT(updateHeaderTitle()));
        containerLayout->addWidget(headerWidget);
        processorWidgetMap_.insert(std::make_pair(proc, headerWidget));
    }
    containerLayout->addStretch();
    delete factory;

    setLevelOfDetail(levelOfDetail_);
    setUpdatesEnabled(true);
}

void PropertyListWidget::hideAll() {
    std::map<Processor*, ProcessorPropertiesWidget*>::iterator it;
    for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it)
        it->second->setVisible(false);
}

void PropertyListWidget::showAll() {
    std::map<Processor*, ProcessorPropertiesWidget*>::iterator it;
    for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it)
        it->second->setVisible(true);
}

void PropertyListWidget::setAllExpandable() {
    std::map<Processor*, ProcessorPropertiesWidget*>::iterator it;
    for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it)
        it->second->setUserExpandable(true);
}

void PropertyListWidget::setAllUnexpandable() {
    std::map<Processor*, ProcessorPropertiesWidget*>::iterator it;
    for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it)
        it->second->setUserExpandable(false);
}

void PropertyListWidget::updateState() {
    
    setUpdatesEnabled(false);

    if (widgetMode_ == LIST) {
        showAll();
        setAllExpandable();
    }
    else if (widgetMode_ == SINGLE) {
        hideAll();
        setAllUnexpandable();
    }

    setUpdatesEnabled(true);

}

PropertyListWidget::WidgetMode PropertyListWidget::widgetMode() const {
    return widgetMode_;
}

void PropertyListWidget::setWidgetMode(WidgetMode mode) {
    if (widgetMode_ != mode) {       
        widgetMode_ = mode;
        updateState();

        if (mode == SINGLE && processorWidgetMap_.find(lastSelectedProcessor_) != processorWidgetMap_.end())
            processorSelected(lastSelectedProcessor_);        
    }
}

void PropertyListWidget::setLevelOfDetail(Property::LODSetting lod) {
    levelOfDetail_ = lod;
    std::map<Processor*, ProcessorPropertiesWidget*>::iterator it;
    for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it)
        it->second->setLevelOfDetail(lod);
}

void PropertyListWidget::setState(WidgetMode mode, Property::LODSetting lod) {
    if (mode == LIST) {
        setWidgetMode(mode);
        setLevelOfDetail(lod);
    }
    else {
        setLevelOfDetail(lod);
        setWidgetMode(mode);
    }
}

void PropertyListWidget::processorNameChanged(Processor* processor){

    std::map<Processor*, ProcessorPropertiesWidget*>::iterator it = processorWidgetMap_.find(processor);
    if (it != processorWidgetMap_.end()) {
        it->second->updateHeaderTitle();
    }
}

} // namespace voreen
