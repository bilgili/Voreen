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

#include "voreen/core/volume/volumegl.h"

#include <algorithm>
#include <typeinfo>

#include "tgt/shadermanager.h"
#include "tgt/gpucapabilities.h"

#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/volume/volumeset.h"
#include "voreen/core/vis/transfunc/transfunc.h"

#ifdef VRN_MODULE_GLYPHS_MESH
#include "voreen/modules/glyphs_mesh/meshpositiondata.h"
#endif

using tgt::vec3;
using tgt::bvec3;
using tgt::ivec3;
using tgt::col4;
using tgt::mat4;

/*
 * helpers
 */

namespace {

// calculates next power of two if it is not already power of two
static int fitPowerOfTwo(int i) {
    int result = 1;

    while (result < i)
        result <<= 1;

    return result;
}

// same as above but for each component of a ivec3
static ivec3 fitPowerOfTwo(const ivec3& v) {
    ivec3 result;

    result.x = fitPowerOfTwo(v.x);
    result.y = fitPowerOfTwo(v.y);
    result.z = fitPowerOfTwo(v.z);

    return result;
}

// returns true, if dims.x, dims.y and dims.z are power of two
bool isPowerOfTwo(const ivec3& dims) {
    return dims == fitPowerOfTwo(dims);
}

} // namespace

