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

#ifndef VRN_CROSSHAIRPP_H
#define VRN_CROSSHAIRPP_H

#include "tgt/shadermanager.h"

#include "voreen/core/opengl/texunitmapper.h"
#include "voreen/core/vis/processors/image/imageprocessor.h"

#include "voreen/core/vis/properties/eventproperty.h"

namespace voreen {

/**
 * This class renders a cross hair cursor.
 */
class CrossHair : public ImageProcessor {
public:
    CrossHair();
    ~CrossHair();
    virtual std::string getCategory() const { return "Image Processing"; }
    virtual std::string getClassName() const { return "CrossHair"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const;
    void initialize() throw (VoreenException);

    void process();

protected:
    float cutDecimalPlaces(float number, int decimalPlaces);
    tgt::Color calcPositionsColor(int pos);
    void createAbsDepthToColorTex();
    void createRelDepthToColorTex(RenderPort* source);

    virtual std::string generateHeader();
    virtual void compile();

    void eventMethod(tgt::MouseEvent* e);

    tgt::Texture* chromaDepthTex_;
    tgt::ivec2 cursorPos_;
    tgt::Shader* crossHairPrg_;
    ColorProperty color_;
    BoolProperty useChromaDepth_;
    BoolProperty removeBackground_;
    IntProperty textureSize_;
    IntProperty cutAfterDecimalPlaces_;
    OptionProperty<int>* mappingModeProp_;
    TemplateMouseEventProperty<CrossHair>* eventProp_;
    bool showCrossHair_;
    tgt::Texture* depthToColorTex_;
    float minValue_;
    float maxValue_;

    RenderPort inport_;
    RenderPort outport_;
};

} // namespace voreen

#endif //VRN_CROSSHAIRPP_H
