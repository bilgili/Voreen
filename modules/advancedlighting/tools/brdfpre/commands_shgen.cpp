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

#include "commands_shgen.h"
#include <IL/il.h>
#include <IL/ilu.h>
#include "tgt/texturereaderdevil.h"

namespace voreen {

CommandSHGenBrdfMerl::CommandSHGenBrdfMerl() : Command("--shcoeffmerl", "",  "Calculate Spherical-Harmonic-BRDF-Coefficients from a Merl Dataset. Writes a tga-texture to disk.",
                                       "<BRDFNAME.binary NUMBANDS NUMSAMPLESSQRT OUTPUT.tga>", 4, false) {
    loggerCat_ += "." + name_;
    brdf_ = 0;
}

bool CommandSHGenBrdfMerl::execute(const std::vector<std::string>& parameters) {
    unsigned int numBands = cast<int>(parameters[1]);
    unsigned int sampleNumSqrt = cast<unsigned int>(parameters[2]);
    brdf_ = new SHBrdfMerl(sampleNumSqrt, numBands, parameters[0]);

     //16-Bit Textures disabled for now, since the TexMgr can't read that format
    ilInit();
    ilEnable(IL_FILE_OVERWRITE);

    ILuint imageID = ilGenImage() ;
    ilBindImage(imageID) ;

    //ilTexImage(6*sampleNumSqrt, 2*sampleNumSqrt, 1, 4, IL_RGBA,
               //IL_UNSIGNED_SHORT, (unsigned short*)(brdf_->getBrdfTex()->getPixelData()));
    ilTexImage(3*(numBands/2)*sampleNumSqrt, (numBands/2)*sampleNumSqrt, 1, 4, IL_RGBA,
               IL_UNSIGNED_BYTE, brdf_->getBrdfTex()->getPixelData());

    //ilTexImage(sampleNumSqrt, sampleNumSqrt, 1, 4, IL_RGBA,
               //IL_UNSIGNED_SHORT, const_cast<GLubyte*>(brdf_->getSampleTex()->getPixelData()));
    //ilTexImage(2*sampleNumSqrt, 2*sampleNumSqrt, 1, 4, IL_RGBA,
               //IL_UNSIGNED_SHORT, const_cast<GLubyte*>(brdf_->getCoeffTex()->getPixelData()));
    //ilTexImage(brdf_->getCoeffTex()->getWidth(), brdf_->getCoeffTex()->getHeight(), 1, 4, IL_RGBA,
               //IL_UNSIGNED_SHORT, const_cast<GLubyte*>(brdf_->getCoeffTex()->getPixelData()));

    // actually save out as png
    //ilSave(IL_PNG, parameters[3].c_str()) ;
    ilSave(IL_TGA, parameters[3].c_str()) ;

    return true;
}

bool CommandSHGenBrdfMerl::checkParameters(const std::vector<std::string>& parameters) {
    return (parameters.size() == 4);
}

//-----------------------------------------------------

CommandSHGenBrdfPhong::CommandSHGenBrdfPhong() : Command("--shcoeffphong", "",  "Calculate Spherical-Harmonic-Phong-Coefficients. Writes a tga-texture to disk.",
                                       "<NUMBANDS NUMSAMPLESSQRT OUTPUT.tga>", 3, false) {
    loggerCat_ += "." + name_;
    brdf_ = 0;
}

bool CommandSHGenBrdfPhong::execute(const std::vector<std::string>& parameters) {
    unsigned int numBands = cast<unsigned int>(parameters[0]);
    unsigned int sampleNumSqrt = cast<unsigned int>(parameters[1]);
    brdf_ = new SHBrdfPhong(sampleNumSqrt, numBands);

     //16-Bit Textures disabled for now, since the TexMgr can't read that format
    ilInit();
    ilEnable(IL_FILE_OVERWRITE);

    ILuint imageID = ilGenImage() ;
    ilBindImage(imageID) ;

    //ilTexImage(6*sampleNumSqrt, 2*sampleNumSqrt, 1, 4, IL_RGBA,
               //IL_UNSIGNED_SHORT, (unsigned short*)(brdf_->getBrdfTex()->getPixelData()));
    ilTexImage(6*sampleNumSqrt, 2*sampleNumSqrt, 1, 4, IL_RGBA,
               IL_UNSIGNED_BYTE, brdf_->getBrdfTex()->getPixelData());

    //ilTexImage(sampleNumSqrt, sampleNumSqrt, 1, 4, IL_RGBA,
               //IL_UNSIGNED_SHORT, const_cast<GLubyte*>(brdf_->getSampleTex()->getPixelData()));
    //ilTexImage(2*sampleNumSqrt, 2*sampleNumSqrt, 1, 4, IL_RGBA,
               //IL_UNSIGNED_SHORT, const_cast<GLubyte*>(brdf_->getCoeffTex()->getPixelData()));
    //ilTexImage(brdf_->getCoeffTex()->getWidth(), brdf_->getCoeffTex()->getHeight(), 1, 4, IL_RGBA,
               //IL_UNSIGNED_SHORT, const_cast<GLubyte*>(brdf_->getCoeffTex()->getPixelData()));

    // actually save out as png
    //ilSave(IL_PNG, parameters[2].c_str()) ;
    ilSave(IL_TGA, parameters[2].c_str()) ;

    return true;
}

bool CommandSHGenBrdfPhong::checkParameters(const std::vector<std::string>& parameters) {
    return (parameters.size() == 3);
}
//-------------------------------------------------------------------------------

CommandSHGenLightProbe::CommandSHGenLightProbe() : Command("--shlightprobe", "",  "Calculate Spherical-Harmonic-Lightprobe-Coefficients. Writes an xml to disk.",
                                       "<LIGHTPROBE.png NUMBANDS NUMSAMPLESSQRT OUTPUT.lpc>", 4, false) {
    loggerCat_ += "." + name_;
    lp_ = 0;
}

bool CommandSHGenLightProbe::execute(const std::vector<std::string>& parameters) {
    tgt::Singleton<tgt::TextureManager>::init();
    TexMgr.registerReader(new tgt::TextureReaderDevil());
    unsigned int numBands = cast<unsigned int>(parameters[1]);
    unsigned int sampleNumSqrt = cast<unsigned int>(parameters[2]);
    lp_ = new SHLightProbe(sampleNumSqrt, numBands, "", parameters[0]);
    lp_->saveCoeffsToFile(parameters[3]);

    return true;
}

bool CommandSHGenLightProbe::checkParameters(const std::vector<std::string>& parameters) {
    return (parameters.size() == 4);
}

//-------------------------------------------------------------------------------

CommandSHGenLightScalar::CommandSHGenLightScalar() : Command("--shlightscalar", "",  "Calculate SH-Scalar-Lightfunc-Coefficients. Writes an xml to disk.",
                                       "<NUMBANDS NUMSAMPLESSQRT OUTPUT.lsc>", 3, false) {
    loggerCat_ += "." + name_;
    ls_ = 0;
}

bool CommandSHGenLightScalar::execute(const std::vector<std::string>& parameters) {
    unsigned int numBands = cast<unsigned int>(parameters[0]);
    unsigned int sampleNumSqrt = cast<unsigned int>(parameters[1]);
    ls_ = new SHScalarFunc(sampleNumSqrt, numBands, "");
    ls_->saveCoeffsToFile(parameters[2]);

    return true;
}

bool CommandSHGenLightScalar::checkParameters(const std::vector<std::string>& parameters) {
    return (parameters.size() == 3);
}

}   //namespace voreen
