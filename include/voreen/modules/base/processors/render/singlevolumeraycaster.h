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

#ifndef VRN_SINGLEVOLUMERAYCASTER_H
#define VRN_SINGLEVOLUMERAYCASTER_H

#include "voreen/core/processors/volumeraycaster.h"

#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/floatproperty.h"

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
class SingleVolumeRaycaster : public VolumeRaycaster {
public:
    SingleVolumeRaycaster();
    virtual Processor* create() const;

    virtual std::string getClassName() const    { return "SingleVolumeRaycaster"; }
    virtual std::string getCategory() const     { return "Raycasting"; }
    virtual CodeState getCodeState() const      { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;

    /// All inports and at least one outport need to be connected
    virtual bool isReady() const;

protected:
    /**
     * Recompiles the shader, if the invalidation level >= Processor::INVALID_PROGRAM.
     */
    virtual void beforeProcess();

    virtual void process();

    /**
     * Loads the shader and initializes the port group.
     */
    virtual void initialize() throw (VoreenException);

    /**
     * Deinitializes the port group and disposes the shader.
     */
    virtual void deinitialize() throw (VoreenException);

    /**
     * Adds compositing macros for the additional outports.
     *
     * @see VolumeRaycaster::generateHeader
     */
    virtual std::string generateHeader(VolumeHandle* volumeHandle = 0);

    /// Rebuilds the loaded shader.
    virtual void compile(VolumeHandle* volumeHandle);

private:
    void adjustPropertyVisibilities();

    VolumePort volumeInport_;
    RenderPort entryPort_;
    RenderPort exitPort_;

    RenderPort outport_;
    RenderPort outport1_;
    RenderPort outport2_;

    PortGroup portGroup_;

    tgt::Shader* raycastPrg_;         ///< The shader program used by this raycaster.

    TransFuncProperty transferFunc_;  ///< the property that controls the transfer-function
    CameraProperty camera_;           ///< the camera used for lighting calculations

    StringOptionProperty compositingMode1_;   ///< What compositing mode should be applied for second outport
    StringOptionProperty compositingMode2_;   ///< What compositing mode should be applied for third outport

    IntOptionProperty texFilterMode_;         ///< texture filtering mode to use for volume access
    GLEnumOptionProperty texClampMode_;       ///< texture clamp mode to use for the volume
    FloatProperty texBorderIntensity_;        ///< clamp border intensity

    static const std::string loggerCat_; ///< category used in logging
};


} // namespace voreen

#endif // VRN_SINGLEVOLUMERAYCASTER_H
