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

#ifndef VRN_PLOTDATAEXTENDEDTABLEMODEL_H
#define VRN_PLOTDATAEXTENDEDTABLEMODEL_H

#include "voreen/core/plotting/plotdata.h"
#include "voreen/core/plotting/functionlibrary.h"

#include "voreen/qt/widgets/plotting/plotdatasimpletablemodel.h"

#include <QModelIndex>
#include <QObject>

namespace voreen {

class PlotDataExtendedTableModel : public PlotDataSimpleTableModel {
public:
    PlotDataExtendedTableModel(PlotData* data,
        const std::vector<std::pair<int, std::string> >& StringVector,  QObject* parent=0,
         const QColor& keyColumnColor = QColor(200,200,200),
         const QColor& dataColumnColor = QColor(240,240,240));

    void setFunctionType(FunctionLibrary::ProcessorFunctionalityType type, bool showColumnType = false);
    int getAddLines() const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant getCellAt(int row, int column) const;
    PlotCellValue getPlotCellAt(int row, int column) const;

private:

    PlotData* data_;
    std::vector<std::pair<int, std::string > > stringVector_;
    int addLines_;
    FunctionLibrary::ProcessorFunctionalityType type_;
    bool showColumnType_;
};

}
#endif // VRN_PLOTSELSORTFILTERTABLEMODEL_H
