#include "voreen/modules/flowreen/colorcodingability.h"

#include <string>
#include <vector>

namespace voreen {

ColorCodingAbility::ColorCodingAbility()
    : colorCodingModeProp_("colorCoding", "color coding: "),
      colorTableProp_("colorTableProp_", "color table: "),
      colorProp_("colorProp_", "color: ", tgt::vec4(1.0f))
{
    colorCodingModeProp_.addOption("magnitude", "magnitude", COLOR_CODING_MODE_MAGNITUDE);
    colorCodingModeProp_.addOption("direction", "direction", COLOR_CODING_MODE_DIRECTION);
    colorCodingModeProp_.addOption("monochrome", "monochrome", COLOR_CODING_MODE_MONOCHROME);
    colorCodingModeProp_.onChange(
        CallMemberAction<ColorCodingAbility>(this, &ColorCodingAbility::onColorCodingModeChange));

    colorTableProp_.addOption("rainbow", "rainbow", 0);
    colorTableProp_.addOption("hot metal", "hot metal", 1);
}

ColorCodingAbility::~ColorCodingAbility() {
}

std::string ColorCodingAbility::getShaderDefines() const {
    std::ostringstream oss;
    oss << "#define COLOR_MODE " << static_cast<int>(colorCodingMode_) << "\n";
    if (colorCodingModeProp_.getValue() == COLOR_CODING_MODE_MAGNITUDE)
        oss << "#define COLOR_TABLE " << colorTableProp_.getValue() << "\n";
    return oss.str();
}

void ColorCodingAbility::onColorCodingModeChange() {
    switch (colorCodingModeProp_.getValue()) {
        default:
        case COLOR_CODING_MODE_MAGNITUDE:
            colorProp_.setVisible(false);
            colorTableProp_.setVisible(true);
            break;
        case COLOR_CODING_MODE_DIRECTION:
            colorProp_.setVisible(false);
            colorTableProp_.setVisible(false);
            break;
        case COLOR_CODING_MODE_MONOCHROME:
            colorProp_.setVisible(true);
            colorTableProp_.setVisible(false);
            break;
    }
}

}   // namespace
