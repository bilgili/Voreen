/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#ifndef VRN_BACKGROUND_H
#define VRN_BACKGROUND_H

#include "voreen/core/vis/processors/image/imageprocessor.h"

namespace voreen {

/**
 * Creates a special background.
 */
class Background : public ImageProcessor {
public:
    Background();
    ~Background();

    virtual const Identifier getClassName() const;
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const;

    virtual int initializeGL();

    /**
    * Render-Method: draws the existing content over a background
    */
    void process(LocalPortMapping* portMapping);

    /// Possible backgrounds:
    enum BackgroundModes {
        MONOCHROME, ///< Paint a monochrome background using the current background color.
        GRADIENT,   ///< Paint a color gradient.
        RADIAL,     ///< Paint a radial
        CLOUD,      ///< Paint procedural clouds
        TEXTURE     ///< Paint an image.
    };

    static const Identifier setBackgroundFirstColor_;
    static const Identifier setBackgroundSecondColor_;
    static const Identifier setBackgroundAngle_;

    static const Identifier shadeTexUnit0_;
    static const Identifier depthTexUnit0_;
    static const Identifier shadeTexUnit1_;
    static const Identifier depthTexUnit1_;

protected:

    virtual std::string generateHeader();
    /**
    * Render the background
    */
    void renderBackground();

    void setBackgroundModeEvt();

    /**
    * load (and create) needed textures
    */
    void loadTexture();
    /**
    * create an alpha-circle
    */
    void createRadialTexture();
    /**
    * create a cloud texture
    */
    void createCloudTexture();
    /**
    * blurs an image really slow
    */
    GLubyte* blur(GLubyte* image, int size);
    /**
    * doubles an image and blurs it afterwards
    */
    GLubyte* resize(GLubyte* image, int size);
    /**
    * copy an image 4 times
    */
    GLubyte* tile(GLubyte* image, int size);

    ColorProp firstcolor_;
    ColorProp secondcolor_;
    IntProp angle_;
    tgt::Texture* tex_;
    bool textureloaded_;
    FileDialogProp filename_;
    FloatProp tile_;
    BackgroundModes mode_;
    EnumProp* modeProp_;

};

} // namespace

#endif
