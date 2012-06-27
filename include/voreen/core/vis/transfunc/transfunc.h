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

#ifndef VRN_TRANSFUNC_H
#define VRN_TRANSFUNC_H

#include "tgt/texture.h"
#include "tgt/vector.h"

#include <vector>
#include <string>

class TiXmlElement;

namespace voreen {

/**
 * Abstract base class for transfer functions.
 */
class TransFunc {
public:
    /**
     * Constructor
     *
     * @param width width of transfer function
     * @param height of transfer function
     */
    TransFunc(int width = 256, int height = 1);

    /**
     * Standard destructor
     */
    virtual ~TransFunc();

    /**
     * Creates a transfer function out of the data contained in the file given by filename.
     *
     * @param filename The path to the file in which the data is stored
     * @return true when the load was successful, false otherwise
     */
    virtual bool load(const std::string& filename) = 0;

    /**
     * Returns a define for the usage of transfer functions in shaders.
     * For 1D transfer functions the define looks like: "#define TF_SAMPLER_TYPE sampler1D \n"
     * and with sampler2D for 2D transfer functions.
     *
     * @return define for usage of transfer functions in shaders
     */
    virtual const std::string getShaderDefines() const = 0;

    /**
     * Returns a string representation of the samplertype, e.g. "sampler1D" for 1D transfer
     * functions and "sampler2D" for 2D transfer functions.
     *
     * @return string representaion of the samplertype used by the transfer function
     */
    virtual const std::string getSamplerType() const = 0;

    /**
     * Sets the width and height of the transfer function to the given values.
     * The texture is not updated at this point.
     *
     * @param width width of the transfer function
     * @param height height of the transfer function
     */
    void setTextureDimension(int width, int height);

    /**
     * Marks the texture of the transfer function as invalid.
     * That means that the texture needs to be updated before next bind.
     */
    void textureUpdateNeeded();

    /**
     * Binds the tf texture.
     */
    void bind();

    /**
     * Returns the texture of the transfer function.
     *
     * @return the texture of the transfer function
     */
    tgt::Texture* getTexture();

    /**
     * Returns a vector that contains the endings of suppported file formats for loading.
     *
     * @return vector with endings of supported file formats
     */
    const std::vector<std::string>& getLoadFileFormats();

    /**
     * Returns a vector that contains the endings of suppported file formats for saving.
     *
     * @return vector with endings of supported file formats
     */
    const std::vector<std::string>& getSaveFileFormats();

    /**
     * Creates a new TinyXML Element consisting of the elements of v.
     *
     * @param root the root element to which the new element is added
     * @param v the vector containing the data which is to be saved
     */
    void saveXml(TiXmlElement* root, const tgt::vec2& v);

    /**
     * Creates a new TinyXML Element consisting of the elements of c.
     *
     * @param root the root element to which the new element is added
     * @param c the vector containing the data which is to be saved
     */
    void saveXml(TiXmlElement* root, const tgt::col4& c);

    /**
     * Loads two entries out of the TinyXML Element root and saves them into v.
     *
     * @param root the TinyXML element containing two elements "x" and "y"
     * @param v the adress of the vector
     */
    void loadXml(TiXmlElement* root, tgt::vec2& v);

    /**
     * Loads two entries out of the TinyXML Element root and saves them into c.
     *
     * @param root the TinyXML element containing four elements "r","g","b" and "a"
     * @param c the adress of the vector
     */
    void loadXml(TiXmlElement* root, tgt::col4& c);

protected:

    /**
     * Updates the texture of the transfer function. It is called before the texture is bound
     * to a textureunit. Must be reimplemented in every subclass.
     */
    virtual void updateTexture() = 0;

    tgt::Texture* tex_; ///< the texture of the transfer function

    std::vector<std::string> loadFileFormats_; ///< endings that are supported for loading a transfer function
    std::vector<std::string> saveFileFormats_; ///< endings that are supported for saving a transfer function

    bool textureUpdateNeeded_; ///< indicates whether the texture of the transfer function has to be updated or not
    tgt::ivec2 dimension_; ///< dimensions of the transfer function texture

private:
    /**
     * Adapts the given width and height of transfer function to graphicscard capabilities.
     * The result is stored in the given parameters.
     *
     * @param width desired width for the transfer function
     * @param height desired height for the transfer function
     */
    void fitDimension(int& width, int& height);

    static const std::string loggerCat_; ///< logger category
};

} // namespace voreen

#endif // VRN_TRANSFUNC_H
