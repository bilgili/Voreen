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

#ifndef VRN_SLICESEQUENCERENDERER_H
#define VRN_SLICESEQUENCERENDERER_H

#include "voreen/core/vis/processors/render/slicerendererbase.h"
#include "tgt/event/eventlistener.h"
#include "voreen/core/vis/properties/eventproperty.h"

namespace voreen {

/**
 * Performs slice rendering of a multiple slices along one of the three main
 * axis of the volume.
 */
class SliceSequenceRenderer : public SliceRendererBase {
public:
    enum SliceAlignment {
        XZ_PLANE = 1,
        ZY_PLANE = 0,
        XY_PLANE = 2};

public:
    /**
     * @param   singleSlice if set to "true", the properties for setting the number of slice per row and
     *                      column will not be added.
     */
    SliceSequenceRenderer(const bool isSingleSlice = false);

    virtual ~SliceSequenceRenderer();

    virtual std::string getCategory() const { return "Slice Rendering"; }
    virtual std::string getClassName() const { return "SliceSequenceRenderer"; }
    virtual std::string getModuleName() const { return "core"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_STABLE; } ///2.0
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const { return new SliceSequenceRenderer(); }

    virtual void process();

protected:
    void updateNumSlices();

    /**
     * Renders a GL_LINE_LOOP with white color and the current slice's
     * dimensions in order to visualize its boudary.
     * Make sure that the shader within transferFuncShader_ is deactivated
     * before calling this method!
     */
    void renderSliceBoundaries(const size_t numSlicesRow, const size_t numSlicesCol);

    void renderInfoTexts(const size_t numSlicesRow, const size_t numSlicesCol);

    /**
     * Converts the given screen coordinates into the corresponding voxel
     * coordinates for the slice.
     */
    tgt::ivec3 screenToVoxelPos(const tgt::ivec2& screenPos);

    void onSliceAlignmentChange();

    void mouseLocalization(tgt::MouseEvent* e);

protected:
    OptionProperty<SliceSequenceRenderer::SliceAlignment>* alignmentProp_;  /**< Property containing the available alignments (axial, coronal, sagittal) */
    IntProperty sliceIndexProp_;        /**< Property containing the currently selected slice */
    IntProperty numSlicesPerRowProp_;   /**< Property containing the currently displayed number of slices per Row */
    IntProperty numSlicesPerColProp_;   /**< Property containing the currently displayed number of slices per Column */
    BoolProperty renderSliceBoundariesProp_;    /**< Determines whether to render the slice boundaries */
    ColorProperty boundaryColor_;
    TemplateMouseEventProperty<SliceSequenceRenderer>* eventPressProp_;
    TemplateMouseEventProperty<SliceSequenceRenderer>* eventMoveProp_;
    SliceAlignment alignment_;      /**< the currently used slice alignment */
    size_t numSlices_;              /**< The number of slices in the direction corresponding to alignment. */

    tgt::vec2 slicePos_;
    tgt::vec2 sliceSize_;
    tgt::ivec3 volumeDimensions_;
    tgt::ivec2 lastMousePosition_;
    tgt::ivec3 voxelPosPermutation_;

    static const std::string fontName_; /**< Path and Name of the font used for text-rendering */
};

}   // namespace

#endif
