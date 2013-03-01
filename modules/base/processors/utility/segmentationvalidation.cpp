/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "segmentationvalidation.h"

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatornumsignificant.h"
#include "voreen/core/ports/conditions/portconditionvolumetype.h"

#include <climits>

namespace voreen {

const std::string SegmentationValidation::loggerCat_("voreen.base.SegmentationValidation");

SegmentationValidation::SegmentationValidation()
    : Processor()
    , inportSegmentation_(Port::INPORT, "segmentation.in", "Segmentation Volume")
    , inportReference_(Port::INPORT, "segmentation.reference", "Reference Volume")
    , computeButton_("computeButton", "Compute")
    , sizeSegmentation_("sizeSegmentation", "Size Segmentation |A|", 0, 0, INT_MAX)
    , sizeReference_("sizeReference", "Size Reference |B|", 0, 0, INT_MAX)
    , truePositive_("truePositive", "True Positive (TP)", 0, 0, INT_MAX)
    , falsePositive_("falsePositive", "False Positive (FP)", 0, 0, INT_MAX)
    , falseNegative_("falseNegative", "False Negative (FN)", 0, 0, INT_MAX)
    , trueNegative_("trueNegative", "True Negative (TN)", 0, 0, INT_MAX)
    , jaccardIndex_("jaccardIndex", "Jaccard Index")
    , diceIndex_("diceIndex", "Dice Index")
    , sensitivity_("sensitivity", "Sensitivity")
    , specificity_("specificity", "Specificity")
    , forceUpdate_(false)
{
    inportSegmentation_.addCondition(new PortConditionVolumeChannelCount(1));
    inportReference_.addCondition(new PortConditionVolumeChannelCount(1));
    addPort(inportSegmentation_);
    addPort(inportReference_);

    jaccardIndex_.setNumDecimals(4);
    diceIndex_.setNumDecimals(4);
    sensitivity_.setNumDecimals(4);
    specificity_.setNumDecimals(4);

    computeButton_.onClick(CallMemberAction<SegmentationValidation>(this, &SegmentationValidation::forceUpdate));
    sizeSegmentation_.setWidgetsEnabled(false);
    sizeReference_.setWidgetsEnabled(false);
    truePositive_.setWidgetsEnabled(false);
    falsePositive_.setWidgetsEnabled(false);
    falseNegative_.setWidgetsEnabled(false);
    trueNegative_.setWidgetsEnabled(false);
    jaccardIndex_.setWidgetsEnabled(false);
    diceIndex_.setWidgetsEnabled(false);
    sensitivity_.setWidgetsEnabled(false);
    specificity_.setWidgetsEnabled(false);

    addProperty(computeButton_);
    addProperty(sizeSegmentation_);
    addProperty(sizeReference_);
    addProperty(truePositive_);
    addProperty(falsePositive_);
    addProperty(falseNegative_);
    addProperty(trueNegative_);
    addProperty(jaccardIndex_);
    addProperty(diceIndex_);
    addProperty(sensitivity_);
    addProperty(specificity_);
}

Processor* SegmentationValidation::create() const {
    return new SegmentationValidation();
}

void SegmentationValidation::process() {
    if (!forceUpdate_)
        return;

    forceUpdate_ = false;

    if (!inportSegmentation_.getData() || !inportReference_.getData()) {
        resetOutput();
    }
    else {
        computeMetrics();
    }
}

void SegmentationValidation::forceUpdate() {
    forceUpdate_ = true;
    process();
}

void SegmentationValidation::computeMetrics() {
    const VolumeBase* segHandle = inportSegmentation_.getData();
    const VolumeBase* refHandle = inportReference_.getData();
    tgtAssert(segHandle && refHandle, "No input data");

    const VolumeRAM* segVolume = segHandle->getRepresentation<VolumeRAM>();
    const VolumeRAM* refVolume = refHandle->getRepresentation<VolumeRAM>();
    tgtAssert(segVolume && refVolume, "Missing volumes");

    // check input data
    if (segVolume->getDimensions() != refVolume->getDimensions()) {
        LWARNING("Volumes must have the same dimensions");
        resetOutput();
        return;
    }

    size_t numVoxels = refVolume->getNumVoxels();

    // compute measures
    size_t sizeSeg = VolumeOperatorNumSignificant::APPLY_OP(segHandle);
    size_t sizeRef = VolumeOperatorNumSignificant::APPLY_OP(refHandle);

    size_t truePositive = getNumCommonVoxels(segVolume, refVolume);
    size_t falsePositive = sizeSeg - truePositive;
    size_t falseNegative = sizeRef - truePositive;
    size_t trueNegative = numVoxels - truePositive - falsePositive - falseNegative;

    tgtAssert(truePositive <= sizeSeg && truePositive <= sizeRef, "Invalid result");
    tgtAssert(numVoxels == (truePositive + falsePositive + falseNegative + trueNegative), "Invalid result");
    tgtAssert(sizeSeg == (truePositive + falsePositive), "Invalid result");
    tgtAssert(sizeRef == (truePositive + falseNegative), "Invalid result");

    // compute derived indices
    float jaccard = truePositive / static_cast<float>(falsePositive + truePositive + falseNegative);
    float dice = 2.f*truePositive / static_cast<float>((falsePositive + truePositive) + (truePositive + falseNegative));
    float sensitivity = truePositive / static_cast<float>((truePositive + falseNegative));
    float specificity = trueNegative / static_cast<float>((trueNegative + falsePositive));

    // update ranges
    sizeSegmentation_.setMaxValue(static_cast<int>(numVoxels));
    sizeReference_.setMaxValue(static_cast<int>(numVoxels));
    truePositive_.setMaxValue(static_cast<int>(numVoxels));
    falsePositive_.setMaxValue(static_cast<int>(numVoxels));
    falseNegative_.setMaxValue(static_cast<int>(numVoxels));
    trueNegative_.setMaxValue(static_cast<int>(numVoxels));

    // put out measures
    sizeSegmentation_.set(static_cast<int>(sizeSeg));
    sizeReference_.set(static_cast<int>(sizeRef));
    truePositive_.set(static_cast<int>(truePositive));
    falsePositive_.set(static_cast<int>(falsePositive));
    falseNegative_.set(static_cast<int>(falseNegative));
    trueNegative_.set(static_cast<int>(trueNegative));
    jaccardIndex_.set(jaccard);
    diceIndex_.set(dice);
    sensitivity_.set(sensitivity);
    specificity_.set(specificity);
}

void SegmentationValidation::resetOutput() {
    sizeSegmentation_.set(0);
    sizeReference_.set(0);
    truePositive_.set(0);
    falsePositive_.set(0);
    falseNegative_.set(0);
    trueNegative_.set(0);
    jaccardIndex_.set(0.f);
    diceIndex_.set(0.f);
    sensitivity_.set(0.f);
    specificity_.set(0.f);
}

size_t SegmentationValidation::getNumCommonVoxels(const VolumeRAM* volumeA, const VolumeRAM* volumeB) {
    tgtAssert(volumeA && volumeB, "Null pointer passed");
    tgtAssert(volumeA->getDimensions() == volumeB->getDimensions(), "Volume dimensions differ");

    size_t result = 0;
    VRN_FOR_EACH_VOXEL(pos, tgt::ivec3(0), volumeA->getDimensions()) {
        if (volumeA->getVoxelNormalized(pos) > 0.f && volumeB->getVoxelNormalized(pos) > 0.f)
            result++;
    }
    return result;
}

} // namespace
