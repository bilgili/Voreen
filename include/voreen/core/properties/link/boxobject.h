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
#include "voreen/core/plotting/colormap.h"
#include "voreen/core/plotting/plotzoomstate.h"
#include "voreen/core/plotting/plotentitysettings.h"
#include "voreen/core/plotting/plotpredicate.h"
#include "tgt/camera.h"

#include <vector>
#include <stack>
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
class BoxObject : public Serializable {
public:

    /**
     * Specifies the data type of the value
     * that is currently stored in the BoxObject.
     */
    enum BoxObjectType {
        NIL,
        BOOL,
        CAMERA,
        COLORMAP,
        DOUBLE,
        FLOAT,
        INTEGER,
        PLOTENTITYSETTINGSVEC,
        LONG,
        SHADER,
        STRING,
        STRINGVEC,
        IVEC2,
        IVEC3,
        IVEC4,
        PLOTPREDICATEVECTOR,
        PLOTZOOM,
        TRANSFUNC,
        VEC2,
        VEC3,
        VEC4,
        DVEC2,
        DVEC3,
        DVEC4,
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
    explicit BoxObject(const tgt::dvec2& value);
    explicit BoxObject(const tgt::dvec3& value);
    explicit BoxObject(const tgt::dvec4& value);
    explicit BoxObject(const tgt::mat2& value);
    explicit BoxObject(const tgt::mat3& value);
    explicit BoxObject(const tgt::mat4& value);
    explicit BoxObject(const ShaderSource& value);
    explicit BoxObject(const ColorMap& value);
    explicit BoxObject(const std::vector<PlotEntitySettings>& value);
    explicit BoxObject(const std::vector<std::pair<int, PlotPredicate*> >& value);
    explicit BoxObject(const std::vector< PlotZoomState >& value);

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

    /**
     * Performs and returns a deep copy of the BoxObject. Will be the same as the copy constructor
     * in most cases, but may be different for certain types (e.g. TransFunc)
     */
    BoxObject deepCopy() const;

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
     * Returns the BoxObjectType for the given type string.
     * It the typeName doesn't match any BoxObjectType, NIL is
     * returned
     */
    static BoxObjectType getType(const std::string& typeName);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to Bool if needed and/or possible and returns it.
     *
     * @throw VoreenException if the stored value cannot be converted
     */
    bool getBool() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to ColorMap if needed and/or possible and returns it.
     *
     * @throw VoreenException if the stored value cannot be converted
     */
    ColorMap getColorMap() const throw (VoreenException);

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
     * to PlotEntitySettingsVec if needed and/or possible and returns it.
     *
     * @throw VoreenException if the stored value cannot be converted
     */
    std::vector<PlotEntitySettings> getPlotEntitySettingsVec() const throw (VoreenException);

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
     * to DVec2 if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    tgt::dvec2 getDVec2() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to DVec3 if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    tgt::dvec3 getDVec3() const throw (VoreenException);

    /**
     * Converts the current value regardless of type of the BoxObject
     * to DVec4 if needed and/or possible and returns it.
     *
     * @throw VoreenException, if the stored value cannot be converted
     */
    tgt::dvec4 getDVec4() const throw (VoreenException);

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
    * to std::vector<std::pair<int, PlotPredicate*> > if needed and/or possible and returns it.
    *
    * @throw VoreenException, if the stored value cannot be converted
    */
    std::vector<std::pair<int, PlotPredicate*> > getPlotPredicateVector() const throw (VoreenException);

    /**
    * Converts the current value regardless of type of the BoxObject
    * to std::vector< PlotZoomState > if needed and/or possible and returns it.
    *
    * @throw VoreenException, if the stored value cannot be converted
    */
    std::vector< PlotZoomState > getPlotZoom() const throw (VoreenException);

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

    void setBool(const bool& value);
    void setColorMap(const ColorMap& value);
    void setDouble(const double& value);
    void setFloat(const float& value);
    void setInt(const int& value);
    void setLong(const long& value);
    void setString(const std::string& value);
    void setStringVec(const std::vector<std::string>& value);
    void setPlotEntitySettingsVec(const std::vector<PlotEntitySettings>& value);
    void setIVec2(const tgt::ivec2& value);
    void setIVec3(const tgt::ivec3& value);
    void setIVec4(const tgt::ivec4& value);
    void setVec2(const tgt::vec2& value);
    void setVec3(const tgt::vec3& value);
    void setVec4(const tgt::vec4& value);
    void setDVec2(const tgt::dvec2& value);
    void setDVec3(const tgt::dvec3& value);
    void setDVec4(const tgt::dvec4& value);
    void setMat2(const tgt::mat2& value);
    void setMat3(const tgt::mat3& value);
    void setMat4(const tgt::mat4& value);
    void setPlotZoom(const std::vector< PlotZoomState >& value);
    void setPlotPredicateVector(const std::vector<std::pair<int, PlotPredicate*> >& value);
    void setShader(const ShaderSource* value);
    /// Assigns a transfer function. The passed object is not copied.
    void setTransFunc(const TransFunc* value);
    /// Assigns a camera. The passed object is not copied.
    void setCamera(const tgt::Camera* value);
    /// Assigns a VolumeHandle. The passed object is not copied.
    void setVolumeHandle(const VolumeHandle* value);

    /// Assigns a VolumeCollection. The passed object is not copied.
    void setVolumeCollection(const VolumeCollection* value);

    void serialize(XmlSerializer& s) const;
    void deserialize(XmlDeserializer& d);

    BoxObject& operator= (const BoxObject& rhs);
    BoxObject& operator= (const bool& rhs);
    BoxObject& operator= (const ColorMap& rhs);
    BoxObject& operator= (const double& rhs);
    BoxObject& operator= (const float& rhs);
    BoxObject& operator= (const int& rhs);
    BoxObject& operator= (const long& rhs);
    BoxObject& operator= (const std::string& rhs);
    BoxObject& operator= (const std::vector<std::string>& rhs);
    BoxObject& operator= (const std::vector<PlotEntitySettings>& rhs);
    BoxObject& operator= (const tgt::ivec2& rhs);
    BoxObject& operator= (const tgt::ivec3& rhs);
    BoxObject& operator= (const tgt::ivec4& rhs);
    BoxObject& operator= (const tgt::vec2& rhs);
    BoxObject& operator= (const tgt::vec3& rhs);
    BoxObject& operator= (const tgt::vec4& rhs);
    BoxObject& operator= (const tgt::dvec2& rhs);
    BoxObject& operator= (const tgt::dvec3& rhs);
    BoxObject& operator= (const tgt::dvec4& rhs);
    BoxObject& operator= (const tgt::mat2& rhs);
    BoxObject& operator= (const tgt::mat3& rhs);
    BoxObject& operator= (const tgt::mat4& rhs);
    BoxObject& operator= (const std::vector<std::pair<int, PlotPredicate*> >& rhs);
    BoxObject& operator= (const std::vector< PlotZoomState >& rhs);
    BoxObject& operator= (const ShaderSource& rhs);
    BoxObject& operator= (const TransFunc* rhs);
    BoxObject& operator= (const tgt::Camera* rhs);
    BoxObject& operator= (const VolumeHandle* rhs);
    BoxObject& operator= (const VolumeCollection* rhs);

    bool operator== (const BoxObject& rhs) const;
    bool operator!= (const BoxObject& rhs) const;

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
