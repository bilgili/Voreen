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

#include "voreen/core/vis/processors/proxygeometry/multipleaxialsliceproxygeometry.h"

namespace voreen {

using tgt::vec3;

const Identifier MultipleAxialSliceProxyGeometry::setNSlicesPerRow_("set.nSlicesPerRow");
const Identifier MultipleAxialSliceProxyGeometry::setNSlicesPerCol_("set.nSlicesPerCol");

MultipleAxialSliceProxyGeometry::MultipleAxialSliceProxyGeometry()
    : AxialSliceProxyGeometry()
    , nSlicesPerRow_(setNSlicesPerRow_, "Select the number of slices per row", 1, 1, 10)
    , nSlicesPerCol_(setNSlicesPerCol_, "Select the number of slices per col", 1, 1, 10)
    , isInBuildMethod(false)
{
    setName("MultipleAxialSliceProxyGeometry");

    nSlicesPerRow_.onChange(CallMemberAction<MultipleAxialSliceProxyGeometry>(this, &MultipleAxialSliceProxyGeometry::setNSlicesPerRow));
    nSlicesPerCol_.onChange(CallMemberAction<MultipleAxialSliceProxyGeometry>(this, &MultipleAxialSliceProxyGeometry::setNSlicesPerCol));

    addProperty(&nSlicesPerRow_);
    addProperty(&nSlicesPerCol_);

    // All the other properties will be used automatically
}

const std::string MultipleAxialSliceProxyGeometry::getProcessorInfo() const {
    return "Creates multiple AxialSliceProxyGeometries and layouts them";
}

void MultipleAxialSliceProxyGeometry::render() {
    if (volume_) {
        // Because these calculations are iteratively, the old values have to be restored afterwards
        vec3 volumeSizeSave = volumeSize_;
        vec3 volumeCenterSave = volumeCenter_;
        int beginSave = begin_.get();

        int nSlicesPerRow = nSlicesPerRow_.get();
        int nSlicesPerCol = nSlicesPerCol_.get();

        // The conglomerate of proxygeometries should have (roughly) the same size as the original one,
        // so reduce the size accordingly
        volumeSize_ /= static_cast<float>(std::max(nSlicesPerRow_.get(), nSlicesPerCol_.get()));


        // If we have an even number of slices per row/column, there is no slice in the origin
        // The whole size of the volume is splitted in 2*nSlices parts, each (new) volumeSize_/2
        // in width. At every second position we have the center of one slice.
        // The (+i%3) in the volumeCenter_ causes the algorithm to work, no matter what axis is currently
        // set as the "active" one
        // In the case of odd numbers, we can just go from (-nSlices/2) to (nSlices/2) [note
        // that both divisions are done with integers] and render those slices with no further
        // calculation

        if (nSlicesPerRow_.get()%2 == 0) {
            if (nSlicesPerCol_.get()%2 == 0) {
                // nSlicesPerRow is even  ;  nSlicesPerCol is even
                for (int iterCol = -nSlicesPerCol+1; iterCol < nSlicesPerCol; ++(++iterCol)) {
                    volumeCenter_[(alignment_+1)%3] = iterCol * (volumeSize_[(alignment_+1)%3]/2.f);
                    for (int iterRow = -nSlicesPerRow+1; iterRow < nSlicesPerRow; ++(++iterRow)) {
                        volumeCenter_[(alignment_+2)%3] = iterRow * (volumeSize_[(alignment_+2)%3]/2.f);

                        build();
                    }
                }
            }
            else {
                // nSlicesPerRow is even  ;  nSlicesPerCol is odd
                for (int iterCol = -nSlicesPerCol/2; iterCol <= nSlicesPerCol/2; ++iterCol) {
                    volumeCenter_[(alignment_+1)%3] = iterCol * volumeSize_[(alignment_+1)%3];
                    for (int iterRow = -nSlicesPerRow+1; iterRow < nSlicesPerRow; ++(++iterRow)) {
                        volumeCenter_[(alignment_+2)%3] = iterRow * (volumeSize_[(alignment_+2)%3]/2.f);

                        build();
                    }
                }
            }
        }
        else {
            if (nSlicesPerCol_.get()%2 == 0) {
                // nSlicesPerRow is odd  ;  nSlicesPerCol is even
                for (int iterCol = -nSlicesPerCol+1; iterCol < nSlicesPerCol; ++(++iterCol)) {
                    volumeCenter_[(alignment_+1)%3] = iterCol * (volumeSize_[(alignment_+1)%3]/2.f);
                    for (int iterRow = -nSlicesPerRow/2; iterRow <= nSlicesPerRow/2; ++iterRow) {
                        volumeCenter_[(alignment_+2)%3] = iterRow * volumeSize_[(alignment_+2)%3];

                        build();
                    }
                }
            }
            else {
                // nSlicesPerRow is odd  ;  nSlicesPerCol is odd
                for (int iterCol = -nSlicesPerCol/2; iterCol <= nSlicesPerCol/2; ++iterCol) {
                    volumeCenter_[(alignment_+1)%3] = iterCol * volumeSize_[(alignment_+1)%3];
                    for (int iterRow = -nSlicesPerRow/2; iterRow <= nSlicesPerRow/2; ++iterRow) {
                        volumeCenter_[(alignment_+2)%3] = iterRow * volumeSize_[(alignment_+2)%3];

                        build();
                    }
                }
            }
        }

        // Restore the old values
        volumeSize_ = volumeSizeSave;
        volumeCenter_ = volumeCenterSave;
        begin_.set(beginSave, false);
    }
}

void MultipleAxialSliceProxyGeometry::build() {
    // Render the current slice
    AxialSliceProxyGeometry::revalidateGeometry();
    glCallList(dl_);

    // Update the slices beginning for the coming slice
    isInBuildMethod = true;
    begin_.set(begin_.get()+thickness_.get(), false);
    isInBuildMethod = false;
}


void MultipleAxialSliceProxyGeometry::setBegin() {
    // The call of begin_.set(...) in the \sa render()-method causes a stack overflow,
    // because invalidate() in setBegin() causes render() to be called an vice versa.
    if (!isInBuildMethod) {
        int nSlices = nSlicesPerRow_.get() * nSlicesPerCol_.get();

        if ((nSlices * thickness_.get() + begin_.get()) > 100)
            begin_.set(100 - (nSlices * thickness_.get()));
    }
    invalidate();
}

void MultipleAxialSliceProxyGeometry::setThickness() {
    int nSlices = nSlicesPerRow_.get() * nSlicesPerCol_.get();

    if ((nSlices * thickness_.get() + begin_.get()) > 100)
        thickness_.set((100-begin_.get()) / nSlices);

    invalidate();
}

void MultipleAxialSliceProxyGeometry::setNSlicesPerRow() {
    if ((nSlicesPerRow_.get() * nSlicesPerCol_.get() * thickness_.get() + begin_.get()) > 100)
        nSlicesPerRow_.set( (100-begin_.get()) / thickness_.get() / nSlicesPerCol_.get() );

    invalidate();
}

void MultipleAxialSliceProxyGeometry::setNSlicesPerCol() {
    if ((nSlicesPerRow_.get() * nSlicesPerCol_.get() * thickness_.get() + begin_.get()) > 100)
        nSlicesPerCol_.set( (100-begin_.get()) / thickness_.get() / nSlicesPerRow_.get() );

    invalidate();
}


} // namespace
