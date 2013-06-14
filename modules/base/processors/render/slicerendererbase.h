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

#ifndef VRN_SLICERENDERERBASE_H
#define VRN_SLICERENDERERBASE_H

#include <cstring>

#include "tgt/vector.h"
#include "tgt/matrix.h"
#include "tgt/shadermanager.h"

#include "voreen/core/processors/processor.h"
#include "voreen/core/processors/volumerenderer.h"
#include "voreen/core/datastructures/transfunc/transfunc.h"
#include "voreen/core/datastructures/volume/volumeslicehelper.h"

#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/fontproperty.h"

#include "voreen/core/ports/volumeport.h"

namespace tgt {
    class TextureUnit;
}

namespace voreen {

/**
 * Base class for all SliceRendering sub classes.
 * Provides basic functionality.
 */
class VRN_CORE_API SliceRendererBase : public VolumeRenderer {

public:
    enum TextureMode {
        TEXTURE_2D,
        TEXTURE_3D
    };

    SliceRendererBase();

protected:
    virtual void initialize() throw (tgt::Exception);
    virtual void deinitialize() throw (tgt::Exception);

    /**
     * \see Processor::beforeProcess()
     */
    virtual void beforeProcess();

    /**
     * Initializes the shader if applicable and sets all uniforms.
     */
    virtual bool setupVolumeShader(tgt::Shader* shader, const VolumeBase* volumeHandle, tgt::TextureUnit* volUnit, tgt::TextureUnit* transferUnit,
        const tgt::Camera* camera = 0, const tgt::vec4& lightPosition = tgt::vec4(0.f));

    /**
     * Activates the shader and passes the basic uniforms to it.
     */
    virtual bool setupSliceShader(tgt::Shader* shader, const VolumeBase* volumeHandle, const tgt::TextureUnit* transferUnit);

    /**
     * Binds the passed texture to the passed texture unit, uploads the texture data, assigns the tex unit to the shader.
     */
    virtual bool bindSliceTexture(tgt::Shader* shader, const VolumeBase* volumeHandle, tgt::Texture* sliceTexture, tgt::TextureUnit* texUnit);

    /**
     * Generates the header for the shader depending on the choice of
     * features to be used.
     */
    virtual std::string buildShaderHeader();

    /**
     * Recompiles the shader.
     */
    bool rebuildShader();

    /**
     * Deactivates the shader
     */
    void deactivateShader();

    /**
     * Creates a 2D texture from an axis-aligned slice of the passed volume, using nearest filtering.
     *
     * @note The passed texture coordinates must represent an axis-aligned slice,
     *      i.e., they must be equal in one dimension.
     *
     * @param volumeHandle the volume to create the slice texture from,
     *      its data type has to be OpenGL compatible.
     * @param texLowerLeft lower left of the slice in 3D texture coordinates (normalized)
     * @param texUpperRight upper right of the slice in 3D texture coordinates (normalized)

     * @return the created slice texture, whose dimensions and data type matches the input volume's properties.
     *      If the slice texture could not be created, 0 is returned.
     *
     */
    tgt::Texture* generateAlignedSliceTexture(const VolumeBase* volumeHandle, const tgt::vec3& texLowerLeft, const tgt::vec3& texUpperRight) const;

    /**
     * This function creates a texture of the distance legend. It is been saved in the legendPort.
     * The size is at least 256x256 to get readable letters.
     *
     * @note make sure no shaders other shaders are active during the call
     *
     * @param volumeHandle volume to create the legend for
     * @param legendColor color and opacity of the legend
     * @param legendPos normalized position of the legend on the slice
     *
     * @return the texture
     */
    tgt::Texture* createDistanceLegend(const VolumeBase* volumeHandle, tgt::vec4 legendColor, tgt::vec2 legendPos);

    /**
     * Draws a quad with the distance legend texture (the legendPort buffer) on it into the outport.
     *
     * @note createDistanceLegend() has to be called at least once before drawing the legend
     *
     * @param xl left coordinate on the x axis
     * @param xr right coordinate on the x axis
     * @param yd lower coordinate on the y axis
     * @param yu upper coordinate on the y axis
     */
    void drawDistanceLegend(float xl = 0.f, float xr = 1.f, float yd = 0.f, float yu = 1.f);

    RenderPort outport_;
    VolumePort inport_;
    RenderPort legendPort_;                   ///< private port used for creating the legend texture

    TransFuncProperty transferFunc_;

    OptionProperty<TextureMode> texMode_;     ///< use 2D slice textures or 3D volume texture?

    tgt::Shader* sliceShader_;
    tgt::Shader* legendShader_;               ///< shader for blending the legend over the slice

    static const std::string transFuncTexUnit_;
    static const std::string volTexUnit_;

    static const std::string loggerCat_;

private:
    /**
     * Helper function used by createLegend. It calculates the best scale unit and rounds the value for a better representation.
     *
     * @param oldValue the exact value befor rounding
     * @param outFloat the new "rounded" value
     * @param the text output
     */
    void scaleLegend(float oldValue, float &outFloat, std::string& outStr);

    void adjustPropertyVisibilities();

};

// ----------------------------------------------------------------------------

} // namespace voreen

#endif // VRN_SLICERENDERERBASE_H
