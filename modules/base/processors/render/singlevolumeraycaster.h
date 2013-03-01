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

#ifndef VRN_SINGLEVOLUMERAYCASTER_H
#define VRN_SINGLEVOLUMERAYCASTER_H

#include "voreen/core/processors/volumeraycaster.h"

#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/shaderproperty.h"

#include "voreen/core/ports/volumeport.h"

namespace voreen {

/**
 * This is the standard raycaster within Voreen. It allows to generate three output
 * renderings, whereas only the first one provides depth values. There are several
 * shading and compositing modes available.
 *
 * @see CubeMeshProxyGeometry, MeshEntryExitPoints
 * @see MultiVolumeRaycaster
 */
class VRN_CORE_API SingleVolumeRaycaster : public VolumeRaycaster {
public:
    SingleVolumeRaycaster();
    virtual Processor* create() const;

    virtual std::string getClassName() const    { return "SingleVolumeRaycaster"; }
    virtual std::string getCategory() const     { return "Raycasting"; }
    virtual CodeState getCodeState() const      { return CODE_STATE_STABLE; }

    /// All inports and at least one outport need to be connected
    virtual bool isReady() const;

protected:
    virtual void setDescriptions() {
        setDescription("This is the standard volume renderer in Voreen. It generates up to three output renderings, where the first one also provides depth values. Several shading and compositing modes are supported.\
<p>See CubeProxyGeometry, MeshEntryExitPoints.</p>");
    }

    /**
     * Recompiles the shader, if the invalidation level >= Processor::INVALID_PROGRAM.
     */
    virtual void beforeProcess();

    virtual void process();

    /**
     * Loads the shader and initializes the port group.
     */
    virtual void initialize() throw (tgt::Exception);

    /**
     * Deinitializes the port group and disposes the shader.
     */
    virtual void deinitialize() throw (tgt::Exception);

    /**
     * Adds compositing macros for the additional outports.
     *
     * @see VolumeRaycaster::generateHeader
     */
    virtual std::string generateHeader();

    /// Rebuilds the loaded shader.
    virtual void compile();

private:
    void adjustPropertyVisibilities();

    VolumePort volumeInport_;
    RenderPort entryPort_;
    RenderPort exitPort_;

    RenderPort outport_;
    RenderPort outport1_;
    RenderPort outport2_;

    // we render into internal buffers, which allows to reduce rendering size in interaction mode (coarseness)
    RenderPort internalRenderPort_;
    RenderPort internalRenderPort1_;
    RenderPort internalRenderPort2_;
    PortGroup internalPortGroup_;

    ShaderProperty shaderProp_;        ///< The shader property used by this raycaster.
    TransFuncProperty transferFunc_;  ///< the property that controls the transfer-function
    CameraProperty camera_;           ///< the camera used for lighting calculations

    StringOptionProperty compositingMode1_;   ///< What compositing mode should be applied for second outport
    StringOptionProperty compositingMode2_;   ///< What compositing mode should be applied for third outport

    FloatProperty gammaValue_;
    FloatProperty gammaValue1_;
    FloatProperty gammaValue2_;

    static const std::string loggerCat_; ///< category used in logging
};


} // namespace voreen

#endif // VRN_SINGLEVOLUMERAYCASTER_H
