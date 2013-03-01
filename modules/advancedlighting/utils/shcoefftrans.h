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

#ifndef VRN_SHCOEFFTRANS_H
#define VRN_SHCOEFFTRANS_H

#include "shcoeffcalc.h"
#include "shlightfunc.h"

#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumegl.h"
#include "voreen/core/datastructures/volume/volume.h"

#include "tgt/framebufferobject.h"

namespace tgt {
    class TextureUnit;
}

namespace voreen {

/**
* This class represents a 3D SH-coefficient volume.
**/

class CoeffVolume {

public:
    /**
    * Constructor.
    */
    CoeffVolume(tgt::vec3 dimensions, std::string id = std::string());
    CoeffVolume(CoeffVolume* cv);
    virtual ~CoeffVolume();

    VolumeGL* getVolumeGL();
    VolumeRAM_4xUInt8* getVolume();
    //VolumeRAM_4xUInt16* getVolume();
    //VolumeRAM_4xFloat* getVolume();

    const std::string& getId() const {
        return id_;
    }

private:
    VolumeRAM_4xUInt8* volume_;
    //VolumeRAM_4xUInt16* volume_;
    //VolumeRAM_4xFloat* volume_;
    VolumeGL* volumeGL_;

    std::string id_;
};

class SHCoeffTrans : public SHCoeffCalc {

public:

    enum BleedingMode {
        SH_UNSHADOWED = 1,
        SH_SHADOWED = 2,
        SH_BLEEDING = 4,
        SH_REFLECT = 8,
        SH_SUBSURFACE = 16,
        SH_BLEED_SUB = SH_BLEEDING | SH_SUBSURFACE
    };

    SHCoeffTrans(int sampleNumSqrt, int numBands, TransFuncProperty* tfprop,
                 const VolumeBase* vol, BleedingMode bm, float sizefac, bool considerNormals, bool eri, float dataScale_ = 0.0);
    virtual ~SHCoeffTrans();

    void init();
    virtual void generateCoeffs();

    std::vector<CoeffVolume*> getIntensityVols() {
        std::vector<CoeffVolume*> ivls;
        ivls.push_back(transCoeffsI0_);
        ivls.push_back(transCoeffsI1_);
        ivls.push_back(transCoeffsI2_);
        ivls.push_back(transCoeffsI3_);
        return ivls;
    }

    virtual std::string getShaderDefines() const;
    virtual void setRCShaderUniforms(tgt::Shader* rcProg);

    BleedingMode getBleedingMode() {
        return bm_;
    }

    CoeffVolume* getTFVolume() {
        return tfVolume_;
    }

    SHLightFunc* getLightFunc() const {
        return lightFunc_;
    }

    void setLightFunc(SHLightFunc* lightFunc) {
        delete lightFunc_;
        lightFunc_ = lightFunc;
    }

private:

    void calcCoeffs();
    void generateTfVolume();
    void generateNormals();
    void executeSecondPass();
    void generateMipMapLevels(std::vector<CoeffVolume*> vols);

    void renderQuad();

    std::string generateHeader();
    void bindCoeffVolume(tgt::Shader* shader, CoeffVolume* cv, tgt::TextureUnit* texUnit);

    tgt::FramebufferObject* fbo_;
    tgt::FramebufferObject* fbo2_;
    tgt::FramebufferObject* fbo3_;

    CoeffVolume* transCoeffsI0_; CoeffVolume* transCoeffsI1_; CoeffVolume* transCoeffsI2_; CoeffVolume* transCoeffsI3_;
    CoeffVolume* transCoeffsR0_; CoeffVolume* transCoeffsR1_; CoeffVolume* transCoeffsR2_; CoeffVolume* transCoeffsR3_;
    CoeffVolume* transCoeffsG0_; CoeffVolume* transCoeffsG1_; CoeffVolume* transCoeffsG2_; CoeffVolume* transCoeffsG3_;
    CoeffVolume* transCoeffsB0_; CoeffVolume* transCoeffsB1_; CoeffVolume* transCoeffsB2_; CoeffVolume* transCoeffsB3_;
    CoeffVolume* tfVolume_;
    CoeffVolume* bcVolume_;
    CoeffVolume* normVolume_;

    CoeffVolume* sssCoeffs0_; CoeffVolume* sssCoeffs1_; CoeffVolume* sssCoeffs2_; CoeffVolume* sssCoeffs3_;
    CoeffVolume* sssCoeffs4_; CoeffVolume* sssCoeffs5_; CoeffVolume* sssCoeffs6_;

    TransFuncProperty* tfProp_;  ///< the property that controls the transfer-function
    const VolumeBase* volume_;
    BleedingMode bm_;
    float sizefac_;
    bool considerNormals_;
    bool eri_;
    float dataScale_;

    tgt::Shader* coeffPrg_;
    tgt::Shader* coeffPrg2_;
    tgt::Shader* tfPrg_;
    tgt::Shader* normPrg_;
    tgt::Shader* sssPrg_;

    SHLightFunc* lightFunc_;

    static const std::string loggerCat_; ///< category used in logging
};

} // namespace voreen
#endif // VRN_SHCOEFFTRANS_H
