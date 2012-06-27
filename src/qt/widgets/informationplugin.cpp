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

#include "voreen/qt/widgets/informationplugin.h"

#include "voreen/qt/widgets/thresholdwidget.h"

namespace voreen {

InformationPlugin::InformationPlugin(QWidget* parent, MessageReceiver* msgReceiver) :
    WidgetPlugin(parent, msgReceiver)
{
    setObjectName(tr("Information"));
    icon_ = QIcon(":/icons/information.png");
    infoText_ = 0;
    createWidgets();
}

void InformationPlugin::createWidgets() {
    if (infoText_) 
        return;
    infoText_ = new QTextEdit(this);
    infoText_->setReadOnly(true);
	QVBoxLayout* vboxLayout = new QVBoxLayout();
    vboxLayout->addWidget(infoText_, 0, Qt::AlignTop);
    setLayout(vboxLayout);
}

void InformationPlugin::createConnections() {
}

void InformationPlugin::setPlainText(QString text) {
    infoText_->setPlainText(text);
}

} // namespace voreen
