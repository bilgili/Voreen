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

#ifndef VRN_ENTRYEXITPOINTS_H
#define VRN_ENTRYEXITPOINTS_H

#include "voreen/core/vis/processors/proxygeometry/proxygeometry.h"
#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/opengl/texunitmapper.h"

#include "tgt/shadermanager.h"

namespace voreen {

/**
 * Calculates the entry and exit points for GPU raycasting. The parameters are stored in
 * (float) textures. The textures are provided by the class TextureContainer.
 */
class EntryExitPoints : public VolumeRenderer {
public:

    /**
     *   Constructor
     */
    EntryExitPoints();
    virtual ~EntryExitPoints();

    virtual const Identifier getClassName() const { return "EntryExitPoints.EntryExitPoints"; }

    virtual Processor* create() const { return new EntryExitPoints(); }

    virtual int initializeGL();

    virtual void process(LocalPortMapping* portMapping);

    /**
    * Process voreen message, accepted identifiers:
    * -  VoreenPainter::cameraChanged_
    */
    virtual void processMessage(Message* msg, const Identifier& dest = Message::all_);

protected:

    /// Complements the parts of the entry points texture clipped by the near plane.
    void complementClippedEntryPoints(LocalPortMapping* portMapping);

    /**
     *  Jitters entry points in ray direction.
     *  Entry and Exit Params have to be generated before
     *  calling this method.
     */
    void jitterEntryPoints(LocalPortMapping* portMapping);

    /// (Re-)generates jitter texture
    void generateJitterTexture();

    void onFilterJitterTextureChange();

    tgt::Shader* shaderProgram_;
    tgt::Shader* shaderProgramJitter_;
    tgt::Shader* shaderProgramClipping_;
    static const Identifier entryPointsTexUnit_;
    static const Identifier entryPointsDepthTexUnit_;
    static const Identifier exitPointsTexUnit_;
    static const Identifier jitterTexUnit_;

    // processor properties
    BoolProp supportCameraInsideVolume_;
    BoolProp jitterEntryPoints_;
    BoolProp filterJitterTexture_;
    FloatProp jitterStepLength_;
    tgt::Texture* jitterTexture_;

    static const std::string loggerCat_;
};

} // namespace voreen

#endif //VRN_ENTRYEXITPOINTS_H
