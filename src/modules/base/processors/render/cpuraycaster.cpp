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

#include "voreen/modules/base/processors/render/cpuraycaster.h"

namespace voreen {

using tgt::vec3;
using tgt::vec4;

CPURaycaster::CPURaycaster()
  : VolumeRaycaster()
  , volumePort_(Port::INPORT, "volumehandle.volumehandle")
  , gradientVolumePort_(Port::INPORT, "volumehandle.gradientvolumehandle")
  , entryPort_(Port::INPORT, "image.entryports")
  , exitPort_(Port::INPORT, "image.exitports")
  , outport_(Port::OUTPORT, "image.output", true, INVALID_RESULT, GL_RGBA16F_ARB)
  , transferFunc_("transferFunction", "Transfer function")
{
    addPort(volumePort_);
    addPort(gradientVolumePort_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(outport_);

    addProperty(transferFunc_);

    sampling_count = 0;
    sampling_last_same = 0;
}

CPURaycaster::~CPURaycaster() {
}

std::string CPURaycaster::getProcessorInfo() const {
    return "Performs a simple raycasting on the CPU.";
}

Processor* CPURaycaster::create() const {
    return new CPURaycaster();
}

bool CPURaycaster::isReady() const {
    return (volumePort_.hasData() && entryPort_.isConnected() && exitPort_.isConnected());
}

float CPURaycaster::textureLookup3D(VolumeUInt8* vol, const vec3& p) {
    tgtAssert(vol, "No volume");
    tgt::ivec3 dim = vol->getDimensions() - 1;
    tgt::ivec3 pi;
    pi.x = static_cast<int>(p.x * static_cast<float>(dim.x));
    pi.y = static_cast<int>(p.y * static_cast<float>(dim.y));
    pi.z = static_cast<int>(p.z * static_cast<float>(dim.z));

    static tgt::ivec3 lastpi = tgt::ivec3(0);
    if (lastpi == pi)
        sampling_last_same++;
    else
        lastpi = pi;

    sampling_count++;

    return vol->voxel(pi) / 255.f;
}

vec4 CPURaycaster::apply1DTF(float intensity) {
    vec4 value = vec4(tf_tex->texel<tgt::col4>(size_t(intensity * (tf_tex->getWidth()-1)))) / 255.f;
    return value;
}

vec4 CPURaycaster::apply2DTF(float intensity, float gradientMagnitude) {
    vec4 value = vec4(tf_tex->texel<tgt::vec4>(size_t(intensity * (tf_tex->getWidth()-1)),
                                          size_t(gradientMagnitude * (tf_tex->getHeight()-1))));
    return value;
}

vec4 CPURaycaster::directRendering(const vec3& first, const vec3& last) {
    const float raycastingQualityFactorRCP_ = 1.f / 1.f;//raycastingQualityFactor_.getValue();

    vec4 result = vec4(0.0f);
    float depthT = -1.0f;
    bool finished = false;

    // calculate ray parameters
    float stepIncr = 0.005f * raycastingQualityFactorRCP_;
    float tend;
    float t = 0.0f;
    vec3 direction = last - first;
    // if direction is a nullvector the entry- and exitparams are the same
    // so special handling for tend is needed, otherwise we divide by zero
    // furthermore both for-loops will cause only 1 pass overall.
    // The test whether last and first are nullvectors is already done in main-function
    // but however the framerates are higher with this test.
    if (direction == vec3(0.0f) && last != vec3(0.0f) && first != vec3(0.0f))
        tend = stepIncr / 2.0f;
    else {
        tend = length(direction);
        direction = normalize(direction);
    }

    // 2 nested loops allow for more than 255 iterations
    // should not be slower than while (t < tend)
    vec3 lastsample = vec3(0.f);
    for (int loop0=0; !finished && loop0<255; ++loop0) {
        for (int loop1=0; !finished && loop1<255; ++loop1) {
            vec3 sample = first + t * direction;
            float intensity = textureLookup3D(vol8, sample);
            float gradientMagnitude = 0.f;
            if (gradientVolume_)
                gradientMagnitude = textureLookup3D(volumeGradientMagnitudes_, sample);

            // no shading is applied
            vec4 color;
            if (!intensityGradientTF_)
                color = apply1DTF(intensity);
            else
                color = apply2DTF(intensity, gradientMagnitude);

            // perform compositing
            if (color.a > 0.0f) {
                // multiply alpha by raycastingQualityFactorRCP_
                // to accommodate for variable slice spacing
                color.a *= raycastingQualityFactorRCP_;
                vec3 result_rgb = vec3(result.elem) + (1.0f - result.a) * color.a * vec3(color.elem);
                result.a = result.a + (1.0f - result.a) * color.a;

                result.r = result_rgb.r;
                result.g = result_rgb.g;
                result.b = result_rgb.b;
            }

            // save first hit ray parameter for depth value calculation
            if (depthT < 0.0f && result.a > 0.0f)
                depthT = t;

            // early ray termination
//             if (result.a >= 1.0f) {
//                 result.a = 1.0f;
//                 finished = true;
//             }

            t += stepIncr;
            finished = finished || (t > tend);
        }
    }
//    result = vec4(1.f,0.f,0.f,1.f);
    // calculate depth value from ray parameter
//     gl_FragDepth = 1.0;
//     if (depthT >= 0.0)
//         gl_FragDepth = calculateDepthValue(depthT / tend);

    return result;
}

void CPURaycaster::process() {

    if (!volumePort_.isReady())
        return;

    transferFunc_.setVolumeHandle(volumePort_.getData());
    LGL_ERROR;

    outport_.activateTarget();
    LGL_ERROR;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vol8 = dynamic_cast<VolumeUInt8*>(volumePort_.getData()->getVolume());
    if (vol8 == 0) {
        LWARNING("CPURaycaster::process: unsupported volume format");
        return;
    }
    gradientVolume_ = false;
    Volume3xUInt8* vol3x8 = 0;
    if (gradientVolumePort_.hasData()) {
        vol3x8 = dynamic_cast<Volume3xUInt8*>(gradientVolumePort_.getData()->getVolume());
        if (!vol3x8) {
            LWARNING("CPURaycaster::process: unsupported gradient volume format");
        }
        else {
        /*    volumeGradientMagnitudes_ = calcGradientMagnitudes<uint8_t>(vol3x8);
            if (volumeGradientMagnitudes_ == 0) {
                LERROR("CPURaycaster::process: Generating gradient magnitude volume from gradient volume failed");
                return;
            }
            else
                gradientVolume_ = true; */
        }
    }

    LGL_ERROR;

    intensityGradientTF_ = false;
    if (transferFunc_.get()) {
        TransFuncIntensity* tfi = dynamic_cast<TransFuncIntensity*>(transferFunc_.get());
        if (tfi == 0) {
            TransFuncIntensityGradient* tfig = dynamic_cast<TransFuncIntensityGradient*>(transferFunc_.get());
            if (tfig == 0) {
                LWARNING("CPURaycaster::process: unsupported tf");
                return;
            }
            else {
                intensityGradientTF_ = true;
                tf_tex = tfig->getTexture();
            }
        } else
            tf_tex = tfi->getTexture();
    }

    LGL_ERROR;

    if (intensityGradientTF_ && !gradientVolume_) {
        LWARNING("CPURaycaster::process: to use 2D tfs a 32 bit gradient volume is needed");
        return;
    }

    sampling_count = 0;
    sampling_last_same = 0;

    entryPort_.getColorTexture()->downloadTexture();
    exitPort_.getColorTexture()->downloadTexture();
    float* entry = (float*)entryPort_.getColorTexture()->getPixelData();
    float* exit = (float*)exitPort_.getColorTexture()->getPixelData();
    LGL_ERROR;

    float* output = new float[entryPort_.getSize().x * entryPort_.getSize().y * 4];

    for (int y=0; y < entryPort_.getSize().y; ++y) {
        for (int x=0; x < entryPort_.getSize().x; ++x) {
            vec4 gl_FragColor = vec4(0.f);
            int p = 4 * (y * entryPort_.getSize().x + x);

            vec3 frontPos = vec3(&entry[p]);
            vec3 backPos = vec3(&exit[p]);

            if ((frontPos == vec3(0.0)) && (backPos == vec3(0.0))) {
                //background needs no raycasting
                //discard;
            }
            else {
                //fragCoords are lying inside the boundingbox
                gl_FragColor = directRendering(frontPos, backPos);
            }

            for (int i=0; i < 4; ++i)
                output[p + i] = gl_FragColor[i];
            output[p + 3] = 1.f;
        }
    }

    glWindowPos2i(0, 0);
    glDrawPixels(outport_.getSize().x, outport_.getSize().y, GL_RGBA, GL_FLOAT, output);

    LGL_ERROR;

#ifdef VRN_DEBUG
    int volume_size = volumePort_.getData()->getVolume()->getNumVoxels();
    LDEBUG("CPURaycaster samples: " << sampling_count
           << " vs. " << volume_size << " ("
           << ((float)sampling_count / (float)volume_size * 100.f)
           << "%), unchanged: " << sampling_last_same << " ("
           << ((float)sampling_last_same / (float)sampling_count * 100.f) << "%)");
#endif

    delete[] output;
    entryPort_.getColorTexture()->destroy();
    exitPort_.getColorTexture()->destroy();
    LGL_ERROR;
}


} // namespace voreen
