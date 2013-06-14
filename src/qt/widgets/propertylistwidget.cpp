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

#include "voreen/qt/widgets/propertylistwidget.h"

#include "voreen/qt/widgets/property/qpropertywidget.h"
#include "voreen/qt/widgets/property/processorpropertieswidget.h"

#include "voreen/core/ports/port.h"

#include <QVBoxLayout>
#include <QToolButton>

namespace voreen {

PropertyListWidget::PropertyListWidget(QWidget* parent, ProcessorNetwork* processorNet,
                                       PropertyListWidget::WidgetMode mode, Property::LODSetting lod)
    : QScrollArea(parent)
    , processorNetwork_(processorNet)
    , widgetMode_(mode)
    , levelOfDetail_(lod)
    , hideLodControl_(0)
    , containerWidget_(0)
    , containerLayout_(0)
    , lodControlVisibility_(false)
{

    if (processorNetwork_)
        processorNetwork_->addObserver(this);

    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setContentsMargins(0, 0, 0, 0);

    setMinimumWidth(250);

    createWidgets();
    updateState();
}

PropertyListWidget::~PropertyListWidget() {
    clear();
}

void PropertyListWidget::setProcessorNetwork(ProcessorNetwork* network) {
    // stop observation of previously assigned network
    stopObservation(processorNetwork_);

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

void PropertyListWidget::processorAdded(const Processor* processor) {

    if (!containerLayout_) {
        LWARNINGC("voreenqt.PropertyListWidget", "No container layout");
        return;
    }

    // generate header widget for the new processor and insert it into the widget map
    ProcessorPropertiesWidget* headerWidget = new ProcessorPropertiesWidget(this, processor, false, widgetMode_ == LIST);
    connect(headerWidget, SIGNAL(modified()), this, SLOT(processorModified()));
    connect(this, SIGNAL(showHeader(bool)), headerWidget, SLOT(showHeader(bool)));
    processorWidgetMap_.insert(std::make_pair(processor, headerWidget));
    containerLayout_->addWidget(headerWidget);

    // Assign volume container to headerWidgets for further propagation to volumehandleproperty- and volumecollectionwidgets
    headerWidget->instantiateWidgets();
    headerWidget->setLevelOfDetail(levelOfDetail_);
    headerWidget->setVisible(widgetMode_ == LIST);
}

void PropertyListWidget::processorRemoved(const Processor* processor){
    // delete processor's header widget and remove it from the map
    std::map<const Processor*, ProcessorPropertiesWidget*>::iterator it = processorWidgetMap_.find(processor);
    if (it != processorWidgetMap_.end()) {
        if (widget() && widget()->layout())
            widget()->layout()->removeWidget(it->second);
        delete it->second;
        processorWidgetMap_.erase(it);
    }
}

void PropertyListWidget::processorRenamed(const Processor* processor, const std::string& /*prevName*/){
    std::map<const Processor*, ProcessorPropertiesWidget*>::iterator it = processorWidgetMap_.find(processor);
    if (it != processorWidgetMap_.end()) {
        it->second->updateHeaderTitle();
    }
}

void PropertyListWidget::connectionsChanged() {}

void PropertyListWidget::propertyLinkAdded(const PropertyLink*) {}

void PropertyListWidget::propertyLinkRemoved(const PropertyLink*) {}

void PropertyListWidget::portConnectionAdded(const Port*, const Port*) {}

void PropertyListWidget::portConnectionRemoved(const Port*, const Port*) {}

QSize PropertyListWidget::sizeHint() const {
    return QSize(300, 0);
}

void PropertyListWidget::processorsSelected(const QList<Processor*>& selectedProcessors) {
    // update visibility/expansion state of property widgets according to selection
    if (selectedProcessors.size() == 0) {
        if (hideLodControl_)
            hideLodControl_->setEnabled(false);
    }
    else if (hideLodControl_)
        hideLodControl_->setEnabled(true);

    std::map<const Processor*, ProcessorPropertiesWidget*>::iterator it;
    for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it) {
        bool isSelected = std::find(selectedProcessors.begin(), selectedProcessors.end(), it->first) != selectedProcessors.end();
        if (widgetMode_ == LIST) {
            it->second->setExpanded(isSelected);
        }
        else if (widgetMode_ == SELECTED){
            it->second->setVisible(isSelected);
            it->second->setExpanded(isSelected);
        }
        else {
            LERRORC("voreen.qt.PropertyListWidget", "Undefined widget mode");
        }
    }
    previouslySelectedProcessors_ = selectedProcessors;
}

void PropertyListWidget::clear() {

    if (processorWidgetMap_.empty() && previouslySelectedProcessors_.empty())
        return;

    setUpdatesEnabled(false);

    //delete all widgets and remove them from container layout
    std::map<const Processor*, ProcessorPropertiesWidget*>::iterator it;
    for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it) {
        if (containerLayout_)
            containerLayout_->removeWidget(it->second);
        delete it->second;
    }

