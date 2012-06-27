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

#ifndef VRN_LINKEVALUATORPLOTSELECTION_H
#define VRN_LINKEVALUATORPLOTSELECTION_H

#include "voreen/core/properties/link/linkevaluatorbase.h"

namespace voreen {

/**
 * This is a LinkEvaluator only for PlotSelectionProperty. It links only the selections and not the flags specifying how the selections are used.
 */
class LinkEvaluatorPlotSelection : public LinkEvaluatorBase {
public:

    virtual void eval(Property* src, Property* dest) throw (VoreenException);

    virtual std::string name() const;

    virtual std::string getClassName() const { return "LinkEvaluatorPlotSelection"; }

    virtual bool arePropertiesLinkable(const Property* p1, const Property* p2) const;

    virtual LinkEvaluatorBase* create() const { return new LinkEvaluatorPlotSelection(); }
};

} // namespace

#endif // VRN_LINKEVALUATORPLOTSELECTION_H
