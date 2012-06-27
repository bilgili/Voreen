#ifdef VRN_MODULE_FLOWREEN
#ifndef VRN_COLORCODINGABILITY_H
#define VRN_COLORCODINGABILITY_H

#include "voreen/core/vis/properties/colorproperty.h"
#include "voreen/core/vis/properties/optionproperty.h"

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
