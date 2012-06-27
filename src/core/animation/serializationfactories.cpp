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

#include "voreen/core/animation/serializationfactories.h"

#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/volumeurllistproperty.h"
#include "voreen/core/properties/shaderproperty.h"

#include "voreen/core/animation/templatepropertytimeline.h"
#include "voreen/core/animation/propertykeyvalue.h"

#include "voreen/core/animation/interpolation/boolinterpolationfunctions.h"
#include "voreen/core/animation/interpolation/camerainterpolationfunctions.h"
#include "voreen/core/animation/interpolation/floatinterpolationfunctions.h"
#include "voreen/core/animation/interpolation/intinterpolationfunctions.h"
#include "voreen/core/animation/interpolation/vec2interpolationfunctions.h"
#include "voreen/core/animation/interpolation/vec3interpolationfunctions.h"
#include "voreen/core/animation/interpolation/vec4interpolationfunctions.h"
#include "voreen/core/animation/interpolation/ivec2interpolationfunctions.h"
#include "voreen/core/animation/interpolation/ivec3interpolationfunctions.h"
#include "voreen/core/animation/interpolation/ivec4interpolationfunctions.h"
#include "voreen/core/animation/interpolation/mat2interpolationfunctions.h"
#include "voreen/core/animation/interpolation/mat3interpolationfunctions.h"
#include "voreen/core/animation/interpolation/mat4interpolationfunctions.h"
#include "voreen/core/animation/interpolation/transfuncinterpolationfunctions.h"
#include "voreen/core/animation/interpolation/shadersourceinterpolationfunctions.h"
#include "voreen/core/animation/interpolation/stringinterpolationfunctions.h"

