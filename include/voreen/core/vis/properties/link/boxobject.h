/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#ifndef VRN_BOXOBJECT_H
#define VRN_BOXOBJECT_H

#include "voreen/core/vis/exception.h"

#include "voreen/core/vis/transfunc/transfuncintensity.h"
#include "tgt/camera.h"

#include <vector>
#include <string>
#include <sstream>

namespace voreen {

class BoxObject {
public:
    enum BoxObjectType {
        NIL,
        BOOL,
        CAMERA,
        DOUBLE,
        FLOAT,
        INTEGER,
        LONG,
        STRING,
        STRINGVEC,
        IVEC2,
        IVEC3,
        IVEC4,
        TRANSFUNC,
        VEC2,
        VEC3,
        VEC4
    };

    BoxObject();
    BoxObject(const BoxObject& obj);
    BoxObject(bool value);
    BoxObject(double value);
    BoxObject(float value);
    BoxObject(int value);
    BoxObject(long value);
    BoxObject(const std::string& value);
    BoxObject(const std::vector<std::string>& value);
    BoxObject(const tgt::ivec2& value);
    BoxObject(const tgt::ivec3& value);
    BoxObject(const tgt::ivec4& value);
    BoxObject(const tgt::vec2& value);
    BoxObject(const tgt::vec3& value);
    BoxObject(const tgt::vec4& value);
    BoxObject(TransFuncIntensity* value);
    BoxObject(tgt::Camera* camera);

    ~BoxObject();

    BoxObjectType getType() const;

    bool getBool() const;
    double getDouble() const;
    float getFloat() const;
    int getInt() const;
    long getLong() const;
    std::string getString() const;
    std::vector<std::string> getStringVec() const;
    tgt::ivec2 getIVec2() const;
    tgt::ivec3 getIVec3() const;
    tgt::ivec4 getIVec4() const;
    tgt::vec2 getVec2() const;
    tgt::vec3 getVec3() const;
    tgt::vec4 getVec4() const;
    TransFuncIntensity* getTransFunc() const;
    tgt::Camera* getCamera() const;

    void setInt(const int& value) { set<int>(value, INTEGER); }
    void setFloat(const float& value) { set<float>(value, FLOAT); }
    void setDouble(const double& value) { set<double>(value, DOUBLE); }
    void setBool(const bool& value) { set<bool>(value, BOOL); }
    void setString(const std::string& value) { set<std::string>(value, STRING); }
    void setStringVec(const std::vector<std::string>& value) { set<std::vector<std::string> >(value, STRINGVEC); }
    void setLong(const long& value) { set<long>(value, LONG); }
    void setVec2(const tgt::vec2& value) { set<tgt::vec2>(value, VEC2); }
    void setVec3(const tgt::vec3& value) { set<tgt::vec3>(value, VEC3); }
    void setVec4(const tgt::vec4& value) { set<tgt::vec4>(value, VEC4); }
    void setIVec2(const tgt::ivec2& value) { set<tgt::ivec2>(value, IVEC2); }
    void setIVec3(const tgt::ivec3& value) { set<tgt::ivec3>(value, IVEC3); }
    void setIVec4(const tgt::ivec4& value) { set<tgt::ivec4>(value, IVEC4); }
    void setTransFunc(const TransFuncIntensity* value) { set(*value, TRANSFUNC); }
    void setCamera(const tgt::Camera* value) { set(*value, CAMERA); }

    /**
     * Releases the stored value without deleting it.
     *
     * Use this function in order to prevent the BoxObject
     * from deleting its value on its own destruction.
     */
    void releaseValue();

    BoxObject& operator= (const BoxObject& rhs);
    BoxObject& operator= (const bool& rhs);
    BoxObject& operator= (const double& rhs);
    BoxObject& operator= (const float& rhs);
    BoxObject& operator= (const int& rhs);
    BoxObject& operator= (const long& rhs);
    BoxObject& operator= (const std::string& rhs);
    BoxObject& operator= (const std::vector<std::string>& rhs);
    BoxObject& operator= (const tgt::ivec2& rhs);
    BoxObject& operator= (const tgt::ivec3& rhs);
    BoxObject& operator= (const tgt::ivec4& rhs);
    BoxObject& operator= (const tgt::vec2& rhs);
    BoxObject& operator= (const tgt::vec3& rhs);
    BoxObject& operator= (const tgt::vec4& rhs);
    BoxObject& operator= (const TransFuncIntensity* rhs);
    BoxObject& operator= (const tgt::Camera* rhs);

protected:
    void switchedDelete();

    template<class T>
    void set(const T& value, BoxObjectType type);

    void set(const TransFuncIntensity& value, BoxObjectType type);
    void set(const tgt::Camera& value, BoxObjectType type);

    template<class T>
    const std::string toString(const T& value) const;

    void* value_;
    BoxObjectType currentType_;
};

} // namespace

#endif // VRN_BOXOBJECT_H
