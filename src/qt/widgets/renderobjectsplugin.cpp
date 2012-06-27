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

#include "voreen/qt/widgets/renderobjectsplugin.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStringList>

namespace voreen {

RenderObjectsPlugin::RenderObjectsPlugin(QWidget* parent, MessageReceiver* msgReceiver) : WidgetPlugin(parent, msgReceiver) {
    setObjectName(tr("Render objects"));
    icon_ = QIcon(":/icons/information.png");
    tree_ = new QTreeWidget();
}

void RenderObjectsPlugin::createWidgets() {
    QStringList header;
    header << tr("Type");
    header << tr("Name");
    tree_->setHeaderLabels(header);
    QVBoxLayout* vboxLayout = new QVBoxLayout();
    vboxLayout->addWidget(tree_);
    //vboxLayout->addWidget(height_);
    setLayout(vboxLayout);
}

void RenderObjectsPlugin::createConnections() {
//	connect(width_, SIGNAL(valueChanged(int)), this, SLOT(setWidth(int)));
//	connect(height_, SIGNAL(valueChanged(int)), this, SLOT(setHeight(int)));
}

void RenderObjectsPlugin::readRenderingPipeline() {
// merge!!
    /*
    tree_->selectAll();
    tree_->clearSelection();
    std::string type;
    std::string name;
    PostProcessing* pp;
    tree_->setColumnCount(2);
    for (unsigned int i = 0; i < volRayCasting->getNumRenderingObjects(); ++i) {
        RayCasting* rcObject = dynamic_cast<RayCasting*>(volRayCasting->getRenderingObject(i));
        if(rcObject) {
            type = rcObject->getType();
            name = rcObject->getName();
            QTreeWidgetItem* typeItem = new QTreeWidgetItem(tree_, QStringList(QString(type.c_str())));
            typeItem->setText(1, name.c_str());
            pp=0;//rcObject->getNextStage();
            while(pp) {
                type = pp->getType();
                std::string ppName = pp->getName();
                QTreeWidgetItem* ppItem = new QTreeWidgetItem(typeItem, QStringList(QString(type.c_str())));
                ppItem->setText(1, ppName.c_str());
//                pp = pp->getNextStage();
            }
        }
    }
    pp = volRayCasting->getMergeStage();
    if(pp) {
        type = pp->getType();
        name = pp->getName();
        QTreeWidgetItem* typeItem = new QTreeWidgetItem(tree_, QStringList(QString(type.c_str())));
        typeItem->setText(1, name.c_str());
    }
    pp = volRayCasting->getFinalMergeStage();
    if (pp) {
        type = pp->getType();
        name = pp->getName();
        QTreeWidgetItem* typeItem = new QTreeWidgetItem(tree_, QStringList(QString(type.c_str())));
        typeItem->setText(1, name.c_str());
    }
    */
}
} // namespace voreen

