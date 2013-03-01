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

#include "voreen/core/animation/interpolation/transfuncinterpolationfunctions.h"
#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"
#include "voreen/core/datastructures/transfunc/transfunc2dprimitives.h"
#include "voreen/core/datastructures/transfunc/transfuncmappingkey.h"
#include "voreen/core/datastructures/transfunc/transfuncprimitive.h"
#include "voreen/core/animation/interpolation/basicfloatinterpolation.h"
#include "voreen/core/animation/interpolation/basicintinterpolation.h"
#include <math.h>

namespace voreen {

GLubyte* TransFuncInterpolationFunctionBase::convertTextureToRGBA(tgt::ivec3 dim, GLubyte* texture, GLuint inputformat) {
    GLubyte* data = new GLubyte[4 * dim.x * dim.y * dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                switch (inputformat) {
                case GL_RED:
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+0] = texture[x*dim.y*dim.z+y*dim.z+z];
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+1] = 0;
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+2] = 0;
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+3] = 1;
                    break;
                case GL_GREEN:
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+0] = 0;
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+1] = texture[x*dim.y*dim.z+y*dim.z+z];
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+2] = 0;
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+3] = 1;
                    break;
                case GL_BLUE:
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+0] = 0;
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+1] = 0;
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+2] = texture[x*dim.y*dim.z+y*dim.z+z];
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+3] = 1;
                    break;
                case GL_ALPHA:
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+0] = 0;
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+1] = 0;
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+2] = 0;
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+3] = texture[x*dim.y*dim.z+y*dim.z+z];
                    break;
                case GL_RGB:
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+0] = texture[3*(x*dim.y*dim.z+y*dim.z+z)+0];
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+1] = texture[3*(x*dim.y*dim.z+y*dim.z+z)+1];
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+2] = texture[3*(x*dim.y*dim.z+y*dim.z+z)+2];
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+3] = 1;
                    break;
                case GL_RGBA:
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+0] = texture[4*(x*dim.y*dim.z+y*dim.z+z)+0];
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+1] = texture[4*(x*dim.y*dim.z+y*dim.z+z)+1];
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+2] = texture[4*(x*dim.y*dim.z+y*dim.z+z)+2];
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+3] = texture[4*(x*dim.y*dim.z+y*dim.z+z)+3];
                    break;
                case GL_ABGR_EXT:
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+0] = texture[4*(x*dim.y*dim.z+y*dim.z+z)+3];
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+1] = texture[4*(x*dim.y*dim.z+y*dim.z+z)+2];
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+2] = texture[4*(x*dim.y*dim.z+y*dim.z+z)+1];
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+3] = texture[4*(x*dim.y*dim.z+y*dim.z+z)+0];
                    break;
                case GL_LUMINANCE:
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+0] = texture[x*dim.y*dim.z+y*dim.z+z];
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+1] = texture[x*dim.y*dim.z+y*dim.z+z];
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+2] = texture[x*dim.y*dim.z+y*dim.z+z];
                    data[(x*dim.y*dim.z+y*dim.z+z)*4+3] = 1;
                    break;
                }
            }
        }
    }
    return data;
}

//Resizes a given 3D Textur in RGBA to another dimension by trilinear interpolation
GLubyte* TransFuncInterpolationFunctionBase::changeTextureDimension(tgt::ivec3 in_dim, tgt::ivec3 out_dim, GLubyte* indata) {
    GLubyte* outdata;
    if (in_dim.x != out_dim.x) {
        outdata = new GLubyte[4 * out_dim.x * in_dim.y * in_dim.z];
        for (int x = 0; x < out_dim.x; ++x) {
            float x_position = static_cast<float>(in_dim.x-1) / static_cast<float>(out_dim.x-1) * static_cast<float>(x);
            int x1 = static_cast<int>(std::floor(x_position));
            float a1 = 1-(x_position-x1);
            int x2 = static_cast<int>(std::ceil(x_position));
            float a2 = 1-a1;
            int x_value = -1;
            if (x1 == x2)
                x_value = x1;
            if (std::abs(a1) < 0.001f)
                x_value = x2;
            if (std::abs(a2) < 0.001f)
                x_value = x1;
            if (x_value >= 0) {
                for (int y = 0; y < in_dim.y; ++y) {
                    for (int z = 0; z < in_dim.z; ++z) {
                        outdata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z)+0] = indata[4*(x_value*in_dim.y*in_dim.z+y*in_dim.z+z)+0];
                        outdata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z)+1] = indata[4*(x_value*in_dim.y*in_dim.z+y*in_dim.z+z)+1];
                        outdata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z)+2] = indata[4*(x_value*in_dim.y*in_dim.z+y*in_dim.z+z)+2];
                        outdata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z)+3] = indata[4*(x_value*in_dim.y*in_dim.z+y*in_dim.z+z)+3];
                    }
                }
            }
            else {
                for (int y = 0; y < in_dim.y; ++y) {
                    for (int z = 0; z < in_dim.z; ++z) {
                        outdata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z)+0] = static_cast<GLubyte>(a1*indata[4*(x1*in_dim.y*in_dim.z+y*in_dim.z+z)+0]+a2*indata[4*(x2*in_dim.y*in_dim.z+y*in_dim.z+z)+0]);
                        outdata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z)+1] = static_cast<GLubyte>(a1*indata[4*(x1*in_dim.y*in_dim.z+y*in_dim.z+z)+1]+a2*indata[4*(x2*in_dim.y*in_dim.z+y*in_dim.z+z)+1]);
                        outdata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z)+2] = static_cast<GLubyte>(a1*indata[4*(x1*in_dim.y*in_dim.z+y*in_dim.z+z)+2]+a2*indata[4*(x2*in_dim.y*in_dim.z+y*in_dim.z+z)+2]);
                        outdata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z)+3] = static_cast<GLubyte>(a1*indata[4*(x1*in_dim.y*in_dim.z+y*in_dim.z+z)+3]+a2*indata[4*(x2*in_dim.y*in_dim.z+y*in_dim.z+z)+3]);
                    }
                }
            }
        }
        in_dim.x = out_dim.x;
        delete indata;
        indata = outdata;
    }
    if (in_dim.y != out_dim.y) {
        outdata = new GLubyte[4 * out_dim.x * out_dim.y * in_dim.z];
        for (int y = 0; y < out_dim.y; ++y) {
            float y_position = static_cast<float>(in_dim.y-1) / static_cast<float>(out_dim.y-1) * static_cast<float>(y);
            int y1 = static_cast<int>(std::floor(y_position));
            float a1 = 1-(y_position-y1);
            int y2 = static_cast<int>(std::ceil(y_position));
            float a2 = 1 - a1;
            int y_value = -1;
            if (y1 == y2)
                y_value = y1;
            if (std::abs(a1) < 0.001f)
                y_value = y2;
            if (std::abs(a2) < 0.001f)
                y_value = y1;
            if (y_value >= 0) {
                for (int x = 0; x < out_dim.x; ++x) {
                    for (int z = 0; z < in_dim.z; ++z) {
                        outdata[4*(x*out_dim.y*in_dim.z+y*in_dim.z+z)+0] = indata[4*(x*in_dim.y*in_dim.z+y_value*in_dim.z+z)+0];
                        outdata[4*(x*out_dim.y*in_dim.z+y*in_dim.z+z)+1] = indata[4*(x*in_dim.y*in_dim.z+y_value*in_dim.z+z)+1];
                        outdata[4*(x*out_dim.y*in_dim.z+y*in_dim.z+z)+2] = indata[4*(x*in_dim.y*in_dim.z+y_value*in_dim.z+z)+2];
                        outdata[4*(x*out_dim.y*in_dim.z+y*in_dim.z+z)+3] = indata[4*(x*in_dim.y*in_dim.z+y_value*in_dim.z+z)+3];
                    }
                }
            }
            else {
                for (int x = 0; x < out_dim.x; ++x) {
                    for (int z = 0; z < in_dim.z; ++z) {
                        outdata[4*(x*out_dim.y*in_dim.z+y*in_dim.z+z)+0] = static_cast<GLubyte>(a1*indata[4*(x*in_dim.y*in_dim.z+y1*in_dim.z+z)+0]+a2*indata[4*(x*in_dim.y*in_dim.z+y2*in_dim.z+z)+0]);
                        outdata[4*(x*out_dim.y*in_dim.z+y*in_dim.z+z)+1] = static_cast<GLubyte>(a1*indata[4*(x*in_dim.y*in_dim.z+y1*in_dim.z+z)+1]+a2*indata[4*(x*in_dim.y*in_dim.z+y2*in_dim.z+z)+1]);
                        outdata[4*(x*out_dim.y*in_dim.z+y*in_dim.z+z)+2] = static_cast<GLubyte>(a1*indata[4*(x*in_dim.y*in_dim.z+y1*in_dim.z+z)+2]+a2*indata[4*(x*in_dim.y*in_dim.z+y2*in_dim.z+z)+2]);
                        outdata[4*(x*out_dim.y*in_dim.z+y*in_dim.z+z)+3] = static_cast<GLubyte>(a1*indata[4*(x*in_dim.y*in_dim.z+y1*in_dim.z+z)+3]+a2*indata[4*(x*in_dim.y*in_dim.z+y2*in_dim.z+z)+3]);
                    }
                }
            }
        }
        in_dim.y = out_dim.y;
        delete indata;
        indata = outdata;
    }
    if (in_dim.z != out_dim.z) {
        outdata = new GLubyte[4 * out_dim.x * out_dim.y * out_dim.z];
        for (int z = 0; z < out_dim.z; ++z) {
            float z_position = static_cast<float>(in_dim.z-1) / static_cast<float>(out_dim.z-1) * static_cast<float>(z);
            int z1 = static_cast<int>(std::floor(z_position));
            float a1 = 1-(z_position-z1);
            int z2 = static_cast<int>(std::ceil(z_position));
            float a2 = 1-a1;
            int z_value = -1;
            if (z1 == z2)
                z_value = z1;
            if (std::abs(a1) < 0.001f)
                z_value = z2;
            if (std::abs(a2) < 0.001f)
                z_value = z1;
            if (z_value >= 0) {
                for (int x = 0; x < out_dim.x; ++x) {
                    for (int y = 0; y < out_dim.y; ++y) {
                        outdata[4*(x*out_dim.y*out_dim.z+y*out_dim.z+z)+0] = indata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z_value)+0];
                        outdata[4*(x*out_dim.y*out_dim.z+y*out_dim.z+z)+1] = indata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z_value)+1];
                        outdata[4*(x*out_dim.y*out_dim.z+y*out_dim.z+z)+2] = indata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z_value)+2];
                        outdata[4*(x*out_dim.y*out_dim.z+y*out_dim.z+z)+3] = indata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z_value)+3];
                    }
                }
            }
            else {
                for (int x = 0; x < out_dim.x; ++x) {
                    for (int y = 0; y < out_dim.y; ++y) {
                        outdata[4*(x*out_dim.y*out_dim.z+y*out_dim.z+z)+0] = static_cast<GLubyte>(a1*indata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z1)+0]+a2*indata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z2)+0]);
                        outdata[4*(x*out_dim.y*out_dim.z+y*out_dim.z+z)+1] = static_cast<GLubyte>(a1*indata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z1)+1]+a2*indata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z2)+1]);
                        outdata[4*(x*out_dim.y*out_dim.z+y*out_dim.z+z)+2] = static_cast<GLubyte>(a1*indata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z1)+2]+a2*indata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z2)+2]);
                        outdata[4*(x*out_dim.y*out_dim.z+y*out_dim.z+z)+3] = static_cast<GLubyte>(a1*indata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z1)+3]+a2*indata[4*(x*in_dim.y*in_dim.z+y*in_dim.z+z2)+3]);
                    }
                }
            }
        }
        in_dim.y = out_dim.y;
        delete indata;
        indata = outdata;
    }
    return indata;
}

