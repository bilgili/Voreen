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

#ifndef VRN_SHLIGHTFUNC_H
#define VRN_SHLIGHTFUNC_H

#include "shrot.h"
#include "shcoeffcalc.h"

namespace voreen {

class SHLightFunc : public SHCoeffCalc {

public:

    SHLightFunc(int sampleNumSqrt, int numBands, std::string uniPrefix, bool unshadowed = false)
    : SHCoeffCalc(sampleNumSqrt, numBands)
    , rot_(SHRot(numBands))
    , uniPrefix_(uniPrefix)
    , unshadowed_(unshadowed)
    {}

    SHLightFunc(int numBands, std::string uniPrefix, bool unshadowed = false)
    : SHCoeffCalc(numBands)
    , rot_(SHRot(numBands))
    , uniPrefix_(uniPrefix)
    , unshadowed_(unshadowed)
    {}

    virtual ~SHLightFunc() {}

    const std::vector<float>& getProjectedLightCoeffs() const {
        return lightCoeffs_;
    }

    virtual void updateRotation(const tgt::mat4& diff) = 0;
    virtual void setRCShaderUniforms(tgt::Shader* rcProg) = 0;
    virtual void setRCShaderUniformsArray(tgt::Shader* rcProg) = 0;
    virtual std::string getShaderDefines() const = 0;
    virtual std::vector<float> loadCoeffsFromFile(const std::string& filename) = 0;

    const SHRot& getRot() const {
        return rot_;
    }

protected:

    std::vector<float> lightCoeffs_;
    SHRot rot_;
    std::string uniPrefix_;
    bool unshadowed_;
};

class SHScalarFunc : public SHLightFunc {
public:

    SHScalarFunc(int sampleNumSqrt, int numBands, std::string uniPrefix, bool unshadowed = false)
    : SHLightFunc(sampleNumSqrt, numBands, uniPrefix, unshadowed)
    {
        generateCoeffs();
    }

    SHScalarFunc(int numBands, std::string uniPrefix, const std::string& filename, bool unshadowed = false)
    : SHLightFunc(numBands, uniPrefix, unshadowed)
    {
        loadCoeffsFromFile(filename);
    }

    virtual ~SHScalarFunc() {}

    virtual void saveCoeffsToFile(const std::string& filename) const;

    virtual std::vector<float> loadCoeffsFromFile(const std::string& filename);

    void decompress(const std::string& filename);

    virtual std::string getShaderDefines() const {
        std::string header = std::string("");

        std::ostringstream s;
        s << "#define SH_NUMBANDS_" << uniPrefix_ << " " << numBands_ << "\n";
        header += s.str();
        return header;
    }

    virtual void setRCShaderUniforms(tgt::Shader* rcProg);
    virtual void updateRotation(const tgt::mat4& diff) {
        lightCoeffs_ = rot_.calcNewSHCoeffs(lightCoeffs_, diff);
    }

    virtual void setRCShaderUniformsArray(tgt::Shader* rcProg);

private:

    virtual void generateCoeffs();
    float lightFunc(float theta, float phi) const;
};

class SHLightProbe : public SHLightFunc {
public:

    SHLightProbe(int sampleNumSqrt, int numBands, std::string uniPrefix, const std::string& filename, bool unshadowed = false)
    : SHLightFunc(sampleNumSqrt, numBands, uniPrefix, unshadowed)
    {
        lightProbe_ = TexMgr.load(filename, tgt::Texture::LINEAR, false, true, false, false);
        generateCoeffs();
    }

    SHLightProbe(int numBands, std::string uniPrefix, const std::string& filename, bool unshadowed = false)
    : SHLightFunc(numBands, uniPrefix, unshadowed)
    {
        lightProbe_ = 0;
        loadCoeffsFromFile(filename);
    }

    virtual ~SHLightProbe() {
        delete lightProbe_;
    }

    void decompress(const std::string& filename);

    virtual void saveCoeffsToFile(const std::string& filename) const;

    virtual std::vector<float> loadCoeffsFromFile(const std::string& filename);

    virtual std::string getShaderDefines() const {
        std::string header = std::string("#define SH_LIGHTPROBE\n");
        std::ostringstream s;
        s << "#define SH_NUMBANDS_" << uniPrefix_ << " " << numBands_ << "\n";
        header += s.str();

        return header;
    }

    virtual void setRCShaderUniforms(tgt::Shader* rcProg);
    virtual void setRCShaderUniformsArray(tgt::Shader* /*rcProg*/) {} //TODO

    virtual void updateRotation(const tgt::mat4& diff) {
        std::vector<float> lightCoeffsR = rot_.calcNewSHCoeffs(std::vector<float>(lightCoeffs_.begin(), lightCoeffs_.begin() + numCoeffs_), diff);
        std::vector<float> lightCoeffsG = rot_.calcNewSHCoeffs(std::vector<float>(lightCoeffs_.begin() + numCoeffs_, lightCoeffs_.begin() + 2*numCoeffs_), diff);
        std::vector<float> lightCoeffsB = rot_.calcNewSHCoeffs(std::vector<float>(lightCoeffs_.begin() + 2*numCoeffs_, lightCoeffs_.end()), diff);
        for(size_t i = 0; i < numCoeffs_; i++) {
            lightCoeffs_.at(i               ) = lightCoeffsR.at(i);
            lightCoeffs_.at(i + numCoeffs_  ) = lightCoeffsG.at(i);
            lightCoeffs_.at(i + 2*numCoeffs_) = lightCoeffsB.at(i);
        }
    }

private:

    virtual void generateCoeffs();

    tgt::Texture* lightProbe_;
};

} //namespace

#endif // VRN_SHLIGHTFUNC_H

