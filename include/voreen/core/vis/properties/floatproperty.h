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

#ifndef VRN_FLOATPROPERTY_H
#define VRN_FLOATPROPERTY_H

#include "voreen/core/vis/properties/numericproperty.h"

namespace voreen {

class FloatProperty : public NumericProperty<float> {
public:
    FloatProperty(const std::string& id, const std::string& guiText, float value = 0.0f,
              float minValue = 0.0f, float maxValue = 1.0f, bool instantValueChange = false,
              Processor::InvalidationLevel invalidationLevel=Processor::INVALID_RESULT);

    virtual ~FloatProperty() {}

    /**
     * For convenience only: this method converts the given double to float
     * and calls the method inherited from NumericProperty<float>.
     */
/*
    void set(const double value, const bool updateWidgets = true) {
        NumericProperty<float>::set(static_cast<const float>(value), updateWidgets);
    }
*/
    PropertyWidget* createWidget(PropertyWidgetFactory* f);
};

}   // namespace

#endif
