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

#include "voreen/qt/widgets/dockbarhandler.h"
#include "tgt/assert.h"

#include <QVBoxLayout>
#include <QAction>
#include <QScrollBar>

namespace voreen {

DockBarHandler::DockBarHandler(QWidget* parent, QString title) : 
    QDockWidget(title, parent),
    parent_(parent)
{
    setObjectName(title);
    
    mainFrame_ = new QFrame();
    mainFrame_->setFrameStyle(QFrame::NoFrame);
    mainFrame_->setContentsMargins(-6, -6, -6, -6);
    mainFrame_->setLayout(new QVBoxLayout());

    scrollArea_ = new QScrollArea();
    scrollArea_->setWidget(mainFrame_);
    scrollArea_->setFrameStyle(QFrame::NoFrame);
    scrollArea_->setWidgetResizable(true);
    setWidget(scrollArea_);

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);

    finished_ = false;
}

void DockBarHandler::addDialog(PluginDialog* pluginDialog) {
    tgtAssert(!finished_, "DockBarHandler layout already finished!");
    mainFrame_->layout()->addWidget(pluginDialog);
    dockWidgets_.push_back(pluginDialog);
    connect(pluginDialog, SIGNAL(topLevelChanged(bool)), this, SLOT(topLevelChanged(bool)));
    connect(pluginDialog->toggleViewAction(), SIGNAL(triggered(bool)), this, SLOT(viewActionToggled(bool)));
    connect(pluginDialog, SIGNAL(dialogClose(PluginDialog*)), this, SLOT(dialogClose(PluginDialog*)));
}

void DockBarHandler::finishLayout() {
    (static_cast<QVBoxLayout*>(mainFrame_->layout()))->addStretch(100);
    adjustWidthAndVisibility();
    finished_ = true;
}

void DockBarHandler::topLevelChanged(bool /*topLevel*/) {
    adjustWidthAndVisibility();
}

void DockBarHandler::viewActionToggled(bool /*checked*/) {
    adjustWidthAndVisibility();
}

void DockBarHandler::dialogClose(PluginDialog* dialog) {
    adjustWidthAndVisibility(dialog);
}

void DockBarHandler::adjustWidthAndVisibility(PluginDialog* dialogToIgnore) {
    int minWidth = 0;
    for (size_t i=0; i<dockWidgets_.size(); ++i) {
        if (!dockWidgets_[i]->isHidden() && !dockWidgets_[i]->isFloating()
            && dockWidgets_[i] != dialogToIgnore )
        {
            minWidth = std::max(minWidth, dockWidgets_[i]->sizeHint().width());
        }
    }
    if (minWidth > 0) {
        minWidth += scrollArea_->verticalScrollBar()->sizeHint().width() + 8; 
        setMaximumWidth(minWidth);
        setMinimumWidth(minWidth);
        setVisible(true);
        updateGeometry();
    }
    else
        setVisible(false);
}

} // namespace voreen
