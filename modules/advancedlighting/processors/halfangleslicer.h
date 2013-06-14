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

#ifndef VRN_HALFANGLESLICER_H
#define VRN_HALFANGLESLICER_H

#include "voreen/core/processors/volumeslicer.h"
#include "voreen/core/interaction/camerainteractionhandler.h"
#include "voreen/core/properties/lightsourceproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/shaderproperty.h"

#include "voreen/core/ports/volumeport.h"

namespace voreen {

class HalfAngleSlicer : public VolumeSlicer {
public:
    HalfAngleSlicer();
    virtual Processor* create() const;

    virtual std::string getCategory() const  { return "Slice Rendering"; }
    virtual std::string getClassName() const { return "HalfAngleSlicer"; }
    virtual CodeState getCodeState() const   { return CODE_STATE_TESTING; }

    virtual bool isReady() const;

protected:
    virtual void setDescriptions() {
        setDescription("Performs half-angle slicing producing shadows as described in the publication \"Interactive Translucentvolume Rendering and Procedural Modeling\" by Kniss et al.");
    }

    virtual void beforeProcess();
    virtual void process();
    virtual void initialize() throw (tgt::Exception);

    virtual std::string generateHeader();
    virtual void compile();

private:

    RenderPort outport_;
    RenderPort lightport_;

    tgt::Camera lightCamera_;
    CameraProperty eyeCamera_;

    ShaderProperty shaderProp_;

    // interaction handlers
    CameraInteractionHandler cameraHandler_;

    LightSourceProperty halfLight_;
    bool invert_;
};


} // namespace voreen

#endif // VRN_HALFANGLESLICER_H
