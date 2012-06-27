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

#ifndef VRN_MULTISURFACERENDERER_H
#define VRN_MULTISURFACERENDERER_H

#include "voreen/modules/base/processors/geometry/geometryrenderer.h"
#include <hpmc.h>

namespace voreen {

class MultiSurfaceRenderer : public GeometryRenderer {
public:

    MultiSurfaceRenderer();
    virtual ~MultiSurfaceRenderer();

    virtual std::string getCategory() const { return "Renderer"; }
    virtual std::string getClassName() const { return "MultiSurfaceRenderer"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual CodeState getCodeState() const { return Processor::CODE_STATE_EXPERIMENTAL; }
    virtual std::string getProcessorInfo() const;
    virtual Processor* create() const { return new MultiSurfaceRenderer(); }

    virtual void render();


protected:
    virtual void process();
    virtual void initialize() throw (VoreenException);

    /**
     * Load the needed shader.
     */
    virtual void loadShader();

    virtual std::string generateHeader(VolumeHandle* volumeHandle=0);
    virtual void compile(VolumeHandle* volumeHandle);

    // hpmc variables
    struct HPMCConstants* hpmcC_;
    struct HPMCHistoPyramid* hpmcH_;
    struct HPMCTraversalHandle* hpmcTraversalHandle_;
    GLsizei numVertices_;

    // stuff needed for vertex buffers
    GLuint* mcTriVBO_;
    GLsizei* mcTriVBON_;
    std::vector<float> isoValues_;
    std::vector<bool> isoValueHasChanged_;

    tgt::Shader* mcExtractPrg_;
    tgt::Shader* mcRenderPrg_;

    VolumePort volumeInport_;
    RenderPort privatePort_;

    static const std::string loggerCat_;
};

} // namespace voreen

#endif //VRN_MULTISURFACERENDERER_H
