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

#include "voreen/qt/widgets/network/processorlistwidget.h"

#include "voreen/core/vis/processors/processorfactory.h"

#include <QHeaderView>
#include <QLineEdit>
#include <QMouseEvent>
#include <QVBoxLayout>

namespace voreen {

ProcessorListWidget::ProcessorListWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    edit_ = new QLineEdit;
    tree_ = new ProcessorListTreeWidget;
    connect(edit_, SIGNAL(textChanged(const QString&)), tree_, SLOT(filter(const QString&)));

    layout->addWidget(edit_);
    layout->addWidget(tree_);
}

// ----------------------------------------------------------------------------

ProcessorListTreeWidget::ProcessorListTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setHeaderLabel("Processors");
    setColumnCount(1);
    processorVector_ = ProcessorFactory::getInstance()->getKnownClasses();
    buildItems();
}

void ProcessorListTreeWidget::buildItems() {
    int position;
    QStringList categories;
    QString categoryIdentifier("");
    QString identifier("");

    for (unsigned int i=0; i< processorVector_.size(); i++) {
        std::string id = processorVector_.at(i).getSubString(0);
        categoryIdentifier = processorVector_.at(i).getSubString(0).c_str();
        if (!categories.contains(categoryIdentifier)){
            items_.append(new QTreeWidgetItem(QStringList(categoryIdentifier)));
            categories << categoryIdentifier;
        }
    }

    for (unsigned int i=0; i< processorVector_.size(); i++) {
        categoryIdentifier = processorVector_.at(i).getSubString(0).c_str();
        position = categories.indexOf(categoryIdentifier);
        items_.at(position)->addChild(new ProcessorListItem(processorVector_.at(i).getSubString(1)));
    }

    insertTopLevelItems(0, items_);
    expandAll();
    header()->hide();
}

void ProcessorListTreeWidget::filter(const QString& text) {
    // First clear the former searches
    processorVector_.clear();
    items_.clear();

    // Get all available processor ids
    std::vector<Identifier> processors = ProcessorFactory::getInstance()->getKnownClasses();

    // For each available processor id: test, if the text is in the name of the processors
    for (std::vector<Identifier>::iterator iter = processors.begin(); iter != processors.end(); ++iter) {
        QString proc = QString::fromStdString((*iter).getSubString(1));
        if (proc.indexOf(text, 0, Qt::CaseInsensitive) != -1)
            processorVector_.push_back(*iter);
    }

    clear();
    buildItems();
}

void ProcessorListTreeWidget::mousePressEvent(QMouseEvent *event) {
    QTreeWidget::mousePressEvent(event);

    // if not left button - return
    if (!(event->buttons() & Qt::LeftButton))
        return;

    // if no item selected, return
    if (currentItem() == NULL)
        return;

    ProcessorListItem* item = dynamic_cast<ProcessorListItem*> (itemAt(event->pos()));

    // if no ProcessorListItem selected, return
    if (!item)
        return;

    Identifier id = item->getId();
    QString idStr(id.getName().c_str());

    QMimeData *mimeData = new QMimeData;
    mimeData->setText(idStr);

    QDrag *drag = new QDrag(parentWidget());
    drag->setMimeData(mimeData);
    //drag->setPixmap(pixmap);
    //drag->setHotSpot(event->pos()-item->pos());

    drag->start(Qt::CopyAction);

}

// ----------------------------------------------------------------------------

ProcessorListItem::ProcessorListItem (Identifier id)
    :  QTreeWidgetItem(QStringList(QString(id.getName().c_str()))),
       id_(id)
{}

} //namespace voreen
