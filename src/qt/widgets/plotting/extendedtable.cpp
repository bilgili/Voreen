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



namespace voreen {


ExtendedTable::ExtendedTable(QWidget* parent)
    : QTableView(parent)
{
    setLocale(QLocale(QLocale::English));
}

ExtendedTable::~ExtendedTable() {
}


std::pair<QPoint,QPoint> ExtendedTable::selectedArea() const {
    QModelIndexList list = selectedIndexes();
    int x1 = -1;
    int y1 = -1;
    int y2 = -1;
    int x2 = -1;
    for (int i = 0; i < list.size(); ++i) {
        if (i == 0) {
            x1 = list.at(i).column();
            x2 = list.at(i).column();
            y1 = list.at(i).row();
            y2 = list.at(i).row();
        }
        x1 = std::min<int>(x1,list.at(i).column());
        y1 = std::min<int>(y1,list.at(i).row());
        x2 = std::max<int>(x2,list.at(i).column());
        y2 = std::max<int>(y2,list.at(i).row());
    }
    std::pair<QPoint,QPoint> ergebnis;
    ergebnis.first = QPoint(x1,y1);
    ergebnis.second = QPoint(x2,y2);
    return ergebnis;
}

std::vector<int> ExtendedTable::selectedColumns() const {
    QModelIndexList list = selectedIndexes();
    std::vector<int> result;
    bool match;
    for (int i = 0; i < list.size(); ++i) {
        match = true;
        for (size_t j = 0; j < result.size(); ++j) {
            if (result.at(j) == list.at(i).column()) {
                match = false;
                break;
            }
        }
        if (match) {
            result.push_back(list.at(i).column());
        }
    }
    return result;
}

std::vector<int> ExtendedTable::selectedRows() const {
    QModelIndexList list = selectedIndexes();
    std::vector<int> result;
    bool match;
    for (int i = 0; i < list.size(); ++i) {
        match = true;
        for (size_t j = 0; j < result.size(); ++j) {
            if (result.at(j) == list.at(i).row()) {
                match = false;
                break;
            }
        }
        if (match) {
            result.push_back(list.at(i).row());
        }
    }
    return result;
}

} // namespace voreen
