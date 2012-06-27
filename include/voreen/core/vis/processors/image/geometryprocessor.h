/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/processors/processor.h"

namespace voreen {

class GeometryRenderer;

/**
 * Renders GeometryRenderer objects.
 *
 * This processor renders all to it connected coprocessors. The projection and modelview matrices 
 * are set according to the current viewing position.
 */
class GeometryProcessor : public Processor {
public:
    GeometryProcessor();
    ~GeometryProcessor();

    virtual int initializeGL();

	virtual const Identifier getClassName() const;
	virtual const std::string getProcessorInfo() const;
    virtual Processor* create();

	virtual void process(LocalPortMapping* portMapping);

private:
    tgt::Shader* shaderPrg_;
};

//---------------------------------------------------------------------------

/**
 * Class for rendering Geometry onto raycasted images.
 */
class GeometryRenderer : public Processor {
public:
    GeometryRenderer();

    virtual void process(LocalPortMapping* portMapping);
    virtual Message* call(Identifier ident, LocalPortMapping* portMapping=0);

protected:
    virtual void render(LocalPortMapping* localPortMapping) = 0;
    ///Get 3D vector from screen position (unproject)
    tgt::vec3 getOGLPos(int x, int y,float z);
};

typedef TemplateMessage<GeometryRenderer*> GeomRendererMsg;

//---------------------------------------------------------------------------

/**
 * Light widget
 */
class GeomLightWidget : public GeometryRenderer, public tgt::EventListener {
public:
	GeomLightWidget();
	~GeomLightWidget();

	virtual const Identifier getClassName() const;
	virtual const std::string getProcessorInfo() const;
    virtual Processor* create();

    virtual void mousePressEvent(tgt::MouseEvent *e);
    virtual void mouseMoveEvent(tgt::MouseEvent *e);
    virtual void mouseReleaseEvent(tgt::MouseEvent *e);

protected:
    /**
     * Renders the light widget
     */
	virtual void render(LocalPortMapping* localPortMapping);

private:
	BoolProp showLightWidget_;

    bool isClicked_;

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
    ~GeomBoundingBox();

    /**
    *   Process voreen message, accepted identifiers:
    * - set.BoundingboxColor tgt::Color
    * - set.BoundingBoxWidth int
    * - set.BoundingBoxStippleFactor int
    * - set.BoundingBoxStipplePattern int
    * - set.BoundingBoxShowGrid bool
    * - set.BoundingBoxTilesX int
    * - set.BoundingBoxTilesY int
    * - set.BoundingBoxTilesZ int
    */
	virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);

	void setLineWidth(float width);
    ///Set the stipplePattern to be used. @see OpenGL docs
	void setStipplePattern(int stippleFactor, int stipplePattern);
	virtual const Identifier getClassName() const {return "GeometryRenderer.BoundingBox";}
	virtual const std::string getProcessorInfo() const;
    virtual Processor* create() {return new GeomBoundingBox();}


protected:
    virtual void render(LocalPortMapping*  portMapping);
private:
	ColorProp bboxColor_;
	FloatProp width_;
	IntProp stippleFactor_;
	IntProp stipplePattern_;
	BoolProp showGrid_;
	IntProp tilesX_;
	IntProp tilesY_;
	IntProp tilesZ_;

    ConditionProp* cond_;
};

//---------------------------------------------------------------------------

class PickingBoundingBox : public GeometryRenderer {
public:
	PickingBoundingBox();
	virtual const Identifier getClassName() const {return "GeometryRenderer.PickingBoundingBox";}
	virtual const std::string getProcessorInfo() const;
    virtual Processor* create() {return new PickingBoundingBox();}
    /**
    *   Process voreen message, accepted identifiers:
    * - set.lowerLeftFront
    * - set.upperRightBack
    * - set.Hide
    */
	virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);

protected:
	virtual void render(LocalPortMapping* portMapping);

private:
    tgt::vec3 lowerLeftFront_;
    tgt::vec3 upperRightBack_;
    bool displaySelection_;
};

//---------------------------------------------------------------------------

class GeomRegistrationMarkers : public GeometryRenderer {
public:
    GeomRegistrationMarkers();
   
	virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);
	virtual const Identifier getClassName() const {return "GeometryRenderer.Registration Marker-Display";}
	virtual const std::string getProcessorInfo() const;
    virtual Processor* create() {return new GeomRegistrationMarkers();}
    void setDescription(std::string description);

protected:
    void render(LocalPortMapping*  portMapping);

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
};

} // namespace voreen

#endif // VRN_GEOMETRYPROCESSOR_H
