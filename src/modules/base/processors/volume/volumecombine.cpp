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

#include "voreen/modules/base/processors/volume/volumecombine.h"

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumeoperator.h"
#include "voreen/core/datastructures/geometry/meshlistgeometry.h"

using tgt::ivec3;
using tgt::vec3;

namespace voreen {

const std::string VolumeCombine::loggerCat_("voreen.VolumeCombine");

VolumeCombine::VolumeCombine()
    : VolumeProcessor()
    , inportFirst_(Port::INPORT, "volume.first")
    , inportSecond_(Port::INPORT, "volume.second")
    , outport_(Port::OUTPORT, "outport", true)
    , enableProcessing_("enabled", "Enable", true)
    , combineFunction_("combineFunction", "Combine Function")
    , factorC_("factorC", "Factor c", 0.5f, -2.f, 2.f)
    , factorD_("factorD", "Factor d", 0.5f, -2.f, 2.f)
    , referenceVolume_("referenceVolume", "Reference Volume")
    , volumeOwner_(false)
{
    addPort(inportFirst_);
    addPort(inportSecond_);
    addPort(outport_);

    combineFunction_.addOption("max",              "Max (max(A,B))",             OP_MAX);
    combineFunction_.addOption("min",              "Min (min(A,B))",             OP_MIN);
    combineFunction_.addOption("add",              "Add (A+B)",                  OP_ADD);
    combineFunction_.addOption("a-minus-b",        "Subtract (A-B)",             OP_A_MINUS_B);
    combineFunction_.addOption("b-minus-a",        "Subtract (B-A)",             OP_B_MINUS_A);
    combineFunction_.addOption("avg",              "Avg ((A+B)/2)",              OP_AVG);
    combineFunction_.addOption("weightedSum",      "Weighted Sum (c*A+(1-c)*B)", OP_WEIGHTED_SUM);
    combineFunction_.addOption("weightedSum2p",    "Weighted Sum (c*A+d*B)",     OP_WEIGHTED_SUM_2P);
    combineFunction_.addOption("blend",            "Blend (A+B*(1-A))",          OP_BLEND);
    combineFunction_.addOption("mask-a-by-b",      "Mask A by B (B?A:0)",        OP_MASK_A_BY_B);
    combineFunction_.addOption("mask-b-by-a",      "Mask B by A (A?B:0)",        OP_MASK_B_BY_A);
    combineFunction_.addOption("priorityFirst",    "Priority First (A?A:B)",     OP_PRIORITY_FIRST);
    combineFunction_.addOption("prioritySecond",   "Priority Second (B?B:A)",    OP_PRIORITY_SECOND);
    combineFunction_.addOption("takeFirst",        "Take First (A)",             OP_TAKE_FIRST);
    combineFunction_.addOption("takeSecond",       "Take Second (B)",            OP_TAKE_SECOND);
    combineFunction_.select("max");

    referenceVolume_.addOption("first", "First");
    referenceVolume_.addOption("second", "Second");

    combineFunction_.onChange(CallMemberAction<VolumeCombine>(this, &VolumeCombine::adjustPropertyVisibilities));

    factorC_.setTracking(false);
    factorD_.setTracking(false);

    addProperty(enableProcessing_);
    addProperty(combineFunction_);
    addProperty(factorC_);
    addProperty(factorD_);
    addProperty(referenceVolume_);

    adjustPropertyVisibilities();
}

VolumeCombine::~VolumeCombine() {}

Processor* VolumeCombine::create() const {
    return new VolumeCombine();
}

std::string VolumeCombine::getProcessorInfo() const {
    return "Combines two volumes based on a selectable function.";
}

void VolumeCombine::process() {
    tgtAssert(inportFirst_.getData() && inportFirst_.getData()->getVolume(), "No input volume");
    tgtAssert(inportSecond_.getData() && inportSecond_.getData()->getVolume(), "No input volume");

    Volume* firstVolume = inportFirst_.getData()->getVolume();
    Volume* secondVolume = inportSecond_.getData()->getVolume();
    Volume* combinedVolume = 0;

    if (!enableProcessing_.get()) {
        outport_.setData(inportFirst_.getData(), volumeOwner_);
        volumeOwner_ = false;
        return;
    }
    else if (firstVolume->getNumChannels() > 1 || secondVolume->getNumChannels() > 1) {
        LWARNING("Combination of multi-channel volumes currently not supported.");
        outport_.setData(inportFirst_.getData(), volumeOwner_);
        volumeOwner_ = false;
        return;
    }

    // optimized combination for volumes that share a common grid in world-space
    if (firstVolume->getDimensions() == secondVolume->getDimensions() &&
        firstVolume->getCubeSize() == secondVolume->getCubeSize()     &&
        firstVolume->getTransformation() == secondVolume->getTransformation()) {

        try {
            if (referenceVolume_.isSelected("first"))
                combinedVolume = firstVolume->clone();
            else
                combinedVolume = secondVolume->clone();
        }
        catch (const std::bad_alloc&) {
            LERROR("Failed to create combined volume with dimensions " << firstVolume->getDimensions()
                << " : bad allocation");
        }

        if (combinedVolume) {
            LINFO("Performing optimized combination on common grid with dimensions "
                << firstVolume->getDimensions() << "...");
            combineVolumesOnCommonGrid(combinedVolume, firstVolume, secondVolume, combineFunction_.getValue());
        }
    }
    // standard combination with resampling
    else {
        if (referenceVolume_.isSelected("first"))
            combinedVolume = createCombinedVolume(firstVolume, secondVolume);
        else
            combinedVolume = createCombinedVolume(secondVolume, firstVolume);

        if (combinedVolume) {
            LINFO("Creating combined volume with dimensions " << combinedVolume->getDimensions() << " ...");
            combineVolumes(combinedVolume, firstVolume, secondVolume, combineFunction_.getValue());
        }
    }

    // put out combined volume
    if (combinedVolume) {
        outport_.setData(new VolumeHandle(combinedVolume), volumeOwner_);
        volumeOwner_ = true;
    }
    else {
        outport_.setData(0, volumeOwner_);
        volumeOwner_ = false;
    }
}

void VolumeCombine::deinitialize() throw (VoreenException) {
    if (volumeOwner_) {
        outport_.deleteVolume();
        volumeOwner_ = false;
    }

    VolumeProcessor::deinitialize();
}

inline bool withinRange(const tgt::vec3& pos, const tgt::vec3& llf, const tgt::vec3& urb) {
    return tgt::hand(tgt::greaterThanEqual(pos, llf)) &&
           tgt::hand(tgt::lessThanEqual(   pos, urb));
}

void VolumeCombine::combineVolumes(Volume* combinedVolume, const Volume* firstVolume,
                                   const Volume* secondVolume, CombineOperation operation) const {

    tgtAssert(combinedVolume && firstVolume && secondVolume, "Null pointer passed");

    //
    // compute transformation from voxel coordinates of combined volume
    // to voxel coords of input volumes
    //
    tgt::mat4 combinedToFirst = computeConversionMatrix(firstVolume, combinedVolume);
    tgt::mat4 combinedToSecond = computeConversionMatrix(secondVolume, combinedVolume);

    LDEBUG("Voxel-to-world (First): " << combinedToFirst);
    LDEBUG("Voxel-to-world (Second) " << combinedToSecond);

    //
    // voxel-wise combination
    //
    tgt::vec3 dimFirst(firstVolume->getDimensions()-1);
    tgt::vec3 dimSecond(secondVolume->getDimensions()-1);
    const float c = factorC_.get();
    const float d = factorD_.get();
    VRN_FOR_EACH_VOXEL_WITH_PROGRESS(pos, tgt::ivec3(0), combinedVolume->getDimensions(), progressBar_) {

        // transform sampling pos to coordinate systems of input volumes
        tgt::vec3 posFirst = combinedToFirst*tgt::vec3(pos);
        tgt::vec3 posSecond = combinedToSecond*tgt::vec3(pos);

        // sample input volumes, if transformed voxel position lies inside respective volume
        float valFirst = 0.f;
        float valSecond = 0.f;
        if (withinRange(posFirst, tgt::vec3::zero, dimFirst))
            valFirst = firstVolume->getVoxelFloatLinear(posFirst);
        if (withinRange(posSecond, tgt::vec3::zero, dimSecond))
            valSecond = secondVolume->getVoxelFloatLinear(posSecond);

        // apply operation to sampled values (note: a switch-block within a volume traversal loop
        // should normally be avoided, however in this case the main operations are way more expensive)
        float result = 0.f;
        switch (operation) {
            case OP_MAX:
                result = std::max(valFirst, valSecond);
                break;
            case OP_MIN:
                result = std::min(valFirst, valSecond);
                break;
            case OP_ADD:
                result = valFirst + valSecond;
                break;
            case OP_A_MINUS_B:
                result = valFirst - valSecond;
                break;
            case OP_B_MINUS_A:
                result = valSecond - valFirst;
                break;
            case OP_AVG:
                result = (valFirst + valSecond) / 2.f;
                break;
            case OP_WEIGHTED_SUM:
                result = c*valFirst + (1.f-c)*valSecond;
                break;
            case OP_WEIGHTED_SUM_2P:
                result = c*valFirst + d*valSecond;
                break;
            case OP_BLEND:
                result = valFirst + valSecond*(1.f - valFirst);
                break;
            case OP_MASK_A_BY_B:
                result = (valSecond > 0.f) ? valFirst : 0.f;
                break;
            case OP_MASK_B_BY_A:
                result = (valFirst > 0.f) ? valSecond : 0.f;
                break;
            case OP_PRIORITY_FIRST:
                result = (valFirst > 0.f) ? valFirst : valSecond;
                break;
            case OP_PRIORITY_SECOND:
                result = (valSecond > 0.f) ? valSecond : valFirst;
                break;
            case OP_TAKE_FIRST:
                result = valFirst;
                break;
            case OP_TAKE_SECOND:
                result = valSecond;
                break;
            default:
                LERROR("Unknown operation: " << combineFunction_.get());
                return;
        }

        // assign clamped result to combined volume
        combinedVolume->setVoxelFloat(tgt::clamp(result, 0.f, 1.f), pos);

    } // VRN_FOR_EACH_VOXEL_WITH_PROGRESS
}

void VolumeCombine::combineVolumesOnCommonGrid(Volume* combinedVolume, const Volume* firstVolume,
                                               const Volume* secondVolume, CombineOperation operation) const {
    tgtAssert(combinedVolume && firstVolume && secondVolume, "Null pointer passed");
    tgtAssert(combinedVolume->getDimensions() == firstVolume->getDimensions() &&
              combinedVolume->getDimensions() == secondVolume->getDimensions(), "Volume dimensions mismatch");

    const float c = factorC_.get();
    const float d = factorD_.get();
    VRN_FOR_EACH_VOXEL_WITH_PROGRESS(pos, tgt::ivec3(0), combinedVolume->getDimensions(), progressBar_) {
        float valFirst = firstVolume->getVoxelFloat(pos);
        float valSecond = secondVolume->getVoxelFloat(pos);

        // apply operation to input voxel values
        float result = 0.f;
        switch (operation) {
            case OP_MAX:
                result = std::max(valFirst, valSecond);
                break;
            case OP_MIN:
                result = std::min(valFirst, valSecond);
                break;
            case OP_ADD:
                result = valFirst + valSecond;
                break;
            case OP_A_MINUS_B:
                result = valFirst - valSecond;
                break;
            case OP_B_MINUS_A:
                result = valSecond - valFirst;
                break;
            case OP_AVG:
                result = (valFirst + valSecond) / 2.f;
                break;
            case OP_WEIGHTED_SUM:
                result = c*valFirst + (1.f-c)*valSecond;
                break;
            case OP_WEIGHTED_SUM_2P:
                result = c*valFirst + d*valSecond;
                break;
            case OP_BLEND:
                result = valFirst + valSecond*(1.f - valFirst);
                break;
            case OP_MASK_A_BY_B:
                result = (valSecond > 0.f) ? valFirst : 0.f;
                break;
            case OP_MASK_B_BY_A:
                result = (valFirst > 0.f) ? valSecond : 0.f;
                break;
            case OP_PRIORITY_FIRST:
                result = (valFirst > 0.f) ? valFirst : valSecond;
                break;
            case OP_PRIORITY_SECOND:
                result = (valSecond > 0.f) ? valSecond : valFirst;
                break;
            case OP_TAKE_FIRST:
                result = valFirst;
                break;
            case OP_TAKE_SECOND:
                result = valSecond;
                break;
            default:
                LERROR("Unknown operation: " << combineFunction_.get());
                return;
        }

        // assign clamped result to combined volume
        combinedVolume->setVoxelFloat(tgt::clamp(result, 0.f, 1.f), pos);

    } // VRN_FOR_EACH_VOXEL_WITH_PROGRESS
}

Volume* VolumeCombine::createCombinedVolume(const Volume* refVolume, const Volume* secondVolume) const {
    // compute untransformed bounding box of reference volume
    tgt::vec3 refLLF = refVolume->getLLF();  // all components negative
    tgt::vec3 refURB = refVolume->getURB();  // all components positive
    std::pair<vec3, vec3> refBB = std::pair<vec3, vec3>(refVolume->getLLF(), refVolume->getURB());

    // create bounding box for second volume and transform into reference coordinate system
    MeshGeometry secondBB = MeshGeometry::createCube(secondVolume->getLLF(), secondVolume->getURB());
    tgt::mat4 transformToRef;
    refVolume->getTransformation().invert(transformToRef);
    transformToRef *= secondVolume->getTransformation();
    secondBB.transform(transformToRef);

    // determine combined bounding box of reference volume and transformed second volume
    tgt::vec3 combinedLLF = refLLF;
    tgt::vec3 combinedURB = refURB;
    for (MeshGeometry::const_iterator face = secondBB.begin(); face != secondBB.end(); ++face) {
        for (FaceGeometry::const_iterator vertex = face->begin(); vertex != face->end(); ++vertex) {
            combinedLLF = tgt::min(combinedLLF, vertex->getCoords());
            combinedURB = tgt::max(combinedURB, vertex->getCoords());
        }
    }
    std::pair<vec3, vec3> combinedBB = std::pair<vec3, vec3>(combinedLLF, combinedURB);
    LDEBUG("Combined BB: " << combinedBB.first << ", " << combinedBB.second);
    tgtAssert(tgt::hand(tgt::lessThanEqual(combinedBB.first, refBB.first)) &&
        tgt::hand(tgt::greaterThanEqual(combinedBB.second, refBB.second)), "Invalid combined bounding box");

    // derive resolution of combined volume from size ratios of combinedBB/refBB and the ref volume's
    // original resolution => spatial resolution of combined volume equals spatial res. of reference volume
    tgt::vec3 scaleFactors = (combinedBB.second - combinedBB.first) / (refBB.second - refBB.first);
    tgt::ivec3 combinedDim = tgt::iceil(tgt::vec3(refVolume->getDimensions()) * scaleFactors);
    LDEBUG("Scale factors: " << scaleFactors);
    LDEBUG("Common Dim: " << combinedDim);
    tgtAssert(tgt::hand(tgt::greaterThanEqual(combinedDim, refVolume->getDimensions())),
        "Invalid combined volume dimensions");

    // create combined volume with proper dimensions and spacing
    Volume* combinedVolume = 0;
    try {
        VolumeOperatorResize voResize(combinedDim);
        combinedVolume = voResize.apply<Volume*>(const_cast<Volume*>(refVolume));
    }
    catch (const std::bad_alloc&) {
        LERROR("Failed to create combined volume with dimensions " << combinedDim << " : bad allocation");
        delete combinedVolume;
        return 0;
    }

    // determine transformation of combinedVolume
    // to the scale and position of the computed combinedBB
    tgt::vec3 combinedShift = (combinedBB.second + combinedBB.first) / 2.f;
    tgt::vec3 combinedScale = (combinedBB.second - combinedBB.first) /
        (combinedVolume->getURB() - combinedVolume->getLLF());
    tgt::mat4 combinedTrafo = tgt::mat4::createTranslation(combinedShift);
    combinedTrafo *= tgt::mat4::createScale(combinedScale);

    // apply transformation to combined volume
    combinedVolume->setTransformation(combinedVolume->getTransformation() * combinedTrafo);

    return combinedVolume;
}

void VolumeCombine::adjustPropertyVisibilities() {
    factorC_.setVisible(combineFunction_.isSelected("weightedSum") || combineFunction_.isSelected("weightedSum2p"));
    factorD_.setVisible(combineFunction_.isSelected("weightedSum2p"));
}

} // namespace
