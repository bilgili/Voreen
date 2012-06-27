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

#ifndef VRN_EXTENDEDTABLE_H
#define VRN_EXTENDEDTABLE_H

#include <QTableView>

namespace voreen {

class ExtendedTable : public QTableView {

public:
    ExtendedTable(QWidget* parent=0);
    ~ExtendedTable();

    /// Gives back the topleft and buttomright Point of the selected Area.
    std::pair<QPoint,QPoint> selectedArea() const;
    /// Gives back the selected Columns.
    std::vector<int> selectedColumns() const;
    /// Gives back the selected Rows.
    std::vector<int> selectedRows() const;
    /// @see: QTableView::selectedIndexes();
    virtual QModelIndexList selectedIndexes() const;
};

}
#endif // VRN_EXTENDEDTABLE_H
