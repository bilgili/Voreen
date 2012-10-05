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

#include "voreen/core/datastructures/transfunc/preintegrationtable.h"
#include "math.h"
#include "tgt/texture.h"

namespace voreen {

using tgt::vec3;
using tgt::vec4;

PreIntegrationTable::PreIntegrationTable(const TransFunc1DKeys* transFunc, size_t resolution, float d, bool useIntegral)
    : transFunc_(transFunc), resolution_(resolution), samplingStepSize_(d), useIntegral_(useIntegral), table_(0), tex_(0) {

    //check values
    if (resolution <= 1)
        resolution_ = 256;
    if (d <= 0.f)
        samplingStepSize_ = 1.f;

    //create the table
    table_ = new tgt::vec4[resolution_ * resolution_];

    //compute the values within the table
    computeTable();
}

PreIntegrationTable::~PreIntegrationTable() {
    //delete[] table_;
    delete tex_;
}

void PreIntegrationTable::computeTable() {
    if (!transFunc_)
        return;

    // retrieve tf texture
    const tgt::Texture* tfTexture = const_cast<TransFunc1DKeys*>(transFunc_)->getTexture();
    tfTexture->downloadTexture();

    if(!useIntegral_) { // Correct (but slow) calculation of PI-table:
        int lookupindex = 0;
        for (int sb = 0; sb < static_cast<int>(resolution_); ++sb) {
            for (int sf = 0; sf < static_cast<int>(resolution_); ++sf) {
                if (sb != sf) {
                    float scale = 1.0f / fabs(static_cast<float>(sf - sb));

                    int incr = 1;
                    if(sf < sb)
                        incr = -1;

                    vec4 result = vec4(0.0f);
                    for(int s = sb; (incr == 1 ? s<=sf : s>=sf); s += incr) {
                        float nIndex = static_cast<float>(s) / static_cast<float>(resolution_ - 1);
                        vec4 curCol = apply1DTF(tfTexture,nIndex);

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
                    float nIndex = static_cast<float>(sf) / static_cast<float>(resolution_ - 1);
                    table_[lookupindex] = tgt::clamp(apply1DTF(tfTexture,nIndex), 0.f, 1.f);
                }

                lookupindex++;
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
            float nIndex = static_cast<float>(i) / static_cast<float>(resolution_ - 1);
            vec4 curCol = apply1DTF(tfTexture,nIndex);
            //calculate new integral function
            if (curCol.a > 0.0f) {
                //actual compositing
                accumResult.xyz() += curCol.xyz() * curCol.a;
                accumResult.a += curCol.a;
            }
            intFunc[i] = accumResult;
        }
        float factor;
        int lookupindex = 0;

        // compute look-up table from integral functions
        for (int sb = 0; sb < static_cast<int>(resolution_); ++sb)
            for (int sf = 0; sf < static_cast<int>(resolution_); ++sf) {

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
                    float nIndex = static_cast<float>(smin) / static_cast<float>(resolution_ - 1);
                    col = apply1DTF(tfTexture,nIndex);
                }

                table_[lookupindex] = tgt::clamp(col, 0.f, 1.f);
                lookupindex++;
            }
        delete[] intFunc;
    }
}

tgt::vec4 PreIntegrationTable::classify(float fs, float fe) const {
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

void PreIntegrationTable::createTex() const {
    delete tex_;

    //tex_ = new tgt::Texture(reinterpret_cast<GLubyte*>(table_), tgt::ivec3(static_cast<int>(resolution_),static_cast<int>(resolution_),1),GL_RGBA, GL_RGBA, GL_FLOAT, tgt::Texture::LINEAR);
    tex_ = new tgt::Texture(reinterpret_cast<GLubyte*>(table_), tgt::ivec3(static_cast<int>(resolution_),static_cast<int>(resolution_),1),GL_RGBA, GL_RGBA32F, GL_FLOAT, tgt::Texture::LINEAR);
    tex_->setWrapping(tgt::Texture::CLAMP);
    LGL_ERROR;
    tex_->uploadTexture();
}

const tgt::Texture* PreIntegrationTable::getTexture() const {
    if (!tex_)
        createTex();
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

tgt::vec4 PreIntegrationTable::apply1DTF(const tgt::Texture* tfTexture, float intensity) {
    int widthMinusOne = tfTexture->getWidth()-1;
    tgt::vec4 value = tgt::vec4(tfTexture->texel<tgt::col4>(static_cast<size_t>(tgt::clamp(tgt::iround(intensity * widthMinusOne), 0, widthMinusOne)))) / 255.0f;

    return value;
}

} //namespace
