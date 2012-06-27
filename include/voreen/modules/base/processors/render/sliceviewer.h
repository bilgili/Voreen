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

#ifndef VRN_SLICEVIEWER_H
#define VRN_SLICEVIEWER_H

#include "voreen/modules/base/processors/render/slicerendererbase.h"
#include "voreen/core/properties/eventproperty.h"
#include "voreen/core/interaction/mwheelnumpropinteractionhandler.h"

namespace voreen {

/**
 * Performs slice rendering of a single or multiple slices
 * along one of the three main axis of the volume.
 */
class SliceViewer : public SliceRendererBase {

public:
    SliceViewer();
    virtual ~SliceViewer();
    virtual Processor* create() const;

    virtual std::string getClassName() const    { return "SliceViewer";      }
    virtual std::string getCategory() const     { return "Slice Rendering";  }
    virtual CodeState getCodeState() const      { return CODE_STATE_STABLE;  }
    virtual std::string getProcessorInfo() const;

    /// Determines the current axis-alignment if the displayed slices.
    enum SliceAlignment {
        YZ_PLANE = 0,
        XZ_PLANE = 1,
        XY_PLANE = 2
    };

protected:

    virtual void process();

    /**
     * Adapts the min/max ranges of the respective properties to the
     * dimensions of the currently connected volume.
     */
    void updateSliceProperties();

    /**
     * Adapts the volume coord permutation and the states
     * of the slice properties when the slice alignment has changed.
     */
    void onSliceAlignmentChange();

    /**
     * Renders a GL_LINE_LOOP with white color and the current slice's
     * dimensions in order to visualize its boundary.
     */
    void renderSliceBoundaries() const;

    /**
     * Displays the cursor position in volume coordinates, the data value under the cursor,
     * and/or the current slice number. What data is actually shown, is determined by the
     * respective properties.
     *
     * @note requires freetype (VRN_WITH_FONTRENDERING)
     */
    void renderInfoTexts() const;

    /**
     * Tiny helper function returning true when numSlicesPerRow_ and
     * numSlicesPerCol_ are set to one.
     */
    bool singleSliceMode() const;

    /**
     * Converts the given screen coordinates into the corresponding voxel
     * coordinates for the slice.
     */
    tgt::vec3 screenToVoxelPos(tgt::ivec2 screenPos) const;

    /**
     * Generates a matrix that maps from viewport coordinates
     * to volume coordinates.
     *
     * @note This matrix is only valid in single slice mode,
     *  since for multi slice mode, the mapping from viewport
     *  to volume coordinates cannot be expressed by a linear function.
     */
    tgt::mat4 generatePickingMatrix() const;

    /**
     * Is called by the respective event properties on mouse move or
     * mouse click events and saves the current cursor position.
     */
    void mouseLocalization(tgt::MouseEvent* e);

    /**
     * Is called by the mouseEventShift_ property when a slice shift
     * event has occurred.
     */
    void shiftEvent(tgt::MouseEvent* e);

protected:

    /// Property containing the available alignments: xy (axial), xz (coronal), yz (sagittal)
    OptionProperty<SliceAlignment> sliceAlignment_;
    IntProperty sliceIndex_;                ///< Property containing the currently selected slice
    IntProperty numSlicesPerRow_;           ///< Property containing the currently displayed number of slices per Row
    IntProperty numSlicesPerCol_;           ///< Property containing the currently displayed number of slices per Column

    BoolProperty renderSliceBoundaries_;    ///< Determines whether to render the slice boundaries
    ColorProperty boundaryColor_;           ///< Color to be used for rendering of the slice boundary
    StringOptionProperty showCursorInfos_;  ///< Determines whether information about the cursor position is to be shown
    BoolProperty showSliceNumber_;          ///< Determines whether the slice number is to displayed on each slice
    IntProperty fontSize_;                  ///< Font size to be used for info texts

    FloatVec2Property voxelOffset_;         ///< The 2D voxel position at which the view area is shifted
    FloatProperty zoomFactor_;              ///< Specifies the current slice zoom factor: the standard value of 1.0
                                            ///  causes the whole slice to be displayed

    FloatMat4Property pickingMatrix_;       ///< Read-only (generated) matrix mapping from viewport to
                                            ///  volume coordinates.

    EventProperty<SliceViewer>* mouseEventPress_;
    EventProperty<SliceViewer>* mouseEventMove_;
    EventProperty<SliceViewer>* mouseEventShift_;

    MWheelNumPropInteractionHandler<int> mwheelCycleHandler_;
    MWheelNumPropInteractionHandler<float> mwheelZoomHandler_;

    tgt::ivec3 voxelPosPermutation_;    ///< permutation of voxel coordinates according to current alignment
    tgt::vec2 sliceLowerLeft_;          ///< lower left of the whole slice plate in viewport coordinates
    tgt::vec2 sliceSize_;               ///< size of a single slice in viewport coordinates
    tgt::mat4 textureMatrix_;           ///< matrix that is currently applied to texture coordinates

    static const std::string fontName_; ///< path and name of the font used for text-rendering

private:
    mutable tgt::ivec2 mousePosition_;          ///< Current mouse position
    mutable tgt::ivec3 lastPickingPosition_;    ///< Mouse position during previous interaction

};

} // namespace

#endif // VRN_SLICEVIEWER_H