//-------------------------------------------------------------------------------------------------

TransFuncInterpolationFunction::TransFuncInterpolationFunction() {}

std::string TransFuncInterpolationFunction::getGuiName() const {
    return "linear interpolation";
}

std::string TransFuncInterpolationFunction::getCategory() const {
    return "default linear (keywise if possible)";
}

TransFunc* TransFuncInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    if (!startvalue || !endvalue) {
        LERROR("Null pointer passed");
        return 0;
    }

    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds((1-time)*t1.x+time*t2.x,
                                (1-time)*t1.y+time*t2.y);

            tgt::vec2 d1 = func1->getDomain(0);
            tgt::vec2 d2 = func2->getDomain(0);
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain((1-time)*d1.x+time*d2.x,
                            (1-time)*d1.y+time*d2.y, 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>((1-time)*(*it1)->getColorL().r + time*(*it2)->getColorL().r);
                col.g = static_cast<uint8_t>((1-time)*(*it1)->getColorL().g + time*(*it2)->getColorL().g);
                col.b = static_cast<uint8_t>((1-time)*(*it1)->getColorL().b + time*(*it2)->getColorL().b);
                col.a = static_cast<uint8_t>((1-time)*(*it1)->getColorL().a + time*(*it2)->getColorL().a);
                key->setColorL(col);

                col.r = static_cast<uint8_t>((1-time)*(*it1)->getColorR().r + time*(*it2)->getColorR().r);
                col.g = static_cast<uint8_t>((1-time)*(*it1)->getColorR().g + time*(*it2)->getColorR().g);
                col.b = static_cast<uint8_t>((1-time)*(*it1)->getColorR().b + time*(*it2)->getColorR().b);
                col.a = static_cast<uint8_t>((1-time)*(*it1)->getColorR().a + time*(*it2)->getColorR().a);
                key->setColorR(col);

                key->setIntensity((1-time)*(*it1)->getIntensity() + time*(*it2)->getIntensity());

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    float a2 = BasicFloatInterpolation::linearInterpolation(0, 1, time);
    float a1 = 1 - a2;

    tgtAssert(startvalue && endvalue, "null pointer");
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x, dim.y, dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4 * dim.x * dim.y * dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    int index = 4*(x * dim.y * dim.z + y * dim.z + z) + i;
                    float f = (a1 * texture1[index] + a2 * texture2[index]);
                    texture[index] = static_cast<GLubyte>(f);
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncInterpolationFunction::create() const {
    return new TransFuncInterpolationFunction();
}

//-------------------------------------------------------------------------------------------------

TransFuncStartInterpolationFunction::TransFuncStartInterpolationFunction() {}

std::string TransFuncStartInterpolationFunction::getGuiName() const {
    return "focus on startvalue";
}

std::string TransFuncStartInterpolationFunction::getCategory() const {
    return "boolean";
}

TransFunc* TransFuncStartInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}
InterpolationFunction<TransFunc*>* TransFuncStartInterpolationFunction::create() const {
    return new TransFuncStartInterpolationFunction();
}

TransFuncEndInterpolationFunction::TransFuncEndInterpolationFunction() {}

std::string TransFuncEndInterpolationFunction::getGuiName() const {
    return "focus on endvalue";
}

std::string TransFuncEndInterpolationFunction::getCategory() const {
    return "boolean";
}

