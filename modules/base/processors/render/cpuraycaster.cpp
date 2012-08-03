/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "cpuraycaster.h"
#include "voreen/core/datastructures/transfunc/transfunc2dprimitives.h"

#ifdef VRN_MODULE_OPENMP
#include "omp.h"
#endif

namespace voreen {

using tgt::vec3;
using tgt::vec4;
using tgt::svec3;

const float SAMPLING_BASE_INTERVAL_RCP = 200.0;

CPURaycaster::CPURaycaster()
  : VolumeRaycaster()
  , volumePort_(Port::INPORT, "volumehandle.volumehandle")
  , gradientVolumePort_(Port::INPORT, "volumehandle.gradientvolumehandle")
  , entryPort_(Port::INPORT, "image.entryports")
  , exitPort_(Port::INPORT, "image.exitports")
  , outport_(Port::OUTPORT, "image.output", true, INVALID_RESULT, GL_RGBA16F_ARB)
  , transferFunc_("transferFunction", "Transfer function")
  , texFilterMode_("textureFilterMode_", "Texture Filtering")
{
    addPort(volumePort_);
    addPort(gradientVolumePort_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(outport_);

    addProperty(transferFunc_);

    // volume texture filtering
    texFilterMode_.addOption("nearest", "Nearest",  GL_NEAREST);
    texFilterMode_.addOption("linear",  "Linear",   GL_LINEAR);
    texFilterMode_.selectByKey("linear");
    addProperty(texFilterMode_);
}

Processor* CPURaycaster::create() const {
    return new CPURaycaster();
}

bool CPURaycaster::isReady() const {
    return (volumePort_.hasData() && entryPort_.isConnected() && exitPort_.isConnected());
}

void CPURaycaster::process() {

    tgtAssert(volumePort_.getData()->getRepresentation<VolumeRAM>(), "no input volume");

    transferFunc_.setVolumeHandle(volumePort_.getData());
    LGL_ERROR;

    // determine TF type
    intensityGradientTF_ = false;
    if (transferFunc_.get()) {
        TransFunc1DKeys* tfi = dynamic_cast<TransFunc1DKeys*>(transferFunc_.get());
        if (tfi == 0) {
            TransFunc2DPrimitives* tfig = dynamic_cast<TransFunc2DPrimitives*>(transferFunc_.get());
            if (tfig == 0) {
                LWARNING("CPURaycaster::process: unsupported tf");
                return;
            }
            else {
                intensityGradientTF_ = true;
            }
        }
    }

    // if 2D TF: check whether gradient volume is supplied
    if (intensityGradientTF_ ) {
        if (!gradientVolumePort_.hasData() || gradientVolumePort_.getData()->getRepresentation<VolumeRAM>()->getNumChannels() < 3) {
            LERROR("To use 2D tfs a RGB or RGBA gradient volume is needed");
            return;
        }
        if (gradientVolumePort_.getData()->getRepresentation<VolumeRAM>()->getDimensions() != volumePort_.getData()->getRepresentation<VolumeRAM>()->getDimensions()) {
            LERROR("Gradient volume dimensions differ from intensity volume dimensions");
            return;
        }
    }

    // activate outport
    outport_.activateTarget();
    outport_.clearTarget();
    LGL_ERROR;

    // create output buffer
    tgt::vec4* output = new tgt::vec4[entryPort_.getSize().x * entryPort_.getSize().y];

    // download entry/exit point textures
    tgt::vec4* entryBuffer = reinterpret_cast<tgt::vec4*>(
        entryPort_.getColorTexture()->downloadTextureToBuffer(GL_RGBA, GL_FLOAT));
    tgt::vec4* exitBuffer = reinterpret_cast<tgt::vec4*>(
        exitPort_.getColorTexture()->downloadTextureToBuffer(GL_RGBA, GL_FLOAT));
    LGL_ERROR;

    // retrieve tf texture
    tgt::Texture* tfTexture = transferFunc_.get()->getTexture();
    tfTexture->downloadTexture();

    // iterate over viewport and perform ray casting for each fragment
    for (int y=0; y < entryPort_.getSize().y; ++y) {
#ifdef VRN_MODULE_OPENMP
#pragma omp parallel for schedule(dynamic) shared(output,entryBuffer,exitBuffer,tfTexture,y)
#endif
        for (int x=0; x < entryPort_.getSize().x; ++x) {
            vec4 gl_FragColor = vec4(0.f);
            int p = (y * entryPort_.getSize().x + x);

            vec4 frontPos = entryBuffer[p];
            vec4 backPos = exitBuffer[p];

            if ((frontPos == vec4(0.0)) && (backPos == vec4(0.0))) {
                //background needs no raycasting
            }
            else {
                //fragCoords are lying inside the boundingbox
                gl_FragColor = directRendering(frontPos.xyz(), backPos.xyz(), tfTexture);
            }

            output[p] = gl_FragColor;
        }
    }
    delete[] entryBuffer;
    delete[] exitBuffer;

    // draw output buffer to outport
    glWindowPos2i(0, 0);
    glDrawPixels(outport_.getSize().x, outport_.getSize().y, GL_RGBA, GL_FLOAT, output);
    LGL_ERROR;

    delete[] output;
    outport_.deactivateTarget();
    LGL_ERROR;
}

vec4 CPURaycaster::directRendering(const vec3& first, const vec3& last, tgt::Texture* tfTexture) {

    tgtAssert(transferFunc_.get(), "no transfunc");

    // retrieve intensity volume
    const VolumeRAM* volume = volumePort_.getData()->getRepresentation<VolumeRAM>();
    tgtAssert(volume, "no input volume");
    tgt::svec3 volDim = volume->getDimensions();
    tgt::vec3 volDimF = tgt::vec3((volume->getDimensions() - svec3(1)));

    // retrieve gradient volume, if 2D TF is to be applied
    const VolumeRAM* volumeGradient = 0;
    if (intensityGradientTF_) {
        volumeGradient = gradientVolumePort_.getData()->getRepresentation<VolumeRAM>();
        tgtAssert(volumeGradient, "no gradient volume");
        tgtAssert(volumeGradient->getNumChannels() >= 3, "gradient volume has less than three channels");
        tgtAssert(volumeGradient->getDimensions() == volDim, "dimensions mismatch");
    }

    // use dimension with the highest resolution for calculating the sampling step size
    float samplingStepSize = 1.f / (tgt::max(volDim) * samplingRate_.get());

    // calculate ray parameters
    float tend;
    float t = 0.0f;
    vec3 direction = last - first;
    // if direction is a nullvector the entry- and exitparams are the same
    // so special handling for tend is needed, otherwise we divide by zero
    // furthermore both for-loops will cause only 1 pass overall.
    // The test whether last and first are nullvectors is already done in main-function
    // but however the framerates are higher with this test.
    if (direction == vec3(0.0f) && last != vec3(0.0f) && first != vec3(0.0f))
        tend = samplingStepSize / 2.0f;
    else {
        tend = length(direction);
        direction = normalize(direction);
    }

    // ray-casting loop
    vec4 result = vec4(0.0f);
    float depthT = -1.0f;
    bool finished = false;
    for (int loop=0; !finished && loop<255*255; ++loop) {
        vec3 sample = first + t * direction;
        float intensity = 0.f;
        if (texFilterMode_.getValue() == GL_NEAREST)
            intensity = volume->getVoxelNormalized(tgt::iround(sample*volDimF));
        else if (texFilterMode_.getValue() == GL_LINEAR)
            intensity = volume->getVoxelNormalizedLinear(sample*volDimF);
        else
            LERROR("Unknown texture filter mode");

        // no shading is applied
        vec4 color;
        if (!intensityGradientTF_)
            color = apply1DTF(tfTexture, intensity);
        else {
            tgt::vec3 grad;
            if (texFilterMode_.getValue() == GL_NEAREST) {
                tgt::ivec3 iSample = tgt::iround(sample*volDimF);
                grad.x = volumeGradient->getVoxelNormalized(iSample, 0);
                grad.y = volumeGradient->getVoxelNormalized(iSample, 1);
                grad.z = volumeGradient->getVoxelNormalized(iSample, 2);
            }
            else if (texFilterMode_.getValue() == GL_LINEAR) {
                grad.x = volumeGradient->getVoxelNormalizedLinear(sample*volDimF, 0);
                grad.y = volumeGradient->getVoxelNormalizedLinear(sample*volDimF, 1);
                grad.z = volumeGradient->getVoxelNormalizedLinear(sample*volDimF, 2);
            }
            else
                LERROR("Unknown texture filter mode");

            float gradMag = tgt::clamp(tgt::length(grad), 0.f, 1.f);
            color = apply2DTF(tfTexture, intensity, gradMag);
        }

        // perform compositing
        if (color.a > 0.f) {

            // apply opacity correction to accomodate for variable sampling intervals
            color.a = 1.f - pow(1.f - color.a, samplingStepSize * SAMPLING_BASE_INTERVAL_RCP);

            /*// multiply alpha by samplingStepSize
            // to accommodate for variable sampling rate
            color.a *= samplingStepSize*200.f;*/

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
        if (result.a >= 0.95f) {
             result.a = 1.0f;
             finished = true;
        }

        t += samplingStepSize;
        finished = finished || (t > tend);

    } // ray-casting loop

    // calculate depth value from ray parameter (todo)
    // gl_FragDepth = 1.0;
    // if (depthT >= 0.0)
    //  gl_FragDepth = calculateDepthValue(depthT / tend);

    return result;
}

vec4 CPURaycaster::apply1DTF(tgt::Texture* tfTexture, float intensity) {
    vec4 value = vec4(tfTexture->texel<tgt::col4>(size_t(intensity * (tfTexture->getWidth()-1)))) / 255.f;
    return value;
}

vec4 CPURaycaster::apply2DTF(tgt::Texture* tfTexture, float intensity, float gradientMagnitude) {
    vec4 value = vec4(tfTexture->texel<tgt::vec4>(size_t(intensity * (tfTexture->getWidth()-1)),
        size_t(gradientMagnitude * (tfTexture->getHeight()-1))));
    return value;
}

} // namespace voreen
