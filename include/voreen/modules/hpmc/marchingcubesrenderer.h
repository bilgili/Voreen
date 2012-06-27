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

#ifndef VRN_MARCHINGCUBESRENDERER_H
#define VRN_MARCHINGCUBESRENDERER_H

#include "voreen/core/processors/geometryrendererbase.h"
#include "hpmc/hpmc.h"

#include "voreen/core/ports/allports.h"
#include "voreen/core/properties/allproperties.h"

namespace voreen {

class MarchingCubesRenderer : public GeometryRendererBase {
public:
    MarchingCubesRenderer();
    virtual ~MarchingCubesRenderer();
    virtual Processor* create() const;

    virtual std::string getCategory() const { return "Renderer"; }
    virtual std::string getClassName() const { return "MarchingCubesRenderer"; }
    virtual CodeState getCodeState() const { return CODE_STATE_EXPERIMENTAL; }
    virtual std::string getProcessorInfo() const;

    virtual bool isReady() const;

    virtual void render();

protected:
    virtual void process();
    virtual void initialize() throw (VoreenException);
    virtual void deinitialize() throw (VoreenException);

    /**
     * Load the needed shader.
     */
    virtual void loadShader();

    virtual std::string generateExtractHeader(VolumeHandle* volumeHandle=0);
    virtual std::string generateRenderHeader();
    virtual void compile(VolumeHandle* volumeHandle);

    void setLightingParameters();

    void releaseHPMCResources();

    VolumePort volumeInport_;
    RenderPort privatePort_;

    // hpmc variables
    struct HPMCConstants* hpmcC_;
    struct HPMCHistoPyramid* hpmcH_;
    struct HPMCTraversalHandle* hpmcTraversalHandle_;
    tgt::ivec3 gridSize_;
    GLsizei numVertices_;

    // stuff needed for vertex buffers
    GLuint mcTriVBO_;
    GLsizei mcTriVBON_;

    static const std::string volTexUnit_;
    tgt::Shader* mcExtractPrg_;
    tgt::Shader* mcRenderPrg_;

    FloatProperty isoValue_;
    IntOptionProperty gridSizeOption_;

    FloatVec4Property surfaceColor_;
    FloatProperty surfaceAlpha_;

    BoolProperty useLighting_;
    BoolProperty lightPosRelativeToViewer_;

    /// The position of the light source used for lighting calculations in world coordinates
    FloatVec4Property lightPosition_;
    /// The light source's ambient color according to the Phong lighting model
    FloatVec4Property lightAmbient_;
    /// The light source's diffuse color according to the Phong lighting model
    FloatVec4Property lightDiffuse_;
    /// The light source's specular color according to the Phong lighting model
    FloatVec4Property lightSpecular_;
    /// The material's specular exponent according to the Phong lighting model
    FloatProperty materialShininess_;

    static const std::string loggerCat_;
};

} // namespace voreen

#endif //VRN_MARCHINGCUBESRENDERER_H
