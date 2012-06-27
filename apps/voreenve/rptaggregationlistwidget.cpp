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

#include "rptaggregationlistwidget.h"
#include <iostream>


namespace voreen {

RptAggregationListWidget::RptAggregationListWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setHeaderLabel("Aggregations");
    setColumnCount(1);
    buildItems();
    header()->hide();  
}

void RptAggregationListWidget::buildItems() {
    items_.clear();
    clear();

    QDir dir = QDir("../../data/networks/aggregations");
    QStringList filters;
    filters << "*.svnf";
    filters << "*.vnw";
    QStringList savedAggregations = dir.entryList(filters, QDir::Files, QDir::Name);
    
    for (int i=0; i<savedAggregations.size(); i++) {
        items_.append(new RptAggregationListItem(savedAggregations[i].toStdString()));
    }

    insertTopLevelItems(0, items_);
    expandAll();
}


void RptAggregationListWidget::mousePressEvent(QMouseEvent *event) {
    QTreeWidget::mousePressEvent(event);

    // if not left button - return
    if (!(event->buttons() & Qt::LeftButton))
        return;

    // if no item selected, return
    if (currentItem() == NULL)
        return;

    RptAggregationListItem* item = dynamic_cast<RptAggregationListItem*> (itemAt(event->pos()));

    // if no RptRenderListItem selected, return
    if (!item)
        return;
    
    QString filename = "../../data/networks/aggregations/";
    filename += QString(item->getFileName().c_str());

    QMimeData *mimeData = new QMimeData;
    mimeData->setText(filename);
        
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);

    drag->start(Qt::CopyAction);

}

void RptAggregationListWidget::removeActionSlot() {
    QString s = selectedItems().at(0)->text(0);
    QDir dir = QDir("../../data/networks/aggregations");
    dir.remove(s);
    buildItems();
}

void RptAggregationListWidget::contextMenuEvent(QContextMenuEvent *event) {
    QList<QTreeWidgetItem*> selected = selectedItems();
    if (selected.count() > 0) {
        QMenu menu;
        QAction* removeAction = new QAction(tr("Remove"),this);
        menu.addAction(removeAction);

        QObject::connect(removeAction, SIGNAL(triggered()), this, SLOT(removeActionSlot()));

        menu.exec(event->globalPos());
    }

    QTreeWidget::contextMenuEvent(event);
}

// ----------------------------------------------------------------------------

RptAggregationListItem::RptAggregationListItem(std::string filename)
    : QTreeWidgetItem(QStringList(QString(filename.c_str())))
    , filename_(filename)
{}

} // namespace
