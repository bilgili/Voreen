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

#ifndef VRN_PLOTSELECTION_H
#define VRN_PLOTSELECTION_H

#include "voreen/core/plotting/plotpredicate.h"
#include "tgt/vector.h"

namespace voreen {

/**
 *
 */
class PlotSelection : public Serializable {
public:

    PlotSelection(tgt::ivec2 tablePosition);
    PlotSelection(std::vector<std::pair<int, PlotPredicate*> > selection);

    /**
    * @see Property::serialize
    */
    virtual void serialize(XmlSerializer& s) const;

    /**
    * @see Property::deserialize
    */
    virtual void deserialize(XmlDeserializer& s);

    bool isTablePosition();

    tgt::ivec2 getTablePosition();
    std::vector<std::pair<int, PlotPredicate*> > getSelection();

private:

    bool isTablePositionFlag_;
    tgt::ivec2 tablePosition_;
    std::vector<std::pair<int, PlotPredicate*> > selection_;

};

} // namespace voreen

#endif // VRN_PLOTSELECTION_H