namespace voreen {

/*
 * init statics
 */

VolumeGL::LargeVolumeSupport VolumeGL::lvSupport_ = RESIZE_LINEAR;
int VolumeGL::max3DTexSize_ = 0;
int VolumeGL::availableGpuMemory_ = 0; 
const std::string VolumeGL::loggerCat_("Voreen.VolumeGL");


/*
 * constructor and destructor
 */

VolumeGL::VolumeGL(Volume* volume, const TransFunc* tf /*= 0*/, float alphaScale /*= 1.f*/,
                   tgt::Texture::Filter filter /*= tgt::Texture::LINEAR*/) throw (std::bad_alloc)
  : origVolume_(volume),
    volume_(volume),
    volumeType_( typeid(*volume) ),
    filter_(filter)
{
    /*
        check volume type and set appropiate format_, internalFormat_ and dataType_ values
    */

    // VolumeUIntX
    if (volumeType_ == typeid(VolumeUInt8)) {
        format_ = GL_ALPHA;
        internalFormat_ = GL_ALPHA8;
        dataType_ = GL_UNSIGNED_BYTE;
    }
    else if (volumeType_ == typeid(VolumeUInt16)) {
        format_ = GL_ALPHA;
        internalFormat_ = GL_ALPHA16;
        dataType_ = GL_UNSIGNED_SHORT;
    }
    else if (volumeType_ == typeid(VolumeUInt32)) {
        format_ = GL_ALPHA;
        internalFormat_ = GL_ALPHA;
        dataType_ = GL_UNSIGNED_INT;
    }
    // VolumeIntX
    else if (volumeType_ == typeid(VolumeInt8)) {
        format_ = GL_ALPHA;
        internalFormat_ = GL_ALPHA8;
        dataType_ = GL_BYTE;
    }
    else if (volumeType_ == typeid(VolumeInt16)) {
        format_ = GL_ALPHA;
        internalFormat_ = GL_ALPHA16;
        dataType_ = GL_SHORT;
    }
    else if (volumeType_ == typeid(VolumeInt32)) {
        format_ = GL_ALPHA;
        internalFormat_ = GL_ALPHA;
        dataType_ = GL_INT;
    }
    // VolumeFloat and VolumeDouble
    else if (volumeType_ == typeid(VolumeFloat)) {
        format_ = GL_ALPHA;
        internalFormat_ = GL_ALPHA;
        dataType_ = GL_FLOAT;
    }
    else if (volumeType_ == typeid(VolumeDouble)) {
        tgtAssert(false, "OpenGL does not support this kind of volume directly");
    }
    // Volume2x with int16 types
    else if (volumeType_ == typeid(Volume2xUInt16)) {
        format_ = GL_LUMINANCE_ALPHA;
        internalFormat_ = GL_LUMINANCE16_ALPHA16;
        dataType_ = GL_UNSIGNED_SHORT;
    }
    else if (volumeType_ == typeid(Volume2xInt16)) {
        format_ = GL_LUMINANCE_ALPHA;
        internalFormat_ = GL_LUMINANCE16_ALPHA16;
        dataType_ = GL_SHORT;
    }
    // Volume3x with int8 types
    else if (volumeType_ == typeid(Volume3xUInt8)) {
        format_ = GL_RGB;
        internalFormat_ = GL_RGB8;
        dataType_ = GL_UNSIGNED_BYTE;
    }
    else if (volumeType_ == typeid(Volume3xInt8)) {
        format_ = GL_RGB;
        internalFormat_ = GL_RGB8;
        dataType_ = GL_BYTE;
    }
    // Volume4x with int8 types
    else if (volumeType_ == typeid(Volume4xUInt8)) {
        format_ = GL_RGBA;
        internalFormat_ = GL_RGBA8;
        dataType_ = GL_UNSIGNED_BYTE;
    }
    else if (volumeType_ == typeid(Volume4xInt8)) {
        format_ = GL_RGBA;
        internalFormat_ = GL_RGBA8;
        dataType_ = GL_BYTE;
    }
    // Volume3x with int16 types
    else if (volumeType_ == typeid(Volume3xUInt16)) {
        format_ = GL_RGB;
        internalFormat_ = GL_RGB16;
        dataType_ = GL_UNSIGNED_SHORT;
    }
    else if (volumeType_ == typeid(Volume3xInt16)) {
        format_ = GL_RGB;
        internalFormat_ = GL_RGB16;
        dataType_ = GL_SHORT;
    }
    // Volume4x with int16 types
    else if (volumeType_ == typeid(Volume4xUInt16)) {
        format_ = GL_RGBA;
        internalFormat_ = GL_RGBA16;
        dataType_ = GL_UNSIGNED_SHORT;
    }
    else if (volumeType_ == typeid(Volume4xInt16)) {
        format_ = GL_RGBA;
        internalFormat_ = GL_RGBA16;
        dataType_ = GL_SHORT;
    }
    // Volume3x with real types
    else if (volumeType_ == typeid(Volume3xFloat)) {
        format_ = GL_RGB;
        internalFormat_ = GL_RGB;
        dataType_ = GL_FLOAT;
    }
#ifdef VRN_MODULE_GLYPHS_MESH
    else if (volumeType_ == typeid(MeshPositionData)) {
        format_ = GL_RGB;
        internalFormat_ = GL_RGB;
        dataType_ = GL_FLOAT;
    }
#endif
    else if (volumeType_ == typeid(Volume3xDouble)) {
        tgtAssert(false, "OpenGL does not support this kind of volume directly");
    }
    // Volume4x with real types
    else if (volumeType_ == typeid(Volume4xFloat)) {
        format_ = GL_RGBA;
        internalFormat_ = GL_RGBA;
        dataType_ = GL_FLOAT;
    }
    else if (volumeType_ == typeid(Volume4xDouble)) {
        tgtAssert(false, "OpenGL does not support this kind of volume directly");
    }
    // -> not found
    else {
        tgtAssert(false, "unsupported volume format_");
    }

    // check hardware support
    if (GpuCaps.areShadersSupported())
        tfSupport_ = SHADER;
    else {
        if ( GpuCaps.areSharedPalettedTexturesSupported() ) {
            LINFO("Fragment shaders cannot be used for transfer function lookups. Using paletted textures instead.");
            tfSupport_ = PALETTED_TEXTURES;

            // force color index mode format
            format_ = GL_COLOR_INDEX;

            // use paletted textures
            glEnable(GL_SHARED_TEXTURE_PALETTE_EXT);

            // convert GL_ALPHAx to GL_COLOR_INDEXx
            switch (internalFormat_) {
                case GL_ALPHA4:
                    internalFormat_ = GL_COLOR_INDEX4_EXT;
                    break;
                case GL_ALPHA8:
                    internalFormat_ = GL_COLOR_INDEX8_EXT;
                    break;
                case GL_ALPHA12:
                    internalFormat_ = GL_COLOR_INDEX12_EXT;
                    break;
                case GL_ALPHA16:
                    internalFormat_ = GL_COLOR_INDEX16_EXT;
                    break;
                default:
                    tgtAssert(false, "unsupported volume type");
            }
        }
        else {
            LINFO("Textures must be created every time the transfer function changes");
            tfSupport_ = SOFTWARE;

            // force RGBA and RGBA8 format with unsigned byte
            format_ = GL_RGBA;
            internalFormat_ = GL_RGBA8;
            dataType_ = GL_UNSIGNED_BYTE;
        }
    }

    // do not call generateTextures if tf is zero and tfSupport_ == SOFTWARE
    if (tf != 0 || tfSupport_ != SOFTWARE) {
        try {
            generateTextures(tf, alphaScale);
        }
        catch (std::bad_alloc) {
            // release all resources
            destroy();

            throw; // throw it to the caller
        }
    }
}

VolumeGL::~VolumeGL() {
    destroy();
}

void VolumeGL::destroy() {
    // delete created VolumeTexture objects
    for (size_t i = 0; i < textures_.size(); ++i)
        delete textures_[i];

    // check whether a temporary scaled volume was created and delete it
    if (volume_ != origVolume_)
        delete volume_;
}

/*
 * getters and setters
 */

size_t VolumeGL::getNumTextures() const {
    return textures_.size();
}

const VolumeTexture* VolumeGL::getTexture(size_t i /*= 0*/) const {
    tgtAssert(((i>=0) && (i<getNumTextures())), "Index out of bounds!");
    return textures_[i];
}

Volume* VolumeGL::getVolume() {
    return volume_;
}

const Volume* VolumeGL::getVolume() const {
    return volume_;
}

tgt::Texture::Filter VolumeGL::getFilter() const {
    return filter_;
}

void VolumeGL::setFilter(tgt::Texture::Filter filter) {
    filter_ = filter;
}

/*
 * static getters and setters
 */

void VolumeGL::setLargeVolumeSupport(VolumeGL::LargeVolumeSupport lvSupport) {
    lvSupport_ = lvSupport;
}

VolumeGL::LargeVolumeSupport VolumeGL::getLargeVolumeSupport() {
    return lvSupport_;
}

void VolumeGL::setMax3DTexSize(int max3DTexSize) {
    // check user errors
    tgtAssert(fitPowerOfTwo(max3DTexSize) == max3DTexSize, "Max3DTexSize must be a power of two");
    tgtAssert(max3DTexSize >= 0, "Max3DTexSize must be greater than equal zero");

#ifdef VRN_DEBUG
    if ( tgt::Singleton<tgt::TextureManager>::isInited())
        tgtAssert(max3DTexSize_ <= GpuCaps.getMax3DTextureSize(),
                  "max3DTexSize must be less than equal GpuCaps.getMax3DTextureSize()");
#endif // VRN_DEBUG

    max3DTexSize_ = max3DTexSize;
}

int VolumeGL::getMax3DTexSize() {
    return max3DTexSize_;
}

void VolumeGL::setAvailableGpuMemory(int availableGpuMemory) {
	availableGpuMemory_ = availableGpuMemory;
}

int VolumeGL::getAvailableGpuMemory() {
	return availableGpuMemory_;
}

/*
 * further methods
 */

void VolumeGL::applyTransFunc(const TransFunc* tf, float alphaScale /*= 1.f*/) throw (std::bad_alloc) {
    if (tf == 0)
        return;

    switch (tfSupport_) {
    case SHADER:
        return; // nothing to do

    case PALETTED_TEXTURES:
        // TODO use alphaScale here
        glColorTable(
            GL_SHARED_TEXTURE_PALETTE_EXT,          // target
            tf->getTexture()->getInternalFormat(),  // internalformat
            GpuCaps.getColorTableWidth(),           // width
            tf->getTexture()->getFormat(),          // format
            tf->getTexture()->getDataType(),        // type
            tf->getTexture()->getPixelData()        // table
            );
        return; // not neccessary to regenerate textures

    case SOFTWARE:
        // destroy all textures which were created prior
        for (size_t i = 0; i < textures_.size(); ++i)
            delete textures_[i];

        // reset vector
        textures_.clear();

        // and regenerate textures
        try {
            generateTextures(tf, alphaScale);
        }
        catch (std::bad_alloc) {
            // release all resources
            destroy();

            throw; // throw it to the caller
        }
    }
}

void VolumeGL::generateTextures(const TransFunc* tf, float alphaScale /*= 1.f*/) throw (std::bad_alloc) {
    bool fitsInVideoRam = true;
    
    #ifdef VERY_LARGE_DATASETS
        fitsInVideoRam = false;
    #endif
    
    
    if (fitsInVideoRam) {
/*
    WARNING THE ALGORITHM BELOW WAS HARD WORK AND IT CAN BE CONSIDERED AS
        __VERY__ __EASY__ TO BREAK SOMETHING ALTHOUGH A BUG IN THIS CODE
        IS STILL POSSIBLE. PLEASE CONTACT ME BEFORE CHANGING ANYTHING BELOW THIS
        COMMENT UNTIL THE END OF THIS WARNING. THANK YOU! (roland)

    TODO bricking is currently only correct when using LINEAR texture filter (roland)
*/
        /*
            check whether the volume must be split
        */
        ivec3 maxTexSize = ivec3( max3DTexSize_ ? max3DTexSize_ : GpuCaps.getMax3DTextureSize() );
    
        if (maxTexSize.x == 0)
            return; // We don't have 3d texture support, maybe one day we could support 2d textures instead
    
        ivec3 volumeDims = volume_->getDimensions();
        vec3 volumeSpacing = volume_->getSpacing();
    
        /*
            this bool vec knows for each dimensions
            whether splitting must be performed in this direction
        */
        bvec3 splitNecessary = lessThan(maxTexSize, volumeDims);

		//This checks if the texture would fit into the gpu memory
		long numVoxels = static_cast<long>(volume_->getNumVoxels());
		int bytesAllocated = volume_->getBitsAllocated() / 8;
		int volumeSizeMB = static_cast<int>(ceil((numVoxels*bytesAllocated) / (1024.0 * 1024.0)));
		bool fitsInGpuMemory;

		//if availableGpuMemory_ is 0 we assume the volume fits into gpu memory
		if ( (volumeSizeMB < availableGpuMemory_) || (availableGpuMemory_ ==0) )
			fitsInGpuMemory = true;
		else
			fitsInGpuMemory = false;

		//Do the dimensions of the texture exceed the max3DTexSize?
		bool dimensionsFit = !hor(splitNecessary);
    
        // set pixel store with no alignment when reading from main memory
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		//This means the volume does fit into gpu memory and the dimensions
		//aren't too big either
		bool volumeOK = true;

		//The ratio we are using to downscale the volume in case it is not ok
		float ratio;

		if (fitsInGpuMemory && (!dimensionsFit) ) {
			//Only the dimensions are too big, the memory of the gpu is enough
			volumeOK = false;
			ratio = float(maxTexSize.x) / max( vec3(volumeDims) ); 	
		} 
		else if ( (!fitsInGpuMemory) && dimensionsFit) {
			//The dimensions are ok, but the volume is too big for the gpu memory
			volumeOK = false;
			//Calculate the ratio for the downscaling. This is a little complicated. The
			//ratio is the third root of (maxNumberOfAvailableVoxels / v.x * v.y * v.z)
			long maxMemorySizeInByte = availableGpuMemory_ * 1024*1024;
			long maxNumberOfVoxels = maxMemorySizeInByte / bytesAllocated;
			float temp = static_cast<float>(maxNumberOfVoxels) / static_cast<float>(numVoxels);
			ratio = pow(temp,static_cast<float>(1.0/3.0));
		} 
		else if ( (!fitsInGpuMemory) && (!dimensionsFit) ) {
			//The dimensions are too big AND the volume is too big for the gpu memory
			volumeOK = false;
			//Check if the volume would fit into gpu memory if we scaled it down to
			//max3DTexSize
			ratio = static_cast<float>(maxTexSize.x) / max( vec3(volumeDims) );
			tgt::ivec3 newDims = ivec3(ratio * vec3(volumeDims));
			int newVolumeSizeMB = static_cast<int>(ceil((newDims.x * newDims.y * newDims.z *bytesAllocated) / (1024.0 * 1024.0)));

			//If the newVolumeSize is sufficiently small, we're done, otherwise we have to downscale even more.
			if (newVolumeSizeMB >= availableGpuMemory_) {
				//Calculate the ratio for the downscaling. This is a little complicated. The
				//ratio is the third root of (maxNumberOfAvailableVoxels / v.x * v.y * v.z)
				long maxMemorySizeInByte = availableGpuMemory_ * 1024*1024;
				long maxNumberOfVoxels = maxMemorySizeInByte / bytesAllocated;
				float temp = static_cast<float>(maxNumberOfVoxels) / static_cast<float>(numVoxels);
				ratio = pow(temp,static_cast<float>(1.0/3.0));
			}
		}
    
        if (!volumeOK) {
            Volume::Filter filter = Volume::LINEAR;
    
            switch (lvSupport_) {
                case RESIZE_NEAREST:
                    filter = Volume::NEAREST;
                    // fall through the next case statement
                case RESIZE_LINEAR: {
                    //float ratio = float(maxTexSize.x) / max( vec3(volumeDims) );
    
                    try {
                        volume_ = origVolume_->scale( ivec3(ratio * vec3(volumeDims)), filter);
                    }
                    catch (std::bad_alloc) {
                        throw;
                    }
    
                    // update new dims and spacing values
                    volumeDims = volume_->getDimensions();
                    volumeSpacing = volume_->getSpacing();
    
                    // bricking not necessary -> we have scaled
                    volumeOK = true;
                    break;
                }
                case BRICK: { /*do nothing*/ }
            }
        }
    
        if (!volumeOK) {
            // -> we must split
            LINFO("Starting splitting now...");
    
            ivec3 size(maxTexSize);
    
            // usually we have a left and a right edge, so subtract 2
            ivec3 incr = size - 2;
    
            /*
                knows whether size has been decreased to delta in a direction
                AKA this is the last iteration of the loop
            */
            bvec3 last(false);
    
            // knows the size from the current postion until the end of the dataset
            ivec3 delta;
    
            // knows the size of the useful portion, i.e. size without edges, in each direction
            vec3 usefulSize;
    
            for (ivec3 i(0, 0, 0); i.z != volumeDims.z; i.z += incr.z) {
                delta.z = volumeDims.z - i.z;
                if (delta.z <= size.z) {
                    last.z = true;
                    incr.z = delta.z;
                    size.z = GpuCaps.isNpotSupported() ? delta.z : fitPowerOfTwo(delta.z);
                }
    
                /*
                    case: start position
                        -> i == 0 && last == false
                        -> usefulSize = size - 1
                    case: start position AND end position; AKA no splitting necessary
                        -> i == 0 && last == true
                        -> usefulSize = delta
                    case: somewhere in the middle
                        -> i.x > 0 && last == false
                        -> usefulSize = size - 2
                    case: end position while not beeing start postion
                        -> i.x > 0 && last == true
                        -> usefulSize = delta - 1
                */
                usefulSize.z = i.z
                    ? (last.z ? delta.z - 1.0f : size.z - 2.0f)
                    : (last.z ? delta.z        : size.z - 1.0f);
    
                // find out left edge
                vec3 leftEdge;
                leftEdge.z = i.z ? 1.f : 0.f;
    
                for (i.y = 0; i.y != volumeDims.y; i.y += incr.y) {
                    delta.y = volumeDims.y - i.y;
                    if (delta.y <= size.y) {
                        last.y = true;
                        incr.y = delta.y;
                        size.y = GpuCaps.isNpotSupported() ? delta.y : fitPowerOfTwo(delta.y);
                    }
    
                    // see above for details
                    usefulSize.y = i.y
                        ? (last.y ? delta.y - 1.0f : size.y - 2.0f)
                        : (last.y ? delta.y        : size.y - 1.0f);
    
                    // find out left edge
                    leftEdge.y = i.y ? 1.f : 0.f;
    
                    for (i.x = 0; i.x != volumeDims.x; i.x += incr.x) {
                        delta.x = volumeDims.x - i.x;
                        if (delta.x <= size.x) {
                            last.x = true;
                            incr.x = delta.x;
                            size.x = GpuCaps.isNpotSupported() ? delta.x : fitPowerOfTwo(delta.x);
                        }
    
                        // see above for details
                        usefulSize.x = i.x
                            ? (last.x ? delta.x - 1.0f : size.x - 2.0f)
                            : (last.x ? delta.x        : size.x - 1.0f);
    
                        // find out left edge
                        leftEdge.x = i.x ? 1.f : 0.f;
    
                        Volume* subset;
                        try {
                            subset = volume_->createSubset(i, size);
                        }
                        catch (std::bad_alloc) {
                            throw; // throw it to the caller
                        }
    
                        /*
                            create proper matrix:
                            m = trans(leftEdge/size) * scale(usefulSize/size)
                        */
    
                        // translate one position if we have started on a position != 0
                        vec3 trans = leftEdge/vec3(size);
                        vec3 scale = usefulSize/vec3(size);
    
                        /*
                            calculate urb and llf
                        */
                        vec3 cubeSize = vec3(volumeDims) * volumeSpacing;
                        float maxElem = max(cubeSize);
                        vec3 llf = ( leftEdge + vec3(i) ) * volumeSpacing;
                        vec3 urb = llf + usefulSize * volumeSpacing;
                        llf -= cubeSize * 0.5f; // center around orign
                        urb -= cubeSize * 0.5f; // center around orign
                        llf /= maxElem * 0.5f;  // map to [-1, 1]
                        urb /= maxElem * 0.5f;  // map to [-1, 1]
    
                        // swap z of llf and urb since we use a right handed coordinate system
                        std::swap(llf.z, urb.z);
    
                        // build texture matrix
                        mat4 mTex = mat4::createTranslation(trans);
                        mTex *= mat4::createScale(scale);
    
                        // create and upload texture
                        uploadTexture(tf, alphaScale, subset, mTex, llf, urb);
    
                        // destroy the created subset and its associated data
                        delete subset;
                    } // for -> x
    
                    // reset last.x, incr.x and size.x
                    last.x = false;
                    incr.x = maxTexSize.x - 2;
                    size.x = maxTexSize.x;
                } // for -> y
    
                // reset last.y, incr.y and size.y
                last.y = false;
                incr.y = maxTexSize.y - 2;
                size.y = maxTexSize.y;
            } // for -> z
		} // if split neccessary
    
    // WARNING END
        
		else {
            // -> we don't have to split
    
        /*
                check if non-power-of-two textures are not supported
            */
    
            if (!GpuCaps.isNpotSupported() && !isPowerOfTwo(volumeDims)) {
                /*
                    This is could also be done in the loop above without changes,
                    but the code is clearer like this
                */
    
                // this stores the new POT-dataset
                Volume* potVolume = 0;
    
                // -> no non-power-of-two-support
                LINFO("No NPOT-support available, starting to bloat to POT...");
    
                try {
                    potVolume = volume_->createSubset( ivec3(0, 0, 0), fitPowerOfTwo(volumeDims) );
                }
                catch (std::bad_alloc) {
                    throw; // throw it to the caller
                }
    
                ivec3 potVolumeDims = potVolume->getDimensions();
    
                // store transformat_ion for texCoords
                vec3 scale = vec3(volumeDims)/vec3(potVolumeDims);
    
                // create and upload texture
                uploadTexture( tf, alphaScale, potVolume, mat4::createScale(scale), volume_->getLLF(), volume_->getURB() );
    
                // destroy the newly create potVolume
                delete potVolume;
            }
            else {
                // non-power-of-two-support available    
                // create and upload texture
                uploadTexture( tf, alphaScale, volume_, mat4::identity, volume_->getLLF(), volume_->getURB() );
            }
        }
    }
    else {
        // data set does not fit in video ram
        LWARNING("dataset is too large for video ram!");
		/*
		VolumeOctreeNode* von = new VolumeOctreeNode(volume_, ivec3(0,0,0), volume_->getDimensions()-ivec3(1,1,1), 128);
		// append to internal data structure
		textures_.push_back(von->getTexture());
		*/
    }
}

/*
 * internal helper
 */

void VolumeGL::uploadTexture(const TransFunc* tf, float alphaScale /*= 1.f*/,
                             Volume* v,
                             const mat4& matrix,
                             const vec3& llf,
                             const vec3& urb)
{
    // needed for software transfer function lookup
    GLubyte* temp = 0;
    float max = float( (1 << v->getBitsStored()) - 1 );

    if (tfSupport_ == SOFTWARE) {
        if (tf == 0)
            return;

        const tgt::Texture* tex = tf->getTexture();
        // alloc texture data
        temp = new GLubyte[ v->getNumVoxels() * 4 ];
        int width = tex->getDimensions().x;

        /*
            do software lookup of the transfer function
        */
        if ( volumeType_ == typeid(VolumeUInt8) ) {
            VolumeUInt8* v8 = (VolumeUInt8*) v;

            for (size_t i = 0; i < v->getNumVoxels(); ++i) {
                // calculate index: map voxel(i) to [0, 1] then map to [0, tex->width - 1] and round
                int index = int((float(v8->voxel(i))/max) * float(width - 1) + 0.5f);
                temp[i*4    ] = tf->getTexture()->texel<col4>(index)[0];
                temp[i*4 + 1] = tf->getTexture()->texel<col4>(index)[1];
                temp[i*4 + 2] = tf->getTexture()->texel<col4>(index)[2];
                temp[i*4 + 3] = static_cast<GLubyte>(tf->getTexture()->texel<col4>(index)[3] * alphaScale);
            }
        }
        else if ( volumeType_ == typeid(VolumeUInt16) ) {
            // cast to VolumeUInt16
            VolumeUInt16* v16 = (VolumeUInt16*) v;

            for (size_t i = 0; i < v->getNumVoxels(); ++i) {
                // calculate index: map voxel(i) to [0, 1] then map to [0, tex->width - 1] and round
                int index = int( (float(v16->voxel(i))/max) * float(width - 1) + 0.5f );
                temp[i*4    ] = tf->getTexture()->texel<col4>(index)[0];
                temp[i*4 + 1] = tf->getTexture()->texel<col4>(index)[1];
                temp[i*4 + 2] = tf->getTexture()->texel<col4>(index)[2];
                temp[i*4 + 3] = static_cast<GLubyte>(tf->getTexture()->texel<col4>(index)[3] * alphaScale);
            }
        }
        else {
            tgtAssert( false, "type must be either VolumeUInt8 or VolumeUInt16 here" );
        }
    }

    // create texture
    VolumeTexture* vTex = new VolumeTexture(
        temp ? temp : (GLubyte*) v->getData(), // use temp data if this was created
        matrix, llf, urb, v->getDimensions(),
        format_, internalFormat_, dataType_, filter_
    );

    vTex->bind();
    // call glTexImage3D
    vTex->uploadTexture();
    // set texture wrap to clamp
    vTex->setWrapping(tgt::Texture::CLAMP);

    // delete temporary data that has eventually be created
    if (temp)
        delete[] temp;

    // prevent deleting twice
    vTex->setPixelData(0);

    // append to internal data structure
    textures_.push_back(vTex);
}

/*
 * depth sorting
 */

// needed for sorting in VolumeGL::getSortedTextures
struct VolumeTextureAndDistance {
    VolumeTexture* texture_; //< the texture
    float distance_;         //< and its associated distance

