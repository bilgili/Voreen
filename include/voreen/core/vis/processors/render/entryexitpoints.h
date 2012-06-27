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

#ifndef VRN_ENTRYEXITPOINTS_H
#define VRN_ENTRYEXITPOINTS_H

#include "voreen/core/vis/processors/render/proxygeometry.h"
#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/opengl/texunitmapper.h"

#include "tgt/shadermanager.h"

namespace voreen {

/**
 * Calculates the entry and exit parameter for gpu raycasting.
 * The parameters are stored in (float) textures. The textures are provided
 * by the class TextureContainer.
 * The ctor initializes a shader program in shaderProgram_. It handles the
 * vertex information in the vertex stage and pass it to the fragment stage.
 * The fragment program put it into the OpenGL color variable . The program
 * can (and should) be used by derived classes.
 */
class EntryExitPoints : public VolumeRenderer {
public:
    /**
    *   Constructor
    */
    EntryExitPoints();
    virtual ~EntryExitPoints();
    /**
    * Initialize the shader program.
    */
    virtual int initializeGL();
    /**
    * Process voreen message, accepted identifiers:
    * - set.jitterEntryParams bool
    * - set.jitterStepLength float
    */
    virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);
    ///Set the size of the generated textures.
    virtual void setSize(const tgt::ivec2& size);
    ///Change the proxy geometry
    virtual void setProxyGeometry(ProxyGeometry* pg);
    ProxyGeometry* getProxyGeometry();

    ///Get all renderers using this EntryExitPoints
    virtual std::vector<Processor*> getAttachedProcessor();

    ///Set a Transformationmatrix that will be multiplied with current modelView-Matrix
    ///before rendering of Proxygeometry
    void setTransformationMatrix(tgt::mat4 trans);

    ///Set scaling of entryExitParams in x, y, and z-direction
    void setScaling(tgt::vec3 scale);

    ///Set translation of entryExitParams in x,y and z-direction
    void setTranslation(tgt::vec3 trans);

    ///Set angle of rotation around x-axis
    void setRotationX(float angle);

    ///Set angle of rotation around y-axis
    void setRotationY(float angle);

    ///Set angle of rotation around z-axis
    void setRotationZ(float angle);

    void setPropertyDestination(Identifier tag);

    /** Overwriten method inherited from VolumeRenderer:
     * sets the currently active volume and adds the necessary 
     * adaption of transformation.
     */
    virtual void setVolumeHandle(VolumeHandle* const handle);

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

    ProxyGeometry* pg_;
    tgt::Shader* shaderProgram_;
    tgt::Shader* shaderProgramJitter_;
    tgt::Shader* shaderProgramClipping_;
    static const Identifier entryParamsTexUnit_;
    static const Identifier entryParamsDepthTexUnit_;
    static const Identifier exitParamsTexUnit_;
    static const Identifier jitterTexUnit_;

    //properties for gui generation:
    BoolProp supportCameraInsideVolume_;
    GroupProp scalingGroup_;
    FloatProp scaleX_;
    FloatProp scaleY_;
    FloatProp scaleZ_;
    GroupProp translationGroup_;
    FloatProp translationX_;
    FloatProp translationY_;
    FloatProp translationZ_;
    BoolProp jitterEntryParams_;
    BoolProp filterJitterTexture_;
    FloatProp jitterStepLength_;
    tgt::Texture* jitterTexture_;

    tgt::mat4 transformationMatrix_;
    bool switchFrontAndBackFaces_;

    ConditionProp* condJitter_;
    
    static const std::string loggerCat_;
};

//---------------------------------------------------------------------------

/**
 * Calculates entry/exit params for the SimpleRayCasting
 */
class CubeEntryExitPoints : public EntryExitPoints {
public:
	CubeEntryExitPoints();
    virtual ~CubeEntryExitPoints();

	virtual const Identifier getClassName() const {return "EntryExitPoints.CubeEntryExitPoints";}
	virtual const std::string getProcessorInfo() const;
    virtual Processor* create() {return new CubeEntryExitPoints();}

    virtual std::string generateHeader();

    /**
    * Process voreen message, accepted identifiers:
    * - switch.virtualClipplane
    */
    virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);
	virtual void process(LocalPortMapping* portMapping);
    
    virtual bool isMultipassCompatible() { return true; }

    void setNeedToReRender(bool needed);
protected:
    bool needToReRender_;
};

//---------------------------------------------------------------------------

//FIXME: replaced by NullRenderer
class OutputProcessor : public Processor {
public:
	OutputProcessor();
    ~OutputProcessor();
    virtual void process(LocalPortMapping* portMapping);

	virtual const Identifier getClassName() const {return "Miscellaneous.OutputProcessor";}
    virtual Processor* create() { return new OutputProcessor(); }

    int getOutputTarget() const { return outputTarget_; }
    
protected:
    int outputTarget_;	
};


} // namespace voreen

#endif //VRN_ENTRYEXITPOINTS_H
