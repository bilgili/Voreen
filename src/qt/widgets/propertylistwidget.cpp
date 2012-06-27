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

#include "voreen/qt/widgets/propertylistwidget.h"

#include "voreen/qt/widgets/qpropertywidget.h"
#include "voreen/qt/widgets/compactproperty/compactpropertywidgetfactory.h"
#include "voreen/core/vis/processors/processor.h"

#include <QVBoxLayout>

namespace voreen {

PropertyListWidget::PropertyListWidget(QWidget* parent)
    : QScrollArea(parent)
{
    //start with empty widget
    emptyWidget_ = new QWidget();

    setWidget(emptyWidget_);
    takeWidget();

    setWidgetResizable(true);

    setMinimumWidth(300);
}

PropertyListWidget::~PropertyListWidget() {
    takeWidget();
    delete emptyWidget_;
    //delete all widgets
    std::map<Processor*, QWidget*>::iterator it;
    for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it)
        delete it->second;

    processorWidgetMap_.clear();
}

void PropertyListWidget::setProcessor(Processor* processor) {
    if (processor) {
        std::map<Processor*, QWidget*>::iterator it = processorWidgetMap_.find(processor);
        if (it != processorWidgetMap_.end()) {
            //the widgets for this processor have already been created
            //-> display it
            takeWidget(); //removes current displayed widget but not deletes it
            setWidget(it->second);
        }
        else {
            //processor is not in the map
            //-> create widget, display the new widget and add processor to the map
            QWidget* widget = createWidget(processor);
            takeWidget(); //removes current displayed widget but not deletes it
            setWidget(widget);
            processorWidgetMap_.insert(std::make_pair(processor, widget));
        }
    }
    else {
        takeWidget(); //removes current displayed widget but not deletes it
        setWidget(emptyWidget_); //set to empty widget
    }
}

void PropertyListWidget::processorDeleted(Processor* processor){
    std::map<Processor*, QWidget*>::iterator it = processorWidgetMap_.find(processor);
    if (it != processorWidgetMap_.end()) {
        //take control of current displayed widget
        takeWidget();
        //set to empty widget
        setWidget(emptyWidget_);
        //delete widget
        delete it->second;
        //remove widget from map
        processorWidgetMap_.erase(it);
    }
}

QWidget* PropertyListWidget::createWidget(Processor* processor) {
    CompactPropertyWidgetFactory f = CompactPropertyWidgetFactory();

    QWidget* procWidget = new QWidget();
    QVBoxLayout* vbox = new QVBoxLayout();

    const std::vector<Property*> propertyList = processor->getProperties();
    //create widget for every property and put them into a vertical layout
    for (size_t i=0; i < propertyList.size(); i++) {
        QWidget* w = dynamic_cast<QPropertyWidget*>(propertyList[i]->createAndAddWidget(&f));
        connect(w, SIGNAL(propertyChanged()), this, SIGNAL(repaintSignal()));
        vbox->addWidget(w);
    }
    vbox->addStretch();
    procWidget->setLayout(vbox);
    //return widget with layout that contains all property widgets
    return procWidget;
}

void PropertyListWidget::clear() {
    takeWidget();
    //delete all widgets
    std::map<Processor*, QWidget*>::iterator it;
    for (it = processorWidgetMap_.begin(); it != processorWidgetMap_.end(); ++it)
        delete it->second;

    processorWidgetMap_.clear();

    //set to empty widget
    setWidget(emptyWidget_);
}

} // namespace voreen
