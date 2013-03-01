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

#ifndef VRN_CAMERAPOSITIONRENDERER_H
#define VRN_CAMERAPOSITIONRENDERER_H

#include "voreen/core/processors/geometryrendererbase.h"

#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/cameraproperty.h"

namespace voreen {

class VRN_CORE_API CameraPositionRenderer : public GeometryRendererBase {
public:
    CameraPositionRenderer();
    ~CameraPositionRenderer();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "CameraPositionRenderer"; }
    virtual std::string getCategory() const  { return "Geometry";               }
    virtual CodeState getCodeState() const   { return CODE_STATE_STABLE;        }

    virtual void render();
    virtual void renderPicking() {}
    virtual void setIDManager(IDManager* /*idm*/) {}

private:
    virtual void setDescriptions() {
        setDescription("Depicts the position of a camera in a 3D scene. The camera position to display is retrieved from the processor's camera property.");
    }

    BoolProperty enable_;
    CameraProperty displayCamera_;

    //material yellow plastic
    GLfloat ye_ambient[4];
    GLfloat ye_diffuse[4];
    GLfloat ye_specular[4];
    GLfloat ye_shininess;

    //light parameters
    GLfloat light_pos[4];
    GLfloat light_ambient[4];
    GLfloat light_diffuse[4];
    GLfloat light_specular[4];
};

}

#endif