TransFunc* TransFuncEndInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    if (time > 0.f)
        return endvalue->create();
    else
        return startvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncEndInterpolationFunction::create() const {
    return new TransFuncEndInterpolationFunction();
}

TransFuncStartEndInterpolationFunction::TransFuncStartEndInterpolationFunction() {}

std::string TransFuncStartEndInterpolationFunction::getGuiName() const {
    return "bisection";
}

std::string TransFuncStartEndInterpolationFunction::getCategory() const {
    return "boolean";
}

TransFunc* TransFuncStartEndInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    if (time < 0.5f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncStartEndInterpolationFunction::create() const {
    return new TransFuncStartEndInterpolationFunction();
}

TransFuncKeyWiseInterpolationFunction::TransFuncKeyWiseInterpolationFunction() {}

std::string TransFuncKeyWiseInterpolationFunction::getGuiName() const {
    return "keywise linear";
}

std::string TransFuncKeyWiseInterpolationFunction::getCategory() const {
    return "keywise linear";
}

TransFunc* TransFuncKeyWiseInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds((1-time)*t1.x+time*t2.x,
                                (1-time)*t1.y+time*t2.y);

            tgt::vec2 d1 = func1->getDomain(0);
            tgt::vec2 d2 = func2->getDomain(0);
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain((1-time)*d1.x+time*d2.x,
                            (1-time)*d1.y+time*d2.y, 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>((1-time)*(*it1)->getColorL().r + time*(*it2)->getColorL().r);
                col.g = static_cast<uint8_t>((1-time)*(*it1)->getColorL().g + time*(*it2)->getColorL().g);
                col.b = static_cast<uint8_t>((1-time)*(*it1)->getColorL().b + time*(*it2)->getColorL().b);
                col.a = static_cast<uint8_t>((1-time)*(*it1)->getColorL().a + time*(*it2)->getColorL().a);
                key->setColorL(col);

                col.r = static_cast<uint8_t>((1-time)*(*it1)->getColorR().r + time*(*it2)->getColorR().r);
                col.g = static_cast<uint8_t>((1-time)*(*it1)->getColorR().g + time*(*it2)->getColorR().g);
                col.b = static_cast<uint8_t>((1-time)*(*it1)->getColorR().b + time*(*it2)->getColorR().b);
                col.a = static_cast<uint8_t>((1-time)*(*it1)->getColorR().a + time*(*it2)->getColorR().a);
                key->setColorR(col);

                key->setIntensity((1-time)*(*it1)->getIntensity()+time*(*it2)->getIntensity());

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseInterpolationFunction::create() const {
    return new TransFuncKeyWiseInterpolationFunction();
}

TransFuncKeyWiseQuadInInterpolationFunction::TransFuncKeyWiseQuadInInterpolationFunction() {}

std::string TransFuncKeyWiseQuadInInterpolationFunction::getGuiName() const {
    return "easing in";
}

std::string TransFuncKeyWiseQuadInInterpolationFunction::getCategory() const {
    return "keywise quadratic";
}

TransFunc* TransFuncKeyWiseQuadInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::inQuadInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::inQuadInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain(0);
            tgt::vec2 d2 = func2->getDomain(0);
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::inQuadInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::inQuadInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inQuadInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inQuadInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inQuadInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inQuadInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inQuadInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inQuadInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inQuadInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inQuadInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::inQuadInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseQuadInInterpolationFunction::create() const {
    return new TransFuncKeyWiseQuadInInterpolationFunction();
}

TransFuncKeyWiseQuadOutInterpolationFunction::TransFuncKeyWiseQuadOutInterpolationFunction() {}

std::string TransFuncKeyWiseQuadOutInterpolationFunction::getGuiName() const {
    return "easing out";
}

std::string TransFuncKeyWiseQuadOutInterpolationFunction::getCategory() const {
    return "keywise quadratic";
}

TransFunc* TransFuncKeyWiseQuadOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::outQuadInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::outQuadInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outQuadInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outQuadInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outQuadInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outQuadInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outQuadInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outQuadInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outQuadInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outQuadInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::outQuadInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseQuadOutInterpolationFunction::create() const {
    return new TransFuncKeyWiseQuadOutInterpolationFunction();
}

TransFuncKeyWiseQuadInOutInterpolationFunction::TransFuncKeyWiseQuadInOutInterpolationFunction() {}

std::string TransFuncKeyWiseQuadInOutInterpolationFunction::getGuiName() const {
    return "first easing in, then easing out";
}

std::string TransFuncKeyWiseQuadInOutInterpolationFunction::getCategory() const {
    return "keywise quadratic";
}

TransFunc* TransFuncKeyWiseQuadInOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::inOutQuadInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::inOutQuadInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::inOutQuadInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::inOutQuadInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuadInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuadInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuadInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuadInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuadInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuadInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuadInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuadInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::inOutQuadInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseQuadInOutInterpolationFunction::create() const {
    return new TransFuncKeyWiseQuadInOutInterpolationFunction();
}

TransFuncKeyWiseQuadOutInInterpolationFunction::TransFuncKeyWiseQuadOutInInterpolationFunction() {}

std::string TransFuncKeyWiseQuadOutInInterpolationFunction::getGuiName() const {
    return "first easing out, then easing in";
}

std::string TransFuncKeyWiseQuadOutInInterpolationFunction::getCategory() const {
    return "keywise quadratic";
}

TransFunc* TransFuncKeyWiseQuadOutInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::outInQuadInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::outInQuadInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::outInQuadInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::outInQuadInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outInQuadInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outInQuadInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outInQuadInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outInQuadInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outInQuadInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outInQuadInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outInQuadInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outInQuadInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::outInQuadInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseQuadOutInInterpolationFunction::create() const {
    return new TransFuncKeyWiseQuadOutInInterpolationFunction();
}

TransFuncKeyWiseCubicInInterpolationFunction::TransFuncKeyWiseCubicInInterpolationFunction() {}

std::string TransFuncKeyWiseCubicInInterpolationFunction::getGuiName() const {
    return "easing in";
}

std::string TransFuncKeyWiseCubicInInterpolationFunction::getCategory() const {
    return "keywise cubic";
}

TransFunc* TransFuncKeyWiseCubicInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::inQuadInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::inQuadInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::inQuadInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::inQuadInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inCubicInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inCubicInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inCubicInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inCubicInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inCubicInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inCubicInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inCubicInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inCubicInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::inCubicInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseCubicInInterpolationFunction::create() const {
    return new TransFuncKeyWiseCubicInInterpolationFunction();
}

TransFuncKeyWiseCubicOutInterpolationFunction::TransFuncKeyWiseCubicOutInterpolationFunction() {}

std::string TransFuncKeyWiseCubicOutInterpolationFunction::getGuiName() const {
    return "easing out";
}

std::string TransFuncKeyWiseCubicOutInterpolationFunction::getCategory() const {
    return "keywise cubic";
}

TransFunc* TransFuncKeyWiseCubicOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::outCubicInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::outCubicInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::outCubicInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::outCubicInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outCubicInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outCubicInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outCubicInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outCubicInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outCubicInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outCubicInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outCubicInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outCubicInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::outCubicInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseCubicOutInterpolationFunction::create() const {
    return new TransFuncKeyWiseCubicOutInterpolationFunction();
}

TransFuncKeyWiseCubicInOutInterpolationFunction::TransFuncKeyWiseCubicInOutInterpolationFunction() {}

std::string TransFuncKeyWiseCubicInOutInterpolationFunction::getGuiName() const {
    return "first easing in, then easing out";
}

std::string TransFuncKeyWiseCubicInOutInterpolationFunction::getCategory() const {
    return "keywise cubic";
}

TransFunc* TransFuncKeyWiseCubicInOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::inOutCubicInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::inOutCubicInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::inOutCubicInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::inOutCubicInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inOutCubicInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inOutCubicInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inOutCubicInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inOutCubicInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inOutCubicInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inOutCubicInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inOutCubicInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inOutCubicInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::inOutCubicInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseCubicInOutInterpolationFunction::create() const {
    return new TransFuncKeyWiseCubicInOutInterpolationFunction();
}

TransFuncKeyWiseCubicOutInInterpolationFunction::TransFuncKeyWiseCubicOutInInterpolationFunction() {}

std::string TransFuncKeyWiseCubicOutInInterpolationFunction::getGuiName() const {
    return "first easing out, then easing in";
}

std::string TransFuncKeyWiseCubicOutInInterpolationFunction::getCategory() const {
    return "keywise cubic";
}

TransFunc* TransFuncKeyWiseCubicOutInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::outInCubicInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::outInCubicInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::outInCubicInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::outInCubicInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outInCubicInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outInCubicInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outInCubicInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outInCubicInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outInCubicInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outInCubicInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outInCubicInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outInCubicInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::outInCubicInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseCubicOutInInterpolationFunction::create() const {
    return new TransFuncKeyWiseCubicOutInInterpolationFunction();
}

TransFuncKeyWiseQuartInInterpolationFunction::TransFuncKeyWiseQuartInInterpolationFunction() {}

std::string TransFuncKeyWiseQuartInInterpolationFunction::getGuiName() const {
    return "easing in";
}

std::string TransFuncKeyWiseQuartInInterpolationFunction::getCategory() const {
    return "keywise quartetic";
}

TransFunc* TransFuncKeyWiseQuartInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::inQuartInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::inQuartInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::inQuartInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::inQuartInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inQuartInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inQuartInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inQuartInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inQuartInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inQuartInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inQuartInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inQuartInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inQuartInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::inQuartInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseQuartInInterpolationFunction::create() const {
    return new TransFuncKeyWiseQuartInInterpolationFunction();
}

TransFuncKeyWiseQuartOutInterpolationFunction::TransFuncKeyWiseQuartOutInterpolationFunction() {}

std::string TransFuncKeyWiseQuartOutInterpolationFunction::getGuiName() const {
    return "easing out";
}

std::string TransFuncKeyWiseQuartOutInterpolationFunction::getCategory() const {
    return "keywise quartetic";
}

TransFunc* TransFuncKeyWiseQuartOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::outQuartInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::outQuartInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::outQuartInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::outQuartInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outQuartInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outQuartInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outQuartInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outQuartInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outQuartInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outQuartInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outQuartInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outQuartInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::outQuartInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseQuartOutInterpolationFunction::create() const {
    return new TransFuncKeyWiseQuartOutInterpolationFunction();
}

TransFuncKeyWiseQuartInOutInterpolationFunction::TransFuncKeyWiseQuartInOutInterpolationFunction() {}

std::string TransFuncKeyWiseQuartInOutInterpolationFunction::getGuiName() const {
    return "first easing in, then easing out";
}

std::string TransFuncKeyWiseQuartInOutInterpolationFunction::getCategory() const {
    return "keywise quartetic";
}

TransFunc* TransFuncKeyWiseQuartInOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::inOutQuartInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::inOutQuartInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::inOutQuartInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::inOutQuartInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuartInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuartInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuartInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuartInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuartInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuartInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuartInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuartInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::inOutQuartInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseQuartInOutInterpolationFunction::create() const {
    return new TransFuncKeyWiseQuartInOutInterpolationFunction();
}

TransFuncKeyWiseQuartOutInInterpolationFunction::TransFuncKeyWiseQuartOutInInterpolationFunction() {}

std::string TransFuncKeyWiseQuartOutInInterpolationFunction::getGuiName() const {
    return "first easing out, then easing in";
}

std::string TransFuncKeyWiseQuartOutInInterpolationFunction::getCategory() const {
    return "keywise quartetic";
}

TransFunc* TransFuncKeyWiseQuartOutInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::outInQuartInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::outInQuartInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::outInQuartInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::outInQuartInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outInQuartInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outInQuartInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outInQuartInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outInQuartInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outInQuartInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outInQuartInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outInQuartInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outInQuartInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::outInQuartInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseQuartOutInInterpolationFunction::create() const {
    return new TransFuncKeyWiseQuartOutInInterpolationFunction();
}

TransFuncKeyWiseQuintInInterpolationFunction::TransFuncKeyWiseQuintInInterpolationFunction() {}

std::string TransFuncKeyWiseQuintInInterpolationFunction::getGuiName() const {
    return "easing in";
}

std::string TransFuncKeyWiseQuintInInterpolationFunction::getCategory() const {
    return "keywise quintic";
}

TransFunc* TransFuncKeyWiseQuintInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::inQuintInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::inQuintInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::inQuintInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::inQuintInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inQuintInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inQuintInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inQuintInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inQuintInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inQuintInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inQuintInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inQuintInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inQuintInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::inQuintInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseQuintInInterpolationFunction::create() const {
    return new TransFuncKeyWiseQuintInInterpolationFunction();
}

TransFuncKeyWiseQuintOutInterpolationFunction::TransFuncKeyWiseQuintOutInterpolationFunction() {}

std::string TransFuncKeyWiseQuintOutInterpolationFunction::getGuiName() const {
    return "easing out";
}

std::string TransFuncKeyWiseQuintOutInterpolationFunction::getCategory() const {
    return "keywise quintic";
}

TransFunc* TransFuncKeyWiseQuintOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::outQuintInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::outQuintInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::outQuintInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::outQuintInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outQuintInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outQuintInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outQuintInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outQuintInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outQuintInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outQuintInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outQuintInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outQuintInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::outQuintInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseQuintOutInterpolationFunction::create() const {
    return new TransFuncKeyWiseQuintOutInterpolationFunction();
}

TransFuncKeyWiseQuintInOutInterpolationFunction::TransFuncKeyWiseQuintInOutInterpolationFunction() {}

std::string TransFuncKeyWiseQuintInOutInterpolationFunction::getGuiName() const {
    return "first easing in, then easing out";
}

std::string TransFuncKeyWiseQuintInOutInterpolationFunction::getCategory() const {
    return "keywise quintic";
}

TransFunc* TransFuncKeyWiseQuintInOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::inOutQuintInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::inOutQuintInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::inOutQuintInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::inOutQuintInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuintInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuintInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuintInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuintInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuintInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuintInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuintInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inOutQuintInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::inOutQuintInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseQuintInOutInterpolationFunction::create() const {
    return new TransFuncKeyWiseQuintInOutInterpolationFunction();
}

TransFuncKeyWiseQuintOutInInterpolationFunction::TransFuncKeyWiseQuintOutInInterpolationFunction() {
}

std::string TransFuncKeyWiseQuintOutInInterpolationFunction::getGuiName() const {
    return "first easing out, then easing in";
}

std::string TransFuncKeyWiseQuintOutInInterpolationFunction::getCategory() const {
    return "keywise quintic";
}

TransFunc* TransFuncKeyWiseQuintOutInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::outInQuintInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::outInQuintInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::outInQuintInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::outInQuintInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outInQuintInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outInQuintInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outInQuintInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outInQuintInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outInQuintInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outInQuintInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outInQuintInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outInQuintInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::outInQuintInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseQuintOutInInterpolationFunction::create() const {
    return new TransFuncKeyWiseQuintOutInInterpolationFunction();
}

TransFuncKeyWiseSineInInterpolationFunction::TransFuncKeyWiseSineInInterpolationFunction() {}

std::string TransFuncKeyWiseSineInInterpolationFunction::getGuiName() const {
    return "easing in";
}

std::string TransFuncKeyWiseSineInInterpolationFunction::getCategory() const {
    return "keywise sineousidal";
}

TransFunc* TransFuncKeyWiseSineInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::inSineInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::inSineInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::inSineInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::inSineInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inSineInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inSineInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inSineInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inSineInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inSineInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inSineInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inSineInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inSineInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::inSineInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseSineInInterpolationFunction::create() const {
    return new TransFuncKeyWiseSineInInterpolationFunction();
}

TransFuncKeyWiseSineOutInterpolationFunction::TransFuncKeyWiseSineOutInterpolationFunction() {}

std::string TransFuncKeyWiseSineOutInterpolationFunction::getGuiName() const {
    return "easing out";
}

std::string TransFuncKeyWiseSineOutInterpolationFunction::getCategory() const {
    return "keywise sineousidal";
}

TransFunc* TransFuncKeyWiseSineOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::outSineInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::outSineInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::outSineInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::outSineInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outSineInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outSineInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outSineInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outSineInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outSineInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outSineInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outSineInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outSineInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::outSineInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseSineOutInterpolationFunction::create() const {
    return new TransFuncKeyWiseSineOutInterpolationFunction();
}

TransFuncKeyWiseSineInOutInterpolationFunction::TransFuncKeyWiseSineInOutInterpolationFunction() {}

std::string TransFuncKeyWiseSineInOutInterpolationFunction::getGuiName() const {
    return "first easing in, then easing out";
}

std::string TransFuncKeyWiseSineInOutInterpolationFunction::getCategory() const {
    return "keywise sineousidal";
}

TransFunc* TransFuncKeyWiseSineInOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::inOutSineInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::inOutSineInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::inOutSineInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::inOutSineInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inOutSineInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inOutSineInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inOutSineInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inOutSineInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inOutSineInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inOutSineInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inOutSineInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inOutSineInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::inOutSineInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseSineInOutInterpolationFunction::create() const {
    return new TransFuncKeyWiseSineInOutInterpolationFunction();
}

TransFuncKeyWiseSineOutInInterpolationFunction::TransFuncKeyWiseSineOutInInterpolationFunction() {}

std::string TransFuncKeyWiseSineOutInInterpolationFunction::getGuiName() const {
    return "first easing out, then easing in";
}

std::string TransFuncKeyWiseSineOutInInterpolationFunction::getCategory() const {
    return "keywise sineousidal";
}

TransFunc* TransFuncKeyWiseSineOutInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::outInSineInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::outInSineInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::outInSineInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::outInSineInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outInSineInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outInSineInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outInSineInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outInSineInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outInSineInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outInSineInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outInSineInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outInSineInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::outInSineInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseSineOutInInterpolationFunction::create() const {
    return new TransFuncKeyWiseSineOutInInterpolationFunction();
}

TransFuncKeyWiseExponentInInterpolationFunction::TransFuncKeyWiseExponentInInterpolationFunction() {}

std::string TransFuncKeyWiseExponentInInterpolationFunction::getGuiName() const {
    return "easing in";
}

std::string TransFuncKeyWiseExponentInInterpolationFunction::getCategory() const {
    return "keywise exponential";
}

TransFunc* TransFuncKeyWiseExponentInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::inExponentInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::inExponentInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::inExponentInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::inExponentInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inExponentInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inExponentInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inExponentInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inExponentInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inExponentInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inExponentInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inExponentInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inExponentInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::inExponentInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseExponentInInterpolationFunction::create() const {
    return new TransFuncKeyWiseExponentInInterpolationFunction();
}

TransFuncKeyWiseExponentOutInterpolationFunction::TransFuncKeyWiseExponentOutInterpolationFunction() {}

std::string TransFuncKeyWiseExponentOutInterpolationFunction::getGuiName() const {
    return "easing out";
}

std::string TransFuncKeyWiseExponentOutInterpolationFunction::getCategory() const {
    return "keywise exponential";
}

TransFunc* TransFuncKeyWiseExponentOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::outExponentInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::outExponentInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::outExponentInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::outExponentInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outExponentInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outExponentInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outExponentInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outExponentInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outExponentInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outExponentInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outExponentInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outExponentInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::outExponentInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseExponentOutInterpolationFunction::create() const {
    return new TransFuncKeyWiseExponentOutInterpolationFunction();
}

TransFuncKeyWiseExponentInOutInterpolationFunction::TransFuncKeyWiseExponentInOutInterpolationFunction() {}

std::string TransFuncKeyWiseExponentInOutInterpolationFunction::getGuiName() const {
    return "first easing in, then easing out";
}

std::string TransFuncKeyWiseExponentInOutInterpolationFunction::getCategory() const {
    return "keywise exponential";
}

TransFunc* TransFuncKeyWiseExponentInOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::inOutExponentInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::inOutExponentInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::inOutExponentInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::inOutExponentInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inOutExponentInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inOutExponentInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inOutExponentInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inOutExponentInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inOutExponentInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inOutExponentInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inOutExponentInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inOutExponentInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::inOutExponentInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseExponentInOutInterpolationFunction::create() const {
    return new TransFuncKeyWiseExponentInOutInterpolationFunction();
}

TransFuncKeyWiseExponentOutInInterpolationFunction::TransFuncKeyWiseExponentOutInInterpolationFunction() {}

std::string TransFuncKeyWiseExponentOutInInterpolationFunction::getGuiName() const {
    return "first easing out, then easing in";
}

std::string TransFuncKeyWiseExponentOutInInterpolationFunction::getCategory() const {
    return "keywise exponential";
}

TransFunc* TransFuncKeyWiseExponentOutInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::outInExponentInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::outInExponentInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::outInExponentInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::outInExponentInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outInExponentInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outInExponentInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outInExponentInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outInExponentInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outInExponentInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outInExponentInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outInExponentInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outInExponentInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::outInExponentInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseExponentOutInInterpolationFunction::create() const {
    return new TransFuncKeyWiseExponentOutInInterpolationFunction();
}

TransFuncKeyWiseCircInInterpolationFunction::TransFuncKeyWiseCircInInterpolationFunction() {}

std::string TransFuncKeyWiseCircInInterpolationFunction::getGuiName() const {
    return "easing in";
}

std::string TransFuncKeyWiseCircInInterpolationFunction::getCategory() const {
    return "keywise circular";
}

TransFunc* TransFuncKeyWiseCircInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::inCircInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::inCircInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::inCircInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::inCircInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inCircInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inCircInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inCircInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inCircInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inCircInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inCircInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inCircInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inCircInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::inCircInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseCircInInterpolationFunction::create() const {
    return new TransFuncKeyWiseCircInInterpolationFunction();
}

TransFuncKeyWiseCircOutInterpolationFunction::TransFuncKeyWiseCircOutInterpolationFunction() {}

std::string TransFuncKeyWiseCircOutInterpolationFunction::getGuiName() const {
    return "easing out";
}

std::string TransFuncKeyWiseCircOutInterpolationFunction::getCategory() const {
    return "keywise circular";
}

TransFunc* TransFuncKeyWiseCircOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::outCircInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::outCircInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::outCircInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::outCircInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outCircInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outCircInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outCircInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outCircInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outCircInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outCircInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outCircInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outCircInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::outCircInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseCircOutInterpolationFunction::create() const {
    return new TransFuncKeyWiseCircOutInterpolationFunction();
}

TransFuncKeyWiseCircInOutInterpolationFunction::TransFuncKeyWiseCircInOutInterpolationFunction() {}

std::string TransFuncKeyWiseCircInOutInterpolationFunction::getGuiName() const {
    return "first easing in, then easing out";
}

std::string TransFuncKeyWiseCircInOutInterpolationFunction::getCategory() const {
    return "keywise circular";
}

TransFunc* TransFuncKeyWiseCircInOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::inOutCircInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::inOutCircInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::inOutCircInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::inOutCircInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inOutCircInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inOutCircInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inOutCircInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inOutCircInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::inOutCircInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::inOutCircInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::inOutCircInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::inOutCircInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::inOutCircInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseCircInOutInterpolationFunction::create() const {
    return new TransFuncKeyWiseCircInOutInterpolationFunction();
}

TransFuncKeyWiseCircOutInInterpolationFunction::TransFuncKeyWiseCircOutInInterpolationFunction() {}

std::string TransFuncKeyWiseCircOutInInterpolationFunction::getGuiName() const {
    return "first easing out, then easing in";
}

std::string TransFuncKeyWiseCircOutInInterpolationFunction::getCategory() const {
    return "keywise circular";
}

TransFunc* TransFuncKeyWiseCircOutInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    TransFunc1DKeys* func1 = dynamic_cast<TransFunc1DKeys*>(startvalue);
    TransFunc1DKeys* func2 = dynamic_cast<TransFunc1DKeys*>(endvalue);
    if (func1 && func2) {
        std::vector<TransFuncMappingKey*> keys1 = func1->getKeys();
        std::vector<TransFuncMappingKey*> keys2 = func2->getKeys();
        if (keys1.size() == keys2.size()) {
            TransFunc1DKeys* func = new TransFunc1DKeys();

            tgt::vec2 t1 = func1->getThresholds();
            tgt::vec2 t2 = func2->getThresholds();

            func->setThresholds(BasicFloatInterpolation::outInCircInterpolation(t1.x,t2.x,time),
                                BasicFloatInterpolation::outInCircInterpolation(t1.y,t2.y,time));

            tgt::vec2 d1 = func1->getDomain();
            tgt::vec2 d2 = func2->getDomain();
            if(d1 != d2)
                LWARNING("Transfer functions have different domains...interpolation is (probably) incorrect.");

            func->setDomain(BasicFloatInterpolation::outInCircInterpolation(d1.x,d2.x,time),
                            BasicFloatInterpolation::outInCircInterpolation(d1.y,d2.y,time), 0);

            func->clearKeys();
            std::vector<TransFuncMappingKey*>::iterator it1 = keys1.begin();
            std::vector<TransFuncMappingKey*>::iterator it2 = keys2.begin();
            while ((it1 != keys1.end()) && (it2 != keys2.end())) {
                tgt::col4 col = tgt::col4();
                TransFuncMappingKey* key = new TransFuncMappingKey(0, col);
                key->setSplit((*it1)->isSplit()||(*it2)->isSplit(), true);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outInCircInterpolation((*it1)->getColorL().r,(*it2)->getColorL().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outInCircInterpolation((*it1)->getColorL().g,(*it2)->getColorL().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outInCircInterpolation((*it1)->getColorL().b,(*it2)->getColorL().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outInCircInterpolation((*it1)->getColorL().a,(*it2)->getColorL().a,time));
                key->setColorL(col);

                col.r = static_cast<uint8_t>(BasicFloatInterpolation::outInCircInterpolation((*it1)->getColorR().r,(*it2)->getColorR().r,time));
                col.g = static_cast<uint8_t>(BasicFloatInterpolation::outInCircInterpolation((*it1)->getColorR().g,(*it2)->getColorR().g,time));
                col.b = static_cast<uint8_t>(BasicFloatInterpolation::outInCircInterpolation((*it1)->getColorR().b,(*it2)->getColorR().b,time));
                col.a = static_cast<uint8_t>(BasicFloatInterpolation::outInCircInterpolation((*it1)->getColorR().a,(*it2)->getColorR().a,time));
                key->setColorR(col);

                key->setIntensity(BasicFloatInterpolation::outInCircInterpolation((*it1)->getIntensity(),(*it2)->getIntensity(),time));

                func->addKey(key);

                it1++;
                it2++;
            }
            func->invalidateTexture();
            return func;
        }
    }
    if (time < 1.f)
        return startvalue->create();
    else
        return endvalue->create();
}

InterpolationFunction<TransFunc*>* TransFuncKeyWiseCircOutInInterpolationFunction::create() const {
    return new TransFuncKeyWiseCircOutInInterpolationFunction();
}

TransFuncTextureLinearInterpolationFunction::TransFuncTextureLinearInterpolationFunction() {}

std::string TransFuncTextureLinearInterpolationFunction::getGuiName() const {
    return "texturebased linear";
}

std::string TransFuncTextureLinearInterpolationFunction::getCategory() const {
    return "texture linear";
}

TransFunc* TransFuncTextureLinearInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::linearInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureLinearInterpolationFunction::create() const {
    return new TransFuncTextureLinearInterpolationFunction();
}

TransFuncTextureQuadInInterpolationFunction::TransFuncTextureQuadInInterpolationFunction() {}

std::string TransFuncTextureQuadInInterpolationFunction::getGuiName() const {
    return "easing in";
}

std::string TransFuncTextureQuadInInterpolationFunction::getCategory() const {
    return "texture quadratic";
}

TransFunc* TransFuncTextureQuadInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::inQuadInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureQuadInInterpolationFunction::create() const {
    return new TransFuncTextureQuadInInterpolationFunction();
}

TransFuncTextureQuadOutInterpolationFunction::TransFuncTextureQuadOutInterpolationFunction() {}

std::string TransFuncTextureQuadOutInterpolationFunction::getGuiName() const {
    return "easing out";
}

std::string TransFuncTextureQuadOutInterpolationFunction::getCategory() const {
    return "texture quadratic";
}

TransFunc* TransFuncTextureQuadOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::outQuadInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureQuadOutInterpolationFunction::create() const {
    return new TransFuncTextureQuadOutInterpolationFunction();
}

TransFuncTextureQuadInOutInterpolationFunction::TransFuncTextureQuadInOutInterpolationFunction() {}

std::string TransFuncTextureQuadInOutInterpolationFunction::getGuiName() const {
    return "first easing in, then easing out";
}

std::string TransFuncTextureQuadInOutInterpolationFunction::getCategory() const {
    return "texture quadratic";
}

TransFunc* TransFuncTextureQuadInOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::inOutQuadInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureQuadInOutInterpolationFunction::create() const {
    return new TransFuncTextureQuadInOutInterpolationFunction();
}

TransFuncTextureQuadOutInInterpolationFunction::TransFuncTextureQuadOutInInterpolationFunction() {}

std::string TransFuncTextureQuadOutInInterpolationFunction::getGuiName() const {
    return "first easing out, then easing in";
}

std::string TransFuncTextureQuadOutInInterpolationFunction::getCategory() const {
    return "texture quadratic";
}

TransFunc* TransFuncTextureQuadOutInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::outInQuadInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureQuadOutInInterpolationFunction::create() const {
    return new TransFuncTextureQuadOutInInterpolationFunction();
}

TransFuncTextureCubicInInterpolationFunction::TransFuncTextureCubicInInterpolationFunction() {}

std::string TransFuncTextureCubicInInterpolationFunction::getGuiName() const {
    return "easing in";
}

std::string TransFuncTextureCubicInInterpolationFunction::getCategory() const {
    return "texture cubic";
}

TransFunc* TransFuncTextureCubicInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::inCubicInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureCubicInInterpolationFunction::create() const {
    return new TransFuncTextureCubicInInterpolationFunction();
}

TransFuncTextureCubicOutInterpolationFunction::TransFuncTextureCubicOutInterpolationFunction() {}

std::string TransFuncTextureCubicOutInterpolationFunction::getGuiName() const {
    return "easing out";
}

std::string TransFuncTextureCubicOutInterpolationFunction::getCategory() const {
    return "texture cubic";
}

TransFunc* TransFuncTextureCubicOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::outCubicInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureCubicOutInterpolationFunction::create() const {
    return new TransFuncTextureCubicOutInterpolationFunction();
}

TransFuncTextureCubicInOutInterpolationFunction::TransFuncTextureCubicInOutInterpolationFunction() {}

std::string TransFuncTextureCubicInOutInterpolationFunction::getGuiName() const {
    return "first easing in, then easing out";
}

std::string TransFuncTextureCubicInOutInterpolationFunction::getCategory() const {
    return "texture cubic";
}

TransFunc* TransFuncTextureCubicInOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::inOutCubicInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureCubicInOutInterpolationFunction::create() const {
    return new TransFuncTextureCubicInOutInterpolationFunction();
}

TransFuncTextureCubicOutInInterpolationFunction::TransFuncTextureCubicOutInInterpolationFunction() {}

std::string TransFuncTextureCubicOutInInterpolationFunction::getGuiName() const {
    return "first easing out, then easing in";
}

std::string TransFuncTextureCubicOutInInterpolationFunction::getCategory() const {
    return "texture cubic";
}

TransFunc* TransFuncTextureCubicOutInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::outInCubicInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureCubicOutInInterpolationFunction::create() const {
    return new TransFuncTextureCubicOutInInterpolationFunction();
}

TransFuncTextureQuartInInterpolationFunction::TransFuncTextureQuartInInterpolationFunction() {}

std::string TransFuncTextureQuartInInterpolationFunction::getGuiName() const {
    return "easing in";
}

std::string TransFuncTextureQuartInInterpolationFunction::getCategory() const {
    return "texture quartetic";
}

TransFunc* TransFuncTextureQuartInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::inQuartInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureQuartInInterpolationFunction::create() const {
    return new TransFuncTextureQuartInInterpolationFunction();
}

TransFuncTextureQuartOutInterpolationFunction::TransFuncTextureQuartOutInterpolationFunction() {}

std::string TransFuncTextureQuartOutInterpolationFunction::getGuiName() const {
    return "easing out";
}

std::string TransFuncTextureQuartOutInterpolationFunction::getCategory() const {
    return "texture quartetic";
}

TransFunc* TransFuncTextureQuartOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::outQuartInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureQuartOutInterpolationFunction::create() const {
    return new TransFuncTextureQuartOutInterpolationFunction();
}

TransFuncTextureQuartInOutInterpolationFunction::TransFuncTextureQuartInOutInterpolationFunction() {}

std::string TransFuncTextureQuartInOutInterpolationFunction::getGuiName() const {
    return "first easing in, then easing out";
}

std::string TransFuncTextureQuartInOutInterpolationFunction::getCategory() const {
    return "texture quartetic";
}

TransFunc* TransFuncTextureQuartInOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::inOutQuartInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureQuartInOutInterpolationFunction::create() const {
    return new TransFuncTextureQuartInOutInterpolationFunction();
}

TransFuncTextureQuartOutInInterpolationFunction::TransFuncTextureQuartOutInInterpolationFunction() {}

std::string TransFuncTextureQuartOutInInterpolationFunction::getGuiName() const {
    return "first easing out, then easing in";
}

std::string TransFuncTextureQuartOutInInterpolationFunction::getCategory() const {
    return "texture quartetic";
}

TransFunc* TransFuncTextureQuartOutInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::outInQuartInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureQuartOutInInterpolationFunction::create() const {
    return new TransFuncTextureQuartOutInInterpolationFunction();
}

TransFuncTextureQuintInInterpolationFunction::TransFuncTextureQuintInInterpolationFunction() {}

std::string TransFuncTextureQuintInInterpolationFunction::getGuiName() const {
    return "easing in";
}

std::string TransFuncTextureQuintInInterpolationFunction::getCategory() const {
    return "texture quintic";
}

TransFunc* TransFuncTextureQuintInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::inQuintInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureQuintInInterpolationFunction::create() const {
    return new TransFuncTextureQuintInInterpolationFunction();
}

TransFuncTextureQuintOutInterpolationFunction::TransFuncTextureQuintOutInterpolationFunction() {}

std::string TransFuncTextureQuintOutInterpolationFunction::getGuiName() const {
    return "easing out";
}

std::string TransFuncTextureQuintOutInterpolationFunction::getCategory() const {
    return "texture quintic";
}

TransFunc* TransFuncTextureQuintOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::outQuintInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureQuintOutInterpolationFunction::create() const {
    return new TransFuncTextureQuintOutInterpolationFunction();
}

TransFuncTextureQuintInOutInterpolationFunction::TransFuncTextureQuintInOutInterpolationFunction() {}

std::string TransFuncTextureQuintInOutInterpolationFunction::getGuiName() const {
    return "first easing in, then easing out";
}

std::string TransFuncTextureQuintInOutInterpolationFunction::getCategory() const {
    return "texture quintic";
}

TransFunc* TransFuncTextureQuintInOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::inOutQuintInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureQuintInOutInterpolationFunction::create() const {
    return new TransFuncTextureQuintInOutInterpolationFunction();
}

TransFuncTextureQuintOutInInterpolationFunction::TransFuncTextureQuintOutInInterpolationFunction() {}

std::string TransFuncTextureQuintOutInInterpolationFunction::getGuiName() const {
    return "first easing out, then easing in";
}

std::string TransFuncTextureQuintOutInInterpolationFunction::getCategory() const {
    return "texture quintic";
}

TransFunc* TransFuncTextureQuintOutInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::outInQuintInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureQuintOutInInterpolationFunction::create() const {
    return new TransFuncTextureQuintOutInInterpolationFunction();
}

TransFuncTextureSineInInterpolationFunction::TransFuncTextureSineInInterpolationFunction() {}

std::string TransFuncTextureSineInInterpolationFunction::getGuiName() const {
    return "easing in";
}

std::string TransFuncTextureSineInInterpolationFunction::getCategory() const {
    return "texture sineousidal";
}

TransFunc* TransFuncTextureSineInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::inSineInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureSineInInterpolationFunction::create() const {
    return new TransFuncTextureSineInInterpolationFunction();
}

TransFuncTextureSineOutInterpolationFunction::TransFuncTextureSineOutInterpolationFunction() {}

std::string TransFuncTextureSineOutInterpolationFunction::getGuiName() const {
    return "easing out";
}

std::string TransFuncTextureSineOutInterpolationFunction::getCategory() const {
    return "texture sineousidal";
}

TransFunc* TransFuncTextureSineOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::outSineInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureSineOutInterpolationFunction::create() const {
    return new TransFuncTextureSineOutInterpolationFunction();
}

TransFuncTextureSineInOutInterpolationFunction::TransFuncTextureSineInOutInterpolationFunction() {}

std::string TransFuncTextureSineInOutInterpolationFunction::getGuiName() const {
    return "first easing in, then easing out";
}

std::string TransFuncTextureSineInOutInterpolationFunction::getCategory() const {
    return "texture sineousidal";
}
TransFunc* TransFuncTextureSineInOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::inOutSineInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureSineInOutInterpolationFunction::create() const {
    return new TransFuncTextureSineInOutInterpolationFunction();
}

TransFuncTextureSineOutInInterpolationFunction::TransFuncTextureSineOutInInterpolationFunction() {}

std::string TransFuncTextureSineOutInInterpolationFunction::getGuiName() const {
    return "first easing out, then easing in";
}

std::string TransFuncTextureSineOutInInterpolationFunction::getCategory() const {
    return "texture sineousidal";
}

TransFunc* TransFuncTextureSineOutInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::outInSineInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureSineOutInInterpolationFunction::create() const {
    return new TransFuncTextureSineOutInInterpolationFunction();
}

TransFuncTextureExponentInInterpolationFunction::TransFuncTextureExponentInInterpolationFunction() {}

std::string TransFuncTextureExponentInInterpolationFunction::getGuiName() const {
    return "easing in";
}

std::string TransFuncTextureExponentInInterpolationFunction::getCategory() const {
    return "texture exponential";
}

TransFunc* TransFuncTextureExponentInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::inExponentInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureExponentInInterpolationFunction::create() const {
    return new TransFuncTextureExponentInInterpolationFunction();
}

TransFuncTextureExponentOutInterpolationFunction::TransFuncTextureExponentOutInterpolationFunction() {}

std::string TransFuncTextureExponentOutInterpolationFunction::getGuiName() const {
    return "easing out";
}

std::string TransFuncTextureExponentOutInterpolationFunction::getCategory() const {
    return "texture exponential";
}

TransFunc* TransFuncTextureExponentOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::outExponentInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureExponentOutInterpolationFunction::create() const {
    return new TransFuncTextureExponentOutInterpolationFunction();
}

TransFuncTextureExponentInOutInterpolationFunction::TransFuncTextureExponentInOutInterpolationFunction() {}

std::string TransFuncTextureExponentInOutInterpolationFunction::getGuiName() const {
    return "first easing in, then easing out";
}

std::string TransFuncTextureExponentInOutInterpolationFunction::getCategory() const {
    return "texture exponential";
}

TransFunc* TransFuncTextureExponentInOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::inOutExponentInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureExponentInOutInterpolationFunction::create() const {
    return new TransFuncTextureExponentInOutInterpolationFunction();
}

TransFuncTextureExponentOutInInterpolationFunction::TransFuncTextureExponentOutInInterpolationFunction() {}

std::string TransFuncTextureExponentOutInInterpolationFunction::getGuiName() const {
    return "first easing out, then easing in";
}

std::string TransFuncTextureExponentOutInInterpolationFunction::getCategory() const {
    return "texture exponential";
}

TransFunc* TransFuncTextureExponentOutInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::outInExponentInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureExponentOutInInterpolationFunction::create() const {
    return new TransFuncTextureExponentOutInInterpolationFunction();
}

TransFuncTextureCircInInterpolationFunction::TransFuncTextureCircInInterpolationFunction() {}

std::string TransFuncTextureCircInInterpolationFunction::getGuiName() const {
    return "easing in";
}

std::string TransFuncTextureCircInInterpolationFunction::getCategory() const {
    return "texture circular";
}

TransFunc* TransFuncTextureCircInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::inCircInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureCircInInterpolationFunction::create() const {
    return new TransFuncTextureCircInInterpolationFunction();
}

TransFuncTextureCircOutInterpolationFunction::TransFuncTextureCircOutInterpolationFunction() {}

std::string TransFuncTextureCircOutInterpolationFunction::getGuiName() const {
    return "easing out";
}

std::string TransFuncTextureCircOutInterpolationFunction::getCategory() const {
    return "texture circular";
}

TransFunc* TransFuncTextureCircOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::outCircInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureCircOutInterpolationFunction::create() const {
    return new TransFuncTextureCircOutInterpolationFunction();
}

TransFuncTextureCircInOutInterpolationFunction::TransFuncTextureCircInOutInterpolationFunction() {}

std::string TransFuncTextureCircInOutInterpolationFunction::getGuiName() const {
    return "first easing in, then easing out";
}

std::string TransFuncTextureCircInOutInterpolationFunction::getCategory() const {
    return "texture circular";
}

TransFunc* TransFuncTextureCircInOutInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::inOutCircInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureCircInOutInterpolationFunction::create() const {
    return new TransFuncTextureCircInOutInterpolationFunction();
}

TransFuncTextureCircOutInInterpolationFunction::TransFuncTextureCircOutInInterpolationFunction() {}

std::string TransFuncTextureCircOutInInterpolationFunction::getGuiName() const {
    return "first easing out, then easing in";
}

std::string TransFuncTextureCircOutInInterpolationFunction::getCategory() const {
    return "texture circular";
}

TransFunc* TransFuncTextureCircOutInInterpolationFunction::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
    float a2 = BasicFloatInterpolation::outInCircInterpolation(0, 1, time);
    float a1 = 1-a2;

    // new dimensions: maxima of each dimension (x,y,z)
    tgt::ivec3 dimensions_start = startvalue->getDimensions();
    tgt::ivec3 dimensions_end = endvalue->getDimensions();
    tgt::ivec3 dim = tgt::ivec3();
    dim.x = std::max(dimensions_start.x,dimensions_end.x);
    dim.y = std::max(dimensions_start.y,dimensions_end.y);
    dim.z = std::max(dimensions_start.z,dimensions_end.z);

    GLubyte* texture1 = startvalue->getPixelData();
    GLubyte* texture2 = endvalue->getPixelData();
    texture1 = convertTextureToRGBA(startvalue->getDimensions(), texture1 , startvalue->getFormat());
    texture2 = convertTextureToRGBA(endvalue->getDimensions(), texture2 , endvalue->getFormat());
    texture1 = changeTextureDimension(startvalue->getDimensions(), dim, texture1);
    texture2 = changeTextureDimension(startvalue->getDimensions(), dim, texture2);

    TransFunc* func = new TransFunc(dim.x,dim.y,dim.z);

    tgt::vec2 d1 = startvalue->getDomain();
    tgt::vec2 d2 = endvalue->getDomain();
    func->setDomain(BasicFloatInterpolation::outQuadInterpolation(d1.x,d2.x,time),
                    BasicFloatInterpolation::outQuadInterpolation(d1.y,d2.y,time), 0);

    GLubyte* texture = new GLubyte[4*dim.x*dim.y*dim.z];
    for (int x = 0; x < dim.x; ++x) {
        for (int y = 0; y < dim.y; ++y) {
            for (int z = 0; z < dim.z; ++z) {
                for (int i = 0; i < 4; ++i) {
                    float f = (a1*texture1[4*(x*dim.y*dim.z+y*dim.z+z)+i]+a2*texture2[4*(x*dim.y*dim.z+y*dim.z+z)+i]);
                    GLubyte b = static_cast<GLubyte>(f);
                    texture[4*(x*dim.y*dim.z+y*dim.z+z)+i] = b;
                }
            }
        }
    }
    func->setPixelData(texture);
    return func;
}

InterpolationFunction<TransFunc*>* TransFuncTextureCircOutInInterpolationFunction::create() const {
    return new TransFuncTextureCircOutInInterpolationFunction();
}

} // namespace voreen
