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

#ifndef VRN_OCCLUSIONSLICER_H
#define VRN_OCCLUSIONSLICER_H

#include "voreen/core/interaction/camerainteractionhandler.h"
#include "voreen/core/processors/volumeslicer.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/shaderproperty.h"

#include "voreen/core/ports/volumeport.h"

namespace voreen {

class OcclusionSlicer : public VolumeSlicer {
public:
    OcclusionSlicer();
    virtual Processor* create() const { return new OcclusionSlicer(); }

    virtual std::string getCategory() const { return "Slice Rendering"; }
    virtual std::string getClassName() const { return "OcclusionSlicer"; }
    virtual CodeState getCodeState() const { return CODE_STATE_TESTING; }

    virtual bool isReady() const;

protected:
    virtual void setDescriptions() {
        setDescription("This processor performs directional occlusion shading as introduced in \"Directional Occlusion Shading Model for Interactive Direct Volume Rendering\" by Schott et al.\
                It includes a modification to change the lighting direction inspired by the publication \"A Multidirectional Occlusion Shading Model for Direct Volume Rendering\" by Solteszova et al.");
    }

    virtual void process();

    virtual void initialize() throw (tgt::Exception);

    virtual std::string generateHeader();
    virtual void compile();

private:

    RenderPort occlusionbuffer0_;
    RenderPort occlusionbuffer1_;
    RenderPort outport_;

    PortGroup portGroup0_;
    PortGroup portGroup1_;

    // interaction handlers
    CameraInteractionHandler cameraHandler_;

    ShaderProperty shaderProp_;
    FloatProperty sigma_;
    IntProperty radius_;
};


} // namespace voreen

#endif // VRN_SINGLEVOLUMESLICER_H
