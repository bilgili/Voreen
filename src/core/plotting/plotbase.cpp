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

#include "voreen/core/plotting/plotbase.h"
#include "voreen/core/utils/exception.h"
#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include <algorithm>


namespace voreen {

PlotBase::PlotBase(int keyColumnCount, int dataColumnCount)
    : keyColumnCount_(keyColumnCount)
    , dataColumnCount_(dataColumnCount)
{
    tgtAssert((keyColumnCount_ >= 0 && dataColumnCount_ >= 0), "PlotBase: negativ ColumnCounts are not allowed!!");
    int columnCount = keyColumnCount + dataColumnCount;
    if (columnCount > 0) {
        columnLabels_ = new std::string[columnCount];
        columnTypes_ = new ColumnType[columnCount];
        for (int i = 0; i < columnCount; ++i) {
            columnTypes_[i] = EMPTY;
            columnLabels_[i] = "";
        }
    }
    else {
        columnLabels_ = NULL;
        columnTypes_ = NULL;
    }
}

PlotBase::PlotBase(const PlotBase& rhs)
    : keyColumnCount_(rhs.keyColumnCount_)
    , dataColumnCount_(rhs.dataColumnCount_)
{
    int columnCount = keyColumnCount_ + dataColumnCount_;
    if (columnCount > 0) {
        columnLabels_ = new std::string[columnCount];
        columnTypes_ = new ColumnType[columnCount];
    }
    else {
        columnLabels_ = NULL;
        columnTypes_ = NULL;
    }
    for (int i=0; i<columnCount; ++i) {
        columnLabels_[i] = rhs.columnLabels_[i];
        columnTypes_[i] = rhs.columnTypes_[i];
    }
}

PlotBase::~PlotBase() {
    delete [] columnLabels_;
    delete [] columnTypes_;
}

PlotBase& PlotBase::operator=(const PlotBase& rhs) {
    // prevent self assignment
    if (this == &rhs)
        return *this;

    std::string* newColumnLabels = 0;
    ColumnType* newColumnTypes = 0;

    // copy array data to new arrays
    // (exception safety: if anything fails the old data is still intact and the object in a valid state)
    try {
        int sum = rhs.keyColumnCount_ + rhs.dataColumnCount_;
        if (sum > 0) {
            newColumnLabels = new std::string[sum];
            newColumnTypes = new ColumnType[sum];
        }
        for (int i=0; i<sum; ++i) {
            newColumnLabels[i] = rhs.columnLabels_[i];
            newColumnTypes[i] = rhs.columnTypes_[i];
        }
    }
    catch (std::bad_alloc&) {
        // nothing has been destroyed yet, current state is valid
        LERRORC("PlotBase::operator=()", "bad_alloc occured, object was not changed!");
        return *this;
    }
    catch (...) {
        // nothing has been destroyed yet, current state is valid
        LERRORC("PlotBase::operator=()", "unknown exception occured, object was not changed!");
        return *this;
    }

    // everything here is exception safe:
    keyColumnCount_ = rhs.keyColumnCount_;
    dataColumnCount_ = rhs.dataColumnCount_;
    // exchange pointers via swap (exception safe) and delete old arrays
    std::swap(columnLabels_, newColumnLabels);
    std::swap(columnTypes_, newColumnTypes);
    delete [] newColumnLabels;
    delete [] newColumnTypes;

    return *this;
}

int PlotBase::getKeyColumnCount() const {
    return keyColumnCount_;
}

int PlotBase::getDataColumnCount() const {
    return dataColumnCount_;
}

int PlotBase::getColumnCount() const {
    return keyColumnCount_ + dataColumnCount_;
}

const std::string& PlotBase::getColumnLabel(int column) const {
    tgtAssert((column >= 0 && column < keyColumnCount_ + dataColumnCount_), "PlotBase::getColumnLabel(): Column out of bounds.");
    return columnLabels_[column];
}

void PlotBase::setColumnLabel(int column, std::string label) {
    tgtAssert((column < keyColumnCount_ + dataColumnCount_), "PlotBase::setColumnLabel(): Column out of bounds.");
    columnLabels_[column] = label;
}

PlotBase::ColumnType PlotBase::getColumnType(int column) const {
    tgtAssert((column < keyColumnCount_ + dataColumnCount_), "PlotBase::getColumnType(): Column out of bounds.");
    return columnTypes_[column];
}

void PlotBase::setColumnType(int column, ColumnType type) {
    tgtAssert((column < keyColumnCount_ + dataColumnCount_), "PlotBase::setColumnType(): Column out of bounds.");
    columnTypes_[column] = type;
}

void PlotBase::reset(int keyColumnCount, int dataColumnCount) {
    delete [] columnLabels_;
    delete [] columnTypes_;

    keyColumnCount_ = keyColumnCount;
    dataColumnCount_ = dataColumnCount;
    int columnCount = keyColumnCount+dataColumnCount;
    if (columnCount > 0) {
        columnLabels_ = new std::string[columnCount];
        columnTypes_ = new ColumnType[columnCount];
        for (int i = 0; i < columnCount; ++i) {
            columnTypes_[i] = EMPTY;
            columnLabels_[i] = "";
        }
    }
    else {
        columnLabels_ = NULL;
        columnTypes_ = NULL;
    }
}

bool PlotBase::compareStructure(const PlotBase* other) const {
    if (!other)
        return false;

    if (keyColumnCount_ != other->getKeyColumnCount() || dataColumnCount_ != other->getDataColumnCount())
        return false;

    for (int i=0; i< keyColumnCount_ * dataColumnCount_; ++i) {
        if (columnLabels_[i] != other->getColumnLabel(i) || columnTypes_[i] != other->getColumnType(i))
            return false;
    }

    return true;
}

}
