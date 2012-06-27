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

#ifndef VRN_VOLUMEGL_H
#define VRN_VOLUMEGL_H

#include <typeinfo>
#include <vector>

#include "tgt/tgt_gl.h"

#include "voreen/core/volume/volume.h"
#include "voreen/core/volume/volumetexture.h"

namespace voreen {

// forward declarations
class TransFunc;

/**
 * This class is the OpenGL interface to Volume objects.
 * One or several 3D-textures are created which hold the complete dataset.
 * Furthermore VolumeGL handles the application of the
 * TransFunc, see enum TFSupport.
 */
class VolumeGL {
public:
    /**
     * Indicates what must be done for the underlying hardware.
     */
    enum TFSupport {
        /// The TransFunc will be implemented via texture lookup in the fragment shader.
        SHADER,
        /// If no fragment shader is available, but support for paletted textures is, this will be used.
        PALETTED_TEXTURES,
        /// If neither fragment shaders nor paletted textures are available a software implementation must be used.
        SOFTWARE
    };

    /**
     * Indicates how large datasets are handled. Default is BRICK.
     */
    enum LargeVolumeSupport {
        /// Large datasets are handled via resizing to \a getMax3DTexSize using a nearest filter
        RESIZE_NEAREST,
        /// Large datasets are handled via resizing to \a getMax3DTexSize using a linear filter
        RESIZE_LINEAR,
        /// Large datasets are handled via splitting the Volume into several \a getMax3DTexSize -ed VolumeTexture objects.
        BRICK
    };

    //
    // constructor and destructor
    //

    /**
     * Creates the VolumeTexture instances and applies the TransFunc.
     * \p tf ist only needed, when \a tfSupport_ isn't \A SHADER. You can set
     * Set this parameter to 0 if you want to apply the TransFunc later. Use
     * \a applyTransFunc for this. Hower, if \a tfSupport_ is \A SHADER
     * everthing will work fine and a call to \a applyTransFunc isn't
     * necessary at all.
     */
    VolumeGL(Volume* volume, const TransFunc* tf = 0, float alphaScale = 1.f,
             tgt::Texture::Filter filter = tgt::Texture::LINEAR) throw (std::bad_alloc);

    /// This class will not delete its \a volume_.
    ~VolumeGL();

    //
    //    getters and setters
    //

    /// Returns the number of textures which are held by this class
    size_t getNumTextures() const;

    /**
     * Returns a const pointer to the texture with index \p i.
     * The VolumeTexture pointers \e DO \e NOT have any special order. If you
     * want to sort them by distance use \a getSortedTextures
     *
     * @param i The index of the desired VolumeTexture
     * @return The desired VolumeTexture
     */
    const VolumeTexture* getTexture(size_t i = 0) const;

    /**
     * Returns a pointer to the Volume which was used to build an instance
     * of this class
     *
     * @return The Volume which was used in the contructor.
     */
    Volume* getVolume();

    /**
     * Returns a const pointer to the Volume which was used to build an instance
     * of this class
     *
     * @return The Volume which was used in the contructor.
     */
    const Volume* getVolume() const;

    /**
     * Returns the filter which is used during the creation of textures.
     *
     * @return The used filter.
     */
    tgt::Texture::Filter getFilter() const;

    /**
     * Sets the filter which should be used during the creation of textures.
     *
     * @param filter The filter which should be used.
     */
    void setFilter(tgt::Texture::Filter filter);

    //
    // static getters and setters
    //

    /**
     * Use this method to tell VolumeGL how large datasets should be handled.
     * Default is BRICK.
     *
     * @param lvSupport How should large datasets be handled?
     */
    static void setLargeVolumeSupport(LargeVolumeSupport lvSupport);

    /// How are large datasets beeing handled?
    static LargeVolumeSupport getLargeVolumeSupport();

    /**
     * Use this method to dictate VolumeGL which maximal 3d texture size
     * should be used for the handling of large datasets. Setting this to 0
     * (the default value) will indicate VolumeGL to use
     * GpuCaps.getMax3DTextureSize() instead. Using a greater value than this will
     * cause errors. Furthermore, it is assumed that \p Max3DTexSize is a power
     * of two.
     *
     * @param lvSupport How should large datasets be handled?
     */
    static void setMax3DTexSize(int max3DTexSize);

    /// What is the maximal 3d textures size used in VolumeGL?
    static int getMax3DTexSize();

    //
    // further methods
    //

    /// Type used to access sorted textures
    typedef std::vector<const VolumeTexture*> SortedTextures;

    /**
     * Use this method to sort textures by distance with a given matrix
     * to transform the position of each VolumeTexture in the internal
     * data structure.
     *
     * @param m The matrix used to transform the Volume into camera system.
     * @param eye The eye point.
     * @return A std::vector\<const VolumeTexture*> sorted by distance, farest first
     */
    SortedTextures getSortedTextures(const tgt::mat4& m, tgt::vec3 eye = tgt::vec3::zero);

    /**
     * Use this method in order to apply the TransFunc to the volume. If your
     * Renderer just works with a fragment shader you don't need to call this method.
     * On the other hand it won't harm if you do it anyway.
     * Invoke it when you want either support for paletted textures or a
     * software implementation of the transfer function.
     *
     * @param tf The TransFunc which should be applied.
     * @param alphaScale You can scale the alpha values. This is useful
     *      if you have a different sampling rate then 1.f. <br>
     *      \e Warinng This will only have an effect when tfSupport_ is \e NOT
     *      equal SHADER!.
     */
    void applyTransFunc(const TransFunc* tf, float alphaScale = 1.f) throw(std::bad_alloc);

protected:
    /**
     * Used internally by the constructor and applyTransFunc.
     */
    void generateTextures(const TransFunc* tf, float alphaScale = 1.f) throw(std::bad_alloc);

    /**
     * Used internally to upload newly created textures. If there is no Fragment
     * Shader support available a paletted Texture will be created.
     */
    void uploadTexture( const TransFunc* tf, float alphaScale,
                        Volume* v,
                        const tgt::mat4& matrix,
                        const tgt::vec3& llf,
                        const tgt::vec3& urb);

    Volume* origVolume_; ///< The original Volume which was specified when calling the constructor.
    Volume* volume_;     ///< All work is done on this Volume. Can be just a pointer to *origVolume_ or a resized Volume.

    const std::type_info& volumeType_;///< The type_info of the Volume which is used to create this class.

    tgt::Texture::Filter filter_; ///< Filter mode used when creating textures.

    TFSupport tfSupport_; ///< Which method is used to implement the TransFunc?

    GLint format_;        ///< The format of textures which will are created.
    GLint internalFormat_;///< The internal format of the textures which are created.
    GLenum dataType_;     ///< The data type of the textures which are created.

    typedef std::vector<VolumeTexture*> VolumeTextures;
    VolumeTextures textures_;

    static const std::string loggerCat_;

private:
    /// Used internally for destruction of the data.
    void destroy();

    static LargeVolumeSupport lvSupport_;
    static int max3DTexSize_;
};

typedef TemplateMessage<VolumeGL*> VolumeGLMsg;

} // namespace voreen

#endif // VRN_VOLUMEGL_H
