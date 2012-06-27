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

#ifndef VRN_FLOWSLICERENDERER2D_H
#define VRN_FLOWSLICERENDERER2D_H

#include "voreen/modules/flowreen/flowslicerenderer.h"

namespace voreen {

class FlowSliceRenderer2D : public FlowSliceRenderer {
public:
    FlowSliceRenderer2D();
    virtual ~FlowSliceRenderer2D();

    virtual Processor* create() const { return new FlowSliceRenderer2D(); }

    virtual std::string getModule() const { return "flowreen"; }
    virtual std::string getCategory() const { return "Flow Visualization"; }
    virtual std::string getClassName() const { return "FlowSliceRenderer2D"; }
    virtual CodeState getCodeState() const  { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;
    virtual void process();

protected:
    virtual std::string generateShaderHeader();

private:
    float getTextureScalingFactor(const tgt::vec2& viewportSize, const tgt::vec2& textureSize);
    inline void invalidateTexture() { rebuildTexture_ = true; }
    void onSliceAlignmentChange();
    void onTextureZoomChange();
    void updateNumSlices();

private:
    /** Property containing the available alignments (axial, coronal, sagittal) */
    OptionProperty<SliceAlignment>* alignmentProp_;

    /** Property containing the currently selected slice */
    IntProperty sliceIndexProp_;

    /** Property defining the current texture magnification */
    OptionProperty<TextureZoom>* textureZoomProp_;

    tgt::Texture* sliceTexture_;    /** the current 2D texture containing the flow image */
    SliceAlignment alignment_;      /** the currently used slice alignment */
    int numSlices_;                 /** number of slices being available in the current alignment */
};

}   // namespace

#endif  // VRN_FLOWSLICERENDERER2D_H

