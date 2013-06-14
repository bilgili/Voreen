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

#include "voreen/core/datastructures/transfunc/preintegrationtable.h"
#include "math.h"
#include "tgt/texture.h"

namespace voreen {

using tgt::vec3;
using tgt::vec4;

PreIntegrationTable::PreIntegrationTable(TransFunc1DKeys* transFunc, size_t resolution, float d, bool useIntegral, bool computeOnGPU, tgt::Shader* program)
    : transFunc_(transFunc), resolution_(resolution), samplingStepSize_(d), useIntegral_(useIntegral), computeOnGPU_(computeOnGPU), table_(0), tex_(0), program_(program)
{

    //check values
    if (resolution <= 1)
        resolution_ = 256;
    if (d <= 0.f)
        samplingStepSize_ = 1.f;

    //initialize render target if necessary
    if (computeOnGPU_) {
        renderTarget_.initialize(GL_RGBA16, GL_DEPTH_COMPONENT24);
        renderTarget_.setDebugLabel("Pre-Integration Table");
        renderTarget_.resize(tgt::vec2(static_cast<float>(resolution_)));
    }
}

PreIntegrationTable::~PreIntegrationTable() {

    //deinitialize render target or delete texture
    if (computeOnGPU_)
        renderTarget_.deinitialize();
    else
        delete tex_;

    //delete renderTarget_;
}

void PreIntegrationTable::computeTable() const {
    if (!transFunc_)
        return;

    //buffer for TF values
    tgt::vec4* tfBuffer = new tgt::vec4[resolution_];

    int front_end = tgt::iround(transFunc_->getThresholds().x * resolution_);
    int back_start = tgt::iround(transFunc_->getThresholds().y * resolution_);
    //all values before front_end and after back_start are set to zero
    //all other values remain the same
    for (int i = 0; i < front_end; ++i)
        tfBuffer[i] = tgt::vec4(0.f);

    for (int i = front_end; i < back_start; ++i) {
        //fetch current value from TF
        float intensity = static_cast<float>(i) / static_cast<float>(resolution_ - 1);
        tgt::vec4 value = tgt::vec4(transFunc_->getMappingForValue(intensity)) / 255.f;
        tfBuffer[i] = value;
    }

    for (int i = back_start; i < static_cast<int>(resolution_); ++i)
        tfBuffer[i] = tgt::vec4(0.f);

    if(!useIntegral_) { // Correct (but slow) calculation of PI-table:
//do not use OpenMP for parallelizing computation of pre-integration table as it actually reduces performance :/
//#ifndef VRN_MODULE_OPENMP
        int lookupindex = 0;
//#endif
        for (int sb = 0; sb < static_cast<int>(resolution_); ++sb) {
//#ifdef VRN_MODULE_OPENMP
//#pragma omp parallel for schedule(dynamic) default(shared)
//#endif
            for (int sf = 0; sf < static_cast<int>(resolution_); ++sf) {
//#ifdef VRN_MODULE_OPENMP
//                int lookupindex = sb * resolution_ + sf;
//#endif
                if (sb != sf) {
                    float scale = 1.0f / fabs(static_cast<float>(sf - sb));

                    int incr = 1;
                    if(sf < sb)
                        incr = -1;

                    vec4 result = vec4(0.0f);
                    for(int s = sb; (incr == 1 ? s<=sf : s>=sf) && (result.a < 0.95); s += incr) {
                        //float nIndex = static_cast<float>(s) / static_cast<float>(resolution_ - 1);
                        //vec4 curCol = apply1DTF(nIndex);
                        vec4 curCol = tfBuffer[s];

                        if (curCol.a > 0.0f) {
                            // apply opacity correction to accomodate for variable sampling intervals
                            curCol.a = 1.f - pow(1.f - curCol.a, samplingStepSize_ * 200.0f * scale);

                            //actual compositing
                            vec3 result_rgb = vec3(result.elem) + (1.0f - result.a) * curCol.a * vec3(curCol.elem);
                            result.a = result.a + (1.0f - result.a) * curCol.a;

                            result.xyz() = result_rgb;
                        }
                    }
                    result.xyz() /= std::max(result.a, 0.001f);

                    result.a = 1.f - pow(1.f - result.a, 1.0f / (samplingStepSize_ * 200.0f));
                    table_[lookupindex] = tgt::clamp(result, 0.f, 1.f);
                } else {
                    //float nIndex = static_cast<float>(sf) / static_cast<float>(resolution_ - 1);
                    //table_[lookupindex] = tgt::clamp(apply1DTF(nIndex), 0.f, 1.f);
                    table_[lookupindex] = tgt::clamp(tfBuffer[sf], 0.f, 1.f);
                }
//#ifndef VRN_MODULE_OPENMP
                lookupindex++;
//#endif
            }
        }
    }
    else { //faster version using integral functions, see Real-Time Volume Graphics, p96
        //compute integral functions
        tgt::vec4* intFunc = new tgt::vec4[resolution_];

        tgt::vec4 accumResult(0.f);
        tgt::vec4 curColor;

        for (int i = 0; i < static_cast<int>(resolution_); ++i) {
            //fetch current value from TF
            //float nIndex = static_cast<float>(i) / static_cast<float>(resolution_ - 1);
            //vec4 curCol = apply1DTF(nIndex);
            vec4 curCol = tfBuffer[i];

            //calculate new integral function
            if (curCol.a > 0.0f) {
                //actual compositing
                accumResult.xyz() += curCol.xyz() * curCol.a;
                accumResult.a += curCol.a;
            }
            intFunc[i] = accumResult;
        }
        float factor;

//do not use OpenMP for parallelizing computation of pre-integration table as it actually reduces performance :/
//#ifndef VRN_MODULE_OPENMP
        int lookupindex = 0;
//#endif
        int endIndex = static_cast<int>(resolution_);

        // compute look-up table from integral functions
        for (int sb = 0; sb < endIndex; ++sb)
//#ifdef VRN_MODULE_OPENMP
//#pragma omp parallel for schedule(dynamic) default(shared)
//#endif
            for (int sf = 0; sf < endIndex; ++sf) {

                int smin = std::min(sb, sf);
                int smax = std::max(sb, sf);

                tgt::vec4 col;
                if (smax != smin) {
                    factor = samplingStepSize_ * 200.f / static_cast<float>(smax - smin);

                    col.xyz() = (intFunc[smax].xyz() - intFunc[smin].xyz()) * factor;
                    col.a = 1.f - exp(-(intFunc[smax].a - intFunc[smin].a) * factor);

                    col.xyz() /= std::max(col.a, 0.001f);
                    col.a = 1.f - pow(1.f - col.a, 1.0f / (samplingStepSize_ * 200.0f));
                } else {
                    //float nIndex = static_cast<float>(smin) / static_cast<float>(resolution_ - 1);
                    //col = apply1DTF(nIndex);
                    col = tfBuffer[smin];
                }
//#ifdef VRN_MODULE_OPENMP
//                int lookupindex = sb * resolution_ + sf;
//#endif
                table_[lookupindex] = tgt::clamp(col, 0.f, 1.f);
//#ifndef VRN_MODULE_OPENMP
                lookupindex++;
//#endif
            }
        delete[] intFunc;
    }

    delete[] tfBuffer;
}

void PreIntegrationTable::computeTableGPU() const {

    //render pre-integration texture into render target
    renderTarget_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //get texture of transfer function
    tgt::TextureUnit transferUnit;
    transferUnit.activate();

    transFunc_->getTexture()->bind();
    transFunc_->getTexture()->enable();

    //activte shader program
    program_->activate();

    program_->setUniform("samplingStepSize_", samplingStepSize_);

    //set transfer function texture
    program_->setUniform("tfTex_", transferUnit.getUnitNumber());

    bool oldIgnoreError = program_->getIgnoreUniformLocationError();
    program_->setIgnoreUniformLocationError(true);
    program_->setUniform("texParams_.dimensions_", tgt::vec2((float)resolution_));
    program_->setUniform("texParams_.dimensionsRCP_", tgt::vec2(1.f) / tgt::vec2((float)resolution_));
    program_->setUniform("texParams_.matrix_", tgt::mat4::identity);
    program_->setIgnoreUniformLocationError(oldIgnoreError);

    //render quad
    glDepthFunc(GL_ALWAYS);
    glBegin(GL_QUADS);
        glVertex2f(-1.f, -1.f);
        glVertex2f( 1.f, -1.f);
        glVertex2f( 1.f,  1.f);
        glVertex2f(-1.f,  1.f);
    glEnd();
    glDepthFunc(GL_LESS);

    //clean up
    transFunc_->getTexture()->disable();
    renderTarget_.deactivateTarget();
    program_->deactivate();

    tgt::TextureUnit::setZeroUnit();
    LGL_ERROR;

    //set output texture
    tex_ = renderTarget_.getColorTexture();

}



tgt::vec4 PreIntegrationTable::classify(float fs, float fe) const {

    //lazy computation
    if (!table_) {
        //create the table
        table_ = new tgt::vec4[resolution_ * resolution_];
        computeTable();
    }

    if (!table_)
        return tgt::vec4(0.f,0.f,0.f,0.f);
    else {
        //compute indices to table
        int i1 = tgt::iround(fs * (resolution_ - 1));
        int i2 = tgt::iround(fe * (resolution_ - 1));

        //check indices
        i1 = tgt::clamp(i1, 0, static_cast<int>(resolution_) - 1);
        i2 = tgt::clamp(i2, 0, static_cast<int>(resolution_) - 1);

        //table lookup
        return table_[i2 * resolution_ + i1];
    }
}

void PreIntegrationTable::createTexFromTable() const {
    delete tex_;

    //lazy computation
    if (!table_) {
        //create the table
        table_ = new tgt::vec4[resolution_ * resolution_];
        computeTable();
    }

    //tex_ = new tgt::Texture(reinterpret_cast<GLubyte*>(table_), tgt::ivec3(static_cast<int>(resolution_),static_cast<int>(resolution_),1),GL_RGBA, GL_RGBA, GL_FLOAT, tgt::Texture::LINEAR);
    tex_ = new tgt::Texture(reinterpret_cast<GLubyte*>(table_), tgt::ivec3(static_cast<int>(resolution_),static_cast<int>(resolution_),1),GL_RGBA, GL_RGBA32F, GL_FLOAT, tgt::Texture::LINEAR);
    tex_->setWrapping(tgt::Texture::CLAMP);
    LGL_ERROR;
    tex_->uploadTexture();
}

const tgt::Texture* PreIntegrationTable::getTexture() const {

    //check if texture has to be created and if computation should be made on the gpu
    if (!tex_) {
        if (computeOnGPU_ /*|| useIntegral_*/)
            computeTableGPU();
        else
            createTexFromTable();
    }

    tgtAssert(tex_, "No texture");

    return tex_;
}

float PreIntegrationTable::getSamplingStepSize() const {
    return samplingStepSize_;
}

bool PreIntegrationTable::usesIntegral() const {
    return useIntegral_;
}

size_t PreIntegrationTable::getDimension() const {
    return resolution_;
}

} //namespace
