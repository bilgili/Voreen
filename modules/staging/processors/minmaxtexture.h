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

#ifndef VRN_MINMAXTEXTURE_H
#define VRN_MINMAXTEXTURE_H

#include "tgt/vector.h"
#include "tgt/texture.h"

namespace voreen {

class MinMaxTexture {
public:
    virtual ~MinMaxTexture() {};
    virtual float getMax(float minvox, float maxvox, float mingrad, float maxgrad) = 0;
};

class MinMaxTexture1DTF : public MinMaxTexture {
public:
    MinMaxTexture1DTF(int resolution);
    ~MinMaxTexture1DTF();
    static MinMaxTexture1DTF* create(tgt::Texture* transfunc, int maxres = 512);
    float getMax(float min, float max);
    float getMax(float minvox, float maxvox, float mingrad, float maxgrad);
private:
    float* tex_;
    int resolution_;
};

class MinMaxTexture2DTF : public MinMaxTexture {
public:
    MinMaxTexture2DTF(int xres, int yres);
    ~MinMaxTexture2DTF();
    static MinMaxTexture2DTF* create(tgt::Texture* transfunc, int xresmax = 64, int yresmax = 64);
    float getMax(float minvox, float maxvox, float mingrad, float maxgrad);
private:
    float* tex_;
    int xres_;
    int yres_;
};

} // namespace

#endif // VRN_MINMAXTEXTURE_H
