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

#ifndef VRN_PLOTDATASIMPLETABLEMODEL_H
#define VRN_PLOTDATASIMPLETABLEMODEL_H

#include <QAbstractTableModel>
#include <QObject>
#include <QVBoxLayout>
#include <QHeaderView>

#include <set>
#include "voreen/core/plotting/plotrow.h"

namespace voreen {

class PlotData;

class PlotDataSimpleTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    PlotDataSimpleTableModel(const PlotData* data, QObject *parent=0,
        const QColor& keyColumnColor = QColor(200,200,200,255),
        const QColor& dataColumnColor = QColor(240,240,240,255),
        const QColor& highlightedColor_ = QColor(200,240,240,255),
        const QColor& selectColumnColor = QColor(245,245,245,255),
        int selectedColumn = -1);

    /// @see QAbstractTableModel
    int rowCount(const QModelIndex &parent) const;
    /// @see QAbstractTableModel
    int columnCount(const QModelIndex &parent) const;
    /// @see QAbstractTableModel
    QVariant data(const QModelIndex &index, int role) const;
    /// @see QAbstractTableModel
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    /// @see QAbstractTableModel
    Qt::ItemFlags flags(const QModelIndex &index) const;
    /// change the KeyColumnColor
    void setKeyColumnColor(const QColor& keyColumnColor);
    /// change the DataColumnColor
    void setDataColumnColor(const QColor& dataColumnColor);

protected:
    // override the functions, if you want to show more lines above the datatable
    virtual QVariant getCellAt(int row, int column) const;
    // override the functions, if you want to show more lines above the datatable
    virtual PlotCellValue getPlotCellAt(int row, int column) const;

private:
    const PlotData* pData_;
    QColor keyColumnColor_;
    QColor dataColumnColor_;
    QColor highlightedColor_;
    QColor selectColumnColor_;
    int selectedColumn_;
};

}
#endif // VRN_PLOTDATASIMPLETABLEMODEL_H
