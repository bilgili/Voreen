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

#ifndef VRN_RENDERPROCESSOR_H
#define VRN_RENDERPROCESSOR_H

#include <vector>

#include "tgt/shadermanager.h"
#include "tgt/camera.h"

#include "voreen/core/datastructures/rendertarget.h"
#include "voreen/core/processors/processor.h"
#include "voreen/core/ports/allports.h"
#include "voreen/core/properties/allproperties.h"

namespace voreen {

/**
 * The base class for all processor classes that render to RenderPorts.
 */
class RenderProcessor : public Processor {
public:

    RenderProcessor();
    virtual ~RenderProcessor();

    /**
     * In addition to the invalidations performed by the Processor base class,
     * the results of all RenderPort outports are invalidated.
     *
     * @see Processor::invalidate
     */
    virtual void invalidate(int inv = INVALID_RESULT);

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

    /**
     * Returns the registered private render ports of this processor.
     *
     * \sa addPrivateRenderPort
     */
    const std::vector<RenderPort*>& getPrivateRenderPorts() const;

protected:
    /// This method is called when the processor should be processed.
    virtual void process() = 0;

    /**
     * Initializes all private RenderPorts.
     */
    virtual void initialize() throw (VoreenException);

    /**
     * Deinitializes all private RenderPorts.
     */
    virtual void deinitialize() throw (VoreenException);

    /**
     * Calls adjustRenderOutportDimensions().
     *
     * @see Processor::beforeProcess
     */
    virtual void beforeProcess();

    /**
     * Adjusts the dimensions of all render outports to the size
     * of the largest render outport (if the outports have no size origin assigned).
     */
    virtual void adjustRenderOutportDimensions();

    /**
     * Allocate or deallocate RenderTargets in outports.
     */
    void manageRenderTargets();

    static const std::string loggerCat_; ///< category used in logging

    /// @todo documentation
    void addPrivateRenderPort(RenderPort* port);
    void addPrivateRenderPort(RenderPort& port);

    //---------------------------------------------------------
    //Some deprecated stuff:

    /// Renders a screen aligned quad.
    void renderQuad();

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
     * @param camera camera whose position is passed to uniform cameraPosition_, also needed for passing matrices
     * @param screenDim dimensions of the render target's viewport. Is no parameter is passed,
     *      the dimensions of the first outport is chosen.
     */
    virtual void setGlobalShaderParameters(tgt::Shader* shader, tgt::Camera* camera = 0, tgt::ivec2 screenDim = tgt::ivec2(-1));
    //---------------------------------------------------------

    /// used for cycle prevention during render port size propagation
    bool portResizeVisited_;
private:
    /// The private render ports this processor has. Private ports
    /// are mapped to rendertargets no other processor has access to.
    std::vector<RenderPort*> privateRenderPorts_;

    /// used for cycle prevention during size origin test
    mutable bool testSizeOriginVisited_;

};

} // namespace voreen

#endif // VRN_RENDERPROCESSOR_H
