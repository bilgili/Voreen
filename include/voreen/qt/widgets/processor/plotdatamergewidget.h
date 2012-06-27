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

#ifndef VRN_PLOTDATAMERGEWIDGET_H
#define VRN_PLOTDATAMERGEWIDGET_H

#include "voreen/modules/base/processors/plotting/plotdatamerge.h"
#include "voreen/qt/widgets/processor/qprocessorwidget.h"

class QMenu;

namespace voreen {

class ExtendedTable;
class PlotDataExtendedTableModel;

class PlotDataMergeWidget : public QProcessorWidget {
    Q_OBJECT
public:
    PlotDataMergeWidget(QWidget* parent, PlotDataMerge* plotDataMerge);
    virtual ~PlotDataMergeWidget();

    void initialize();

    virtual void updateFromProcessor();

private:
    PlotDataExtendedTableModel* proxyModelFirst_;
    PlotDataExtendedTableModel* proxyModelSecond_;
    PlotDataExtendedTableModel* proxyModelOut_;
    ExtendedTable* tableFirst_;
    ExtendedTable* tableSecond_;
    ExtendedTable* tableOut_;
    QMenu* contextMenu_;

private slots:
    void tableContextMenu(const QPoint& pos);
    void swapPlotData();
    void ignoreColumnLabels();

};

} // namespace voreen

#endif // VRN_PLOTDATAMERGEWIDGET_H

