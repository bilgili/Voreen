/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "voreen/qt/widgets/property/qpropertywidgetwitheditorwindow.h"

#include "voreen/qt/widgets/voreentoolwindow.h"
#include "voreen/qt/voreenapplicationqt.h"

#include <QAction>
#include <QApplication>
#include <QMainWindow>
#include <QDesktopWidget>

namespace voreen {

QPropertyWidgetWithEditorWindow::QPropertyWidgetWithEditorWindow(Property* prop, QWidget* parent, bool showNameLabel, bool isEditorWindowResizable)
    : QPropertyWidget(prop, parent, showNameLabel)
    , isEditorWindowResizable_(isEditorWindowResizable)
    , editorWindow_(0)
{
}

QPropertyWidgetWithEditorWindow::~QPropertyWidgetWithEditorWindow() {
    delete editorWindow_;
}

MetaDataBase* QPropertyWidgetWithEditorWindow::getWidgetMetaData() const {

    // calculate position relative to mainwindow
    QPoint mainPindowPos(0,0);
    if (VoreenApplicationQt::qtApp() && VoreenApplicationQt::qtApp()->getMainWindow()) {
        mainPindowPos = VoreenApplicationQt::qtApp()->getMainWindow()->pos();
    }

    WindowStateMetaData* meta;
    if (editorWindow_ && isEditorWindowResizable_) {
        meta = new WindowStateMetaData(
            editorWindow_->isVisible(),
            editorWindow_->pos().x() - mainPindowPos.x(),
            editorWindow_->pos().y() - mainPindowPos.y(),
            editorWindow_->width(),
            editorWindow_->height());
    }
    else if (editorWindow_ && !isEditorWindowResizable_) {
        meta = new WindowStateMetaData(
            editorWindow_->isVisible(),
            editorWindow_->pos().x() - mainPindowPos.x(),
            editorWindow_->pos().y() - mainPindowPos.y());
    }
    else
        meta = new WindowStateMetaData(false);

    const_cast<QPropertyWidgetWithEditorWindow*>(this)->getProperty()->getMetaDataContainer().addMetaData("EditorWindow", meta);

    return meta;
}

void QPropertyWidgetWithEditorWindow::createEditorWindow(Qt::DockWidgetArea area, const QString& titlePostfix, const int& initialWidth, const int& initialHeight) {
    tgtAssert(!editorWindow_, "Editor window already instantiated");

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QString title;
    if (getProperty()->getOwner()) {
        title.append(getProperty()->getOwner()->getID().c_str());
        title.append(" - ");
    }
    title.append(QString::fromStdString(getProperty()->getGuiName()));
    title.append(titlePostfix);
    // replace line breaks by spaces
    title.replace("\r\n", " ");
    title.replace("\r", " ");
    title.replace("\n", " ");

    QMainWindow* mainWindow = VoreenApplicationQt::qtApp()->getMainWindow();
    editorWindow_ = new VoreenToolWindow(new QAction(title, mainWindow), mainWindow, createEditorWindowWidget(), title, false);
    if (mainWindow && area != Qt::NoDockWidgetArea)
        mainWindow->addDockWidget(area, editorWindow_);
    customizeEditorWindow();

    // set default size, might be overwritten by meta data
    if (initialWidth >= 0 && initialHeight >= 0)
        editorWindow_->resize(initialWidth, initialHeight);

    WindowStateMetaData* meta = dynamic_cast<WindowStateMetaData*>(getProperty()->getMetaDataContainer().getMetaData("EditorWindow"));
    // restore serialized geometry
    if (meta) {
        // compute position relative to mainwindow
        int xrel = meta->getX();
        int yrel = meta->getY();
        if (VoreenApplicationQt::qtApp() && VoreenApplicationQt::qtApp()->getMainWindow()) {
            QPoint mainPindowPos = VoreenApplicationQt::qtApp()->getMainWindow()->pos();
            xrel += mainPindowPos.x();
            yrel += mainPindowPos.y();
        }

        // check whether serialized left-top corner lies inside the available screen geometry
        QRect screenGeometry = QApplication::desktop()->availableGeometry(QPoint(xrel+25, yrel+25));
        if (screenGeometry.contains(QPoint(xrel+25, yrel+25))) {
            editorWindow_->move(xrel, yrel);
        }
        else {
            LWARNINGC("voreenqt.QPropertyWidgetWithEditorWindow",
                getPropertyGuiName() << " editor: Serialized position (" << meta->getX() << ", " << meta->getY() << ") " <<
                "outside visible desktop area. Ignoring.");
        }

        // size
        if (isEditorWindowResizable_ && meta->getWidth() > 0 && meta->getHeight() > 0)
            editorWindow_->resize(meta->getWidth(), meta->getHeight());

        // visibility
        editorWindow_->setVisible(meta->getVisible());
    }

    QApplication::restoreOverrideCursor();
}

bool QPropertyWidgetWithEditorWindow::editorVisibleOnStartup() const {
    WindowStateMetaData* meta = dynamic_cast<WindowStateMetaData*>(
        const_cast<QPropertyWidgetWithEditorWindow*>(this)->getProperty()->getMetaDataContainer().getMetaData("EditorWindow"));

    if (!meta)
        return false;

    return meta->getVisible();
}

} // namespace
