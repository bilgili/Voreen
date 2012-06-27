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

#ifndef VRN_EDGEDETECT_H
#define VRN_EDGEDETECT_H

#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/image/imageprocessordepth.h"

namespace voreen {

/**
 * Performs an edge detection.
 *
 * The detected edge is then colored.
 */
class EdgeDetect : public ImageProcessorDepth {

public:

    /**
     * The Constructor.
     *
     * @param camera The camera from wich we will get information about the current modelview-matrix.
     * @param tc The TextureContainer that will be used to manage TextureUnits for all render-to-texture work done by the PostProcessing.
     */
    EdgeDetect();
    virtual const Identifier getClassName() const { return "ImageProcessor.EdgeDetect"; }
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const { return new EdgeDetect(); }

    virtual ~EdgeDetect();

    void process(LocalPortMapping*  portMapping);

    /**
     *  Takes care of incoming messages.  Accepts the following message-ids:
     *      - set.backgroundColor, which is used to set the background-color,  Msg-Type: tgt::Color
     *      - set.edgeColor, which is used to set the edge-color,  Msg-Type: tgt::Color
     *      - set.fillColor, which is used to set the fill-color,  Msg-Type: tgt::Color
     *      - set.edgeThreshold, which is used to set the edge-threshold, Msg-Type: float
     *      - set.showImage, which is used to control whether or not a picture is rendered, must be \c int because it's not possible to pass bool as a
     *        uniform, Msg-Type: int
     *      - set.blendMode, which is used to control the mode of blending, Msg-Type: int
     *      - set.edgeStyle, which is used to control the style of the painting of the edges, Msg-Type: int
     *      - set.labelMode, which is used to control a sophisticated labeling variable, Msg-Type: bool
     *      - switchCoarseness, which is used to switch coarseness on/off, Msg-Type: bool
     *   @param msg The incoming message.
     *   @param dest The destination of the message.
     */
    virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);

protected:
    FloatProp edgeThreshold_;
    IntProp showImage_;
    EnumProp* blendMode_;
    std::vector<std::string> blendModes_;
    EnumProp* edgeStyle_;
    std::vector<std::string> edgeStyles_;
    ColorProp edgeColor_;
    ColorProp fillColor_;
    bool coarsnessOn_;
    // determines if edgedetect uses some special settings for labeling
    BoolProp labelMode_;
};


} // namespace voreen

#endif //VRN_EDGEDETECT_H

