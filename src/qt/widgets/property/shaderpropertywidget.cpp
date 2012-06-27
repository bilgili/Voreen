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

#include "voreen/qt/widgets/property/shaderpropertywidget.h"

#include "voreen/qt/widgets/voreentoolwindow.h"
#include "voreen/qt/widgets/shaderplugin.h"
#include "voreen/qt/voreenapplicationqt.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/properties/shaderproperty.h"

#include <QAction>
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QDesktopWidget>

namespace voreen {

ShaderPropertyWidget::ShaderPropertyWidget(ShaderProperty* prop, QWidget* parent)
    : QPropertyWidget(prop, parent)
    , plugin_(0)
    , property_(prop)
    , window_(0)
    , editBt_(new QPushButton(tr("edit")))
{

    if (editorVisibleOnStartup())
        createEditorWindow();

    addWidget(editBt_);

    connect(editBt_, SIGNAL(clicked()), this, SLOT(setProperty()));

    addVisibilityControls();
}

ShaderPropertyWidget::~ShaderPropertyWidget() {
    delete window_;
}

void ShaderPropertyWidget::updateFromProperty() {
    if (plugin_)
        plugin_->update();
}

void ShaderPropertyWidget::setProperty() {
    if (!disconnected_) {
        // lazy instantiation of shader editor window
        if (!window_) {
            createEditorWindow();
            tgtAssert(window_, "Shader editor not instantiated");
        }

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

void ShaderPropertyWidget::disconnect() {
    disconnected_ = true;
    if (plugin_)
        plugin_->disconnect();
}

void ShaderPropertyWidget::createEditorWindow() {
    tgtAssert(!window_ && !plugin_, "Shader editor already instantiated");

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    plugin_ = new ShaderPlugin(property_, parentWidget());
    plugin_->createWidgets();
    plugin_->createConnections();
    connect(plugin_, SIGNAL(modified()), this, SIGNAL(modified()));

    QString title;
    if (property_->getOwner()) {
        title.append(property_->getOwner()->getName().c_str());
        title.append(" - ");
    }
    title.append(QString::fromStdString(property_->getGuiText()));
    title.append(" (original source: ");
    title.append(QString::fromStdString(property_->getFileName()));
    title.append(")");
    QMainWindow* mainWindow = VoreenApplicationQt::qtApp()->getMainWindow();
    window_ = new VoreenToolWindow(new QAction(title, 0), mainWindow, plugin_, title, false);
    mainWindow->addDockWidget(Qt::LeftDockWidgetArea, window_);

    // set default size, might be overwriten by meta data
    window_->resize(700, 700);
    
    WindowStateMetaData* meta = dynamic_cast<WindowStateMetaData*>(property_->getMetaDataContainer().getMetaData("EditorWindow"));
    if (meta) {
        // check whether serialized left-top corner of lies inside the available screen geometry
        QRect screenGeometry = QApplication::desktop()->availableGeometry(QPoint(meta->getX(), meta->getY()));
        if (screenGeometry.contains(QPoint(meta->getX(), meta->getY())))
            window_->move(meta->getX(), meta->getY());

        if (meta->getWidth() > 0 && meta->getHeight() > 0)
            window_->resize(meta->getWidth(), meta->getHeight());

        window_->setVisible(meta->getVisible());
    }

    QApplication::restoreOverrideCursor();
}

MetaDataBase* ShaderPropertyWidget::getWidgetMetaData() const {
    WindowStateMetaData* meta;
    if (window_) {
        meta = new WindowStateMetaData(
            window_->isVisible(),
            window_->pos().x(),
            window_->pos().y(),
            window_->width(),
            window_->height());
    }
    else
        meta = new WindowStateMetaData(false);

    property_->getMetaDataContainer().addMetaData("EditorWindow", meta);

    return meta;
}

bool ShaderPropertyWidget::editorVisibleOnStartup() const {
    WindowStateMetaData* meta = dynamic_cast<WindowStateMetaData*>(property_->getMetaDataContainer().getMetaData("EditorWindow"));
    if (!meta)
        return false;

    return meta->getVisible();
}


} // namespace voreen
