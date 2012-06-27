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

#include "voreen/modules/base/processors/utility/segmentationvalidation.h"

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumeoperator.h"

#include <climits>

namespace voreen {

const std::string SegmentationValidation::loggerCat_("voreen.SegmentationValidation");

SegmentationValidation::SegmentationValidation()
    : Processor()
    , inportSegmentation_(Port::INPORT, "segmentation.in")
    , inportReference_(Port::INPORT, "segmentation.reference")
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

std::string SegmentationValidation::getProcessorInfo() const {
    return "Measures the quality of a volume segmentation by comparing it against a reference segmentation. "
           "Several similarity measures are computed. The results are displayed by read-only properties.";
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
    VolumeHandle* segHandle = inportSegmentation_.getData();
    VolumeHandle* refHandle = inportReference_.getData();
    tgtAssert(segHandle && refHandle, "No input data");

    Volume* segVolume = segHandle->getVolume();
    Volume* refVolume = refHandle->getVolume();
    tgtAssert(segVolume && refVolume, "Missing volumes");

    // check input data
    if (segVolume->getDimensions() != refVolume->getDimensions()) {
        LWARNING("Volumes must have the same dimensions");
        resetOutput();
        return;
    }

    if ((segVolume->getNumChannels() > 1) || (refVolume->getNumChannels() > 1)) {
        LWARNING("Multi channel volumes not supported");
        resetOutput();
        return;
    }

    size_t numVoxels = refVolume->getNumVoxels();

    // compute measures
    VolumeOperatorNumSignificant volOpNum;
    size_t sizeSeg = volOpNum.apply<size_t>(segVolume);
    size_t sizeRef = volOpNum.apply<size_t>(refVolume);

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

size_t SegmentationValidation::getNumCommonVoxels(Volume* volumeA, Volume* volumeB) {
    tgtAssert(volumeA && volumeB, "Null pointer passed");
    tgtAssert(volumeA->getDimensions() == volumeB->getDimensions(), "Volume dimensions differ");

    size_t result = 0;
    VRN_FOR_EACH_VOXEL(pos, tgt::ivec3(0), volumeA->getDimensions()) {
        if (volumeA->getVoxelFloat(pos) > 0.f && volumeB->getVoxelFloat(pos) > 0.f)
            result++;
    }
    return result;
}

} // namespace
