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

#include "tgt/tgt_math.h"
#include <time.h>
#include "shcoeffcalc.h"

namespace voreen {

SHCoeffCalc::SHCoeffCalc(int sampleNumSqrt, int numBands) {
    sampleNumSqrt_ = sampleNumSqrt;
    numBands_ = numBands;
    numCoeffs_ = numBands_*numBands_;

    // we need the faculties for the normalization factors later
    facs_ = std::vector<float>(2*numBands_);
    facs_[0] = 1.f;
    facs_[1] = 1.f;
    for(size_t i = 2; i < facs_.size(); i++)
        facs_[i] = i*facs_[i-1];

    sampleTex_ = 0;
    coeffTex_ = 0;

    // generate samples and calculate geometric sh-coefficients
    initSamples();
}

SHCoeffCalc::SHCoeffCalc(int numBands)
    : sampleNumSqrt_(0)
    , numBands_(numBands) {

    // we need the faculties for the normalization factors later
    facs_ = std::vector<float>(2*numBands_);
    facs_[0] = 1.f;
    facs_[1] = 1.f;
    for(size_t i = 2; i < facs_.size(); i++)
        facs_[i] = i*facs_[i-1];

    numCoeffs_ = numBands_*numBands_;
    sampleTex_ = 0;
    coeffTex_  = 0;
}

void SHCoeffCalc::initSamples() {

#ifdef VRN_NO_RANDOM
    std::srand(0);
#else
    // for randomized samples, initialize random number generator
    std::srand((unsigned)time(NULL));
#endif

    //init the sample vector
    samples_ = std::vector<Sample>();

    float oneoverN = 1.f / (float)sampleNumSqrt_;

    // the first texture contains the cartesian vectors pointing from the origin at (0, 0, 0) towards
    // the sample, the second texture contains the geometric sh-coefficients of the sample (they are
    // saved in a four-quadrant texture, each quadrant containing four coefficients for each sample in
    // form of a RGBA-pixel)
    sampleTex_ = new tgt::Texture(tgt::ivec3(sampleNumSqrt_, sampleNumSqrt_, 1),
                                          GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT,
                                          tgt::Texture::LINEAR);
                                          //tgt::Texture::NEAREST);
    coeffTex_ = new tgt::Texture(tgt::ivec3((numBands_ / 2)*sampleNumSqrt_, (numBands_ / 2)*sampleNumSqrt_, 1),
                                          GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT,
                                          tgt::Texture::LINEAR);
                                          //tgt::Texture::NEAREST);

    for(int a = 0; a < sampleNumSqrt_; a++) {
        for(int b = 0; b < sampleNumSqrt_; b++) {

            // generate unbiased distribution of spherical coords
            float x = (float(a) + getRandFloat()) * oneoverN;
            float y = (float(b) + getRandFloat()) * oneoverN;
            float theta = 2.f * acosf(sqrtf(1.f - x));
            float phi = 2.f * tgt::PIf * y;

            // generate sample
            Sample s;
            // save normal-vector in cartesian coordinates
            s.cart = tgt::vec3(sinf(theta)*cosf(phi), sinf(theta)*sinf(phi), cosf(theta));
            // save normal-vector in spherical coordinates
            s.sph  = tgt::vec3(theta, phi, 1.f);

            // calculate the geometric sh-coefficients of this sample, depending on its theta and phi angles, for
            // each band
            s.coeff = std::vector<float>(numCoeffs_);
            for(int l = 0; l < (int)numBands_; ++l) {
                for(int m = -l; m <= l; ++m) {
                    // this form of index calculation guarantees that indices are not used more than once
                    int index = l*(l+1) + m;
                    // this is where the magic happens: calculate the spherical harmonic value for this sample and this index
                    s.coeff.at(index) = spherHarm(l,m,theta,phi);
                }
            }

            tgt::Vector4<uint16_t> normSample((s.cart + tgt::vec3(1.f))*32767.5f,
                                              static_cast<uint16_t>(getRandFloat()*65536.f));
            //tgt::Vector4<uint16_t> normSample(tgt::vec4(getRandFloat(), getRandFloat(), getRandFloat(), getRandFloat())*65536.f);

            // save them into the sample- and coefficient-textures which we can later, for example, use in a shader
            sampleTex_->texel<tgt::Vector4<uint16_t> >(a, b) = normSample;

            // remap the coefficients and sample vectors into 16-bit unsigned shorts and save them into the coefficient-texture
            tgt::Vector4<uint16_t> cf;
            for(size_t i = 0; i < numBands_ / 2; i++) {
                for(size_t j = 0; j < numBands_ / 2; j++) {
                    cf.x = uint16_t((s.coeff.at(i*numBands_*2 + j*4    ) + 1.f)*32767.5f);
                    cf.y = uint16_t((s.coeff.at(i*numBands_*2 + j*4 + 1) + 1.f)*32767.5f);
                    cf.z = uint16_t((s.coeff.at(i*numBands_*2 + j*4 + 2) + 1.f)*32767.5f);
                    cf.w = uint16_t((s.coeff.at(i*numBands_*2 + j*4 + 3) + 1.f)*32767.5f);
                    coeffTex_->texel<tgt::Vector4<uint16_t> >(j*sampleNumSqrt_+a, i*sampleNumSqrt_+b) = cf;
                }
            }

            // finally, save the generated sample in the sample-vector
            samples_.push_back(s);
        }
    }

    // when we are done with all samples, upload the textures so opengl can use them later
    sampleTex_->uploadTexture();
    coeffTex_->uploadTexture();
}

// the following functions are all implemented from papers like the "nitty gritty spherical harmonics"-paper
float SHCoeffCalc::legPol(int l, int m, float x) const {
    // the Legendre-polynomes are the basis functions for spherical harmonics
    float pmm = 1.f;

    if(m > 0) {
        float somx2 = sqrtf(1.f-x*x);
        float fact = 1.f;
        for(int i = 1; i <= m; i++) {
            pmm *= (-fact) * somx2;
            fact += 2.f;
        }
    }

    if(l==m)
        return pmm;

    float pmmp1 = x * (2.f*m + 1.f) * pmm;

    if(l==m+1)
        return pmmp1;

    float pll = 0.f;

    for(int ll = m+2; ll <= l; ++ll) {
        pll = ((2.f*ll - 1.f)*x*pmmp1 - (ll + m - 1.f) * pmm) / (ll - m);
        pmm = pmmp1;
        pmmp1 = pll;
    }

    return pll;

    // recursive version:

    //if(m == l) {
        //int fac = std::pow(-1, m);
        //int doublefak = 1;
        //for(int i = 2*m-1; i > 1; i -= 2)
            //doublefak *= i;

        //float ret = fac*doublefak*std::pow((1-x*x), m / 2);
        //return ret;
    //}
    //else if(l == m+1) {
        //float ret = x*(2*m+1)*legPol(m, m, x);
        //return ret;
    //}
    //else {
        //float ret = (x*(2*l-1)*legPol(l-1,m,x) - (l+m-1)*legPol(l-2,m,x)) / (l-m);
        //return ret;
    //}
}

float SHCoeffCalc::spherHarm(int l, int m, float theta, float phi) const {

    // renormalisation constant for SH function
    float temp = sqrtf(((2.f*l + 1.f) * facs_[l-abs(m)]) / (4.f*tgt::PIf*facs_[l+abs(m)]));
    //float temp = sqrtf(((2.f*l + 1.f) * facs_[l-m]) / (4.f*tgt::PIf*facs_[l+m]));
    if(m == 0)
        return temp * legPol(l, 0, cosf(theta));
    else if(m > 0)
        return sqrtf(2.f) * temp * cosf(phi*m) * legPol(l, m, cosf(theta));
    else
        return sqrtf(2.f) * temp * sinf(-m*phi) * legPol(l, -m, cosf(theta));
        //return sqrtf(2.f) * temp * sinf(phi*m) * legPol(l, abs(m), cosf(theta));
}

// just a helper function to save a texture to a file, for debugging and other purposes
void SHCoeffCalc::saveSamplesToFile(tgt::Texture* tex, std::string filename) const {
    //taken from gamedev.net forum:
    FILE *pFile;               // The file pointer.
    unsigned char uselessChar; // used for useless char.
    short int uselessInt;      // used for useless int.
    unsigned char imageType;   // Type of image we are saving.
    unsigned char bits;    // Bit depth.
    long size;                 // Size of the picture.
    unsigned char tempColors;
    short int width  = tex->getWidth();
    short int height = tex->getHeight();
    unsigned char* image = tex->getPixelData();

    // Set the image type, the color mode, and the bit depth.
    bits = tex->getBpp()*8;
    imageType = 2;

    // Open file for output.
    pFile = fopen(filename.c_str(), "wb");

   // Check if the file opened or not.
    if(!pFile) {
        fclose(pFile);
        return;
    }

    // Set these two to 0.
    uselessChar = 0; uselessInt = 0;

    // Write useless data.
    fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);
    fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);

    // Now image type.
    fwrite(&imageType, sizeof(unsigned char), 1, pFile);

    // Write useless data.
    fwrite(&uselessInt, sizeof(short int), 1, pFile);
    fwrite(&uselessInt, sizeof(short int), 1, pFile);
    fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);
    fwrite(&uselessInt, sizeof(short int), 1, pFile);
    fwrite(&uselessInt, sizeof(short int), 1, pFile);

    // Write the size that you want.
    fwrite(&width, sizeof(short int), 1, pFile);
    fwrite(&height, sizeof(short int), 1, pFile);
    fwrite(&bits, sizeof(unsigned char), 1, pFile);

    // Write useless data.
    fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);

    // Get image size.
    size = width * height * tex->getBpp();

     //Now switch image from RGB to BGR.
    for(int i  = 0; i < size; i += tex->getBpp()) {
        tempColors = image[i];
        image[i] = image[i + 2];
        image[i + 2] = tempColors;
    }

    // Finally write the image.
    fwrite(image, sizeof(unsigned char), size, pFile);

    // close the file.
    fclose(pFile);
}

} // namespace

