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

#ifndef VRN_VOLUMESLICEHELPER_H
#define VRN_VOLUMESLICEHELPER_H

#include "tgt/texture.h"
#include "tgt/matrix.h"

#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/utils/backgroundthread.h"

#include <boost/thread/mutex.hpp>

namespace voreen {

class Processor;
class TriangleMeshGeometryVec3;

enum SliceAlignment {
    YZ_PLANE = 0,
    XZ_PLANE = 1,
    XY_PLANE = 2,
    UNALIGNED_PLANE = 3,
};

//-------------------------------------------------------------------------------------------------

class VRN_CORE_API VolumeSliceGL {

public:
    VolumeSliceGL(const tgt::svec2& sliceDim, SliceAlignment alignment, const std::string& format, const std::string& baseType,
        tgt::vec3 origin, tgt::vec3 xVec, tgt::vec3 yVec, RealWorldMapping rwm,
        void* data, GLint textureFormat, GLint internalFormat, GLenum textureDataType);
    VolumeSliceGL(const tgt::svec2& sliceDim, SliceAlignment alignment, const std::string& format, const std::string& baseType,
        tgt::vec3 origin, tgt::vec3 xVec, tgt::vec3 yVec, RealWorldMapping rwm,
        tgt::Texture* tex);
    virtual ~VolumeSliceGL();

    /**
     * Binds the slice texture to the currently active texture unit and updates the texture beforehand,
     * if necessary (i.e., the texture is invalid).
     */
    void bind() const;

    std::string getFormat() const;
    std::string getBaseType() const;
    tgt::svec2 getDimensions() const;

    SliceAlignment getAlignment() const;

    tgt::mat4 getTextureToWorldMatrix() const;
    tgt::mat4 getWorldToTextureMatrix() const;

    RealWorldMapping getRealWorldMapping() const;

    const tgt::Texture* getTexture() const;

private:
    void createTexture() const;

    std::string format_;        ///< pixel format (see VolumeFactory)
    std::string baseType_;      ///< pixel base type (e.g., "float" for format "Vector3(float)")

    SliceAlignment alignment_;
    tgt::svec2 dimensions_;

    tgt::vec3 origin_;
    tgt::vec3 xVec_;
    tgt::vec3 yVec_;

    RealWorldMapping rwm_;

    void* data_;

