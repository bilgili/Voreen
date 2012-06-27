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

#ifdef VRN_MODULE_FLOWREEN
#ifndef VRN_COLORCODINGABILITY_H
#define VRN_COLORCODINGABILITY_H

#include "voreen/core/properties/colorproperty.h"
#include "voreen/core/properties/optionproperty.h"

namespace voreen {

class ColorCodingAbility {
public:
    enum ColorCodingMode {
        COLOR_CODING_MODE_MAGNITUDE,
        COLOR_CODING_MODE_DIRECTION,
        COLOR_CODING_MODE_MONOCHROME
    };

public:
    ColorCodingAbility();
    ~ColorCodingAbility();

    const ColorCodingMode& getColorCodingMode() const { return colorCodingMode_; }
    OptionProperty<ColorCodingMode>& getColorCodingModeProp() const { return colorCodingModeProp_; }
    IntOptionProperty& getColorTableProp() const { return colorTableProp_; }
    ColorProperty& getColorProp() const { return colorProp_; }
    std::string getShaderDefines() const;

    void onColorCodingModeChange();

private:
    mutable OptionProperty<ColorCodingMode> colorCodingModeProp_;
    mutable IntOptionProperty colorTableProp_;
    mutable ColorProperty colorProp_;

    ColorCodingMode colorCodingMode_;
};

}   // namespace

#endif
#endif  // VRN_MODULE_FLOWREEN
