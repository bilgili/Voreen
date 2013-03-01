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

#ifndef VRN_MULTIPLANARSLICERENDERER_H
#define VRN_MULTIPLANARSLICERENDERER_H

#include "slicerendererbase.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/interaction/camerainteractionhandler.h"
#include "tgt/glmath.h"
#include "tgt/camera.h"

namespace tgt {
    class TextureUnit;
}

namespace voreen {

class VRN_CORE_API MultiplanarSliceRenderer : public SliceRendererBase {
public:
    MultiplanarSliceRenderer();
    virtual ~MultiplanarSliceRenderer();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "MultiplanarSliceRenderer"; }
    virtual std::string getCategory() const  { return "Slice Rendering"; }
    virtual CodeState getCodeState() const   { return CODE_STATE_STABLE; }

protected:
    virtual void process();
    virtual void beforeProcess();
    enum SliceAlignment { SLICE_XY, SLICE_XZ, SLICE_YZ };

protected:
    virtual void renderSlice(SliceAlignment sliceAlign, int sliceNo, tgt::TextureUnit& texUnit);
    virtual void updateNumSlices();

protected:
    virtual void setDescriptions() {
        setDescription("Renders three orthogonal slices, aligned to the x-, y- and z-axis.");
    }

    BoolProperty renderXYSlice_;
    BoolProperty renderXZSlice_;
    BoolProperty renderYZSlice_;
    IntProperty sliceNumberXY_;
    IntProperty sliceNumberXZ_;
    IntProperty sliceNumberYZ_;
    CameraProperty camProp_;
    CameraInteractionHandler* cameraHandler_;

    static const std::string loggerCat_;
};

}   // namespace

#endif
