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
class ImageOverlay : public ImageProcessor {
public:
    ImageOverlay();
    virtual Processor* create() const;

    virtual std::string getClassName() const  { return "ImageOverlay";     }
    virtual std::string getCategory() const   { return "Image Processing"; }
    virtual CodeState getCodeState() const    { return CODE_STATE_STABLE;  }
    virtual std::string getProcessorInfo() const;

    bool isReady() const;

    virtual void sizeOriginChanged(RenderPort* p);
    virtual void portResized(RenderPort* p, tgt::ivec2 newsize);
    virtual bool testSizeOrigin(const RenderPort* p, void* so) const;

protected:
    virtual void beforeProcess();
    virtual void process();
    virtual void initialize() throw (VoreenException);
    virtual void deinitialize() throw (VoreenException);

    virtual std::string generateHeader();

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
