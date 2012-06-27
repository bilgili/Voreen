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

#include "voreen/core/animation/interpolationfunction.h"
#include "voreen/core/properties/property.h"
#include "voreen/core/properties/shaderproperty.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/volumeurllistproperty.h"
#include "voreen/core/animation/interpolation/basicintinterpolation.h"
#include "voreen/core/animation/interpolation/basicfloatinterpolation.h"
#include "voreen/core/animation/interpolation/camerainterpolationfunctions.h"
#include "voreen/core/animation/interpolation/vec3interpolationfunctions.h"
#include "voreen/core/animation/interpolation/transfuncinterpolationfunctions.h"
#include "voreen/core/datastructures/transfunc/transfuncmappingkey.h"
#include "voreen/core/datastructures/transfunc/transfuncprimitive.h"
#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"

#include "tgt/camera.h"
#include "tgt/quaternion.h"

using tgt::Camera;

namespace voreen {

//template <class T>
//InterpolationFunction<T>::InterpolationFunction() {
//}
//
//template <class T>
//InterpolationFunction<T>::~InterpolationFunction() {}

//template <class T>
//std::string InterpolationFunction<T>::getName() const {
//    return getIdentifier() + ": " + getMode();
//}

template <>
std::string InterpolationFunction<int>::getMode() const {
    return "linear interpolation";
}

template <>
std::string InterpolationFunction<float>::getMode() const {
    return "linear interpolation";
}

template <>
std::string InterpolationFunction<bool>::getMode() const {
    return "focus on startvalue";
}

template <>
std::string InterpolationFunction<tgt::vec2>::getMode() const {
    return "linear interpolation";
}

template <>
std::string InterpolationFunction<tgt::vec3>::getMode() const {
    return "linear interpolation";
}

template <>
std::string InterpolationFunction<tgt::vec4>::getMode() const {
    return "linear interpolation";
}

template <>
std::string InterpolationFunction<tgt::quat>::getMode() const {
    return "spherical linear interpolation";
}

template <>
std::string InterpolationFunction<tgt::mat2>::getMode() const {
    return "linear interpolation";
}

template <>
std::string InterpolationFunction<tgt::mat3>::getMode() const {
    return "linear interpolation";
}

template <>
std::string InterpolationFunction<tgt::mat4>::getMode() const {
    return "linear interpolation";
}

template <>
std::string InterpolationFunction<tgt::ivec2>::getMode() const {
    return "linear interpolation";
}

template <>
std::string InterpolationFunction<tgt::ivec3>::getMode() const {
    return "linear interpolation";
}

template <>
std::string InterpolationFunction<tgt::ivec4>::getMode() const {
    return "linear interpolation";
}

template <>
std::string InterpolationFunction<tgt::Camera>::getMode() const {
    return "linear interpolation";
}

template <>
std::string InterpolationFunction<std::string>::getMode() const {
    return "focus on startvalue";
}

template <>
std::string InterpolationFunction<ShaderSource>::getMode() const {
    return "focus on startvalue";
}

template <>
std::string InterpolationFunction<TransFunc*>::getMode() const {
    return "linear interpolation";
}

template <>
std::string InterpolationFunction<int>::getIdentifier() const {
    return "default linear";
}

template <>
std::string InterpolationFunction<float>::getIdentifier() const {
    return "default linear";
}

template <>
std::string InterpolationFunction<bool>::getIdentifier() const {
    return "default boolean";
}

template <>
std::string InterpolationFunction<tgt::vec2>::getIdentifier() const {
    return "default linear";
}

template <>
std::string InterpolationFunction<tgt::vec3>::getIdentifier() const {
    return "default linear";
}

template <>
std::string InterpolationFunction<tgt::vec4>::getIdentifier() const {
    return "default linear";
}

template <>
std::string InterpolationFunction<tgt::quat>::getIdentifier() const {
    return "linear";
}

template <>
std::string InterpolationFunction<tgt::mat2>::getIdentifier() const {
    return "default linear";
}

template <>
std::string InterpolationFunction<tgt::mat3>::getIdentifier() const {
    return "default linear";
}

template <>
std::string InterpolationFunction<tgt::mat4>::getIdentifier() const {
    return "default linear";
}

template <>
std::string InterpolationFunction<tgt::ivec2>::getIdentifier() const {
    return "default linear";
}

template <>
std::string InterpolationFunction<tgt::ivec3>::getIdentifier() const {
    return "default linear";
}

template <>
std::string InterpolationFunction<tgt::ivec4>::getIdentifier() const {
    return "default linear";
}

template <>
std::string InterpolationFunction<tgt::Camera>::getIdentifier() const {
    return "default linear";
}

template <>
std::string InterpolationFunction<std::string>::getIdentifier() const {
    return "default boolean";
}

template <>
std::string InterpolationFunction<ShaderSource>::getIdentifier() const {
    return "default boolean";
}

template <>
std::string InterpolationFunction<TransFunc*>::getIdentifier() const {
    return "default linear (keywise if possible)";
}

//template <class T>
//InterpolationFunction<T>* InterpolationFunction<T>::clone() const{
//    return new InterpolationFunction<T>();
//}

template <>
int InterpolationFunction<int>::interpolate(int startvalue, int endvalue, float time) const {
    return BasicIntInterpolation::linearInterpolation(static_cast<float>(startvalue), static_cast<float>(endvalue), time);
}

template <>
float InterpolationFunction<float>::interpolate(float startvalue, float endvalue, float time) const {
    return BasicFloatInterpolation::linearInterpolation(startvalue, endvalue, time);
}

template <>
tgt::vec2 InterpolationFunction<tgt::vec2>::interpolate(tgt::vec2 startvalue, tgt::vec2 endvalue, float time) const {
    float compX = BasicFloatInterpolation::linearInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::linearInterpolation(startvalue.y, endvalue.y, time);
    return tgt::vec2(compX, compY);
}

template <>
tgt::vec3 InterpolationFunction<tgt::vec3>::interpolate(tgt::vec3 startvalue, tgt::vec3 endvalue, float time) const {
    float compX = BasicFloatInterpolation::linearInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::linearInterpolation(startvalue.y, endvalue.y, time);
    float compZ = BasicFloatInterpolation::linearInterpolation(startvalue.z, endvalue.z, time);
    return tgt::vec3(compX, compY, compZ);
}

template <>
tgt::vec4 InterpolationFunction<tgt::vec4>::interpolate(tgt::vec4 startvalue, tgt::vec4 endvalue, float time) const {
    float compX = BasicFloatInterpolation::linearInterpolation(startvalue.x, endvalue.x, time);
    float compY = BasicFloatInterpolation::linearInterpolation(startvalue.y, endvalue.y, time);
    float compZ = BasicFloatInterpolation::linearInterpolation(startvalue.z, endvalue.z, time);
    float compA = BasicFloatInterpolation::linearInterpolation(startvalue.a, endvalue.a, time);
    return tgt::vec4(compX, compY, compZ, compA);
}

template <>
tgt::quat InterpolationFunction<tgt::quat>::interpolate(tgt::quat startvalue, tgt::quat endvalue, float time) const {
    return slerpQuat(startvalue, endvalue, time);
}

template <>
tgt::mat2 InterpolationFunction<tgt::mat2>::interpolate(tgt::mat2 startvalue, tgt::mat2 endvalue, float time) const {
    tgt::mat2 result = tgt::Matrix2<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::linearInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

template <>
tgt::mat3 InterpolationFunction<tgt::mat3>::interpolate(tgt::mat3 startvalue, tgt::mat3 endvalue, float time) const {
    tgt::mat3 result = tgt::Matrix3<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::linearInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

template <>
tgt::mat4 InterpolationFunction<tgt::mat4>::interpolate(tgt::mat4 startvalue, tgt::mat4 endvalue, float time) const {
    tgt::mat4 result = tgt::Matrix4<float>();
    for (int i = 0; i < result.size; ++i) {
        result.elem[i] = BasicFloatInterpolation::linearInterpolation(startvalue.elem[i], endvalue.elem[i], time);
    }
    return result;
}

template <>
tgt::ivec2 InterpolationFunction<tgt::ivec2>::interpolate(tgt::ivec2 startvalue, tgt::ivec2 endvalue, float time) const {
    int compX = BasicIntInterpolation::linearInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::linearInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    return tgt::ivec2(compX, compY);
}

template <>
tgt::ivec3 InterpolationFunction<tgt::ivec3>::interpolate(tgt::ivec3 startvalue, tgt::ivec3 endvalue, float time) const {
    int compX = BasicIntInterpolation::linearInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::linearInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::linearInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    return tgt::ivec3(compX, compY, compZ);
}

template <>
tgt::ivec4 InterpolationFunction<tgt::ivec4>::interpolate(tgt::ivec4 startvalue, tgt::ivec4 endvalue, float time) const {
    int compX = BasicIntInterpolation::linearInterpolation(static_cast<float>(startvalue.x), static_cast<float>(endvalue.x), time);
    int compY = BasicIntInterpolation::linearInterpolation(static_cast<float>(startvalue.y), static_cast<float>(endvalue.y), time);
    int compZ = BasicIntInterpolation::linearInterpolation(static_cast<float>(startvalue.z), static_cast<float>(endvalue.z), time);
    int compA = BasicIntInterpolation::linearInterpolation(static_cast<float>(startvalue.a), static_cast<float>(endvalue.a), time);
    return tgt::ivec4(compX, compY, compZ, compA);
}

template <>
bool InterpolationFunction<bool>::interpolate(bool startvalue, bool endvalue, float time) const {
    if (time < 1)
        return startvalue;
    else
        return endvalue;
}

template <>
Camera InterpolationFunction<Camera>::interpolate(Camera startvalue, Camera endvalue, float time) const {
            Vec3LinearInterpolationFunction* intfunc = new Vec3LinearInterpolationFunction();
            Vec3SphericalLinearInterpolationFunction* intfunc2 = new Vec3SphericalLinearInterpolationFunction();
            tgt::vec3 posvec = intfunc->interpolate(startvalue.getPosition(), endvalue.getPosition(), time);
            tgt::vec3 focvec = intfunc->interpolate(startvalue.getFocus(), endvalue.getFocus(), time);
            tgt::vec3 upvec = normalize(intfunc2->interpolate(startvalue.getUpVector(), endvalue.getUpVector(), time));
/*            tgt::vec3 direction = intfunc2->interpolate(startvalue->getDirection(), endvalue->getDirection(), time);
            return new CameraNode(posvec, focvec, upvec, direction); */
            Camera cam(startvalue);
            cam.positionCamera(posvec, focvec, upvec);
            return cam;
}

template <>
std::string InterpolationFunction<std::string>::interpolate(std::string startvalue, std::string endvalue, float time) const {
    if (time < 1)
        return startvalue;
    else
        return endvalue;
}

template <>
ShaderSource InterpolationFunction<ShaderSource>::interpolate(ShaderSource startvalue, ShaderSource endvalue, float time) const {
    if (time<1){
        return startvalue;
    }
    else{
        return endvalue;
    }
}

template <>
TransFunc* InterpolationFunction<TransFunc*>::interpolate(TransFunc* startvalue, TransFunc* endvalue, float time) const {
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

//template <class T>
//void InterpolationFunction<T>::serialize(XmlSerializer& s) const {
//    s.serialize("properties", this->getProperties());
//}
//
//template <class T>
//void InterpolationFunction<T>::deserialize(XmlDeserializer& s) {
//    std::vector<Property*> props;
//    s.deserialize("properties", props);
//    for (size_t i = 0; i < props.size(); ++i)
//        addProperty(props.at(i));
//}

//template class InterpolationFunction<float>;
//template class InterpolationFunction<int>;
//template class InterpolationFunction<bool>;
//template class InterpolationFunction<tgt::ivec2>;
//template class InterpolationFunction<tgt::ivec3>;
//template class InterpolationFunction<tgt::ivec4>;
//template class InterpolationFunction<tgt::vec2>;
//template class InterpolationFunction<tgt::vec3>;
//template class InterpolationFunction<tgt::vec4>;
//template class InterpolationFunction<tgt::quat>;
//template class InterpolationFunction<tgt::mat2>;
//template class InterpolationFunction<tgt::mat3>;
//template class InterpolationFunction<tgt::mat4>;
//template class InterpolationFunction<tgt::Camera>;
//template class InterpolationFunction<std::string>;
//template class InterpolationFunction<ShaderSource>;
//template class InterpolationFunction<TransFunc*>;
//
//template class MultiPointInterpolationFunction<float>;
//template class MultiPointInterpolationFunction<int>;
//template class MultiPointInterpolationFunction<bool>;
//template class MultiPointInterpolationFunction<tgt::ivec2>;
//template class MultiPointInterpolationFunction<tgt::ivec3>;
//template class MultiPointInterpolationFunction<tgt::ivec4>;
//template class MultiPointInterpolationFunction<tgt::vec2>;
//template class MultiPointInterpolationFunction<tgt::vec3>;
//template class MultiPointInterpolationFunction<tgt::vec4>;
//template class MultiPointInterpolationFunction<tgt::quat>;
//template class MultiPointInterpolationFunction<tgt::mat2>;
//template class MultiPointInterpolationFunction<tgt::mat3>;
//template class MultiPointInterpolationFunction<tgt::mat4>;
//template class MultiPointInterpolationFunction<tgt::Camera>;
//template class MultiPointInterpolationFunction<std::string>;
//template class MultiPointInterpolationFunction<ShaderSource>;
//template class MultiPointInterpolationFunction<TransFunc*>;

} // namespace voreen
