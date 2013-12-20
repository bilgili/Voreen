/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#ifndef VRN_TRIANGLEMESHCONVERTER_H
#define VRN_TRIANGLEMESHCONVERTER_H

#include "voreen/core/processors/processor.h"

#include "voreen/core/ports/geometryport.h"

#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/optionproperty.h"

namespace voreen {

/**
 * Clips the input geometry against an arbitrarily oriented clipping plane.
 */
class VRN_CORE_API TriangleMeshConverter : public Processor {
public:
    TriangleMeshConverter();
    virtual ~TriangleMeshConverter();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "TriangleMeshConverter"; }
    virtual std::string getCategory() const  { return "Geometry";         }
    virtual CodeState getCodeState() const   { return CODE_STATE_STABLE;  }

protected:
    virtual void setDescriptions() {
        setDescription("Can be used to transform between different types of triangle mesh");
    }

    virtual void process();

    GeometryPort inport_;        ///< Inport for a triangle mesh geometry to convert.
    GeometryPort outport_;       ///< Outport for a triangle mesh geometry that was converted.

    BoolProperty enabled_;       ///< Determines whether the conversion is performed.

    StringOptionProperty targetMesh_;   ///< Into what kind of mesh should the source mesh be converted?
    /// category used in logging
    static const std::string loggerCat_;
};

} //namespace

#endif // VRN_TRIANGLEMESHCONVERTER_H
