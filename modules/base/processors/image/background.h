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

#ifndef VRN_BACKGROUND_H
#define VRN_BACKGROUND_H

#include "voreen/core/processors/imageprocessor.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/vectorproperty.h"

namespace voreen {

/**
 * Adds a background to the image.
 */
class VRN_CORE_API Background : public ImageProcessor {
public:
    Background();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "Background"; }
    virtual std::string getCategory() const  { return "Image Processing"; }
    virtual CodeState getCodeState() const   { return CODE_STATE_STABLE; }

    virtual bool isReady() const;

protected:
    virtual void setDescriptions() {
        setDescription("Adds a background, either single color, linear or radial gradient, procedural clouds or an image file.");
    }

    void process();
    virtual void initialize() throw (tgt::Exception);
    virtual void deinitialize() throw (tgt::Exception);

    /**
     * Render the background
     */
    void renderBackground();

    void onBackgroundModeChanged();

    /**
     * load (and create) needed textures
     */
    void loadTexture();

    /**
     * create an alpha-circle
     */
    void createRadialTexture();

    /**
     * create an empty texture, i.e. all texels set to zero
     */
    void createEmptyTexture();

    /**
     * create a cloud texture
     */
    void createCloudTexture();

    /**
     * Causes a reload of the texture
     * on next process.
     */
    void invalidateTexture();

    FloatVec4Property firstcolor_;
    FloatVec4Property secondcolor_;
    IntProperty angle_;
    tgt::Texture* tex_;
    bool textureLoaded_;
    FileDialogProperty filename_;
    FloatProperty tile_;
    StringOptionProperty modeProp_;

    RenderPort inport_;
    RenderPort outport_;
    RenderPort privatePort_;

    bool textureInvalid_;
};

} // namespace

#endif
