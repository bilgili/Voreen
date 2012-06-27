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

#ifndef VRN_TRANSFUNC_H
#define VRN_TRANSFUNC_H

#include "voreen/core/io/serialization/serialization.h"

#include "tgt/texture.h"
#include "tgt/vector.h"

#include <vector>
#include <string>

namespace voreen {

/**
 * Base class for transfer functions.
 *
 * The lookup table can be defined by passing pixel data that is directly
 * assigned to the transfer function's texture.
 */
class TransFunc : public Serializable {
public:

    /**
     * Constructor.
     *
     * @param width width of transfer function
     * @param height of transfer function. Pass one for a 1D transfer function.
     * @param depth of transfer function. Pass one for a 1D or 2D transfer function.
     * @param format transfer function texture's format (see tgt::Texture)
     * @param dataType transfer function texture's data type (see tgt::Texture)
     * @param filter transfer function texture's filtering mode (see tgt::Texture)
     */
    TransFunc(int width = 256, int height = 1, int depth = 1,
        GLint format = GL_RGBA, GLenum dataType = GL_UNSIGNED_BYTE, tgt::Texture::Filter filter = tgt::Texture::NEAREST);

    /**
     * Destructor.
     */
    virtual ~TransFunc();

    /**
     * Returns a define for the usage of transfer functions in shaders.
     * For 1D transfer functions the define looks like: "#define TF_SAMPLER_TYPE sampler1D \n"
     * and with sampler2D for 2D transfer functions.
     *
     * @return define for usage of transfer functions in shaders
     */
    virtual std::string getShaderDefines() const;

    /**
     * Returns a string representation of the sampler type: "sampler1D" for 1D transfer
     * functions, "sampler2D" for 2D transfer functions, "sampler3D" for 3D transfer functions
     *
     * @return string representation of the sampler type used by the transfer function
     */
    virtual std::string getSamplerType() const;

    /**
     * Returns the dimensions of the transfer function's texture.
     */
    tgt::ivec3 getDimensions() const;

    /**
     * Binds the transfer function texture. If it has been marked invalid,
     * updateTexture() is called before.
     *
     * @note If the texture is not present when calling this function, it is created.
     *    Therefore, the caller has to make sure that a valid OpenGL context is active.
     */
    void bind();

    /**
     * Returns the texture of the transfer function. If it has been marked invalid,
     * updateTexture() is called before.
     *
     * @note If the texture is not present when calling this function, it is created.
     *    Therefore, the caller has to make sure that a valid OpenGL context is active.
     */
    tgt::Texture* getTexture();

    /**
     * Sets the dimensions of the transfer function to the given values.
     * The texture is not updated at this point, but mark as invalid so it will be updated on next access.
     *
     * @param width width of the transfer function.
     * @param height height of the transfer function. Set to one for 1D transfer functions.
     * @param depth height of the transfer function. Set to one for 1D or 2D transfer functions.
     */
    virtual void resize(int width, int height = 1, int depth = 1);

    /**
     * Marks the texture of the transfer function as invalid,
     * so it will be updated on next access.
     */
    void invalidateTexture();

    /**
     * Returns whether the texture will be updated on next access.
     */
    bool isTextureInvalid() const;

    /**
     * Updates the texture of the transfer function or creates it, if it is not present.
     * The base class implementation uploads the texture data to the GPU.
     *
     * @note This function is called automatically on each texture access, when the texture is marked as invalid.
     *    You might call it directly in order to force an immediate update. But then you have to make sure that
     *    an valid OpenGL context is active.
     *
     */
    virtual void updateTexture();

    /**
     * Use this function to define the transfer function's look up table.
     * The passed pixel data is assigned to the transfer function's texture.
     *
     * @param data the pixel data to assign. Must match the transfer function
     *      in dimension, format and data type. Use reinterpret_cast in order
     *      to pass data of other type than GLubyte.
     *
     * @warning The transfer function object takes ownership of the passed memory chunk.
     *      Therefore, the caller must not delete it!
     */
    virtual void setPixelData(GLubyte* data);

    /**
     * Returns the transfer function texture's pixel data.
     */
    virtual GLubyte* getPixelData();

    /**
     * Creates a transfer function out of the data contained in the file given by filename.
     *
     * @param filename The path to the file in which the data is stored
     * @return true when the load was successful, false otherwise
     */
    virtual bool load(const std::string& filename);

    /**
     * Returns a vector that contains the endings of suppported file formats for loading.
     *
     * @return vector with endings of supported file formats
     */
    const std::vector<std::string>& getLoadFileFormats() const;

    /**
     * Returns a vector that contains the endings of supported file formats for saving.
     *
     * @return vector with endings of supported file formats
     */
    const std::vector<std::string>& getSaveFileFormats() const;

    /**
     * Dummy. The lookup table is not serialized.
     *
     * @see Serializable::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * Dummy. The lookup table is not deserialized.
     *
     * @see Serializable::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

    /**
     * Returns a copy of this object.
     */
    virtual TransFunc* clone() const;

    /**
     * Returns the format of the transfer function texture internally used.
     */
    GLint getFormat() const;

protected:

    /**
     * Generates the transfer function texture according to the specified parameters.
     */
    virtual void createTex();

    tgt::Texture* tex_;           ///< the texture of the transfer function, is generated internally
    tgt::ivec3 dimensions_;       ///< dimensions of the transfer function texture
    GLint format_;                ///< format of the transfer function texture
    GLenum dataType_;             ///< data type of the transfer function texture
    tgt::Texture::Filter filter_; ///< filtering mode of the transfer function texture.

    std::vector<std::string> loadFileFormats_; ///< endings that are supported for loading a transfer function
    std::vector<std::string> saveFileFormats_; ///< endings that are supported for saving a transfer function

    bool textureInvalid_;         ///< indicates whether the transfer function texture has to be updated

private:
    /**
     * Adapts the given width and height of transfer function to graphics board capabilities.
     * The result is stored in the reference parameters.
     *
     * @param width desired width for the transfer function
     * @param height desired height for the transfer function
     * @param depth desired depth for the transfer function
     */
    void fitDimensions(int& width, int& height, int& depth) const;

    static const std::string loggerCat_; ///< logger category
};

} // namespace voreen

#endif // VRN_TRANSFUNC_H
