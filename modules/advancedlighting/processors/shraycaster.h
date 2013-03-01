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

#ifndef VRN_SHRAYCASTER_H
#define VRN_SHRAYCASTER_H

#include "voreen/core/processors/volumeraycaster.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/shaderproperty.h"
#include "voreen/core/ports/volumeport.h"
#include "../utils/shclass.h"

namespace voreen {

class SHRaycaster : public VolumeRaycaster {
public:
    SHRaycaster();
    virtual ~SHRaycaster();
    virtual Processor* create() const {return new SHRaycaster();}

    virtual std::string getCategory() const { return "Raycasting"; }
    virtual std::string getClassName() const {return "SHRaycaster";}
    virtual CodeState getCodeState() const { return CODE_STATE_TESTING; }

    virtual bool isReady() const;

protected:
    virtual void setDescriptions() {
        setDescription("Performs a raycasting using Spherical Harmonic Coefficients, providing global illumination effects like shadows and color bleeding. The light source can be rotated when holding CTRL.\
                Some other effects can be simulated (such as color refraction and subsurface scattering); however, please note that the implementation of these features is\
                experimental at the moment.");
    }

    virtual void process();
    virtual void initialize() throw (tgt::Exception);
    virtual void beforeProcess();

    SHClass* getSHClass() const { return sh_; }

    virtual std::string generateHeader();
    virtual void compile();

private:

    ShaderProperty shaderProp_;
    TransFuncProperty transferFunc_;  ///< the property that controls the transfer-function
    CameraProperty camera_;

    // member needed for spherical harmonics
    SHClass* sh_;

    VolumePort volumeInport_;
    RenderPort entryPort_;
    RenderPort exitPort_;

    RenderPort outport_;
    RenderPort internalRenderPort_;
};


} // namespace

#endif // VRN_SHRAYCASTER_H
