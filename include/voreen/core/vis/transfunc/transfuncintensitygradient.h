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

#ifndef VRN_TRANSFUNCINTENSITYGRADIENT_H
#define VRN_TRANSFUNCINTENSITYGRADIENT_H

#include "voreen/core/vis/transfunc/transfunc.h"

#include "tgt/tgt_gl.h"

class FramebufferObject;
class TiXmlDocument;

namespace voreen {

/** 
 *  This class represents a two-dimensional intensity-gradient based transfer function.
 *
 *  - x-axis: intensity
 *  - y-axis: gradient magnitude
 *  
 *  The transfer function is defined by passing its lookup table directly. If you
 *  are looking for a more convenient way of editing, you should have a look at 
 *  TransFuncIntensityGradientPrimitiveContainer.
 *
 *  Internally, the transfer function is represented by a two-dimensional RGBA texture
 *  of type GL_FLOAT.
 * 
 */
class TransFuncIntensityGradient : public TransFunc {

public:

    /**
     * Constructor.
     *
     * Width and height specifie the lookup table's / texture's dimensions and have to be greater zero.
     *
     */
    TransFuncIntensityGradient(int width = 256, int height = 256);
    virtual ~TransFuncIntensityGradient();
    virtual std::string getShaderDefines();

    /** 
     *  Use this function to define the transfer function's look up table.
     *  
     *  @param data the lookup table in RGBA format. Its dimension must match
     *      the transfer function's dimensions, i.e. width*height*4. 
     *      Value range: [0.0:1.0]
     *
     *  @note data is passed to the transfer function's texture and must therefore not
     *      be externally deleted after calling this function!
     */
    virtual void setPixelData(GLfloat* data);

    /// Returns the transfer function's lookup table in RGBA format. Value range: [0.0:1.0]
    const GLfloat* getPixelData() const;
    
    /// Returns the transfer function's lookup table in RGBA format. Value range: [0.0:1.0]
    GLfloat* getPixelData();
    
    /// Returns the lookup table's width, i.e. the number of elements along the intensity axis.
    int getWidth() const;

    /// Returns the lookup table's height, i.e. the number of elements along the gradient magnitude axis.
    int getHeight() const;
    
    /// Clears the transfer function, i.e. all lookup-table values are set to zero.
    void clear();

    /**
     * Saves the transfer function as PNG.
     *
     * @warning Make sure DevIL is inited before calling this function!
     */
    void savePNG(const std::string& fname) const;

    /// Call this, if the transfer function's pixel data has changed.
    virtual void updateTexture();

protected:
    static const std::string loggerCat_;
};

// ----------------------------------------------------------------------------

class TransFuncPrimitive;

/** 
 *   This class provides an interface for conveniently setting up a two-dimensional 
 *   intensity-gradient based transfer function.
 *   
 *   The transfer function is edited by adding / removing TransFuncPrimitive objects.
 *   Internally, the primitives are rendered to a texture, therefore FBO support is needed. 
 */
class TransFuncIntensityGradientPrimitiveContainer : public TransFuncIntensityGradient {
    FramebufferObject*  fbo_;       // The framebuffer object used for rendering to the texture
    std::vector<TransFuncPrimitive*> primitives_;
    static const std::string loggerCat_;
public:
    TransFuncIntensityGradientPrimitiveContainer(int size);
    virtual ~TransFuncIntensityGradientPrimitiveContainer();
    virtual std::string getShaderDefines();

    void addPrimitive(TransFuncPrimitive* p) { primitives_.push_back(p); }
    void removePrimitive(TransFuncPrimitive* p);
    TransFuncPrimitive* getPrimitive(int i) { return primitives_[i]; }
    ///Paint all the primitives.
    void paint();
    ///Paint all the primitives for selection
    void paintSelection();
    ///Paint all the primitives for editing
    void paintInEditor();

    ///Returns the primitive which has a control point closest to the mouse cursor (or null if none)
    TransFuncPrimitive* getControlPointUnderMouse(tgt::vec2 m);

    ///saves the transfer function to xml file fname
    void save(const std::string& fname);
    ///loads the transfer function from xml file fname
    bool load(const std::string& fname);

