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

#include <QtGui>
#include "voreen/qt/widgets/plugindialog.h"


namespace voreen {

PluginDialog::PluginDialog(WidgetPlugin* plugin, QWidget* parent, bool hideAtStartup /*=true*/)
    : QDockWidget(parent)
{
    plugin_ = plugin;
    plugin_->createWidgets();
    plugin_->createConnections();
        
    setWidget(plugin_);
    adjustSize();
    setFixedSize(sizeHint());
    sizePolicy().setHorizontalPolicy(QSizePolicy::Fixed);
    sizePolicy().setVerticalPolicy(QSizePolicy::Fixed);
    
    setWindowTitle(plugin_->objectName());
    setWindowIcon(plugin_->getIcon());
    if (hideAtStartup)
        hide();
    
    // retrict widget to left or right dock widget area
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    connect(this, SIGNAL(topLevelChanged(bool)), this, SLOT(floating(bool)));
}

QAction* PluginDialog::createAction() {
    QAction* action = toggleViewAction();
    action->setIcon(plugin_->getIcon());
    return action;
}

void PluginDialog::floating(bool fl) {
    // resize to plugin's size hint when floating:
    if (fl)
        resize(plugin_->sizeHint());
}

void PluginDialog::closeEvent(QCloseEvent* /*event*/) {
    emit(dialogClose(this));
}

} // namespace voreen