    // remove remaining layout items from container layout
    if (containerLayout_)
        while (containerLayout_->count() > 0)
            containerLayout_->removeItem(containerLayout_->itemAt(0));

    setUpdatesEnabled(true);

    processorWidgetMap_.clear();
    previouslySelectedProcessors_.clear();
}

void PropertyListWidget::createWidgets() {
    if (!processorNetwork_)
        return;

    // generate widget inside scroll area
    if (!containerWidget_) {
        // container widget storing the processor properties widgets
        containerWidget_ = new QWidget;
        QVBoxLayout* outerLayout = new QVBoxLayout(containerWidget_);
        outerLayout->setContentsMargins(0, 0, 0, 0);
        containerLayout_ = new QVBoxLayout();
        containerLayout_->setContentsMargins(5, 5, 5, 0);
        containerLayout_->setAlignment(Qt::AlignTop);
        outerLayout->addLayout(containerLayout_);
        outerLayout->addStretch(10);

        // lod control selector
        QHBoxLayout* hideLodLayout = new QHBoxLayout();
        hideLodLayout->setContentsMargins(4, 0, 4, 4);
        hideLodControl_ = new QToolButton(this);
        hideLodControl_->setCheckable(true);
        hideLodControl_->setChecked(true);
        hideLodControl_->setIcon(QIcon(":/qt/icons/eye-questionmark.png"));
        hideLodControl_->setToolTip(tr("Hide Level Of Detail Controls"));
        hideLodControl_->setMaximumSize(15,15);
        hideLodControl_->setMinimumSize(15,15);
        hideLodLayout->addStretch();
        hideLodLayout->addWidget(hideLodControl_);
        connect(hideLodControl_, SIGNAL(clicked(bool)), this, SLOT(hideLodControls(bool)));
        hideLodControl_->setEnabled(false);
        hideLodControl_->setVisible(widgetMode_ == SELECTED);
        outerLayout->addLayout(hideLodLayout);

        setWidget(containerWidget_);
    }

    // generate processor property widgets: each processor property contains a processor's properties
    // along with a expansion header
    setUpdatesEnabled(false);
    for (size_t i=0; i<processorNetwork_->numProcessors(); ++i) {
        Processor* proc = processorNetwork_->getProcessors().at(i);
        ProcessorPropertiesWidget* headerWidget = new ProcessorPropertiesWidget(this, proc, false, true);
        connect(headerWidget, SIGNAL(modified()), this, SLOT(processorModified()));
        connect(this, SIGNAL(showHeader(bool)), headerWidget, SLOT(showHeader(bool)));
        //connect(graphicsProcessorNet_->processorItems[i], SIGNAL(processorNameChanged()), headerWidget, SLOT(updateHeaderTitle()));
        containerLayout_->addWidget(headerWidget);
        processorWidgetMap_.insert(std::make_pair(proc, headerWidget));
    }

    setLevelOfDetail(levelOfDetail_);
    setUpdatesEnabled(true);
}

