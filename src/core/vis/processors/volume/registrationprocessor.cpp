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

#include "voreen/core/vis/processors/volume/registrationprocessor.h"
#include "voreen/core/volume/volume.h"
#include "voreen/core/volume/volumehandle.h"

namespace voreen {

using tgt::vec3;
using tgt::vec4;
using tgt::mat4;

const std::string RegistrationProcessor::inportName_("volumehandle.input");
const std::string RegistrationProcessor::referenceInportName_("volumehandle.referenceInput");
const std::string RegistrationProcessor::outportName_("volumehandle.output");

RegistrationProcessor::RegistrationProcessor()
    : VolumeProcessor()
    , inputVolumeHandle_(0)
    , referenceInputVolumeHandle_(0)
    , outputVolumeHandle_(0)
    , processedVolumeHandle_(0)
    , enableProcessingProp_("enableProcessing", "enable: ")
    , p1_("p1", "Point 1", vec3(0.5,0.5,0.5))
    , p2_("p2", "Point 2", vec3(0.0,0.5,0.5))
    , p3_("p3", "Point 3", vec3(1.0,0.5,0.5))
    , ref_p1_("ref_p1", "Point 1 (Reference)", vec3(0.5,0.5,0.5))
    , ref_p2_("ref_p2", "Point 2 (Reference)", vec3(0.0,0.5,0.5))
    , ref_p3_("ref_p3", "Point 3 (Reference)", vec3(1.0,0.5,0.5))
    , forceUpdate_(true)
    , inport_(Port::INPORT, "volumehandle.input")
    , referenceInport_(Port::INPORT, "volumehandle.referenceInput")
    , outport_(Port::OUTPORT, "volumehandle.output", 0)
{
    addPort(inport_);
    addPort(referenceInport_);
    addPort(outport_);

    enableProcessingProp_.onChange(
        CallMemberAction<RegistrationProcessor>(this, &RegistrationProcessor::forceUpdate));
    addProperty(enableProcessingProp_);
    addProperty(p1_);
    addProperty(p2_);
    addProperty(p3_);
    addProperty(ref_p1_);
    addProperty(ref_p2_);
    addProperty(ref_p3_);
}

RegistrationProcessor::~RegistrationProcessor() {
    delete processedVolumeHandle_;
}

const std::string RegistrationProcessor::getProcessorInfo() const {
    return std::string("Registers one volume to another by calculating a transformation matrix.");
}

void RegistrationProcessor::process() {
    bool handleChanged = false;
    //bool res = VolumeHandleValidator::checkVolumeHandle(inputVolumeHandle_,
        //getVolumeHandle(inportName_), &handleChanged);
    inputVolumeHandle_ = inport_.getData();
    referenceInputVolumeHandle_ = referenceInport_.getData();
    //bool res2 = VolumeHandleValidator::checkVolumeHandle(referenceInputVolumeHandle_,
        //getVolumeHandle(referenceInportName_), &handleChanged);
    // if the processor is disable, "short-circuit" the in- and outport
    //
    if (enableProcessingProp_.get() == false) {
        outputVolumeHandle_ = inputVolumeHandle_;
        return;
    }

    if (inputVolumeHandle_ && referenceInputVolumeHandle_ && ((forceUpdate_ == true) || (handleChanged == true)))
        registerVolume();
}

// private methods
//

void RegistrationProcessor::forceUpdate() {
    forceUpdate_ = true;
}

void RegistrationProcessor::registerVolume() {
    forceUpdate_ = false;

    if (inputVolumeHandle_ == 0)
        return;

    Volume* inputVolume = inputVolumeHandle_->getVolume()->clone();
    Volume* referenceInputVolume = referenceInputVolumeHandle_->getVolume();
    Volume* outputVolume = inputVolume->clone();

    //----------------------------------------------------------------------
    //adapted voltool code:

    vec3 p0dest = vec3(p1_.get().x * inputVolume->getDimensions().x, p1_.get().y * inputVolume->getDimensions().y, p1_.get().z * inputVolume->getDimensions().z);
    vec3 p0src = vec3(ref_p1_.get().x * inputVolume->getDimensions().x, ref_p1_.get().y * inputVolume->getDimensions().y, ref_p1_.get().z * inputVolume->getDimensions().z);

    vec3 p1dest = vec3(p2_.get().x * inputVolume->getDimensions().x, p2_.get().y * inputVolume->getDimensions().y, p2_.get().z * inputVolume->getDimensions().z);
vec3 p1src = vec3(ref_p2_.get().x * inputVolume->getDimensions().x, ref_p2_.get().y * inputVolume->getDimensions().y, ref_p2_.get().z * inputVolume->getDimensions().z);


    vec3 p2dest = vec3(p2_.get().x * inputVolume->getDimensions().x, p3_.get().y * inputVolume->getDimensions().y, p3_.get().z * inputVolume->getDimensions().z);
    vec3 p2src = vec3(ref_p3_.get().x * inputVolume->getDimensions().x, ref_p3_.get().y * inputVolume->getDimensions().y, ref_p3_.get().z * inputVolume->getDimensions().z);

    std::cout << p0dest << p0src << p1dest << p1src << p2dest << p2src << std::endl;

    // load volumes
    Volume* destVol = outputVolume;
    Volume* srcVol = referenceInputVolume;

    // transform reference points from voxel coordinates to world coordinates
    p0dest = transformFromVoxelToWorldCoords(p0dest, destVol);
    p0src = transformFromVoxelToWorldCoords(p0src, srcVol);

    p1dest = transformFromVoxelToWorldCoords(p1dest, destVol);
    p1src = transformFromVoxelToWorldCoords(p1src, srcVol);

    p2dest = transformFromVoxelToWorldCoords(p2dest, destVol);
    p2src = transformFromVoxelToWorldCoords(p2src, srcVol);

    std::cout << p0dest << p0src << p1dest << p1src << p2dest << p2src << std::endl;

    // calculate scaling factor (note: the scaling is assumed to be isotrope)
    float scale01 = length(p0dest - p1dest) / length(p0src - p1src);
    float scale02 = length(p0dest - p2dest) / length(p0src - p2src);
    float scale12 = length(p1dest - p2dest) / length(p1src - p2src);
    float avgScale = (scale01 + scale02 + scale12) / 3.f; // values may differ due to inaccuracies of reference points

    std::cout << scale01 << scale02 << scale12;
    // uniform scaling matrix
    mat4 srcScale = mat4::createScale(vec3(avgScale, avgScale, avgScale));

    // calculate corresponding tripods in source and dest coordinate systems from reference points
    vec3 p01dest = p0dest - p1dest;
    vec3 p01src = p0src - p1src;

    vec3 p02dest = p0dest - p2dest;
    vec3 p02src = p0src - p2src;

    vec3 p12dest = p1dest - p2dest;
    vec3 p12src = p1src - p2src;

    vec3 aDest = normalize(p01dest);
    vec3 aSrc = normalize(p01src);

    vec3 bDest = normalize(cross(p01dest, p02dest));
    vec3 bSrc = normalize(cross(p01src, p02src));

    vec3 cDest = cross(bDest, aDest);
    vec3 cSrc = cross(bSrc, aSrc);

    // rotation matrices mapping the tripod in source coordinates to the tripod in dest coordinates
    mat4 srcRot = mat4( vec4(aSrc.x, aSrc.y, aSrc.z, 0.f),
                        vec4(bSrc.x, bSrc.y, bSrc.z, 0.f),
                        vec4(cSrc.x, cSrc.y, cSrc.z, 0.f),
                        vec4(0.f, 0.f, 0.f, 1.f) );
    mat4 destRot = tgt::transpose( mat4( vec4(aDest.x, aDest.y, aDest.z, 0.f),
                                         vec4(bDest.x, bDest.y, bDest.z, 0.f),
                                         vec4(cDest.x, cDest.y, cDest.z, 0.f),
                                         vec4(0.f, 0.f, 0.f, 1.f)) );

    // translation matrices
    mat4 srcTranslate = mat4::createTranslation(-p0src);
    mat4 destTranslate = mat4::createTranslation(p0dest);

    /* compose final registration matrix:
       1. translate p0source to origin
       2. rotate source tripod to be axis-aligned
       3. adjust (uniform!) scaling
       4. rotate axis-aligned tripod to fit dest tripod
       5. translate to p0dest
    */
    mat4 M = destTranslate * destRot * srcScale * srcRot * srcTranslate;

    std::cout << "\nRegistration matrix M mapping from source to dest volume coordinates: \n";
    std::cout << M << "\n";

    vec3 p0trans = (M*vec4(p0src.x, p0src.y, p0src.z, 1.f)).xyz();
    vec3 p1trans = (M*vec4(p1src.x, p1src.y, p1src.z, 1.f)).xyz();
    vec3 p2trans = (M*vec4(p2src.x, p2src.y, p2src.z, 1.f)).xyz();

    std::cout << "Deviation vectors (should be zero): \n";
    std::cout << "M*P0'-P0: " << (p0trans - p0dest) << "\n";
    std::cout << "M*P1'-P1: " << (p1trans - p1dest) << "\n";
    std::cout << "M*P2'-P2: " << (p2trans - p2dest) << "\n\n";

    outputVolume->setTransformation(M);
    //---------------------------------------------------------------------
    delete processedVolumeHandle_;
    processedVolumeHandle_ = new VolumeHandle(outputVolume, 0.0f);
    outputVolumeHandle_ = processedVolumeHandle_;
}

vec3 RegistrationProcessor::transformFromVoxelToWorldCoords(vec3 point, Volume* vol) {
    point += vec3(0.5f, 0.5f, 0.5f);
    vec3 transformed = -(vol->getCubeSize()/2.f) + (point / static_cast<vec3>(vol->getDimensions())) * (vol->getCubeSize());

    return transformed;
}

}   // namespace
