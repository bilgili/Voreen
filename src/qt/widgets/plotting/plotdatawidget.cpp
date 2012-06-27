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

#include "voreen/qt/widgets/plotting/extendedtable.h"
#include "voreen/qt/widgets/plotting/plotdatawidget.h"
#include "voreen/core/properties/plotdataproperty.h"
#include "voreen/core/plotting/plotdata.h"

namespace voreen {

PlotDataWidget::PlotDataWidget(PlotDataProperty* prop, QWidget* parent)
    : QWidget(parent)
    , property_(prop) {
    setObjectName("PlotDataWidget");
    tgtAssert(property_, "No PlotDataProperty");
    //the data in property could be undefined at this position, so used 0 and not property_->get()(savty first)
    proxyModel = new PlotDataSimpleTableModel(0, this);
}

PlotDataWidget::~PlotDataWidget() {
    delete proxyModel;
}

void PlotDataWidget::createWidgets() {
    QVBoxLayout* mainLayout = new QVBoxLayout();
    table_ = new ExtendedTable();
    table_->setModel(proxyModel);
    table_->setSortingEnabled(false);
    table_->verticalHeader()->hide();
    table_->resizeColumnsToContents();
    mainLayout->addWidget(table_);
    setLayout(mainLayout);
}

void PlotDataWidget::updateFromPlotData() {
    tgtAssert(property_, "No PlotDataProperty");
    delete proxyModel;

    proxyModel = new PlotDataSimpleTableModel(property_->get(), this);
    table_->setModel(proxyModel);
    table_->verticalHeader()->hide();
    table_->resizeColumnsToContents();
}

} // namespace voreen
