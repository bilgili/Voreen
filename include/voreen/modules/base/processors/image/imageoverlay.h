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
#include "tgt/texturemanager.h"
#include "tgt/gpucapabilities.h"

namespace voreen {

/**
 * Overlays an image on top of the rendering.
 */
class ImageOverlay : public ImageProcessor {
public:
    ImageOverlay();
    ~ImageOverlay();
    virtual Processor* create() const;

    virtual std::string getClassName() const  { return "ImageOverlay";     }
    virtual std::string getCategory() const   { return "Image Processing"; }
    virtual CodeState getCodeState() const    { return CODE_STATE_STABLE;  }
    virtual std::string getProcessorInfo() const;

protected:
    virtual void process();
    virtual void initialize() throw (VoreenException);
    virtual void deinitialize() throw (VoreenException);

    virtual std::string generateHeader();
    /**
    * Render chosen image over scene
    */
    void renderOverlayImage();

    void setOverlayImageModeEvt();

    /**
    * load (and create) needed textures
    */
    void loadTexture();

    tgt::Texture* tex_;
    bool textureLoaded_;
    FileDialogProperty filename_;

    FloatProperty left_;        // between 0 and 1
    FloatProperty top_;            // between 0 and 1
    FloatProperty width_;        // between 0 and 1-left_
    FloatProperty height_;        // between 0 and 1-top_
    FloatProperty opacity_;        // between 0 and 1

    RenderPort inport_;
    RenderPort outport_;
    RenderPort privatePort_;
};

} // namespace

#endif
