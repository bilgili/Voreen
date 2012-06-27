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

#include "voreen/core/plotting/plotselection.h"
#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

PlotSelection::PlotSelection(tgt::ivec2 tablePosition)
    : isTablePositionFlag_(true)
    , tablePosition_(tablePosition)
{};

PlotSelection::PlotSelection(std::vector<std::pair<int, PlotPredicate*> > selection)
    : isTablePositionFlag_(false)
    , tablePosition_(-1,-1)
    , selection_(selection)
{};

void PlotSelection::serialize(XmlSerializer& s) const {
    s.serialize("isTablePositionFlag", isTablePositionFlag_);
    s.serialize("tablePosition", tablePosition_);
}

void PlotSelection::deserialize(XmlDeserializer& s) {
    s.deserialize("isTablePositionFlag", isTablePositionFlag_);
    s.deserialize("tablePosition", tablePosition_);
}

bool PlotSelection::isTablePosition() {
    return isTablePositionFlag_;
}

tgt::ivec2 PlotSelection::getTablePosition() {
    tgtAssert(isTablePositionFlag_, "No table position");
    return tablePosition_;
}

std::vector<std::pair<int, PlotPredicate*> > PlotSelection::getSelection() {
    tgtAssert(!isTablePositionFlag_, "Is table position");
    return selection_;
}

} // namespace voreen
