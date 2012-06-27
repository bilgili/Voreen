/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#include "voreen/core/utils/exception.h"
#include "tgt/camera.h"

#include <vector>
#include <string>
#include <sstream>

namespace voreen {

class TransFunc;
class VolumeHandle;
class VolumeCollection;
class ShaderSource;

/**
 * The BoxObject is a variant type. It can take different types of values
 * and convert them on demand if a conversion is implemented.
 */
class BoxObject {
public:

    /**
     * Specifies the data type of the value
     * that is currently stored in the BoxObject.
     */
    enum BoxObjectType {
        NIL,
        BOOL,
        CAMERA,
        DOUBLE,
        FLOAT,
        INTEGER,
        LONG,
        SHADER,
        STRING,
        STRINGVEC,
        IVEC2,
        IVEC3,
        IVEC4,
        TRANSFUNC,
        VEC2,
        VEC3,
        VEC4,
        MAT2,
        MAT3,
        MAT4,
        VOLUMEHANDLE,
        VOLUMECOLLECTION
    };

    BoxObject();
    BoxObject(const BoxObject& obj);
    explicit BoxObject(bool value);
    explicit BoxObject(double value);
    explicit BoxObject(float value);
    explicit BoxObject(int value);
    explicit BoxObject(long value);
    explicit BoxObject(const std::string& value);
    explicit BoxObject(const std::vector<std::string>& value);
    explicit BoxObject(const tgt::ivec2& value);
    explicit BoxObject(const tgt::ivec3& value);
    explicit BoxObject(const tgt::ivec4& value);
    explicit BoxObject(const tgt::vec2& value);
    explicit BoxObject(const tgt::vec3& value);
    explicit BoxObject(const tgt::vec4& value);
    explicit BoxObject(const tgt::mat2& value);
    explicit BoxObject(const tgt::mat3& value);
    explicit BoxObject(const tgt::mat4& value);
    explicit BoxObject(const ShaderSource& value);

    /**
     * Constructs a BoxObject from a transfer function.
     * The passed object is not copied, but stored directly.
     */
    explicit BoxObject(const TransFunc* value);

    /**
     * Constructs a BoxObject from a camera.
     * The passed object is not copied, but stored directly.
     */
    explicit BoxObject(const tgt::Camera* value);

    /**
     * Constructs a BoxObject from a volume handle.
     * The passed object is not copied, but stored directly.
     */
    explicit BoxObject(const VolumeHandle* value);

    /**
     * Constructs a BoxObject from a volume collection.
     * The passed object is not copied, but stored directly.
     */
    explicit BoxObject(const VolumeCollection* value);

    ~BoxObject();

    /**
     * Returns the data type of the value
     * that is currently stored in the BoxObject.
     */
    BoxObjectType getType() const;

    /**
     * Returns the passed type as string, which
     * might be used in a user interface.
     */
    static std::string getTypeName(BoxObjectType type);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to Bool if needed and/or possible and returns it.
     *
     * @throw VoreenException if the stored value cannot be converted
     */
    bool getBool() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to Double if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    double getDouble() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to Float if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    float getFloat() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to Int if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    int getInt() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to Long if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    long getLong() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to String if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    std::string getString() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to StringVec if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    std::vector<std::string> getStringVec() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to IVec2 if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    tgt::ivec2 getIVec2() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to IVec3 if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    tgt::ivec3 getIVec3() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to IVec4 if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    tgt::ivec4 getIVec4() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to Vec2 if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    tgt::vec2 getVec2() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to Vec3 if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    tgt::vec3 getVec3() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to Vec4 if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    tgt::vec4 getVec4() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to mat2 if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    tgt::mat2 getMat2() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to mat2 if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    tgt::mat3 getMat3() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to mat2 if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    tgt::mat4 getMat4() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to ShaderSource if needed and/or possible and returns a copy of it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    ShaderSource getShader() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to TransFunc if needed and/or possible and returns it.
     *
     * @note The returned value is not a copy, but is still owned by the link source.
     *      Therefore, it must not be altered or freed by the caller.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    const TransFunc* getTransFunc() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to Camera if needed and/or possible and returns it.
     *
     * @note The returned value is not a copy, but is still owned by the link source.
     *      Therefore, it must not be altered or freed by the caller.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    const tgt::Camera* getCamera() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to VolumeHandle if needed and/or possible and returns it.
     *
     * @note The returned value is not a copy, but is still owned by the link source.
     *      Therefore, it must not be altered or freed by the caller.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    const VolumeHandle* getVolumeHandle() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to VolumeCollection if needed and/or possible and returns it.
     *
     * @note The returned value is not a copy, but is still owned by the link source.
     *      Therefore, it must not be altered or freed by the caller.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    const VolumeCollection* getVolumeCollection() const throw (VoreenException);

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
    void setMat2(const tgt::mat2& value) { set<tgt::mat2>(value, MAT2); }
    void setMat3(const tgt::mat3& value) { set<tgt::mat3>(value, MAT3); }
    void setMat4(const tgt::mat4& value) { set<tgt::mat4>(value, MAT4); }
    void setShader(const ShaderSource* value) { set(*value, SHADER); }

    /// Assigns a transfer function. The passed object is not copied.
    void setTransFunc(const TransFunc* value) { set(*value, TRANSFUNC); }

    /// Assigns a camera. The passed object is not copied.
    void setCamera(const tgt::Camera* value) { set(*value, CAMERA); }

    /// Assigns a VolumeHandle. The passed object is not copied.
    void setVolumeHandle(const VolumeHandle* value) { set(*value, VOLUMEHANDLE); }

    /// Assigns a VolumeCollection. The passed object is not copied.
    void setVolumeCollection(const VolumeCollection* value) { set(*value, VOLUMECOLLECTION); }


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
    BoxObject& operator= (const tgt::mat2& rhs);
    BoxObject& operator= (const tgt::mat3& rhs);
    BoxObject& operator= (const tgt::mat4& rhs);
    BoxObject& operator= (const ShaderSource& rhs);
    BoxObject& operator= (const TransFunc* rhs);
    BoxObject& operator= (const tgt::Camera* rhs);
    BoxObject& operator= (const VolumeHandle* rhs);
    BoxObject& operator= (const VolumeCollection* rhs);

protected:
    /// Frees the currently stored value, if necessary.
    void deleteValue();

    template<class T>
    void set(const T& value, BoxObjectType type);

    void set(const TransFunc& value, BoxObjectType type);
    void set(const tgt::Camera& value, BoxObjectType type);
    void set(const VolumeHandle& value, BoxObjectType type);
    void set(const VolumeCollection& value, BoxObjectType type);

    template<class T>
    const std::string toString(const T& value) const;

    void* value_;                   ///< The currently stored value.
    BoxObjectType currentType_;     ///< Data type of the currently stored value.
};

} // namespace

#endif // VRN_BOXOBJECT_H