    mutable tgt::Texture* tex_;
    GLint textureFormat_;
    GLint internalFormat_;
    GLenum textureDataType_;
};

//------------------------------------------------------------------------------------------------

/**
 * Helper class for the extraction of slices from a volume.
 */
class VRN_CORE_API VolumeSliceHelper {

public:

/**
 * Extracts an axis-aligned slice of the passed volume.
 * The caller takes ownership of the returned object.
 *
 * @param volumeRAM the volume to create the slice texture from.
 * @param alignment Slice direction to be extracted, must be axis-aligned.
 * @param sliceIndex Index of the slice to be extracted, must be smaller than volumeDim[alignment].
 * @param levelOfDetail specifies the desired level of detail of the slice, with 0 being the full resolution.
 *        This parameter is currently only relevant, if the slice if constructed from an octree representation.
 * @param timeLimit maximum time in milliseconds the slice construction is allowed to take. After the time-limit
 *        has been reached, all subsequent octree brick accesses are omitted unless the bricks are already present in the CPU RAM.
 *        A value of 0 is interpreted as no time limit. This parameter is currently only relevant for octree representations.
 * @param complete out-parameter returning whether the slice could be constructed in full resolution (or the desired LOD)
 *        or whether the time limit has caused a resolution reduction. If the null pointer is passed, no value is returned.
 *
 * @see VolumeOctreeBase
 *
 * @return the created slice texture, whose dimensions and data type matches the input volume's properties.
 *         If the slice texture could not be created, 0 is returned.
 */
static VolumeSliceGL* getVolumeSlice(const VolumeBase* volume, SliceAlignment alignment, size_t sliceIndex,
    size_t levelOfDetail = 0, clock_t timeLimit = 0, bool* complete = 0);

/**
 * Extracts an arbitrarily aligned slice from the passed volume.
 * TODO: more doc
 */
static VolumeSliceGL* getVolumeSlice(const VolumeBase* volume, tgt::plane pl, float samplingRate);

/**
 * Extracts an axis-aligned 2D slice from the passed RAM volume and copies the pixel data to the passed buffer.
 *
 * @param volumeRAM the volume to create the slice from.
 * @param alignment Slice direction to be extracted, must be axis-aligned.
 * @param sliceIndex Index of the slice to be extracted, must be smaller than volumeDim[alignment].
 * @param dataBuffer Out-parameter that will hold the pixel data buffer.
 *                   Note: The buffer will be allocated internally by the function!
 * @param textureFormat Out-parameter specifiying the OpenGL texture format to use when creating a tgt::Texture from the returned pixel data buffer.
 * @param internalFormat Out-parameter specifiying the OpenGL internal texture format to use when creating a tgt::Texture from the returned pixel data buffer.
 * @param textureDataType Out-parameter specifiying the OpenGL texture data type to use when creating a tgt::Texture from the returned pixel data buffer.
 *
 */
static void extractAlignedSlicePixelData(const VolumeRAM* volumeRAM, SliceAlignment alignment, size_t sliceIndex,
    void*& dataBuffer, GLint& textureFormat, GLint& internalFormat, GLenum& textureDataType)
    throw (VoreenException);

/**
 * @brief Generates a geometry that represents slice number \p sliceIndex with orientation \p alignment through the volume \p vh
 *
 * @param vh The primary volume.
 * @param applyTransformation Apply the physicalToWorld-Matrix?
 * @param secondaryVolumes You can specify additional volumes to extend the area of the slice geometry to include these volumes. (For multi-volume slicing)
 */
static TriangleMeshGeometryVec3* getSliceGeometry(const VolumeBase* vh, SliceAlignment alignment, float sliceIndex, bool applyTransformation = true, const std::vector<const VolumeBase*> secondaryVolumes = std::vector<const VolumeBase*>());

protected:
    static const std::string loggerCat_;
};

//------------------------------------------------------------------------------------------------


class SliceCreationBackgroundThread;

/**
 * Fixed-size LRU cache for 2D volume slices. On each getVolumeSlice() call the VolumeSliceCache checks whether
 * a slice with the specified parameters is present in the cache and returns it, if this is the case.
 * If not, the slice is extracted from the underlying volume via the VolumeSliceHelper and stored in the cache.
 */
class VRN_CORE_API VolumeSliceCache {
    friend class SliceCreationBackgroundThread;
public:
    VolumeSliceCache(Processor* owner, size_t cacheSize);
    ~VolumeSliceCache();

    size_t getCacheSize() const;
    void setCacheSize(size_t cacheSize);
    void clear();

    VolumeSliceGL* getVolumeSlice(const VolumeBase* volume, SliceAlignment alignment, size_t sliceIndex,
        size_t levelOfDetail = 0, clock_t timeLimit = 0, bool* complete = 0, bool asynchronous = false) const;
    VolumeSliceGL* getVolumeSlice(const VolumeBase* volume, tgt::plane pl, float samplingRate, bool asynchronous = false) const;

    bool hasSliceInCache(const VolumeBase* volume, SliceAlignment alignment, size_t sliceIndex, size_t levelOfDetail = 0) const;
    bool hasSliceInCache(const VolumeBase* volume, tgt::plane pl, float samplingRate) const;

private:
    struct CacheEntry {
        std::string hash_;  //< consisting of the input volume hash and a hash of the slice parameters
        VolumeSliceGL* slice_;
    };

    void addSliceToCache(VolumeSliceGL* slice, const std::string& hash) const;
    void cleanupCache() const;
    VolumeSliceGL* findSliceInCache(const std::string& hash, bool updateUsage) const;

    std::string getHash(const VolumeBase* volume, SliceAlignment alignment, size_t sliceIndex, size_t levelOfDetail) const;
    std::string getHash(const VolumeBase* volume, tgt::plane pl, float samplingRate) const;

    Processor* owner_;

    mutable std::list<CacheEntry> slices_;
    size_t cacheSize_;

    mutable SliceCreationBackgroundThread* currentBackgroundThread_;
    mutable boost::mutex cacheAccessMutex_; ///< mutex for synchronizing cache accesses
};


} // namespace voreen

#endif
