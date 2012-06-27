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
    virtual ~SingleVolumeRaycaster();

    virtual std::string getClassName() const    { return "SingleVolumeRaycaster"; }
    virtual std::string getCategory() const     { return "Raycasting"; }
    virtual CodeState getCodeState() const      { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;

    virtual Processor* create() const;
    virtual bool isReady() const;

protected:
    /**
     * Performs the raycasting.
     *
     * Initialize two texture units with the entry and exit params and renders
     * a screen aligned quad. The render destination is determined by the
     * invoking class.
     */
    virtual void process();

    /**
     * Loads the shader and initializes the port group.
     */
    virtual void initialize() throw (VoreenException);

    /**
     * Deinitializes the port group.
     */
    virtual void deinitialize() throw (VoreenException);

    /**
     * Load the needed shader.
     */
    virtual void loadShader();

    virtual std::string generateHeader(VolumeHandle* volumeHandle = 0);
    virtual void compile(VolumeHandle* volumeHandle);

private:
    TransFuncProperty transferFunc_;  ///< the property that controls the transfer-function
    CameraProperty camera_;           ///< the camera used for lighting calculations

    StringOptionProperty compositingMode1_;   ///< What compositing mode should be applied for second outport
    StringOptionProperty compositingMode2_;   ///< What compositing mode should be applied for third outport

    VolumePort volumeInport_;
    RenderPort entryPort_;
    RenderPort exitPort_;

    RenderPort outport_;
    RenderPort outport1_;
    RenderPort outport2_;

    PortGroup portGroup_;
};


} // namespace voreen

#endif // VRN_SINGLEVOLUMERAYCASTER_H
