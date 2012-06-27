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

#include "voreen/qt/widgets/consoleplugin.h"
#include "voreen/qt/qdebug.h"

#include <QVBoxLayout>

namespace voreen {

ConsolePlugin::ConsolePlugin(QWidget* parent, MessageReceiver* msgReceiver) : WidgetPlugin(parent, msgReceiver) {
    setObjectName(tr("Console"));
    icon_ = QIcon(":/icons/console.png");

#if defined(WIN32) && !defined(_DEBUG)
    // redirect cout und cerr to log file
    freopen("voreenapp-log.txt", "w", stdout);
    freopen("voreenapp-errors.txt", "w", stderr);
#endif

#ifdef WIN32
    // redirect cout to consoleText widget
	QDebugStream* mybuf = new QDebugStream();
    QTextEdit* consoleText = new QTextEdit(this);
    consoleText->setTextColor(Qt::darkGray);
    consoleText->setReadOnly(true);
    consoleText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	QVBoxLayout* vboxLayout = new QVBoxLayout();
	vboxLayout->addWidget(consoleText);
	setLayout(vboxLayout);
	mybuf->setConsole(consoleText);
	std::cout.rdbuf(mybuf);
#endif
}

void ConsolePlugin::createWidgets() {
}

void ConsolePlugin::createConnections() {
}
} // namespace voreen

