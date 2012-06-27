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
#include <QDesktopWidget>


namespace voreen {

QMainWindow* CompactTransFuncPropertyWidget::mainWin_ = 0;

CompactTransFuncPropertyWidget::CompactTransFuncPropertyWidget(TransFuncProp* prop, QWidget* parent)
    : CompactPropertyWidget(prop, parent)
    , plugin_(0)
    , property_(prop)
    , window_(0)
    , editBt_(new QPushButton(tr("edit")))
{

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

    if (!prop->getLazyEditorInstantiation() || editorVisibleOnStartup())
        createEditorWindow();

    addWidget(editBt_);

    connect(editBt_, SIGNAL(clicked()), this, SLOT(setProperty()));

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

        // lazy instantiation of transfunc editor window
        if (!window_) {
            createEditorWindow();
            tgtAssert(window_, "Transfunc editor not instantiated");
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

void CompactTransFuncPropertyWidget::disconnect() {
    disconnected_ = true;
    if (plugin_)
        plugin_->disconnect();
}

void CompactTransFuncPropertyWidget::createEditorWindow() {
    
    tgtAssert(!window_ && !plugin_, "Transfunc editor already instantiated");

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    plugin_ = new TransFuncPlugin(property_, parentWidget(), Qt::Horizontal);
    plugin_->createWidgets();
    plugin_->createConnections();
    connect(plugin_, SIGNAL(transferFunctionChanged()), this, SIGNAL(propertyChanged()));

    QString title;
    if (property_->getOwner()) {
        title.append(property_->getOwner()->getName().c_str());
        title.append(" - ");
    }
    title.append(QString::fromStdString(property_->getGuiText()));
    window_ = new VoreenToolWindow(new QAction(title, 0), mainWin_, plugin_, "");
    window_->adjustSize();

    // update window geometry from serialized meta data, if present
    if (property_->getMetaData()) {
        const TiXmlElement* editorElem = property_->getMetaData()->FirstChildElement("EditorWindow");
        if (editorElem) {
            
            int visible = -1;
            int x = -1;
            int y = -1;
            int width = -1;
            int height = -1;
            editorElem->QueryIntAttribute("visible", &visible);
            editorElem->QueryIntAttribute("x", &x);
            editorElem->QueryIntAttribute("y", &y);
            editorElem->QueryIntAttribute("width", &width);
            editorElem->QueryIntAttribute("height", &height);

            // check whether serialized left-top corner of lies inside the available screen geometry
            QRect screenGeometry = QApplication::desktop()->availableGeometry(QPoint(x,y));
            if (screenGeometry.contains(QPoint(x,y)))
                window_->move(x,y);

            if (width > 0 && height > 0)
                window_->resize(width, height);
            
            window_->setVisible(visible > 0);
        }
    }

    QApplication::restoreOverrideCursor();

}

TiXmlElement* CompactTransFuncPropertyWidget::getWidgetMetaData() const {

    // serialize the editor window's geometry
    TiXmlElement* elem = new TiXmlElement("EditorWindow");
    if (window_) {
        elem->SetAttribute("visible", window_->isVisible());
        elem->SetAttribute("x", window_->pos().x());
        elem->SetAttribute("y", window_->pos().y());
        elem->SetAttribute("width", window_->width());
        elem->SetAttribute("height", window_->height());
    }
    else {
        elem->SetAttribute("visible", false);
    }

    return elem;
}

bool CompactTransFuncPropertyWidget::editorVisibleOnStartup() const {

    if (property_->getMetaData()) {
        const TiXmlElement* editorElem = property_->getMetaData()->FirstChildElement("EditorWindow");
        if (editorElem) {
            int visible = -1;
            editorElem->QueryIntAttribute("visible", &visible);
            return (visible > 0);
        }
    }

    return false;
}


} // namespace
