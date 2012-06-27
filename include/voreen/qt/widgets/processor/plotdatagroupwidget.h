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

#ifndef VRN_PLOTDATAGROUPWIDGET_H
#define VRN_PLOTDATAGROUPWIDGET_H

#include "voreen/modules/base/processors/plotting/plotdatagroup.h"
#include "voreen/qt/widgets/processor/qprocessorwidget.h"

class QMenu;

namespace voreen {

class ExtendedTable;
class PlotDataExtendedTableModel;


class PlotDataGroupWidget : public QProcessorWidget {
    Q_OBJECT
public:
    PlotDataGroupWidget(QWidget* parent, PlotDataGroup* PlotDataGroup);
    virtual ~PlotDataGroupWidget();

    void initialize();

    virtual void updateFromProcessor();

public slots:

protected:

private:
    PlotDataExtendedTableModel* proxyModel_;
    ExtendedTable* table_;
    QMenu* contextMenuTable_;
    QMenu* addFunctionMenu_;
    QMenu* averageFunctionMenu_;
    QMenu* histogramFunctionMenu_;
    std::vector<QWidget*> widgetVector_;
    std::vector<std::pair<int, AggregationFunction*> > aggregatedcolumns;

    static const std::string loggerCat_;

private slots:
    void tableContextMenu(const QPoint& pos);
    void addFunction();

    void selectResetAll();
    void selectResetLast();
    void groupby();

};

} // namespace voreen

#endif // VRN_PLOTDATAGROUPWIDGET_H

