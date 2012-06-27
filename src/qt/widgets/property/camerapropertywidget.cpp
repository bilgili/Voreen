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

#include "voreen/qt/widgets/property/camerapropertywidget.h"

#include "voreen/core/vis/properties/cameraproperty.h"

#include "voreen/qt/widgets/voreentoolwindow.h"
#include "voreen/qt/widgets/property/camerawidget.h"
#include "voreen/qt/voreenapplicationqt.h"

#include "tgt/camera.h"

#include <QApplication>
#include <QMainWindow>
#include <QDesktopWidget>
#include <QPushButton>

namespace voreen {

using tgt::Camera;

CameraPropertyWidget::CameraPropertyWidget(CameraProperty* prop, QWidget* parent)
    : QPropertyWidget(prop, parent)
    , editBt_(new QPushButton(tr("edit")))
    , property_(prop)
    , cameraWidget_(0)
    , toolWindow_(0)
{
    createCameraWidget();

    addWidget(editBt_);
    connect(editBt_, SIGNAL(clicked()), this, SLOT(toggleWidgetVisibility()));

    addVisibilityControls();
}

CameraPropertyWidget::~CameraPropertyWidget() {
    delete toolWindow_;
}

void CameraPropertyWidget::updateFromProperty() {
    if (cameraWidget_)
        cameraWidget_->updateFromCamera();
}

void CameraPropertyWidget::setProperty(Camera* value) {
    if (!disconnected_)
        property_->set(value);
}

void CameraPropertyWidget::createCameraWidget() {
    tgtAssert(!cameraWidget_, "Camera widget already instantiated");

    cameraWidget_ = new CameraWidget(property_, 0.05f, 15.f, parentWidget());
    cameraWidget_->createWidgets();
    cameraWidget_->createConnections();

    QString title;
    if (property_->getOwner()) {
        title.append(property_->getOwner()->getName().c_str());
        title.append(" - ");
    }
    title.append(QString::fromStdString(property_->getGuiText()));
    QMainWindow* mainWindow = VoreenApplicationQt::qtApp()->getMainWindow();
    tgtAssert(mainWindow, "No mainwindow");
    toolWindow_ = new VoreenToolWindow(new QAction(title, 0), mainWindow, cameraWidget_, title, false);
    toolWindow_->adjustSize();
    toolWindow_->setFixedSize(toolWindow_->size());
    mainWindow->addDockWidget(Qt::LeftDockWidgetArea, toolWindow_);

    WindowStateMetaData* meta = dynamic_cast<WindowStateMetaData*>(property_->getMetaDataContainer().getMetaData("EditorWindow"));
    if (meta) {
        // check whether serialized left-top corner of lies inside the available screen geometry
        QRect screenGeometry = QApplication::desktop()->availableGeometry(QPoint(meta->getX(), meta->getY()));
        if (screenGeometry.contains(QPoint(meta->getX(), meta->getY())))
            toolWindow_->move(meta->getX(), meta->getY());

        toolWindow_->setVisible(meta->getVisible());
    }
}

void CameraPropertyWidget::toggleWidgetVisibility() {

    tgtAssert(toolWindow_, "No tool window");

    toolWindow_->setVisible(!toolWindow_->isVisible());
}

MetaDataBase* CameraPropertyWidget::getWidgetMetaData() const {
    WindowStateMetaData* meta;
    if (toolWindow_) {
        meta = new WindowStateMetaData(
            toolWindow_->isVisible(),
            toolWindow_->pos().x(),
            toolWindow_->pos().y());
    }
    else
        meta = new WindowStateMetaData(false);

    property_->getMetaDataContainer().addMetaData("EditorWindow", meta);

    return meta;
}

} // namespace
