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

#ifndef VRN_MULTIVOLUMERAYCASTER_H
#define VRN_MULTIVOLUMERAYCASTER_H

#include "voreen/core/processors/volumeraycaster.h"

#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/cameraproperty.h"

#include "voreen/core/ports/volumeport.h"

namespace voreen {

/**
 * Performs a single-pass multi-volume raycasting of up to four volumes,
 * providing several shading and compositing modes.
 *
 * @see MultiVolumeProxyGeometry, MeshEntryExitPoints
 */
class MultiVolumeRaycaster : public VolumeRaycaster {
public:
    MultiVolumeRaycaster();
    virtual ~MultiVolumeRaycaster();
    virtual Processor* create() const;

    virtual std::string getClassName() const    { return "MultiVolumeRaycaster"; }
    virtual std::string getCategory() const     { return "Raycasting"; }
    virtual CodeState getCodeState() const      { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;

    /**
     * All inports and at least one outport need to be connected.
     */
    virtual bool isReady() const;

protected:
    virtual void process();

    virtual void initialize() throw (VoreenException);

    virtual void deinitialize() throw (VoreenException);

    virtual void loadShader();

    virtual std::string generateHeader(VolumeHandle* volumeHandle = 0);
    virtual void compile(VolumeHandle* volumeHandle);

private:

    VolumePort volumeInport1_;
    VolumePort volumeInport2_;
    VolumePort volumeInport3_;
    VolumePort volumeInport4_;
    RenderPort entryPort_;
    RenderPort exitPort_;

    RenderPort outport_;
    RenderPort outport1_;
    RenderPort outport2_;
    PortGroup portGroup_;

    StringOptionProperty shadeMode1_;  ///< the property that controls the shading mode
    StringOptionProperty shadeMode2_;  ///< the property that controls the shading mode
    StringOptionProperty shadeMode3_;  ///< the property that controls the shading mode
    StringOptionProperty shadeMode4_;  ///< the property that controls the shading mode

    TransFuncProperty transferFunc1_;  ///< the property that controls the transfer-function
    TransFuncProperty transferFunc2_;  ///< the property that controls the transfer-function
    TransFuncProperty transferFunc3_;  ///< the property that controls the transfer-function
    TransFuncProperty transferFunc4_;  ///< the property that controls the transfer-function
    CameraProperty camera_;           ///< the camera used for lighting calculations

    StringOptionProperty compositingMode1_;   ///< What compositing mode should be applied for second outport
    StringOptionProperty compositingMode2_;   ///< What compositing mode should be applied for third outport
};


} // namespace voreen

#endif // VRN_MULTIVOLUMERAYCASTER_H
