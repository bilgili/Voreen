/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "rptprocessorlistwidget.h"
#include "voreen/core/vis/processors/processorfactory.h"

namespace voreen {

RptProcessorListWidget::RptProcessorListWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setHeaderLabel("Processors");
    setColumnCount(1);
    buildItems();
    insertTopLevelItems(0, items_);
    expandAll();
    header()->hide();  
}

void RptProcessorListWidget::buildItems() {
    int position;
    QStringList categories;
    QString categoryIdentifier(""); 
    QString identifier("");

    processorVector_ = ProcessorFactory::getInstance()->getKnownClasses();

    for(unsigned int i=0; i< processorVector_.size(); i++) {
        std::string id = processorVector_.at(i).getSubString(0);
        categoryIdentifier = processorVector_.at(i).getSubString(0).c_str();
        if (!categories.contains(categoryIdentifier)){
            items_.append(new QTreeWidgetItem(QStringList(categoryIdentifier)));
            categories << categoryIdentifier;
        }
    }

    for(unsigned int i=0; i< processorVector_.size(); i++) {
        categoryIdentifier = processorVector_.at(i).getSubString(0).c_str();
        position = categories.indexOf(categoryIdentifier); 
        items_.at(position)->addChild(new RptProcessorListItem(processorVector_.at(i).getSubString(1)));  
    }
}

void RptProcessorListWidget::mousePressEvent(QMouseEvent *event) {
    QTreeWidget::mousePressEvent(event);

    // if not left button - return
    if (!(event->buttons() & Qt::LeftButton))
        return;

    // if no item selected, return
    if (currentItem() == NULL)
        return;

    RptProcessorListItem* item = dynamic_cast<RptProcessorListItem*> (itemAt(event->pos()));

    // if no RptProcessorListItem selected, return
    if (!item)
        return;
    
    Identifier id = item->getId();
    QString idStr(id.getName().c_str());

	QMimeData *mimeData = new QMimeData;
    mimeData->setText(idStr);
        
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    //drag->setPixmap(pixmap);
    //drag->setHotSpot(event->pos()-item->pos());

    drag->start(Qt::CopyAction);

}

// ----------------------------------------------------------------------------

RptProcessorListItem::RptProcessorListItem (Identifier id)
    : id_(id)
    , QTreeWidgetItem(QStringList(QString(id.getName().c_str())))
{
}

} //namespace voreen
