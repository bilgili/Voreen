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

#ifndef VRN_TRANSFUNC_H
#define VRN_TRANSFUNC_H

#include "voreen/core/vis/message.h"

class string;
class TiXmlElement;


namespace voreen {

class TextureManager;
class VolumeRenderer;
class TransFuncIntensity;
class TransFunc;

typedef TemplateMessage<TransFunc*> TransFuncPtrMsg;

/**
 * Base class for transfer functions.
 */
class TransFunc {
public:
    /**
    * Standard constructor
    */
    TransFunc();

    /**
    * Standard destructor
    */
    virtual ~TransFunc();

    /**
    * Creates a transfer function out of the data contained in the file.
    * \param filename The path to the file in which the data is stored
    * \return Was the load successful?
    */
    virtual bool load(const std::string filename);

    /**
    * Returns the shader defines needed to work with this type of TF (see mod_transfunc.frag)
    */
    virtual std::string getShaderDefines() = 0;

    /**
    * Bind the TF texture
    */
    void bind();

    /**
    * Returns the active texture
    * \return the active texture
    */
    tgt::Texture* getTexture();
    
    /**
    * Returns the active constant texture
    * \return the active constant texture
    */
    const tgt::Texture* getTexture() const;

    /**
    * Creates a new TinyXML Element consisting of the elements of v
    * \param root The root element to which the new element is added
    * \param v The vector containing the data which is to be saved
    */
    static void saveXml(TiXmlElement* root, const tgt::vec2& v);


    /**
    * Creates a new TinyXML Element consisting of the elements of c
    * \param root The root element to which the new element is added
    * \param c The vector containing the data which is to be saved
    */
    static void saveXml(TiXmlElement* root, const tgt::col4& c);


    /**
    * Loads two entries out of the TinyXML Element root and saves them into v
    * \param root The TinyXML Element containing two elements "x" and "y"
    * \param v The adress of the vector
    */
    static void loadXml(TiXmlElement* root, tgt::vec2& v);


    /**
    * Loads two entries out of the TinyXML Element root and saves them into c
    * \param root The TinyXML Element containing four elements "r","g","b" and "a"
    * \param c The adress of the vector
    */
    static void loadXml(TiXmlElement* root, tgt::col4& c);

protected:
    tgt::Texture* tex_;
    static const std::string loggerCat_;
};

/**
* Two dimensional transfer function.
* \sa TransFunc
*/
class TransFunc2D : public TransFunc {

public:
    /**
    * Standard constructor
    */
    TransFunc2D();

    /**
    * Standard destructor
    */
	virtual ~TransFunc2D();

    /**
    * Loads a transfer function from nearly any image file, with alpha channel (using DevIL).
    * \param filename The location of the file which is to be loaded
    * \return was the load successful?
    */
    bool load(const std::string& filename);

    //TODO: elaborate doxygen comment necessary
    static TransFunc2D* createPreIntegration(TransFunc *source);
    
    /**
    * Returns the color of a pixel for using in a Qt Pixmap.
    * \param x The x-coordinate of the pixel
    * \param y The y-coordinate of the pixel
    * \return The color value of the pixel
    */
    uint getPixelInQtFormat(uint x, uint y);

    /**
    * Sets color of a pixel. Retrives color in Qt format.
    * Texture must be updated manually by invalidateGLTex() and checkTexture() (which should of
    * course NOT be called for every pixel)
    * \param x The x-coordinate of the pixel
    * \param y The y-coordinate of the pixel
    */
    virtual void setPixelFromQtFormat(uint x, uint y, uint color);

    /**
    * Returns the shader defines needed to work with this type of TF
    * \return The defines
    */
    virtual std::string getShaderDefines();
};

} // namespace

#endif //VRN_TRANSFUNC_H
