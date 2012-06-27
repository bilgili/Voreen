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

#ifndef VRN_RENDERPROCESSOR_H
#define VRN_RENDERPROCESSOR_H

#include <vector>

#include "tgt/shadermanager.h"
#include "tgt/camera.h"

#include "voreen/core/opengl/texunitmapper.h"
#include "voreen/core/vis/rendertarget.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/ports/allports.h"
#include "voreen/core/vis/properties/allproperties.h"

namespace voreen {

/**
 * The base class for all processor classes that render to RenderPorts
 */
class RenderProcessor : public Processor {
public:

    RenderProcessor();
    virtual ~RenderProcessor();

    /// This method is called when the processor should be processed.
    virtual void process() = 0;

    /**
     * @brief Notifies the Processor of a changed SizeOrigin on port p.
     *
     * This default implementation notifies all (render-) inports of this change.
     *
     * If p has a NULL sizeOrigin this is only propagated if all outports have a NULL sizeOrigin.
     */
    virtual void sizeOriginChanged(RenderPort* p);

    /**
     * @brief Requests a resize of RenderPort p to newsize.
     *
     *  This default implementation resizes all (render-) outports and private ports to newsize
     *  and requests a resize on all (render-) inports.
     *
     *  viewportChanged(newsize) is called on all CameraProperties and the Processor is invalidated.
     *
     * @param p The RenderPort to resize. (Not automatically resized to allow interaction coarseness)
     * @param newsize The requested size.
     */
    virtual void portResized(RenderPort* p, tgt::ivec2 newsize);
    ///Test if a textureContainerChanged on port p with so would result in a conflict
    virtual bool testSizeOrigin(const RenderPort* p, void* so) const;

    ///Initialize all RenderPorts and calls Processor::initialize()
    virtual void initialize() throw (VoreenException);

    /**
     * Returns the registered private render ports of this processor.
     *
     * \sa addPrivateRenderPort
     */
    const std::vector<RenderPort*>& getPrivateRenderPorts() const;

    static const std::string XmlElementName_;

protected:
    TexUnitMapper tm_;      ///< manages texture units

    static const std::string loggerCat_; ///< category used in logging

    /// @todo documentation
    void addPrivateRenderPort(RenderPort* port);
    void addPrivateRenderPort(RenderPort& port);

    //---------------------------------------------------------
    //Some deprecated stuff:

    /// Renders a screen aligned quad.
    void renderQuad();

    /// Renders a screen aligned quad and assigns texture coordinates to its vertices.
    void renderQuadWithTexCoords(const RenderPort& port);

    /**
     * This generates the header that will be used at the beginning of the shaders. It includes the necessary #defines that
     * are to be used with the shaders.
     * \note If you overwrite this function in a subclass, you have to the call the superclass' function first and
     * append your additions to its result!
     */
    virtual std::string generateHeader();

    /**
     * Sets some uniforms potentially needed by every shader.
     * @note This function should be called for every shader before every rendering pass!
     *
     * @param shader the shader to set up
     * @param camera camera whose position is passed to uniform cameraPosition_
     */
    virtual void setGlobalShaderParameters(tgt::Shader* shader, tgt::Camera* camera = 0);

    ColorProperty backgroundColor_; ///< the color of the background
    //---------------------------------------------------------

private:
    /// The private render ports this processor has. Private ports
    /// are mapped to rendertargets no other processor has access to.
    std::vector<RenderPort*> privateRenderPorts_;
};

} // namespace voreen

#endif // VRN_RENDERPROCESSOR_H
