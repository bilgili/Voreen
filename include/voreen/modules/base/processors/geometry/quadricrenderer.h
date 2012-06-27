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

#ifndef VRN_QUADRICRENDERER_H
#define VRN_QUADRICRENDERER_H

#include "voreen/core/processors/geometryrendererbase.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/optionproperty.h"

namespace voreen {

/**
 * Renders a GLU quadric.
 */
class QuadricRenderer : public GeometryRendererBase {
public:
    QuadricRenderer();
    virtual Processor* create() const;

    virtual std::string getClassName() const    { return "QuadricRenderer";  }
    virtual std::string getCategory() const     { return "Geometry";         }
    virtual CodeState getCodeState() const      { return CODE_STATE_TESTING; }
    virtual std::string getProcessorInfo() const;

    virtual void render();

protected:
    virtual void initialize() throw (VoreenException);

private:
    void adjustPropertyVisibilities();

    BoolProperty enabled_;
    StringOptionProperty quadricType_;
    
    FloatVec3Property position_;
    FloatVec3Property start_;
    FloatVec3Property end_;
    FloatProperty radius_;
    FloatVec4Property color_;

    BoolProperty applyLighting_;
    FloatVec4Property lightPosition_;
    FloatVec4Property lightAmbient_;
    FloatVec4Property lightDiffuse_;
    FloatVec4Property lightSpecular_;
    FloatProperty materialShininess_;

};

}

#endif // VRN_QUADRICRENDERER_H

