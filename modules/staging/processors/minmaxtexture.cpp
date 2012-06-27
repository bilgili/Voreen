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

#include "minmaxtexture.h"

namespace voreen {

/**
* Begin MinMaxTexture1DTF
*/

MinMaxTexture1DTF::MinMaxTexture1DTF(int resolution) {
    resolution_ = resolution;
    tex_ = new float[(int)pow((float)resolution_,2)];
}

MinMaxTexture1DTF::~MinMaxTexture1DTF() {
    delete[] tex_;
}

MinMaxTexture1DTF* MinMaxTexture1DTF::create(tgt::Texture* transfunc, int maxres) {
    int width = transfunc->getWidth();
    int res = tgt::min(tgt::ivec2(width,maxres));
    MinMaxTexture1DTF* mmt = new MinMaxTexture1DTF(res);
    float* table = new float[res];

    float step = (float)width / res;
    int minindex, maxindex, i;
    float val;
    minindex = 0; maxindex = 0; i = 0;
    while (minindex<width) {
        i++;
        table[i-1] = 0.f;
        maxindex = tgt::min(tgt::ivec2((int)ceil(i * step)-1,width-1));
        for (int j=minindex; j<=maxindex; j++) {
            val = transfunc->texel<tgt::col4>(j).a / 255.f;
            if (val>table[i-1]) table[i-1] = val;
        }
        minindex = maxindex+1;
    }

    for (i=0; i<res; i++) {
        for (int min=0; min<=i; min++) {
            for (int max=res-1; max>=i; max--) {
                if (table[i]>mmt->tex_[min * res +  max]) mmt->tex_[min * res +  max] = table[i];
            }
        }
    }

    delete[] table;

    return mmt;
}

float MinMaxTexture1DTF::getMax(float min, float max) {
    int imin = (int)tgt::round(min * (resolution_-1));
    int imax = (int)tgt::round(max * (resolution_-1));
    return tex_[imin * resolution_ +  imax];
}

float MinMaxTexture1DTF::getMax(float minvox, float maxvox, float /*mingrad*/, float /*maxgrad*/) {
    return getMax(minvox,maxvox);
}

/**
* End MinMaxTexture1DTF
*/

/**
* Begin MinMaxTexture2DTF
*/

MinMaxTexture2DTF::MinMaxTexture2DTF(int xres, int yres) {
    xres_ = xres;
    yres_ = yres;
    tex_ = new float[xres_*xres_*yres_*yres_];
}

MinMaxTexture2DTF::~MinMaxTexture2DTF() {
    delete[] tex_;
}

MinMaxTexture2DTF* MinMaxTexture2DTF::create(tgt::Texture* transfunc, int xresmax, int yresmax) {
    int width = transfunc->getWidth();
    int height = transfunc->getHeight();
    int xres = tgt::min(tgt::ivec2(width,xresmax));
    int yres = tgt::min(tgt::ivec2(height,yresmax));
    MinMaxTexture2DTF* mmt = new MinMaxTexture2DTF(xres,yres);
    float* table = new float[xres*yres];

    float xstep,ystep;
    int xminindex, xmaxindex;
    int yminindex, ymaxindex;
    int i,j;
    int tindex;
    float val;

    xstep = (float)width / xres;
    ystep = (float)height / yres;
    i = 0; xminindex = 0; yminindex = 0;
    while (xminindex<width) {
        i++;
        xmaxindex = tgt::min(tgt::ivec2((int)ceil(i * xstep)-1,width-1));
        j = 0; yminindex = 0;
        while (yminindex<height) {
            j++;
            tindex = (i-1)*yres+(j-1);
            table[tindex] = 0.f;
            ymaxindex = tgt::min(tgt::ivec2((int)ceil(j * ystep)-1,height-1));
            for (int m=xminindex; m<=xmaxindex; m++) {
                for (int n=yminindex; n<=ymaxindex; n++) {
                    val = transfunc->texel<tgt::vec4>(m,n).a;
                    if (val>table[tindex]) table[tindex] = val;
                }
            }
            yminindex = ymaxindex + 1;
        }
        xminindex = xmaxindex + 1;
    }

    for (i=0; i<xres; i++) {
        for (int j=0; j<yres; j++) {
            val = table[i*yres+j];
            for (int xmin=0; xmin<=i; xmin++) {
                for (int xmax=xres-1; xmax>=i; xmax--) {
                    for (int ymin=0; ymin<=j; ymin++) {
                        for (int ymax=yres-1; ymax>=j; ymax--) {
                            tindex = xmin * (xres*yres*yres) +  xmax * (yres*yres) + ymin * yres + ymax;
                            if (val>mmt->tex_[tindex]) mmt->tex_[tindex] = val;
                        }
                    }
                }
            }
        }
    }

    delete[] table;

    return mmt;
}

float MinMaxTexture2DTF::getMax(float minvox, float maxvox, float mingrad, float maxgrad) {
    int ixmin = (int)tgt::round(minvox * (xres_-1));
    int ixmax = (int)tgt::round(maxvox * (xres_-1));
    int iymin = (int)tgt::round(mingrad * (yres_-1));
    int iymax = (int)tgt::round(maxgrad * (yres_-1));
    return tex_[ixmin * (xres_*yres_*yres_) +  ixmax * (yres_*yres_) + iymin * yres_ + iymax];
}

/**
* End MinMaxTexture2DTF
*/

} // namespace
