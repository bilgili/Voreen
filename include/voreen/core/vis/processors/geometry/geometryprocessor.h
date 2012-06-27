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

#ifndef VRN_GEOMETRYPROCESSOR_H
#define VRN_GEOMETRYPROCESSOR_H

#include "voreen/core/vis/processors/renderprocessor.h"
#include "voreen/core/vis/idmanager.h"
#include "voreen/core/vis/properties/boolproperty.h"
#include "voreen/core/vis/properties/eventproperty.h"
#include "voreen/core/vis/processors/proxygeometry/proxygeometry.h"

namespace voreen {

class GeometryRenderer;
class CameraInteractionHandler;

/**
 * Renders GeometryRenderer objects.
 *
 * This processor renders all to it connected coprocessors. The projection and modelview matrices
 * are set according to the current viewing position.
 */
class GeometryProcessor : public RenderProcessor {
public:
    GeometryProcessor();
    ~GeometryProcessor();

    virtual void initialize() throw (VoreenException);
    virtual bool isReady() const;

    virtual std::string getCategory() const { return "Geometry"; }
    virtual std::string getClassName() const { return "GeometryProcessor"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; } ///2.0
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const;

    virtual void process();

private:
    tgt::Shader* shaderPrg_;
    IDManager idm_;

    CameraProperty camera_;
    CameraInteractionHandler* cameraHandler_;

    RenderPort inport_;
    RenderPort outport_;
    RenderPort tempPort_;
    RenderPort pickingPort_;
    GenericCoProcessorPort<GeometryRenderer> cpPort_;
};

//---------------------------------------------------------------------------

class ProxyGeometry;

/**
 * Abstract base class for rendering Geometry onto images.
 */
class GeometryRenderer : public RenderProcessor {
public:
    GeometryRenderer();

    virtual void process();
    virtual void render() = 0;
    virtual void renderPicking() {}
    virtual void setIDManager(IDManager* idm) { idm_ = idm; }
    virtual void setCamera(tgt::Camera* c) { camera_ = c; }

protected:
    ///Get 3D vector from screen position (unproject)
    tgt::vec3 getOGLPos(int x, int y, float z) const;
	///Get screen position from 3D vector (project)
	tgt::vec3 getWindowPos(tgt::vec3 pos) const;

    tgt::Camera* camera_;
    GenericCoProcessorPort<GeometryRenderer> outPort_;
    IDManager* idm_;
};

//---------------------------------------------------------------------------

/**
 * Light widget
 */
class GeomLightWidget : public GeometryRenderer {
public:
    GeomLightWidget();
    ~GeomLightWidget();

    virtual std::string getCategory() const { return "Geometry"; }
    virtual std::string getClassName() const { return "GeomLightWidget"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; } ///2.0
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const;

    /**
     * Renders the light widget
     */
    virtual void render();
    virtual void renderPicking();
    virtual void setIDManager(IDManager* idm);

private:
    void moveSphere(tgt::MouseEvent* e);

    BoolProperty showLightWidget_;
    TemplateMouseEventProperty<GeomLightWidget>* moveSphereProp_;

    bool isClicked_;

    FloatVec4Property lightPosition_;
    tgt::vec4 lightPositionAbs_;
    tgt::ivec2 startCoord_;

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

//---------------------------------------------------------------------------

///Draws bounding box around the data set
class GeomBoundingBox : public GeometryRenderer {
public:
    GeomBoundingBox();

    void setLineWidth(float width);
    ///Set the stipplePattern to be used. @see OpenGL docs
    void setStipplePattern(int stippleFactor, int stipplePattern);
    virtual std::string getCategory() const { return "Geometry"; }
    virtual std::string getClassName() const { return "BoundingBox"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; } ///2.0
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const {return new GeomBoundingBox();}

    virtual void render();
private:
    ColorProperty bboxColor_;
    FloatProperty width_;
    IntProperty stippleFactor_;
    IntProperty stipplePattern_;
    BoolProperty showGrid_;
    IntVec3Property tilesProp_;
    BoolProperty applyDatasetTransformationMatrix_;

    VolumePort inport_;
};

//---------------------------------------------------------------------------

class SlicePositionRenderer : public GeometryRenderer {
public:
    SlicePositionRenderer();

    virtual std::string getCategory() const { return "Geometry"; }
    virtual std::string getClassName() const { return "SlicePositionRenderer"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; } ///2.0
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const {return new SlicePositionRenderer();}

    virtual void process();
    virtual void render();
private:
    ColorProperty xColor_;
    ColorProperty yColor_;
    ColorProperty zColor_;
    IntProperty xSliceIndexProp_;
    IntProperty ySliceIndexProp_;
    IntProperty zSliceIndexProp_;

    FloatProperty width_;
    IntProperty stippleFactor_;
    IntProperty stipplePattern_;
    VolumePort inport_;
};

//---------------------------------------------------------------------------

class PickingBoundingBox : public GeometryRenderer {
public:
    PickingBoundingBox();
    virtual std::string getCategory() const { return "Geometry"; }
    virtual std::string getClassName() const { return "PickingBoundingBox"; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const {return new PickingBoundingBox();}

    virtual void render();

private:
    tgt::vec3 lowerLeftFront_;
    tgt::vec3 upperRightBack_;
    bool displaySelection_;
    GenericCoProcessorPort<ProxyGeometry> proxyGeomPort_;
};

//---------------------------------------------------------------------------

class GeomRegistrationMarkers : public GeometryRenderer {
public:
    GeomRegistrationMarkers();

    virtual std::string getCategory() const { return "Geometry"; }
    virtual std::string getClassName() const { return "Registration Marker-Display"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const {return new GeomRegistrationMarkers();}
    void setDescription(std::string description);

    void render();

protected:
    bool marker1Selected_;
    bool marker2Selected_;
    bool marker3Selected_;
    tgt::vec3 marker1_;
    tgt::vec3 marker2_;
    tgt::vec3 marker3_;
    float marker1Radius_;
    float marker2Radius_;
    float marker3Radius_;
    std::string description_;
    bool disableReceiving_;
    GenericCoProcessorPort<ProxyGeometry> proxyGeomPort_;
};

} // namespace voreen

#endif // VRN_GEOMETRYPROCESSOR_H
