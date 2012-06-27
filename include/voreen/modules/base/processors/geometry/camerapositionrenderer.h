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

#ifndef VRN_CAMERAPOSITIONRENDERER_H
#define VRN_CAMERAPOSITIONRENDERER_H

#include "voreen/core/processors/geometryrendererbase.h"

#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/cameraproperty.h"

namespace voreen {

class CameraPositionRenderer : public GeometryRendererBase {
public:
    CameraPositionRenderer();
    ~CameraPositionRenderer();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "CameraPositionRenderer"; }
    virtual std::string getCategory() const  { return "Geometry"; }
    virtual CodeState getCodeState() const   { return CODE_STATE_TESTING; }
    virtual std::string getProcessorInfo() const;

    virtual void render();
    virtual void renderPicking() {}
    virtual void setIDManager(IDManager* /*idm*/) {}

private:
    BoolProperty showCamera_;
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

