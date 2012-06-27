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

namespace voreen {

/**
 * Performs slice rendering of a multiple slices along one of the three main
 * axis of the volume.
 */
class SliceSequenceRenderer : public SliceRendererBase, public tgt::EventListener {
public:
    enum SliceAlignment {
        SAGITTAL = 0,       /**< view from the volume's right to left (negative x-axis) */
        AXIAL = 1,          /**< view from the volume's front to back (negative z-axis) */
        CORONAL = 2         /**< view from the volume's top to bottom (negative y-axis) */
    };

public:
    /**
     * @param   singleSlice if set to "true", the properties for setting the number of slice per row and
     *                      column will not be added.
     */
    SliceSequenceRenderer(const bool isSingleSlice = false);

    virtual ~SliceSequenceRenderer();

    virtual const Identifier getClassName() const;
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const { return new SliceSequenceRenderer(); }

    virtual void process(LocalPortMapping* portMapping);

    /** Sets the index of the slice to be rendered */
    //void setSliceIndex(size_t sliceIndex) { sliceIndexProp_.set(static_cast<int>(sliceIndex)); }

    /** Returns the number of slices which can be shown i.e. the depth of the slices */
    //size_t getNumSlices() const { return numSlices_; }

    virtual void mouseMoveEvent(tgt::MouseEvent* e);
    virtual void mousePressEvent(tgt::MouseEvent* e);

protected:
    void updateNumSlices();

    void renderSlices(const size_t numSlicesRow, const size_t numSlicesCol);

    /**
     * Renders a textured quad in unit size aligned to the axial plane (xz-plane)
     * in the volumetric texture
     */
    void renderAxialSlice(const float depth);

    /**
     * Renders a textured quad in unit size aligned to the sagittal plane (zy-plane)
     * in the volumetric texture
     */
    void renderSagittalSlice(const float depth);

    /**
     * Renders a textured quad in unit size aligned to the coronal plane (xy-plane)
     * in the volumetric texture
     */
    void renderCoronalSlice(const float depth);

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

protected:
    EnumProp* alignmentProp_;       /**< Property containing the available alignments (axial, coronal, sagittal) */
    IntProp sliceIndexProp_;        /**< Property containing the currently selected slice */
    IntProp numSlicesPerRowProp_;   /**< Property containing the currently displayed number of slices per Row */
    IntProp numSlicesPerColProp_;   /**< Property containing the currently displayed number of slices per Column */
    BoolProp renderSliceBoundariesProp_;    /**< Determines whether to render the slice boundaries */
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
