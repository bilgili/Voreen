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

#ifndef VRN_VECTORPROPERTY_H
#define VRN_VECTORPROPERTY_H

#include "voreen/core/vis/properties/condition.h"
#include "voreen/core/vis/properties/numericproperty.h"

#include <sstream>

namespace voreen {

class FloatVec2Prop : public NumericProperty<tgt::vec2> {
public:
    FloatVec2Prop(const std::string& id, const std::string& guiText, const tgt::vec2& value,
                  const tgt::vec2& minimum = tgt::vec2(0.0f), const tgt::vec2& maximum = tgt::vec2(1.0f),
                  bool invalidate = true, bool invalidateShader = false);

    virtual ~FloatVec2Prop() {}

    virtual PropertyWidget* createWidget(PropertyWidgetFactory* f);
};

//---------------------------------------------------------------------------

class FloatVec3Prop : public NumericProperty<tgt::vec3> {
public:
    FloatVec3Prop(const std::string& id, const std::string& guiText, const tgt::vec3& value,
                  const tgt::vec3& minimum = tgt::vec3(0.0f), const tgt::vec3& maximum = tgt::vec3(1.0f),
                  bool invalidate = true, bool invalidateShader = false);

    virtual ~FloatVec3Prop() {}

    virtual PropertyWidget* createWidget(PropertyWidgetFactory* f);
};

//---------------------------------------------------------------------------

class FloatVec4Prop : public NumericProperty<tgt::vec4> {
public:
    FloatVec4Prop(const std::string& id, const std::string& guiText, const tgt::vec4& value,
                  const tgt::vec4& minimum = tgt::vec4(0.0f), const tgt::vec4& maximum = tgt::vec4(1.0f),
                  bool invalidate = true, bool invalidateShader = false);

    virtual ~FloatVec4Prop() {}

    virtual PropertyWidget* createWidget(PropertyWidgetFactory* f);
};

//---------------------------------------------------------------------------

class IntVec2Prop : public NumericProperty<tgt::ivec2> {
public:
    IntVec2Prop(const std::string& id, const std::string& guiText, const tgt::ivec2& value,
                const tgt::ivec2& minimum = tgt::ivec2(0), const tgt::ivec2& maximum = tgt::ivec2(100),
                bool invalidate = true, bool invalidateShader = false);

    virtual ~IntVec2Prop() {}

    virtual PropertyWidget* createWidget(PropertyWidgetFactory* f);
};

//---------------------------------------------------------------------------

class IntVec3Prop : public NumericProperty<tgt::ivec3> {
public:
    IntVec3Prop(const std::string& id, const std::string& guiText, const tgt::ivec3& value,
                const tgt::ivec3& minimum = tgt::ivec3(0), const tgt::ivec3& maximum = tgt::ivec3(100),
                bool invalidate = true, bool invalidateShader = false);

    virtual ~IntVec3Prop() {}

    virtual PropertyWidget* createWidget(PropertyWidgetFactory* f);
};

//---------------------------------------------------------------------------

class IntVec4Prop : public NumericProperty<tgt::ivec4> {
public:
    IntVec4Prop(const std::string& id, const std::string& guiText, const tgt::ivec4& value,
                const tgt::ivec4& minimum = tgt::ivec4(0), const tgt::ivec4& maximum = tgt::ivec4(100),
                bool invalidate = true, bool invalidateShader = false);

    virtual ~IntVec4Prop() {}

    virtual PropertyWidget* createWidget(PropertyWidgetFactory* f);
};

} // namespace voreen

#endif // VRN_VECTORPROPERTY_H
