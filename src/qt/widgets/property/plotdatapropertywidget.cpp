/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/qt/widgets/property/plotdatapropertywidget.h"

#include "voreen/core/properties/plotdataproperty.h"

#include "voreen/qt/widgets/voreentoolwindow.h"
#include "voreen/qt/widgets/plotting/plotdatawidget.h"

#include "voreen/core/plotting/plotdata.h"

#include <QPushButton>

namespace voreen {


PlotDataPropertyWidget::PlotDataPropertyWidget(PlotDataProperty* prop, QWidget* parent)
    : QPropertyWidgetWithEditorWindow(prop, parent, true, false)
    , editBt_(new QPushButton(tr("Show Data")))
    , property_(prop)
    , plotDataWidget_(0)
    , updateFromPropertyFlag_(true)
{
    createEditorWindow(Qt::LeftDockWidgetArea);

    addWidget(editBt_);
    connect(editBt_, SIGNAL(clicked()), this, SLOT(toggleWidgetVisibility()));

    addVisibilityControls();
    QFontInfo fontInfo(font());
    editBt_->setFont(QFont(fontInfo.family(), QPropertyWidget::fontSize_));
}

void PlotDataPropertyWidget::updateFromProperty() {
    updateFromPropertyFlag_ = true;
}

void PlotDataPropertyWidget::toggleWidgetVisibility() {
    tgtAssert(editorWindow_, "No editor window");
    if (updateFromPropertyFlag_) {
        updateFromPropertyFlag_ = false;
        if (plotDataWidget_)
            plotDataWidget_->updateFromPlotData();
    }
    editorWindow_->setVisible(!editorWindow_->isVisible());
}

void PlotDataPropertyWidget::customizeEditorWindow() {
    editorWindow_->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    editorWindow_->setFloating(true);
}

QWidget* PlotDataPropertyWidget::createEditorWindowWidget() {
    delete plotDataWidget_;

    plotDataWidget_ = new PlotDataWidget(property_, parentWidget());
    plotDataWidget_->createWidgets();

    return plotDataWidget_;
}

Property* PlotDataPropertyWidget::getProperty() {
    return property_;
}

} // namespace
