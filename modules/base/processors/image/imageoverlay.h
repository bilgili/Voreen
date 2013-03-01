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

#ifndef VRN_IMAGEOVERLAY_H
#define VRN_IMAGEOVERLAY_H

#include "voreen/core/processors/imageprocessor.h"

#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "tgt/texturemanager.h"
#include "tgt/gpucapabilities.h"

namespace voreen {

/**
 * Overlays an image on top of the rendering.
 */
class VRN_CORE_API ImageOverlay : public ImageProcessor {
public:
    ImageOverlay();
    virtual Processor* create() const;

    virtual std::string getClassName() const  { return "ImageOverlay";     }
    virtual std::string getCategory() const   { return "Image Processing"; }
    virtual CodeState getCodeState() const    { return CODE_STATE_STABLE;  }

    bool isReady() const;

protected:
    virtual void setDescriptions() {
        setDescription("Renders an overlay on top of the input image. The overlay position and size is either specified in pixel coordinates or in normalized coordinates.");
    }

    virtual void beforeProcess();
    virtual void process();
    virtual void initialize() throw (tgt::Exception);
    virtual void deinitialize() throw (tgt::Exception);

    virtual std::string generateHeader(const tgt::GpuCapabilities::GlVersion* version = 0);

private:
    void overlayDimensionsChanged();

    RenderPort imageInport_;
    RenderPort overlayInport_;
    RenderPort outport_;

    BoolProperty renderOverlay_;
    BoolProperty usePixelCoordinates_;
    IntVec2Property overlayBottomLeft_;             ///< pixel coordinates
    IntVec2Property overlayDimensions_;             ///< pixel coordinates
    FloatVec2Property overlayBottomLeftRelative_;   ///< normalized coordinates
    FloatVec2Property overlayDimensionsRelative_;   ///< normalized coordinates
    FloatProperty overlayOpacity_;
    BoolProperty renderBorder_;
    FloatProperty borderWidth_;
    FloatVec4Property borderColor_;

    tgt::Shader* copyShader_;
};

} // namespace

#endif
