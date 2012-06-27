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

#include "voreen/qt/widgets/compactproperty/compacttransfuncpropertywidget.h"

#include "voreen/qt/widgets/voreentoolwindow.h"
#include "voreen/qt/widgets/transfunc/transfuncplugin.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/properties/transferfuncproperty.h"

#include <QAction>
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>

namespace voreen {

QMainWindow* CompactTransFuncPropertyWidget::mainWin_ = 0;

CompactTransFuncPropertyWidget::CompactTransFuncPropertyWidget(TransFuncProp* prop, QWidget* parent)
    : CompactPropertyWidget(prop, parent)
    , plugin_(new TransFuncPlugin(prop, parent, Qt::Horizontal))
    , property_(prop)
    , window_(0)
    , editBt_(new QPushButton(tr("edit")))
{
    plugin_->createWidgets();
    plugin_->createConnections();

    // it should be sufficient to perform the search for the main window
    // (for what reason ever) once and store the result within the static
    // private member mainWin_.
    if (mainWin_ == 0) {
        QWidgetList list = QApplication::topLevelWidgets();
        for (QWidgetList::iterator it = list.begin(); it != list.end(); ++it) {
            mainWin_ = qobject_cast<QMainWindow*>(*it);
            if (mainWin_ != 0)
                break;
        }
    }

    QString title;
    if (prop->getOwner())
        title.append(prop->getOwner()->getName().c_str());
    title.append(" - Editor for ");
    title.append(prop->getGuiText().c_str());
    window_ = new VoreenToolWindow(new QAction(title, 0), mainWin_, plugin_, "");
    window_->adjustSize();

    addWidget(editBt_);

    connect(editBt_, SIGNAL(clicked()), this, SLOT(setProperty()));
    connect(plugin_, SIGNAL(transferFunctionChanged()), this, SIGNAL(propertyChanged()));

    addVisibilityControls();
}

CompactTransFuncPropertyWidget::~CompactTransFuncPropertyWidget() {
    delete window_;
}

void CompactTransFuncPropertyWidget::update() {
    if (plugin_)
        plugin_->update();
}

void CompactTransFuncPropertyWidget::setProperty() {
    if (!disconnected_) {
        if (window_->isVisible()) {
            //close widget
            window_->close();
        }
        else {
            //open Widget
            window_->showNormal();
        }
    }
}

void CompactTransFuncPropertyWidget::disconnect() {
    disconnected_ = true;
    plugin_->disconnect();
}

} // namespace
