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

#ifndef VRN_ENTRYEXITPOINTS_H
#define VRN_ENTRYEXITPOINTS_H

#include "voreen/modules/base/processors/proxygeometry/proxygeometry.h"
#include "voreen/core/interaction/camerainteractionhandler.h"

#include "tgt/shadermanager.h"

namespace voreen {

/**
 * Calculates the entry and exit points for GPU raycasting.
 */
class EntryExitPoints : public VolumeRenderer {
public:
    EntryExitPoints();
    virtual ~EntryExitPoints();

    virtual std::string getCategory() const { return "EntryExitPoints"; }
    virtual std::string getClassName() const { return "EntryExitPoints"; }
    virtual Processor* create() const { return new EntryExitPoints(); }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;

    virtual void initialize() throw (VoreenException);

    virtual bool isReady() const;
    virtual void process();

protected:
    /// Complements the parts of the entry points texture clipped by the near plane.
    void complementClippedEntryPoints();

    /**
     *  Jitters entry points in ray direction.
     *  Entry and Exit Params have to be generated before
     *  calling this method.
     */
    void jitterEntryPoints();

    /// (Re-)generates jitter texture
    void generateJitterTexture();

    void onJitterEntryPointsChanged();
    void onFilterJitterTextureChanged();

    tgt::Shader* shaderProgram_;
    tgt::Shader* shaderProgramJitter_;
    tgt::Shader* shaderProgramClipping_;

    // processor properties
    BoolProperty supportCameraInsideVolume_;
    BoolProperty jitterEntryPoints_;
    BoolProperty filterJitterTexture_;
    FloatProperty jitterStepLength_;
    tgt::Texture* jitterTexture_;
    CameraProperty camera_;  ///< camera used for rendering the proxy geometry

    // interaction handlers
    CameraInteractionHandler* cameraHandler_;

    // ports
    RenderPort entryPort_;
    RenderPort exitPort_;
    VolumePort inport_;
    GenericCoProcessorPort<ProxyGeometry> cpPort_;
    RenderPort tmpPort_;

    static const std::string loggerCat_;
};

} // namespace voreen

#endif //VRN_ENTRYEXITPOINTS_H
