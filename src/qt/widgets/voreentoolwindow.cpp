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

#include "voreen/qt/widgets/voreentoolwindow.h"
#include "voreen/qt/widgets/voreentoolbar.h"

#include <QAction>
#include <QBoxLayout>

namespace voreen {

VoreenToolWindow::VoreenToolWindow(QAction* action, QWidget* parent, QWidget* child, const QString& name)
    : QWidget(parent, Qt::Tool),
      action_(action)
{
    setWindowTitle(action->text());
    setWindowIcon(action->icon());
 	setObjectName(name);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(child);
    setLayout(layout);

    // connection action and widget visibility
	connect(action, SIGNAL(toggled(bool)), this, SLOT(setVisible(bool)));
  	connect(this, SIGNAL(visibilityChanged(bool)), action, SLOT(setChecked(bool)));
}

void VoreenToolWindow::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    if (!isVisible())
        emit visibilityChanged(false);
}

void VoreenToolWindow::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (isVisible())
        emit visibilityChanged(true);
}

} // namespace
