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

namespace voreen {

/**
 * This class renders a cross hair cursor.
 */
class CrossHair : public ImageProcessor, public tgt::EventListener {
public:
    CrossHair();
    ~CrossHair();
    virtual const Identifier getClassName() const {return "ImageProcessor.CrossHair";}
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const;
    int initializeGL();

    void process(LocalPortMapping* portMapping);

    virtual void mousePressEvent(tgt::MouseEvent *e);
    virtual void mouseMoveEvent(tgt::MouseEvent *e);
    virtual void mouseReleaseEvent(tgt::MouseEvent *e);

protected:
    float cutDecimalPlaces(float number, int decimalPlaces);
    tgt::Color calcPositionsColor(int pos);
    void createAbsDepthToColorTex();
    void createRelDepthToColorTex(int source);

    virtual std::string generateHeader();
    virtual void compile();

    tgt::Texture* chromaDepthTex_;
    tgt::ivec2 cursorPos_;
    tgt::Shader* crossHairPrg_;
    ColorProp color_;
    BoolProp useChromaDepth_;
    BoolProp removeBackground_;
    IntProp textureSize_;
    IntProp cutAfterDecimalPlaces_;
    OptionProperty<int>* mappingModeProp_;
    bool showCrossHair_;
    tgt::Texture* depthToColorTex_;
    float minValue_;
    float maxValue_;
};

} // namespace voreen

#endif //VRN_CROSSHAIRPP_H
