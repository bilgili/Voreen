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

#include "shlightfunc.h"
#include "tinyxml/tinyxml.h"

namespace voreen {

void SHScalarFunc::generateCoeffs() {

    lightCoeffs_ = std::vector<float>(numCoeffs_, 0.f);
    for(int i=0; i < sampleNumSqrt_ * sampleNumSqrt_; ++i) {
        float theta = samples_[i].sph.x;
        float phi   = samples_[i].sph.y;
        float func  = lightFunc(theta,phi);

        for(unsigned int n=0; n < numCoeffs_; ++n)
            lightCoeffs_.at(n) +=  func * samples_[i].coeff.at(n);
    }

    // divide the result by weight and number of samples
    float factor = 4.f*tgt::PIf;
    factor /= (float)(sampleNumSqrt_*sampleNumSqrt_);

    for(size_t i = 0; i < lightCoeffs_.size(); i++)
        lightCoeffs_.at(i) *= factor;

    //std::ostringstream os;
    //os << "/home/meister/dazeug/DA/figures/lightfunc" << numBands_ << ".tga";
    //decompress(os.str());
}

// TODO: Find a beautiful solution for this
float SHScalarFunc::lightFunc(float theta, float phi) const {

    //return 1.f;
    //return (theta > tgt::PIf*0.5) ? 0.f : 1.f;

    // 2 Light-"blobs" in the shape of an 8
    //return std::max(0.f, 5.f* cosf(theta) - 4.f) + std::max(0.f, -4.f*sinf(theta - tgt::PIf)*cosf(phi - 2.5f) - 3.f);

    //// Overcast day
    //return ((theta > tgt::PIf*0.5) ? 0.f : 1.f) * ((1.f + 2.f*sinf(theta)) / 10.f);

    // Sunny day
    if(theta > tgt::PIf*0.5f)
        return 0.f;

    //float S = tgt::PIf / 4.0f;
    float S = 0.f;
    float gamma = acosf(dot(tgt::vec3(sinf(S), 0.f, cosf(S)), tgt::vec3(cosf(phi)*sinf(theta), sinf(phi)*sinf(theta), cosf(theta))));

    float tmp = (0.91f+10.f*std::exp(-3.f*gamma)+0.45f*cosf(gamma)*cosf(gamma))*(1.f-std::exp(-0.32f / cosf(theta)));
    tmp /= (0.91f+10.f*std::exp(-3.f*S)+0.45f*cosf(S)*cosf(S))*(1.f-std::exp(-0.32f));

    return tmp;

    // Partly cloudy
    //if(theta > tgt::PIf*0.5)
        //return 0.f;

    //float S = tgt::PIf / 4.f;
    //float gamma = acosf(dot(tgt::vec3(sinf(S), 0.f, cosf(S)), tgt::vec3(cosf(phi)*sinf(theta), sinf(phi)*sinf(theta), cosf(theta))));

    //float tmp = (0.526f+5.f*std::exp(-1.5f*gamma))*(1.f-std::exp(-0.8f / cosf(theta)));
    //tmp /= (0.526f+5.f*std::exp(-1.5f*S))*(1.f-std::exp(-0.8f));

    //return 0.2f*tmp;

    //if(theta > tgt::PIf*0.5*0.2)
        //return 0.f;
    //else
        //return 1.f;
}

void SHScalarFunc::decompress(const std::string& filename) {
    tgt::Texture* tex = new tgt::Texture(tgt::ivec3(sampleNumSqrt_, sampleNumSqrt_, 1),
                                          GL_RGB, GL_RGB, GL_UNSIGNED_BYTE,
                                          tgt::Texture::LINEAR);

    //for(int i=0; i < sampleNumSqrt_ * sampleNumSqrt_; ++i) {
        //tex->texel<tgt::col3>(i / sampleNumSqrt_, i % sampleNumSqrt_) = tgt::col3(255, 0, 0);
    //}

    for(int i=0; i < sampleNumSqrt_ * sampleNumSqrt_; ++i) {
        tgt::ivec2 texCoord = tgt::ivec2(i / sampleNumSqrt_, i % sampleNumSqrt_);
        tgt::vec2 scaledCoord = tgt::vec2(texCoord) / (float)sampleNumSqrt_;

        float theta = 2.f * acosf(sqrtf(1.f - scaledCoord.x));
        float phi = 2.f * tgt::PIf * scaledCoord.y;
        float color = 0.0;

        for(int l = 0; l < (int)numBands_; l++) {
            for(int m = -l; m <= l; m++) {
                int index = l*(l+1)+m;
                float val = spherHarm(l,m,theta,phi);
                color += lightCoeffs_.at(index) * val;
            }
        }
        tex->texel<tgt::col3>(texCoord) = tgt::col3(tgt::vec3(color) * tgt::vec3(255.f));
    }
    saveSamplesToFile(tex, filename);
}

std::vector<float> SHScalarFunc::loadCoeffsFromFile(const std::string& filename) {

    TiXmlDocument doc(filename.c_str());
    doc.LoadFile();

    TiXmlHandle hDoc(&doc);
    TiXmlHandle hRoot = TiXmlHandle(hDoc.FirstChildElement().Element());
    TiXmlElement* pCoeffsNode = hRoot.FirstChild().Element();

    lightCoeffs_.clear();
    lightCoeffs_ = std::vector<float>(numCoeffs_);

    for(int l = 0; l < (int)numBands_; ++l) {
        for(int m = -l; m <= l; ++m) {
            std::ostringstream tag;
            tag << "c" << l << m;
            float c = 0.f;
            pCoeffsNode->QueryFloatAttribute(tag.str().c_str(), &c);
            lightCoeffs_.at(l*(l+1) + m) = c;
        }
    }

    return lightCoeffs_;
}

void SHScalarFunc::saveCoeffsToFile(const std::string& filename) const {
    TiXmlDocument doc;
    TiXmlDeclaration* declNode = new TiXmlDeclaration("1.0", "ISO-8859-1", "");
    doc.LinkEndChild(declNode);
    TiXmlElement* root = new TiXmlElement("SH");
    doc.LinkEndChild(root);

    TiXmlElement* coeffs = new TiXmlElement("SHCoeffs");
    for(int l = 0; l < (int)numBands_; ++l) {
        for(int m = -l; m <= l; ++m) {
            int index = l*(l+1) + m;
            std::ostringstream tag;
            tag << "c" << l << m;
            coeffs->SetDoubleAttribute(tag.str().c_str(), (double)lightCoeffs_.at(index));
        }
    }
    root->LinkEndChild(coeffs);
    doc.SaveFile(filename);
}

void SHScalarFunc::setRCShaderUniforms(tgt::Shader* rcProg) {

    if(unshadowed_) {
        float c1 = 0.429043;
        float c2 = 0.511664;
        float c3 = 0.743125;
        float c4 = 0.886227;
        float c5 = 0.247708;
        tgt::mat4 lightMatR;

        lightMatR = tgt::mat4(
            tgt::vec4(c1*lightCoeffs_.at(8), c1*lightCoeffs_.at(4), c1*lightCoeffs_.at(7), c2*lightCoeffs_.at(3)),
            tgt::vec4(c1*lightCoeffs_.at(4), -c1*lightCoeffs_.at(8), c1*lightCoeffs_.at(5), c2*lightCoeffs_.at(1)),
            tgt::vec4(c1*lightCoeffs_.at(7), c1*lightCoeffs_.at(5), c3*lightCoeffs_.at(6), c2*lightCoeffs_.at(2)),
            tgt::vec4(c2*lightCoeffs_.at(3), c2*lightCoeffs_.at(1), c2*lightCoeffs_.at(2), c4*lightCoeffs_.at(0)-c5*lightCoeffs_.at(6))
        );

        std::ostringstream oR;
        oR << "lightMatsR" << uniPrefix_ << "_[0]";
        rcProg->setUniform(oR.str(), lightMatR);
    } else {

        float* coeffArray = &(*lightCoeffs_.begin());
        std::vector<tgt::mat4> matsR = std::vector<tgt::mat4>(numCoeffs_ / 16);

        for(size_t i = 0; i < matsR.size(); i++) {
            int indexR = static_cast<int>(i)*16;

            matsR.at(i) = tgt::mat4(
                tgt::vec4(coeffArray+indexR), tgt::vec4(coeffArray+indexR+4), tgt::vec4(coeffArray+indexR+8), tgt::vec4(coeffArray+indexR+12)
            );

            std::ostringstream oR;
            LGL_ERROR;
            oR << "lightMatsR" << uniPrefix_ << "_[" << i << "]";
            rcProg->setUniform(oR.str(), matsR.at(i), true);
        }
    }
}

void SHScalarFunc::setRCShaderUniformsArray(tgt::Shader* rcProg) {
    float* coeffArray = &(*lightCoeffs_.begin());
    std::ostringstream oR;
    oR << "lightArrayR" << uniPrefix_ << "_";
    rcProg->setUniform(oR.str(), coeffArray, numCoeffs_);
}

//--------------------------------------------------------------------------------------

void SHLightProbe::generateCoeffs() {
    lightCoeffs_ = std::vector<float>(3*numCoeffs_, 0.f);
    tgt::vec2 probeSize = tgt::vec2(lightProbe_->getDimensions().xy());

    for(int i=0; i < sampleNumSqrt_ * sampleNumSqrt_; ++i) {
        tgt::vec3 dir = samples_[i].cart;
        tgt::vec2 scaledCoord = (1.f / tgt::PIf) * acosf(dir.z) * normalize(dir.xy());
        scaledCoord = scaledCoord*0.5f + 0.5f;
        scaledCoord.y = 1.f - scaledCoord.y;
        tgt::ivec2 texCoord = tgt::ivec2(scaledCoord * tgt::vec2(probeSize));
        tgt::vec3 color = tgt::vec3(lightProbe_->texel<tgt::col3>(texCoord)) / tgt::vec3(255.f);
        //tgt::vec4 color = tgt::vec4(lightProbe_->texel<tgt::col4>(texCoord)) / tgt::vec4(255.f);

        for(unsigned int n = 0; n < numCoeffs_; ++n) {
            lightCoeffs_.at(n               ) +=  color.x * samples_[i].coeff.at(n);
            lightCoeffs_.at(n + numCoeffs_  ) +=  color.y * samples_[i].coeff.at(n);
            lightCoeffs_.at(n + 2*numCoeffs_) +=  color.z * samples_[i].coeff.at(n);
        }
    }

    // divide the result by weight and number of samples
    float factor = 4.f*tgt::PIf;
    factor /= (float)(sampleNumSqrt_*sampleNumSqrt_);

    for(size_t i = 0; i < lightCoeffs_.size(); i++)
        lightCoeffs_.at(i) *= factor;

    std::ostringstream os;
    os << "/home/meister/dazeug/DA/figures/grace" << numBands_ << ".tga";
    decompress(os.str());
}

void SHLightProbe::decompress(const std::string& filename) {
    tgt::Texture* tex = new tgt::Texture(tgt::ivec3(sampleNumSqrt_, sampleNumSqrt_, 1),
                                          GL_RGB, GL_RGB, GL_UNSIGNED_BYTE,
                                          tgt::Texture::LINEAR);

    for(int i=0; i < sampleNumSqrt_ * sampleNumSqrt_; ++i)
        tex->texel<tgt::col3>(i % sampleNumSqrt_, i / sampleNumSqrt_) = tgt::col3(uint8_t(0));

    //float oneoverN = 1.f / (float)sampleNumSqrt_;
    for(int i=0; i < sampleNumSqrt_ * sampleNumSqrt_; ++i) {

        tgt::vec3 dir = samples_[i].cart;
        tgt::vec2 scaledCoord = (1.f / tgt::PIf) * acosf(dir.z) * normalize(dir.xy());
        scaledCoord = scaledCoord*0.5f + 0.5f;
        scaledCoord.y = 1.f - scaledCoord.y;
        tgt::ivec2 texCoord = tgt::ivec2(scaledCoord * (float)sampleNumSqrt_);

        float theta = samples_[i].sph.x;
        float phi   = samples_[i].sph.y;

        tgt::vec3 color = tgt::vec3(0.0);
        for(int l = 0; l < (int)numBands_; l++) {
            for(int m = -l; m <= l; m++) {
                int index = l*(l+1)+m;
                float val = spherHarm(l,m,theta,phi);
                color.x += lightCoeffs_.at(index) * val;
                color.y += lightCoeffs_.at(index+numCoeffs_) * val;
                color.z += lightCoeffs_.at(static_cast<size_t>(index+2.0*numCoeffs_)) * val;
            }
        }
        tex->texel<tgt::col3>(texCoord) = tgt::col3(tgt::vec3(color) * tgt::vec3(255.f));
    }
    saveSamplesToFile(tex, filename);
}

std::vector<float> SHLightProbe::loadCoeffsFromFile(const std::string& filename) {

    TiXmlDocument doc(filename.c_str());
    doc.LoadFile();

    TiXmlHandle hDoc(&doc);
    TiXmlHandle hRoot = TiXmlHandle(hDoc.FirstChildElement().Element());
    TiXmlElement* pCoeffsNode = hRoot.FirstChild().Element();

    lightCoeffs_.clear();
    lightCoeffs_ = std::vector<float>(3*numCoeffs_);

    for(int l = 0; l < (int)numBands_; ++l) {
        for(int m = -l; m <= l; ++m) {
            int index = l*(l+1) + m;
            for(int c = 0; c < 3; c++) {
                std::ostringstream tag;
                tag << "c" << l << m << c;
                float cf = 0.f;
                pCoeffsNode->QueryFloatAttribute(tag.str().c_str(), &cf);
                lightCoeffs_.at(c*numCoeffs_ + index) = cf;
                //std::cout << cf << std::endl;
            }
        }
    }

    return lightCoeffs_;
}

void SHLightProbe::saveCoeffsToFile(const std::string& filename) const {
    TiXmlDocument doc;
    TiXmlDeclaration* declNode = new TiXmlDeclaration("1.0", "ISO-8859-1", "");
    doc.LinkEndChild(declNode);
    TiXmlElement* root = new TiXmlElement("SH");
    doc.LinkEndChild(root);

    TiXmlElement* coeffs = new TiXmlElement("SHCoeffs");
    for(int l = 0; l < (int)numBands_; ++l) {
        for(int m = -l; m <= l; ++m) {
            int index = l*(l+1) + m;
            for(int c = 0; c < 3; c++) {
                std::ostringstream tag;
                tag << "c" << l << m << c;
                coeffs->SetDoubleAttribute(tag.str().c_str(), (double)lightCoeffs_.at(c*numCoeffs_ + index));
            }
        }
    }

    root->LinkEndChild(coeffs);
    doc.SaveFile(filename);
}

void SHLightProbe::setRCShaderUniforms(tgt::Shader* rcProg) {

    if(unshadowed_) {
        float c1 = 0.429043;
        float c2 = 0.511664;
        float c3 = 0.743125;
        float c4 = 0.886227;
        float c5 = 0.247708;
        tgt::mat4 lightMatR;
        tgt::mat4 lightMatG;
        tgt::mat4 lightMatB;

        lightMatR = tgt::mat4(
            tgt::vec4(c1*lightCoeffs_.at(8), c1*lightCoeffs_.at(4), c1*lightCoeffs_.at(7), c2*lightCoeffs_.at(3)),
            tgt::vec4(c1*lightCoeffs_.at(4), -c1*lightCoeffs_.at(8), c1*lightCoeffs_.at(5), c2*lightCoeffs_.at(1)),
            tgt::vec4(c1*lightCoeffs_.at(7), c1*lightCoeffs_.at(5), c3*lightCoeffs_.at(6), c2*lightCoeffs_.at(2)),
            tgt::vec4(c2*lightCoeffs_.at(3), c2*lightCoeffs_.at(1), c2*lightCoeffs_.at(2), c4*lightCoeffs_.at(0)-c5*lightCoeffs_.at(6))
        );
        lightMatG = tgt::mat4(
            tgt::vec4(c1*lightCoeffs_.at(24), c1*lightCoeffs_.at(20), c1*lightCoeffs_.at(23), c2*lightCoeffs_.at(19)),
            tgt::vec4(c1*lightCoeffs_.at(20), -c1*lightCoeffs_.at(24), c1*lightCoeffs_.at(21), c2*lightCoeffs_.at(17)),
            tgt::vec4(c1*lightCoeffs_.at(23), c1*lightCoeffs_.at(21), c3*lightCoeffs_.at(22), c2*lightCoeffs_.at(18)),
            tgt::vec4(c2*lightCoeffs_.at(19), c2*lightCoeffs_.at(17), c2*lightCoeffs_.at(18), c4*lightCoeffs_.at(16)-c5*lightCoeffs_.at(22))
        );
        lightMatB = tgt::mat4(
            tgt::vec4(c1*lightCoeffs_.at(40), c1*lightCoeffs_.at(36), c1*lightCoeffs_.at(39), c2*lightCoeffs_.at(35)),
            tgt::vec4(c1*lightCoeffs_.at(36), -c1*lightCoeffs_.at(40), c1*lightCoeffs_.at(37), c2*lightCoeffs_.at(33)),
            tgt::vec4(c1*lightCoeffs_.at(39), c1*lightCoeffs_.at(37), c3*lightCoeffs_.at(38), c2*lightCoeffs_.at(34)),
            tgt::vec4(c2*lightCoeffs_.at(35), c2*lightCoeffs_.at(33), c2*lightCoeffs_.at(34), c4*lightCoeffs_.at(32)-c5*lightCoeffs_.at(38))
        );

        std::ostringstream oR, oG, oB;
        oR << "lightMatsR" << uniPrefix_ << "_[0]";
        oG << "lightMatsG" << uniPrefix_ << "_[0]";
        oB << "lightMatsB" << uniPrefix_ << "_[0]";

        rcProg->setUniform(oR.str(), lightMatR);
        rcProg->setUniform(oG.str(), lightMatG);
        rcProg->setUniform(oB.str(), lightMatB);

    } else {

        float* coeffArray = &(*lightCoeffs_.begin());
        std::vector<tgt::mat4> matsR = std::vector<tgt::mat4>(numCoeffs_ / 16);
        std::vector<tgt::mat4> matsG = std::vector<tgt::mat4>(numCoeffs_ / 16);
        std::vector<tgt::mat4> matsB = std::vector<tgt::mat4>(numCoeffs_ / 16);

        for(size_t i = 0; i < matsR.size(); i++) {
            int indexR = static_cast<int>(i)*16;
            int indexG = numCoeffs_ + static_cast<int>(i)*16;
            int indexB = 2*numCoeffs_ + static_cast<int>(i)*16;

            matsR.at(i) = tgt::mat4(tgt::vec4(coeffArray+indexR), tgt::vec4(coeffArray+indexR+4), tgt::vec4(coeffArray+indexR+8), tgt::vec4(coeffArray+indexR+12));
            matsG.at(i) = tgt::mat4(tgt::vec4(coeffArray+indexG), tgt::vec4(coeffArray+indexG+4), tgt::vec4(coeffArray+indexG+8), tgt::vec4(coeffArray+indexG+12));
            matsB.at(i) = tgt::mat4(tgt::vec4(coeffArray+indexB), tgt::vec4(coeffArray+indexB+4), tgt::vec4(coeffArray+indexB+8), tgt::vec4(coeffArray+indexB+12));

            std::ostringstream oR, oG, oB;
            LGL_ERROR;
            oR << "lightMatsR" << uniPrefix_ << "_[" << i << "]";
            oG << "lightMatsG" << uniPrefix_ << "_[" << i << "]";
            oB << "lightMatsB" << uniPrefix_ << "_[" << i << "]";
            rcProg->setUniform(oR.str(), matsR.at(i), true);
            rcProg->setUniform(oG.str(), matsG.at(i), true);
            rcProg->setUniform(oB.str(), matsB.at(i), true);
            //rcProg->setUniform(oR.str(), &(*matsR.begin()), matsR.size());
            //rcProg->setUniform(oR.str(), &(*matsR.begin()), matsR.size());
            //rcProg->setUniform(oG.str(), &(*matsG.begin()), matsG.size());
            //rcProg->setUniform(oB.str(), &(*matsB.begin()), matsB.size());
            LGL_ERROR;
        }
    }
    //rcProg->setUniform("lightMatR_", lightMatR);
    //rcProg->setUniform("lightMatG_", lightMatG);
    //rcProg->setUniform("lightMatB_", lightMatB);
}

} //namespace

