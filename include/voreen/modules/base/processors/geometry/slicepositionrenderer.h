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

#ifndef VRN_SLICEPOSITIONRENDERER_H
#define VRN_SLICEPOSITIONRENDERER_H

#include "voreen/core/processors/geometryrendererbase.h"

#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/vectorproperty.h"

#include "voreen/core/ports/volumeport.h"

namespace voreen {

class SlicePositionRenderer : public GeometryRendererBase {
public:
    SlicePositionRenderer();
    virtual Processor* create() const;

    virtual std::string getCategory() const  { return "Geometry"; }
    virtual std::string getClassName() const { return "SlicePositionRenderer"; }
    virtual CodeState getCodeState() const   { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;

    virtual void render();
    virtual void invalidate(int inv = INVALID_RESULT);

protected:
    virtual void process();

private:
    FloatVec4Property xColor_;
    FloatVec4Property yColor_;
    FloatVec4Property zColor_;
    IntProperty xSliceIndexProp_;
    IntProperty ySliceIndexProp_;
    IntProperty zSliceIndexProp_;

    FloatProperty width_;
    IntProperty stippleFactor_;
    IntProperty stipplePattern_;
    VolumePort inport_;
};

}

#endif