namespace voreen {

PropertyTimelineFactory* PropertyTimelineFactory::instance_ = 0;

PropertyTimelineFactory* PropertyTimelineFactory::getInstance() {
    if (!instance_)
        instance_ = new PropertyTimelineFactory();

    return instance_;
}

const std::string PropertyTimelineFactory::getTypeString(const std::type_info& type) const {
    if (type == typeid(TemplatePropertyTimeline<float>))
        return "TemplatePropertyTimeline_float";
    else if (type == typeid(TemplatePropertyTimeline<int>))
        return "TemplatePropertyTimeline_int";
    else if (type == typeid(TemplatePropertyTimeline<bool>))
        return "TemplatePropertyTimeline_bool";
    else if (type == typeid(TemplatePropertyTimeline<tgt::ivec2>))
        return "TemplatePropertyTimeline_ivec2";
    else if (type == typeid(TemplatePropertyTimeline<tgt::ivec3>))
        return "TemplatePropertyTimeline_ivec3";
    else if (type == typeid(TemplatePropertyTimeline<tgt::ivec4>))
        return "TemplatePropertyTimeline_ivec4";
    else if (type == typeid(TemplatePropertyTimeline<tgt::vec2>))
        return "TemplatePropertyTimeline_vec2";
    else if (type == typeid(TemplatePropertyTimeline<tgt::vec3>))
        return "TemplatePropertyTimeline_vec3";
    else if (type == typeid(TemplatePropertyTimeline<tgt::vec4>))
        return "TemplatePropertyTimeline_vec4";
    else if (type == typeid(TemplatePropertyTimeline<tgt::mat2>))
        return "TemplatePropertyTimeline_mat2";
    else if (type == typeid(TemplatePropertyTimeline<tgt::mat3>))
        return "TemplatePropertyTimeline_mat3";
    else if (type == typeid(TemplatePropertyTimeline<tgt::mat4>))
        return "TemplatePropertyTimeline_mat4";
    else if (type == typeid(TemplatePropertyTimeline<tgt::Camera>))
        return "TemplatePropertyTimeline_Camera";
    else if (type == typeid(TemplatePropertyTimeline<std::string>))
        return "TemplatePropertyTimeline_string";
    else if (type == typeid(TemplatePropertyTimeline<ShaderSource>))
        return "TemplatePropertyTimeline_ShaderSource";
    else if (type == typeid(TemplatePropertyTimeline<TransFunc*>))
        return "TemplatePropertyTimeline_TransFunc";
    else
        return "";
}

Serializable* PropertyTimelineFactory::createType(const std::string& typeString) {
    if (typeString == "TemplatePropertyTimeline_float")
        return new TemplatePropertyTimeline<float>();
    else if (typeString == "TemplatePropertyTimeline_int")
        return new TemplatePropertyTimeline<int>();
    else if (typeString == "TemplatePropertyTimeline_bool")
        return new TemplatePropertyTimeline<bool>();
    else if (typeString == "TemplatePropertyTimeline_ivec2")
        return new TemplatePropertyTimeline<tgt::ivec2>();
    else if (typeString == "TemplatePropertyTimeline_ivec3")
        return new TemplatePropertyTimeline<tgt::ivec3>();
    else if (typeString == "TemplatePropertyTimeline_ivec4")
        return new TemplatePropertyTimeline<tgt::ivec4>();
    else if (typeString == "TemplatePropertyTimeline_vec2")
        return new TemplatePropertyTimeline<tgt::vec2>();
    else if (typeString == "TemplatePropertyTimeline_vec3")
        return new TemplatePropertyTimeline<tgt::vec3>();
    else if (typeString == "TemplatePropertyTimeline_vec4")
        return new TemplatePropertyTimeline<tgt::vec4>();
    else if (typeString == "TemplatePropertyTimeline_mat2")
        return new TemplatePropertyTimeline<tgt::mat2>();
    else if (typeString == "TemplatePropertyTimeline_mat3")
        return new TemplatePropertyTimeline<tgt::mat3>();
    else if (typeString == "TemplatePropertyTimeline_mat4")
        return new TemplatePropertyTimeline<tgt::mat4>();
    else if (typeString == "TemplatePropertyTimeline_Camera")
        return new TemplatePropertyTimeline<tgt::Camera>();
    else if (typeString == "TemplatePropertyTimeline_string")
        return new TemplatePropertyTimeline<std::string>();
    else if (typeString == "TemplatePropertyTimeline_ShaderSource")
        return new TemplatePropertyTimeline<ShaderSource>();
    else if (typeString == "TemplatePropertyTimeline_TransFunc")
        return new TemplatePropertyTimeline<TransFunc*>();
    else
        return 0;
}

KeyValueFactory* KeyValueFactory::instance_ = 0;

KeyValueFactory* KeyValueFactory::getInstance() {
    if (!instance_)
        instance_ = new KeyValueFactory();

    return instance_;
}

const std::string KeyValueFactory::getTypeString(const std::type_info& type) const {
    if (type == typeid(PropertyKeyValue<float>))
        return "KeyValue_float";
    else if (type == typeid(PropertyKeyValue<int>))
        return "KeyValue_int";
    else if (type == typeid(PropertyKeyValue<bool>))
        return "KeyValue_bool";
    else if (type == typeid(PropertyKeyValue<tgt::ivec2>))
        return "KeyValue_ivec2";
    else if (type == typeid(PropertyKeyValue<tgt::ivec3>))
        return "KeyValue_ivec3";
    else if (type == typeid(PropertyKeyValue<tgt::ivec4>))
        return "KeyValue_ivec4";
    else if (type == typeid(PropertyKeyValue<tgt::vec2>))
        return "KeyValue_vec2";
    else if (type == typeid(PropertyKeyValue<tgt::vec3>))
        return "KeyValue_vec3";
    else if (type == typeid(PropertyKeyValue<tgt::vec4>))
        return "KeyValue_vec4";
    else if (type == typeid(PropertyKeyValue<tgt::mat2>))
        return "KeyValue_mat2";
    else if (type == typeid(PropertyKeyValue<tgt::mat3>))
        return "KeyValue_mat3";
    else if (type == typeid(PropertyKeyValue<tgt::mat4>))
        return "KeyValue_mat4";
    else if (type == typeid(PropertyKeyValue<tgt::Camera>))
        return "KeyValue_Camera";
    else if (type == typeid(PropertyKeyValue<std::string>))
        return "KeyValue_string";
    else if (type == typeid(PropertyKeyValue<ShaderSource>))
        return "KeyValue_ShaderSource";
    else if (type == typeid(PropertyKeyValue<TransFunc*>))
        return "KeyValue_TransFunc";
    else
        return "";
}


Serializable* KeyValueFactory::createType(const std::string& typeString) {
    using tgt::ivec2;
    using tgt::ivec3;
    using tgt::ivec4;
    using tgt::vec2;
    using tgt::vec3;
    using tgt::vec4;
    using tgt::mat2;
    using tgt::mat3;
    using tgt::mat4;

    if (typeString == "KeyValue_float")
        return new PropertyKeyValue<float>(0, 0);
    else if (typeString == "KeyValue_int")
        return new PropertyKeyValue<int>(0, 0);
    else if (typeString == "KeyValue_bool")
        return new PropertyKeyValue<bool>(0, 0);
    else if (typeString == "KeyValue_ivec2")
        return new PropertyKeyValue<ivec2>(ivec2(0), 0);
    else if (typeString == "KeyValue_ivec3")
        return new PropertyKeyValue<ivec3>(ivec3(0), 0);
    else if (typeString == "KeyValue_ivec4")
        return new PropertyKeyValue<ivec4>(ivec4(0), 0);
    else if (typeString == "KeyValue_vec2")
        return new PropertyKeyValue<vec2>(vec2(0.0f), 0);
    else if (typeString == "KeyValue_vec3")
        return new PropertyKeyValue<vec3>(vec3(0.0f), 0);
    else if (typeString == "KeyValue_vec4")
        return new PropertyKeyValue<vec4>(vec4(0.0f), 0);
    else if (typeString == "KeyValue_mat2")
        return new PropertyKeyValue<mat2>(mat2(0.0f), 0);
    else if (typeString == "KeyValue_mat3")
        return new PropertyKeyValue<mat3>(mat3(0.0f), 0);
    else if (typeString == "KeyValue_mat4")
        return new PropertyKeyValue<mat4>(mat4(0.0f), 0);
    else if (typeString == "KeyValue_Camera")
        return new PropertyKeyValue<tgt::Camera>(tgt::Camera(vec3(0.0f), vec3(0.0f), vec3(0.0f)), 0);
    else if (typeString == "KeyValue_string")
        return new PropertyKeyValue<std::string>("", 0);
    else if (typeString == "KeyValue_ShaderSource")
        return new PropertyKeyValue<ShaderSource>(ShaderSource(), 0);
    else if (typeString == "KeyValue_TransFunc")
        return new PropertyKeyValue<TransFunc*>(new TransFunc(), 0);
    else
        return 0;
}

TemplatePropertyTimelineStateFactory* TemplatePropertyTimelineStateFactory::instance_ = 0;

TemplatePropertyTimelineStateFactory* TemplatePropertyTimelineStateFactory::getInstance() {
    if (!instance_)
        instance_ = new TemplatePropertyTimelineStateFactory();

    return instance_;
}

const std::string TemplatePropertyTimelineStateFactory::getTypeString(const std::type_info& type) const {
    if (type == typeid(TemplatePropertyTimelineState<float>))
        return "TemplatePropertyTimelineState_float";
    else if (type == typeid(TemplatePropertyTimelineState<int>))
        return "TemplatePropertyTimelineState_int";
    else if (type == typeid(TemplatePropertyTimelineState<bool>))
        return "TemplatePropertyTimelineState_bool";
    else if (type == typeid(TemplatePropertyTimelineState<tgt::ivec2>))
        return "TemplatePropertyTimelineState_ivec2";
    else if (type == typeid(TemplatePropertyTimelineState<tgt::ivec3>))
        return "TemplatePropertyTimelineState_ivec3";
    else if (type == typeid(TemplatePropertyTimelineState<tgt::ivec4>))
        return "TemplatePropertyTimelineState_ivec4";
    else if (type == typeid(TemplatePropertyTimelineState<tgt::vec2>))
        return "TemplatePropertyTimelineState_vec2";
    else if (type == typeid(TemplatePropertyTimelineState<tgt::vec3>))
        return "TemplatePropertyTimelineState_vec3";
    else if (type == typeid(TemplatePropertyTimelineState<tgt::vec4>))
        return "TemplatePropertyTimelineState_vec4";
    else if (type == typeid(TemplatePropertyTimelineState<tgt::mat2>))
        return "TemplatePropertyTimelineState_mat2";
    else if (type == typeid(TemplatePropertyTimelineState<tgt::mat3>))
        return "TemplatePropertyTimelineState_mat3";
    else if (type == typeid(TemplatePropertyTimelineState<tgt::mat4>))
        return "TemplatePropertyTimelineState_mat4";
    else if (type == typeid(TemplatePropertyTimelineState<tgt::Camera>))
        return "TemplatePropertyTimelineState_Camera";
    else if (type == typeid(TemplatePropertyTimelineState<std::string>))
        return "TemplatePropertyTimelineState_string";
    else if (type == typeid(TemplatePropertyTimelineState<ShaderSource>))
        return "TemplatePropertyTimelineState_ShaderSource";
    else if (type == typeid(TemplatePropertyTimelineState<TransFunc*>))
        return "TemplatePropertyTimelineState_TransFunc";
    else if (type == typeid(TransFuncPropertyTimelineState))
        return "TransFuncPropertyTimelineState";
    else if (type == typeid(CameraPropertyTimelineState))
        return "CameraPropertyTimelineState";
    else
        return "";
}

Serializable* TemplatePropertyTimelineStateFactory::createType(const std::string& typeString) {
    if (typeString == "TemplatePropertyTimelineState_float")
        return new TemplatePropertyTimelineState<float>();
    else if (typeString == "TemplatePropertyTimelineState_int")
        return new TemplatePropertyTimelineState<int>();
    else if (typeString == "TemplatePropertyTimelineState_bool")
        return new TemplatePropertyTimelineState<bool>();
    else if (typeString == "TemplatePropertyTimelineState_ivec2")
        return new TemplatePropertyTimelineState<tgt::ivec2>();
    else if (typeString == "TemplatePropertyTimelineState_ivec3")
        return new TemplatePropertyTimelineState<tgt::ivec3>();
    else if (typeString == "TemplatePropertyTimelineState_ivec4")
        return new TemplatePropertyTimelineState<tgt::ivec4>();
    else if (typeString == "TemplatePropertyTimelineState_vec2")
        return new TemplatePropertyTimelineState<tgt::vec2>();
    else if (typeString == "TemplatePropertyTimelineState_vec3")
        return new TemplatePropertyTimelineState<tgt::vec3>();
    else if (typeString == "TemplatePropertyTimelineState_vec4")
        return new TemplatePropertyTimelineState<tgt::vec4>();
    else if (typeString == "TemplatePropertyTimelineState_mat2")
        return new TemplatePropertyTimelineState<tgt::mat2>();
    else if (typeString == "TemplatePropertyTimelineState_mat3")
        return new TemplatePropertyTimelineState<tgt::mat3>();
    else if (typeString == "TemplatePropertyTimelineState_mat4")
        return new TemplatePropertyTimelineState<tgt::mat4>();
    else if (typeString == "TemplatePropertyTimelineState_Camera")
        return new TemplatePropertyTimelineState<tgt::Camera>();
    else if (typeString == "TemplatePropertyTimelineState_string")
        return new TemplatePropertyTimelineState<std::string>();
    else if (typeString == "TemplatePropertyTimelineState_ShaderSource")
        return new TemplatePropertyTimelineState<ShaderSource>();
    else if (typeString == "TemplatePropertyTimelineState_TransFunc")
        return new TemplatePropertyTimelineState<TransFunc*>();
    else if (typeString == "TransFuncPropertyTimelineState")
        return new TransFuncPropertyTimelineState();
    else if (typeString == "CameraPropertyTimelineState")
        return new CameraPropertyTimelineState();
    else
        return 0;
}

} // namespace voreen