void PropertyListWidget::hideLodControls(bool hide) {
    lodControlVisibility_ = !hide;
    if(processorNetwork_ != 0) {
        for (size_t i=0; i<processorNetwork_->numProcessors(); ++i) {
            Processor* proc = processorNetwork_->getProcessors().at(i);
            std::vector<Property*> propertyList = proc->getProperties();
            for (size_t i=0; i<proc->getPorts().size(); i++) {
                std::vector<Property*> portProps = proc->getPorts().at(i)->getProperties();
                propertyList.insert(propertyList.end(), portProps.begin(), portProps.end());
            }

            for (size_t ii = 0; ii < propertyList.size(); ++ii) {
                const std::set<PropertyWidget*> widgets = propertyList.at(ii)->getPropertyWidgets();
                std::set<PropertyWidget*>::const_iterator it = widgets.begin();
                while(it != widgets.end()) {
                    if (QPropertyWidget* qWidget = dynamic_cast<QPropertyWidget*>(*it)) {
                        if (hide)
                            qWidget->hideLODControls();
                        else
                            qWidget->showLODControls();
                    }
                    ++it;
                }
            }
        }
    }
    emit showHeader(!hide);
}

void PropertyListWidget::hideAll() {
    std::map<const Processor*, ProcessorPropertiesWidget*>::iterator it;
    for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it)
        it->second->setVisible(false);
}

void PropertyListWidget::showAll() {
    std::map<const Processor*, ProcessorPropertiesWidget*>::iterator it;
    for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it)
        it->second->setVisible(true);
}

void PropertyListWidget::setAllExpandable() {
    std::map<const Processor*, ProcessorPropertiesWidget*>::iterator it;
    for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it)
        it->second->setUserExpandable(true);
}

void PropertyListWidget::setAllUnexpandable() {
    std::map<const Processor*, ProcessorPropertiesWidget*>::iterator it;
    for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it)
        it->second->setUserExpandable(false);
}

void PropertyListWidget::updateState() {
    setUpdatesEnabled(false);

    if (widgetMode_ == LIST) {
        showAll();
        setAllExpandable();
        setLevelOfDetail(levelOfDetail_);
    }
    else if (widgetMode_ == SELECTED) {
        hideAll();
        setAllUnexpandable();
        hideLodControls(!lodControlVisibility_);
    }
    setUpdatesEnabled(true);

}

PropertyListWidget::WidgetMode PropertyListWidget::widgetMode() const {
    return widgetMode_;
}

void PropertyListWidget::setWidgetMode(WidgetMode mode) {
    widgetMode_ = mode;
    updateState();

    if(hideLodControl_) {
        if (mode == SELECTED)
            hideLodControl_->setVisible(true);
        else
            hideLodControl_->setVisible(false);
    }

    if (mode == SELECTED /*&& processorWidgetMap_.find(currentlySelectedProcessors_) != processorWidgetMap_.end()*/)
        processorsSelected(previouslySelectedProcessors_);
}

void PropertyListWidget::setLevelOfDetail(Property::LODSetting lod) {
    levelOfDetail_ = lod;
    std::map<const Processor*, ProcessorPropertiesWidget*>::iterator it;
    for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it)
        it->second->setLevelOfDetail(lod);
}

Property::LODSetting PropertyListWidget::getLevelOfDetail() const {
    return levelOfDetail_;
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

void PropertyListWidget::processorModified() {
    emit modified();
}

void PropertyListWidget::expandAll() {
    std::map<const Processor*, ProcessorPropertiesWidget*>::iterator it;
    for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it)
        if (it->second->isVisible())
            it->second->setExpanded(true);
}

void PropertyListWidget::collapseAll() {
    std::map<const Processor*, ProcessorPropertiesWidget*>::iterator it;
    for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it)
        it->second->setExpanded(false);
}

} // namespace voreen
