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

#ifndef VRN_ORIENTATIONOVERLAY_H
#define VRN_ORIENTATIONOVERLAY_H

#include "voreen/core/processors/imageprocessor.h"
#include "voreen/core/interaction/camerainteractionhandler.h"

#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/floatproperty.h"

#include "tgt/texture.h"

namespace voreen {

/**
 * Renders an orientation cube over the input rendering and
 * optionally draws a tripod.
 */
class VRN_CORE_API OrientationOverlay : public ImageProcessor {
public:
    OrientationOverlay();
    ~OrientationOverlay();
    virtual Processor* create() const;

    virtual std::string getClassName() const    { return "OrientationOverlay"; }
    virtual std::string getCategory() const     { return "Image Processing"; }
    virtual CodeState getCodeState() const      { return CODE_STATE_STABLE; }

    virtual bool isReady() const;

protected:
    virtual void setDescriptions() {
        setDescription("Adds a cube and a tripod to the input rendering indicating the current orientation of the camera.");
    }

    virtual void beforeProcess();
    virtual void process();
    virtual void initialize() throw (tgt::Exception);
    virtual void deinitialize() throw (tgt::Exception);

private:

    /// Renders the orientation cube.
    void renderCube();

    /// Renders the tripod axes.
    void renderAxes();

    /// Loads (and create) needed textures.
    void loadTextures();

    /// Sets reloadTextures_ flag to true.
    void reloadTextures();

    RenderPort inport_;             ///< Input rendering the orientation overlay is drawn onto.
    RenderPort outport_;            ///< Output: input + orientation overlay
    RenderPort privatePort_;        ///< Stored the rendered orientation overlay.

    BoolProperty drawCube_;         ///< Setting whether to draw orientation cube.
    BoolProperty drawAxes_;         ///< Sets whether to draw orientation axes.
    BoolProperty drawTextures_;     ///< Sets whether to draw textures on cube.
    BoolProperty colorizeTextures_; ///< Sets whether to colorizes the textures by GL_MODULATE.

    FileDialogProperty filenameFront_;  ///< Filename of front texture.
    FileDialogProperty filenameBack_;   ///< Filename of back texture.
    FileDialogProperty filenameTop_;    ///< Filename of top texture.
    FileDialogProperty filenameBottom_; ///< Filename of bottom texture.
    FileDialogProperty filenameLeft_;   ///< Filename of left texture.
    FileDialogProperty filenameRight_;  ///< Filename of Right texture.

    FloatProperty shiftX_;              ///< Distance to shift cube and axis horizontally.
    FloatProperty shiftY_;              ///< Distance to shift cube and axis vertically.

    FloatProperty cubeSize_;            ///< Size of cube indicating orientation.
    FloatProperty axisLength_;          ///< Length of axes indicating orientation.

    CameraProperty camera_;

    // texture handles
    tgt::Texture* frontTex_;
    tgt::Texture* backTex_;
    tgt::Texture* topTex_;
    tgt::Texture* leftTex_;
    tgt::Texture* bottomTex_;
    tgt::Texture* rightTex_;

    std::string textureNames_[6];

    bool reloadTextures_;
    bool loadingTextures_;          ///< Set to true during texture loading for preventing
                                    ///  multiple/cyclic execution of loadTextures().

    /// category used in logging
    static const std::string loggerCat_;
};

} // namespace

#endif
