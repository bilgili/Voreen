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

#ifndef VRN_SHCOEFFCALC_H
#define VRN_SHCOEFFCALC_H

#include "tgt/vector.h"
#include "tgt/texture.h"
#include "tgt/shadermanager.h"
#include "tgt/texturemanager.h"

namespace voreen {

struct Sample {
    tgt::vec3 cart;
    tgt::vec3 sph;
    std::vector<float> coeff;
};

class SHCoeffCalc {

public:

    SHCoeffCalc(int sampleNumSqrt, int numBands);

    SHCoeffCalc(int numBands);

    virtual ~SHCoeffCalc() {
        samples_.clear();
        delete sampleTex_;
        delete coeffTex_;
    }

    const tgt::Texture* getSampleTex() const {
        return sampleTex_;
    }

    const tgt::Texture* getCoeffTex() const {
        return coeffTex_;
    }

    void saveSamplesToFile(tgt::Texture* tex, std::string filename) const;

    virtual std::string getShaderDefines() const = 0;

protected:

    int sampleNumSqrt_;
    std::vector<Sample> samples_;
    std::vector<float> facs_;

    tgt::Texture* sampleTex_;
    tgt::Texture* coeffTex_;

    std::string name_;
    std::string ending_;

    unsigned int numBands_;
    unsigned int numCoeffs_;

    float legPol(int l, int m, float x) const;
    float spherHarm(int l, int m, float theta, float phi) const;
    void initSamples();

    virtual void generateCoeffs() = 0;

    float getRandFloat() const {
        return (float)std::rand() / RAND_MAX;
    }
};

} //namespace
#endif // VRN_SHCOEFFCALC_H
