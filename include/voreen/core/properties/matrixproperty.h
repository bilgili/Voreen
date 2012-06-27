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

#ifndef VRN_MATRIXROPERTY_H
#define VRN_MATRIXROPERTY_H

#include "voreen/core/properties/condition.h"
#include "voreen/core/properties/numericproperty.h"

#include <sstream>

namespace voreen {

class FloatMat2Property : public NumericProperty<tgt::mat2> {
public:
    FloatMat2Property(const std::string& id, const std::string& guiText, const tgt::mat2& value,
        const tgt::mat2& minimum = tgt::mat2(-10.0f), const tgt::mat2& maximum = tgt::mat2(10.0f),
        Processor::InvalidationLevel invalidationLevel=Processor::INVALID_RESULT);

    virtual ~FloatMat2Property() {}

    virtual PropertyWidget* createWidget(PropertyWidgetFactory* f);
};

//---------------------------------------------------------------------------

class FloatMat3Property : public NumericProperty<tgt::mat3> {
public:
    FloatMat3Property(const std::string& id, const std::string& guiText, const tgt::mat3& value,
        const tgt::mat3& minimum = tgt::mat3(-10.0f), const tgt::mat3& maximum = tgt::mat3(10.0f),
        Processor::InvalidationLevel invalidationLevel=Processor::INVALID_RESULT);

    virtual ~FloatMat3Property() {}

    virtual PropertyWidget* createWidget(PropertyWidgetFactory* f);
};

//---------------------------------------------------------------------------

class FloatMat4Property : public NumericProperty<tgt::mat4> {
public:
    FloatMat4Property(const std::string& id, const std::string& guiText, const tgt::mat4& value,
        const tgt::mat4& minimum = tgt::mat4(-10.0f), const tgt::mat4& maximum = tgt::mat4(10.0f),
        Processor::InvalidationLevel invalidationLevel=Processor::INVALID_RESULT);

    virtual ~FloatMat4Property() {}

    virtual PropertyWidget* createWidget(PropertyWidgetFactory* f);
};

} // namespace voreen

#endif // VRN_MATRIXROPERTY_H
