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

#ifndef VRN_RAYTRACINGENTRYEXITPOINTS_H
#define VRN_RAYTRACINGENTRYEXITPOINTS_H

#include "voreen/modules/base/processors/proxygeometry/proxygeometry.h"
#include "voreen/core/processors/volumerenderer.h"
#include "voreen/core/interaction/camerainteractionhandler.h"

#include "voreen/core/datastructures/geometry/meshlistgeometry.h"

#include "voreen/core/ports/allports.h"
#include "voreen/core/properties/allproperties.h"

#include "tgt/shadermanager.h"
#include "voreen/modules/opencl/clwrapper.h"

namespace voreen {

/**
 * Calculates the entry and exit points for GPU raycasting.
 */
class RaytracingEntryExitPoints : public RenderProcessor {
public:
    RaytracingEntryExitPoints();
    virtual ~RaytracingEntryExitPoints();

    virtual std::string getClassName() const    { return "RaytracingEntryExitPoints"; }
    virtual std::string getCategory() const     { return "EntryExitPoints"; }
    virtual CodeState getCodeState() const      { return CODE_STATE_EXPERIMENTAL; }

    virtual std::string getProcessorInfo() const;

    virtual Processor* create() const;
    virtual bool isReady() const;

    virtual void beforeProcess();
protected:
    virtual void process();

    virtual void initialize() throw (VoreenException);

    // processor properties
    BoolProperty useFloatRenderTargets_;
    CameraProperty camera_;  ///< camera used for rendering the proxy geometry

    // interaction handlers
    CameraInteractionHandler* cameraHandler_;

    cl::OpenCL* opencl_;
    cl::Context* context_;
    cl::CommandQueue* queue_;
    cl::Program* prog_;

    // ports
    RenderPort entryPort_;
    RenderPort exitPort_;
    GeometryPort inport_;

    /// Category used for logging.
    static const std::string loggerCat_;
};

} // namespace voreen

#endif //VRN_RAYTRACINGENTRYEXITPOINTS_H