    VolumeTextureAndDistance() {} // needed by std::sort
    VolumeTextureAndDistance(VolumeTexture* texture, float distance)
      : texture_(texture),
        distance_(distance)
    {}

    // used by std::sort
    bool operator < (const VolumeTextureAndDistance v) const {
        // sort farest first so use '>' instead of '<'
        return distance_ > v.distance_;
    }
};

VolumeGL::SortedTextures VolumeGL::getSortedTextures(const mat4& m, vec3 eye /*= vec3::zero*/) {
    // we need a copy of eye here -> so call by value

    // speed up if textures_ holds only one VolumeTexture
    if (textures_.size() == 1)
        return SortedTextures(1, textures_[0]);

    /*
        transform eye appropriately instead of transforming each volume texture
    */
    mat4 inv;
    m.invert(inv);

    // speed up if eye == zero
    if (eye != vec3::zero)
        eye = vec3(inv.t03, inv.t13, inv.t23);
    else // -> eye != vec3::zero
        eye = inv * eye;

    // init data structure for sorting
    std::vector<VolumeTextureAndDistance> v( textures_.size() );

    for (size_t i = 0; i < v.size(); ++i) {
        VolumeTexture* tex = textures_[i];

        vec3 llb = tex->getLLF();
        vec3 urf = tex->getURB();
        std::swap(llb.z, urf.z); // convert to llb and urf

        /*
            calcuate the nearest point of the volume texture
            compared to the transformed eye
        */
        vec3 dist = eye;

        // clamp dist to the quads of the box
        for (size_t j = 0; j < 3; ++j) {
            if (dist[j] < llb[j])
                dist[j] = llb[j];
            if (dist[j] > urf[j])
                dist[j] = urf[j];
        }

        // it is sufficiant (and a bit faster) to just use the squared length
        v[i] = VolumeTextureAndDistance( tex, lengthSq(eye - dist) );
    }

    // sort
    std::sort( v.begin(), v.end() );

    // and build result
    SortedTextures result( textures_.size() );

    for (size_t i = 0; i < result.size(); ++i)
        result[i] = v[i].texture_;

    return result;
}

} // namespace voreen