    ///Removes all primitives
    void clear();
    
    ///Check if init was completed without errors (FBO!)
    bool initOk() { return fbo_ != 0; }
    
    ///Update TF Texture
    ///Note: You have to set the current drawbuffer after calling this method
    void updateTexture();
};

// ----------------------------------------------------------------------------

/// Interface for primitives in 2 dimensional transfer functions
class TransFuncPrimitive {
protected:
    tgt::col4 color_;
    bool selected_;

    float fuzziness_;
    float cpSize_;
public:
    TransFuncPrimitive(tgt::col4 col) : color_(col), selected_(false), cpSize_(0.02f) {}
    TransFuncPrimitive() { color_ = tgt::col4(255, 255, 0, 255); selected_ = false; }
    virtual ~TransFuncPrimitive() {}

    ///Paint for use in transfer function
    virtual void paint()=0;
    ///Paint in editor (with outlines, control points...)
    virtual void paintInEditor()=0;
    ///Paint in given color glColor3ub(id,123,123) for selection
    virtual void paintSelection(GLubyte id)=0;
    ///Paint a control point at position v
    virtual void paintControlPoint(const tgt::vec2& v);

    void setColor(const tgt::col4& c) { color_ = c; }
    tgt::col4 getColor() { return color_; }

    ///Primitives with high fuzziness (0.0->1.0) have a alpha gradient towards their border.
    void setFuzziness(float f) { fuzziness_ = f;}
    float getFuzziness() { return fuzziness_; }

    ///Returns the distance from m to the closest control point of this primitive
    virtual float getClosestControlPointDist(tgt::vec2 m) = 0;
    ///Returns true if a control point is under the mouse cursor and grabs this control point.
    virtual bool mousePress(tgt::vec2 m) = 0;
    ///Finished grabbing of control point
    virtual void mouseRelease(tgt::vec2 m) = 0;
    ///If a control point is grabbed: move only this point else: move the whole primitive
    virtual void mouseDrag(tgt::vec2 offset) = 0;

    ///Set selection state. If a primitive is selected it is usually painted different in the editor.
    virtual void setSelected(bool s) { selected_ = s; }

    ///Save primitive in xml tree.
    virtual void save(TiXmlElement* root) = 0;
    ///Load primitive from xml tree.
    virtual void load(TiXmlElement* root) = 0;
};

// ----------------------------------------------------------------------------

///A Quad.
class TransFuncQuad : public TransFuncPrimitive {
protected:
    tgt::vec2 coords_[4];
    int grabbed_;

public:
    TransFuncQuad(tgt::vec2 center, float size, tgt::col4 col);
    TransFuncQuad() {}
    void paint();
    void paintSelection(GLubyte id);
    void paintInEditor();

    virtual float getClosestControlPointDist(tgt::vec2 m);
    virtual bool mousePress(tgt::vec2 m);
    virtual void mouseRelease(tgt::vec2 /*m*/) { grabbed_ = -1; }
    virtual void mouseDrag(tgt::vec2 offset);

    virtual void save(TiXmlElement* root);
    virtual void load(TiXmlElement* root);
};

// ----------------------------------------------------------------------------

///Two splines, the space inbetween the two is filled.
class TransFuncBanana : public TransFuncPrimitive {
protected:
    tgt::vec2 coords_[4];
    //       1
    //
    //       2
    //0            3
    int steps_;
    int grabbed_;

public:
    TransFuncBanana(tgt::vec2 a, tgt::vec2 b1, tgt::vec2 b2, tgt::vec2 c, tgt::col4 col);
    TransFuncBanana() {}
    void paint();
    void paintSelection(GLubyte id);
    void paintInEditor();
    virtual float getClosestControlPointDist(tgt::vec2 m);
    virtual bool mousePress(tgt::vec2 m);
    virtual void mouseRelease(tgt::vec2 /*m*/) { grabbed_ = -1; }
    virtual void mouseDrag(tgt::vec2 offset);

    virtual void save(TiXmlElement* root);
    virtual void load(TiXmlElement* root);
protected:
    void paintInner();
};

}
#endif //VRN_TRANSFUNCINTENSITYGRADIENT_H
