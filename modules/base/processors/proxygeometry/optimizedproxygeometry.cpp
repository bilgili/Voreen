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

#include "optimizedproxygeometry.h"
#include "voreen/core/datastructures/geometry/trianglemeshgeometry.h"
#include <queue>

namespace {

/**
 * Helper class for iterating over a volume
 */
class VolumeIterator {
private:
    tgt::ivec3 llf_;
    tgt::ivec3 urb_;
    tgt::ivec3 pos_;
public:
    VolumeIterator(tgt::ivec3 llf, tgt::ivec3 urb) : llf_(llf), urb_(urb), pos_(llf) {}
    VolumeIterator(tgt::ivec3 size) : llf_(tgt::ivec3(0)), urb_(size-1), pos_(tgt::ivec3(0)) {}
    void next() {
        pos_.x++;
        if (pos_.x > urb_.x) {
            pos_.x = llf_.x;
            pos_.y++;
            if (pos_.y > urb_.y) {
                pos_.y = llf_.y;
                pos_.z++;
            }
        }
    }
    bool hasnext() {return pos_.x < urb_.x || pos_.y < urb_.y || pos_.z < urb_.z;}
    tgt::ivec3 value() {return pos_;}
    bool outofrange() {return pos_.z > urb_.z;}
    tgt::ivec3 getnext() {next(); return value();}
};

} // namespace anonymous

//-----------------------------------------------------------------------------

namespace voreen {

// OptimizedProxyGeometry
const std::string OptimizedProxyGeometry::loggerCat_("voreen.base.OptimizedProxyGeometry");

OptimizedProxyGeometry::OptimizedProxyGeometry()
    : Processor()
    , inport_(Port::INPORT, "volumehandle.volumehandle", "Volume Input")
    , outport_(Port::OUTPORT, "proxygeometry.geometry", "Proxy Geometry Output")
    , geometry_(0)
    , tmpGeometry_(0)
    , mode_("modeString", "Mode")
    , tfChannel0_("transferfunction", "Transfer Function (Channel 0)")
    , resolutionMode_("resolutionMode", "Resolution Mode", Processor::VALID)
    , resolution_("resolution", "Resolution", 32, 1, 64)
    , resolutionVoxels_("resolutionvoxel", "Edge Length (Voxels)", 16, 1, 1024)
    , threshold_("threshold", "Visibility Threshold (*10e-4)", 1, 0, 100)
    , enableClipping_("useClipping", "Enable Clipping", true)
    , clipRight_("rightClippingPlane", "Right Clip Plane (x)", 0.f, 0.f, 1e5f)
    , clipLeft_("leftClippingPlane", "Left Clip Plane (x)", 1e5f, 0.f, 1e5f)
    , clipFront_("frontClippingPlane", "Front Clip Plane (y)", 0.f, 0.f, 1e5f)
    , clipBack_("backClippingPlane", "Back Clip Plane (y)", 1e5f, 0.f, 1e5f)
    , clipBottom_("bottomClippingPlane", "Bottom Clip Plane (z)", 0.f, 0.f, 1e5f)
    , clipTop_("topClippingPlane", "Top Clip Plane (z)", 1e5f, 0.f, 1e5f)
    , resetClipPlanes_("resetClipPlanes", "Reset Planes")
    , waitForOptimization_("waitForOptimization", "Wait for optimization", false, Processor::VALID)
    , geometryInvalid_(true)
    , structureInvalid_(true)
    , volStructureSize_(0,0,0)
    , backgroundThread_(0)
    , tfChannel1_("transferfunction2", "Transfer Function (Channel 1)")
    , tfChannel2_("transferfunction3", "Transfer Function (Channel 2)")
    , tfChannel3_("transferfunction4", "Transfer Function (Channel 3)")
{
    //create mesh list geometry
    geometry_ = new TriangleMeshGeometryVec4Vec3();
    tmpGeometry_ = new TriangleMeshGeometryVec4Vec3();

    addPort(inport_);
    addPort(outport_);

    mode_.addOption("boundingbox",              "Bounding Box");
    mode_.addOption("minboundingbox",           "Minimal Visible Bounding Box");
    mode_.addOption("visiblebricks",            "Visible Bricks");
    mode_.addOption("outerfaces",               "Visible Bricks (Outer Faces)");
    mode_.addOption("volumeoctree",             "Volume Octree");
    mode_.addOption("volumeoctreeouterfaces",   "Volume Octree (Outer Faces)");
    mode_.set("visiblebricks");
    addProperty(mode_);

    addProperty(tfChannel0_);
    tfChannel0_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onTransFuncChange));

    addProperty(tfChannel1_);
    addProperty(tfChannel2_);
    addProperty(tfChannel3_);
    tfChannel1_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onTransFuncChange));
    tfChannel2_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onTransFuncChange));
    tfChannel3_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onTransFuncChange));

    resolutionMode_.addOption("subdivide", "Subdivide Shortest Side");
    resolutionMode_.addOption("voxel", "Subdivide in Voxels");
    addProperty(resolutionMode_);
    resolutionMode_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onResolutionModeChange));

    addProperty(resolution_);

    addProperty(resolutionVoxels_);

    addProperty(threshold_);
    threshold_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onThresholdChange));

    clipRight_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onClipRightChange));
    clipLeft_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onClipLeftChange));
    clipFront_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onClipFrontChange));
    clipBack_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onClipBackChange));
    clipBottom_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onClipBottomChange));
    clipTop_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onClipTopChange));
    enableClipping_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::adjustClipPropertiesVisibility));
    resetClipPlanes_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::resetClipPlanes));

    addProperty(enableClipping_);
    addProperty(clipRight_);
    addProperty(clipLeft_);
    addProperty(clipFront_);
    addProperty(clipBack_);
    addProperty(clipBottom_);
    addProperty(clipTop_);
    addProperty(resetClipPlanes_);
    addProperty(waitForOptimization_);

    clipRight_.setGroupID("clipping");
    clipLeft_.setGroupID("clipping");
    clipFront_.setGroupID("clipping");
    clipBack_.setGroupID("clipping");
    clipBottom_.setGroupID("clipping");
    clipTop_.setGroupID("clipping");
    resetClipPlanes_.setGroupID("clipping");
    setPropertyGroupGuiName("clipping", "Clipping Planes");
    adjustClipPropertiesVisibility();

    oldVolumeDimensions_ = tgt::ivec3(0,0,0);

    updatePropertyVisibility();

    mode_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onModeChange));
    tfChannel0_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onTransFuncChange));
    resolution_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onResolutionChange));
    resolutionVoxels_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onResolutionVoxelChange));
}

OptimizedProxyGeometry::~OptimizedProxyGeometry() {

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
    }

    geometry_->clear();
    tmpGeometry_->clear();
    delete geometry_;
    delete tmpGeometry_;

    for (std::vector<TransFunc*>::iterator i = tfCopies_.begin(); i != tfCopies_.end(); ++i) {
        delete *i;
    }
    tfCopies_.clear();
}

Processor* OptimizedProxyGeometry::create() const {
    return new OptimizedProxyGeometry();
}

void OptimizedProxyGeometry::volumeDelete(const VolumeBase* source) {

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }

    setVolumeHasChanged();

    tfChannel0_.setVolumeHandle(0);
    tfChannel1_.setVolumeHandle(0);
    tfChannel2_.setVolumeHandle(0);
    tfChannel3_.setVolumeHandle(0);
}

void OptimizedProxyGeometry::volumeChange(const VolumeBase* source) {

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }

    tfChannel0_.setVolumeHandle(0);
    tfChannel1_.setVolumeHandle(0);
    tfChannel2_.setVolumeHandle(0);
    tfChannel3_.setVolumeHandle(0);

    setVolumeHasChanged();
}

void OptimizedProxyGeometry::process() {
    tgtAssert(inport_.getData(), "no input volume");

    // adjust some properties, e.g. ranges for voxel subdivision
    if (volumeHasChanged() || inport_.hasChanged()) {

        // interrupt background thread
        if (backgroundThread_) {
            backgroundThread_->interrupt();
            unlockMutex();
            delete backgroundThread_;
            lockMutex();
            backgroundThread_ = 0;
        }

        onVolumeChange();
        setVolumeHasChanged(false);
    }

    //check if VolumeRAM is availabe (if necessary)
    if (!mode_.isSelected("boundingbox") &&
            (!mode_.hasKey("volumeoctree") || (!mode_.isSelected("volumeoctree") && !mode_.isSelected("volumeoctreeouterfaces")))) {

        inport_.getData()->getRepresentation<VolumeRAM>();
        bool hasVolumeRam = inport_.getData()->hasRepresentation<VolumeRAM>();

        if (!hasVolumeRam) {
            LWARNING("VolumeRAM not available. Falling back to bounding box.");
            mode_.set("boundingbox");
            invalidate();
            return;
        }
    }

    if (mode_.isSelected("boundingbox")) {
         processCube();
    }
    else {
        tgtAssert(tfChannel0_.get(), "no transfunc");

        //if background thread finished computation: do nothing (background thread invalidated processor, mesh geometry is valid)
        //else: compute new geometry in background thread and set temporary geometry to outport
        if (!backgroundThread_ || !backgroundThread_->isFinished()) {

            if (backgroundThread_) {
                backgroundThread_->interrupt();
                unlockMutex();
                delete backgroundThread_;
                lockMutex();
                backgroundThread_ = 0;
            }

            //get number of channels in volume
            size_t numChannels = inport_.getData()->getNumChannels();

            //only 4 channels supported
            if (numChannels > 4) {
                LERROR("Currently only 4 channels supported. Falling back to Bounding Box mode.");
                mode_.set("boundingbox");
                invalidate();
                return;
            }

            //copy transfer functions
            if (tfCopies_.size() != numChannels) {
                for (std::vector<TransFunc*>::iterator i = tfCopies_.begin(); i != tfCopies_.end(); ++i) {
                    delete *i;
                }
                tfCopies_.clear();

                tfCopies_.push_back(tfChannel0_.get()->clone());
                if (numChannels > 1)
                    tfCopies_.push_back(tfChannel1_.get()->clone());
                if (numChannels > 2)
                    tfCopies_.push_back(tfChannel2_.get()->clone());
                if (numChannels > 3)
                    tfCopies_.push_back(tfChannel3_.get()->clone());
            }

            // determine TF types and set up list of transfuncs
            std::vector<TransFunc1DKeys*> tfVector;
            TransFunc1DKeys* tfi = 0;

            for (int i = 0; i < tfCopies_.size(); ++i) {

                if (!tfCopies_.at(i)) {
                    LERROR("No valid transfer function (Channel " << i << ")");
                    return;
                }

                tfi = dynamic_cast<TransFunc1DKeys*>(tfCopies_.at(i));
                if (tfi == 0) {
                    TransFunc2DPrimitives* tfig = dynamic_cast<TransFunc2DPrimitives*>(tfCopies_.at(i));
                    if (tfig == 0)
                        LERROR("Unsupported transfer function (Channel " << i << "): using bounding box mode.");
                    else
                        LWARNING("2D Transfer Function currently not supported (Channel " << i <<"): using bounding box mode.");
                    mode_.set("boundingbox");
                    invalidate();
                    return;
                }

                tfVector.push_back(tfi);
                tfi = 0;
            }

            //get step size (for modes using bricks) and clipping parameters
            int stepSize = resolutionVoxels_.get();
            tgt::vec3 clipLlf(clipRight_.get(), clipFront_.get(), clipBottom_.get());
            tgt::vec3 clipUrb(clipLeft_.get() + 1.f, clipBack_.get() + 1.f, clipTop_.get() + 1.f);


            //create background thread according to selected mode
            if (mode_.isSelected("minboundingbox"))
                backgroundThread_ = new MinCubeBackgroundThread(this, inport_.getData(), tfVector, static_cast<float>(threshold_.get()),
                    geometry_, &volumeStructure_, volStructureSize_, stepSize, false, enableClipping_.get(), clipLlf, clipUrb);
            else if (mode_.isSelected("visiblebricks"))
                backgroundThread_ = new VisibleBricksBackgroundThread(this, inport_.getData(), tfVector, static_cast<float>(threshold_.get()),
                    geometry_, &volumeStructure_, volStructureSize_, stepSize, false, enableClipping_.get(), clipLlf, clipUrb);
            else if (mode_.isSelected("outerfaces"))
                backgroundThread_ = new OuterFacesBackgroundThread(this, inport_.getData(), tfVector, static_cast<float>(threshold_.get()),
                    geometry_, &volumeStructure_, volStructureSize_, stepSize, false, enableClipping_.get(), clipLlf, clipUrb);
            else if (mode_.isSelected("volumeoctree")) {
                //select if the volume has an octree representation and start a VolumeOctreeBackgroundThread or use visible bricks as fallback mode
                if (inport_.getData()->hasRepresentation<VolumeOctreeBase>()) {
                    LDEBUG("VolumeOctree representation available");

                //create worker thread that computes a proxy geometry based on the volume octree
                backgroundThread_ = new VolumeOctreeBackgroundThread(this, inport_.getData(), tfVector,
                        static_cast<float>(threshold_.get()), geometry_, stepSize, enableClipping_.get(), clipLlf, clipUrb);
                }
                else {
                    LERROR("VolumeOctree representation not available (try to use OctreeCreator). Falling back to Visible Bricks mode.");
                    mode_.set("visiblebricks");
                    invalidate();
                    return;
                }
            }
            else if (mode_.isSelected("volumeoctreeouterfaces")) {
                //select if the volume has an octree representation and start a VolumeOctreeOuterFacesBackgroundThread or use outer faces as fallback mode
                if (inport_.getData()->hasRepresentation<VolumeOctreeBase>()) {
                    LDEBUG("VolumeOctree representation available");

                //create worker thread that computes a proxy geometry based on the volume octree
                backgroundThread_ = new VolumeOctreeOuterFacesBackgroundThread(this, inport_.getData(), tfVector,
                        static_cast<float>(threshold_.get()), geometry_, stepSize, enableClipping_.get(), clipLlf, clipUrb);
                }
                else {
                    LERROR("VolumeOctree representation not available (try to use OctreeCreator). Falling back to Outer Faces mode.");
                    mode_.set("outerfaces");
                    invalidate();
                    return;
                }
            }

            //start background computation
            backgroundThread_->run();

            if (waitForOptimization_.get()) {
                // wait for background thread to finish computation
                unlockMutex();
                backgroundThread_->join();
                lockMutex();
            }
            else {
                //while background computation is not finished: use temporary bounding box geometry
                processTmpCube();
                outport_.setData(tmpGeometry_, false);

                return;
            }
        }
    }

    outport_.setData(geometry_, false);
}

void OptimizedProxyGeometry::processCube() {

    const VolumeBase* inputVolume = inport_.getData();
    tgt::vec3 volumeSize = inputVolume->getCubeSize();
    tgt::ivec3 numSlices = inputVolume->getDimensions();

    geometry_->clear();

    // vertex and tex coords of bounding box without clipping
    tgt::vec3 coordLlf = inputVolume->getLLF();
    tgt::vec3 coordUrb = inputVolume->getURB();
    const tgt::vec3 noClippingTexLlf(0, 0, 0);
    const tgt::vec3 noClippingTexUrb(1, 1, 1);

    tgt::vec3 texLlf;
    tgt::vec3 texUrb;
    if (enableClipping_.get()) {
        // adjust vertex and tex coords to clipping
        texLlf = tgt::vec3(
            clipRight_.get()  / static_cast<float>(numSlices.x),
            clipFront_.get()  / static_cast<float>(numSlices.y),
            clipBottom_.get() / static_cast<float>(numSlices.z));
        texUrb = tgt::vec3(
            (clipLeft_.get()+1.0f) / static_cast<float>(numSlices.x),
            (clipBack_.get()+1.0f) / static_cast<float>(numSlices.y),
            (clipTop_.get()+1.0f)  / static_cast<float>(numSlices.z));

        coordLlf -= volumeSize * (noClippingTexLlf - texLlf);
        coordUrb -= volumeSize * (noClippingTexUrb - texUrb);
    }
    else {
        texLlf = noClippingTexLlf;
        texUrb = noClippingTexUrb;
    }

    // create output mesh
    geometry_->addCube(VertexVec3(texLlf, texLlf), VertexVec3(texUrb, texUrb));
    //geometry_->addCube(VertexVec3(coordLlf, texLlf), VertexVec3(coordUrb, texUrb));
    geometry_->setTransformationMatrix(inputVolume->getTextureToWorldMatrix());

}

void OptimizedProxyGeometry::processTmpCube() {
    const VolumeBase* inputVolume = inport_.getData();
    tgt::vec3 volumeSize = inputVolume->getCubeSize();
    tgt::ivec3 numSlices = inputVolume->getDimensions();

    tmpGeometry_->clear();

    // vertex and tex coords of bounding box without clipping
    tgt::vec3 coordLlf = inputVolume->getLLF();
    tgt::vec3 coordUrb = inputVolume->getURB();
    const tgt::vec3 noClippingTexLlf(0, 0, 0);
    const tgt::vec3 noClippingTexUrb(1, 1, 1);

    tgt::vec3 texLlf;
    tgt::vec3 texUrb;
    if (enableClipping_.get()) {
        // adjust vertex and tex coords to clipping
        texLlf = tgt::vec3(
            clipRight_.get()  / static_cast<float>(numSlices.x),
            clipFront_.get()  / static_cast<float>(numSlices.y),
            clipBottom_.get() / static_cast<float>(numSlices.z));
        texUrb = tgt::vec3(
            (clipLeft_.get()+1.0f) / static_cast<float>(numSlices.x),
            (clipBack_.get()+1.0f) / static_cast<float>(numSlices.y),
            (clipTop_.get()+1.0f)  / static_cast<float>(numSlices.z));

        coordLlf -= volumeSize * (noClippingTexLlf - texLlf);
        coordUrb -= volumeSize * (noClippingTexUrb - texUrb);
    }
    else {
        texLlf = noClippingTexLlf;
        texUrb = noClippingTexUrb;
    }

    // create output mesh
    tmpGeometry_->addCube(VertexVec3(texLlf, texLlf), VertexVec3(texUrb, texUrb));
    //tmpGeometry_->addCube(VertexVec3(coordLlf, texLlf), VertexVec3(coordUrb, texUrb));
    tmpGeometry_->setTransformationMatrix(inputVolume->getTextureToWorldMatrix());
}

void OptimizedProxyGeometry::addCubeMesh(TriangleMeshGeometryVec4Vec3* mesh, tgt::Bounds bounds, tgt::ivec3 dim) {
    tgt::vec3 coordllf = tgt::max(bounds.getLLF(), tgt::vec3(0.f));
    tgt::vec3 coordurb = tgt::min(bounds.getURB(), tgt::vec3(dim));

    tgt::vec3 texllf = coordllf / tgt::vec3(dim);
    texllf = tgt::max(texllf,tgt::vec3(0.f));
    texllf = tgt::min(texllf,tgt::vec3(1.f));

    tgt::vec3 texurb = coordurb / tgt::vec3(dim);
    texurb = tgt::max(texurb,tgt::vec3(0.f));
    texurb = tgt::min(texurb,tgt::vec3(1.f));

    mesh->addCube(VertexVec3(texllf, texllf), VertexVec3(texurb, texurb));
    //mesh->addCube(VertexVec3(coordllf, texllf), VertexVec3(coordurb, texurb));
}

void OptimizedProxyGeometry::addCubeMeshClip(TriangleMeshGeometryVec4Vec3* mesh, tgt::Bounds bounds, tgt::ivec3 dim, tgt::Bounds clipBounds) {
    tgt::vec3 clipLlf = clipBounds.getLLF();
    tgt::vec3 clipUrb = clipBounds.getURB();

    tgt::vec3 coordllf = bounds.getLLF();
    tgt::vec3 coordurb = bounds.getURB();

    //clip the coordinates
    coordllf = tgt::max(coordllf, clipLlf);
    coordurb = tgt::min(coordurb, clipUrb);

    tgt::vec3 texllf = bounds.getLLF() / tgt::vec3(dim);
    texllf = tgt::max(texllf,tgt::vec3(0.f));
    texllf = tgt::min(texllf,tgt::vec3(1.f));

    tgt::vec3 texurb = bounds.getURB() / tgt::vec3(dim);
    texurb = tgt::max(texurb,tgt::vec3(0.f));
    texurb = tgt::min(texurb,tgt::vec3(1.f));

    //clip the coordinates
    texllf = tgt::max(texllf, coordllf / tgt::vec3(dim));
    texurb = tgt::min(texurb, coordurb / tgt::vec3(dim));

    mesh->addCube(VertexVec3(texllf, texllf), VertexVec3(texurb, texurb));
    //mesh->addCube(VertexVec3(coordllf, texllf), VertexVec3(coordurb, texurb));
}

void OptimizedProxyGeometry::onClipRightChange() {
    if (clipRight_.get() > clipLeft_.get())
        clipLeft_.set(clipRight_.get());
    geometryInvalid_ = true;

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }
}

void OptimizedProxyGeometry::onClipLeftChange() {
    if (clipRight_.get() > clipLeft_.get())
        clipRight_.set(clipLeft_.get());
    geometryInvalid_ = true;

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }
}

void OptimizedProxyGeometry::onClipFrontChange() {
    if (clipFront_.get() > clipBack_.get())
        clipBack_.set(clipFront_.get());
    geometryInvalid_ = true;

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }
}

void OptimizedProxyGeometry::onClipBackChange() {
    if (clipFront_.get() > clipBack_.get())
        clipFront_.set(clipBack_.get());
    geometryInvalid_ = true;

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }
}

void OptimizedProxyGeometry::onClipBottomChange() {
    if (clipBottom_.get() > clipTop_.get())
        clipTop_.set(clipBottom_.get());
    geometryInvalid_ = true;

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }
}

void OptimizedProxyGeometry::onClipTopChange() {
    if (clipBottom_.get() > clipTop_.get())
        clipBottom_.set(clipTop_.get());
    geometryInvalid_ = true;

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }
}

void OptimizedProxyGeometry::resetClipPlanes() {

    clipRight_.set(0.0f);
    clipLeft_.set(clipLeft_.getMaxValue());

    clipFront_.set(0.0f);
    clipBack_.set(clipBack_.getMaxValue());

    clipBottom_.set(0.0f);
    clipTop_.set(clipTop_.getMaxValue());
}

void OptimizedProxyGeometry::adjustResolutionPropertyRanges() {

    if (inport_.getData()) {
        //store old settings
        int oldVoxels = resolutionVoxels_.get();
        int oldDiv = resolution_.get();

        //get volume dimensions
        tgt::ivec3 dim = inport_.getData()->getDimensions();
        //determine shortest side
        int minDim = std::min(std::min(dim.x, dim.y), dim.z);

        //set max for voxel resolution
        resolutionVoxels_.setMaxValue(minDim);

        //set max for resolution
        resolution_.setMaxValue(minDim);

        /*int div = static_cast<int>(std::ceil(static_cast<float>(minDim) / static_cast<float>(resolutionVoxels_.get())));
        resolution_.set(div);*/
    }
}

void OptimizedProxyGeometry::adjustClipPropertiesRanges() {
    tgtAssert(inport_.getData() && inport_.getData(), "No input volume");

    if (oldVolumeDimensions_ == tgt::ivec3(0,0,0))
        oldVolumeDimensions_ = inport_.getData()->getDimensions();

    tgt::ivec3 numSlices = inport_.getData()->getDimensions();

    // assign new clipping values while taking care that the right>left validation
    // does not alter the assigned values
    float scaleRight = tgt::clamp(clipRight_.get()/static_cast<float>(oldVolumeDimensions_.x-1), 0.f, 1.f);
    float scaleLeft =  tgt::clamp(clipLeft_.get()/static_cast<float>(oldVolumeDimensions_.x-1), 0.f, 1.f);
    float rightVal = scaleRight * (numSlices.x-1);
    float leftVal = scaleLeft * (numSlices.x-1);

    // set new max values now (we cannot set them earlier as they might clip the previous values needed for the relative re-positioning
    // to the new range)
    clipRight_.setMaxValue(numSlices.x-1.0f);
    clipLeft_.setMaxValue(numSlices.x-1.0f);
    clipLeft_.set(clipLeft_.getMaxValue());
    clipRight_.set(rightVal);
    clipLeft_.set(leftVal);

    float scaleFront = tgt::clamp(clipFront_.get()/static_cast<float>(oldVolumeDimensions_.y-1), 0.f, 1.f);
    float scaleBack =  tgt::clamp(clipBack_.get()/static_cast<float>(oldVolumeDimensions_.y-1), 0.f, 1.f);
    float frontVal = scaleFront * (numSlices.y-1);
    float backVal = scaleBack * (numSlices.y-1);
    clipFront_.setMaxValue(numSlices.y-1.0f);
    clipBack_.setMaxValue(numSlices.y-1.0f);
    clipBack_.set(clipBack_.getMaxValue());
    clipFront_.set(frontVal);
    clipBack_.set(backVal);

    float scaleBottom = tgt::clamp(clipBottom_.get()/static_cast<float>(oldVolumeDimensions_.z-1), 0.f, 1.f);
    float scaleTop =  tgt::clamp(clipTop_.get()/static_cast<float>(oldVolumeDimensions_.z-1), 0.f, 1.f);
    float bottomVal = scaleBottom * (numSlices.z-1);
    float topVal = scaleTop * (numSlices.z-1);
    clipBottom_.setMaxValue(numSlices.z-1.0f);
    clipTop_.setMaxValue(numSlices.z-1.0f);
    clipTop_.set(clipTop_.getMaxValue());
    clipBottom_.set(bottomVal);
    clipTop_.set(topVal);

    if (clipRight_.get() > clipRight_.getMaxValue())
        clipRight_.set(clipRight_.getMaxValue());

    if (clipLeft_.get() > clipLeft_.getMaxValue())
        clipLeft_.set(clipLeft_.getMaxValue());

    if (clipFront_.get() > clipFront_.getMaxValue())
        clipFront_.set(clipFront_.getMaxValue());

    if (clipBack_.get() > clipBack_.getMaxValue())
        clipBack_.set(clipBack_.getMaxValue());

    if (clipBottom_.get() > clipBottom_.getMaxValue())
        clipBottom_.set(clipBottom_.getMaxValue());

    if (clipTop_.get() > clipTop_.getMaxValue())
        clipTop_.set(clipTop_.getMaxValue());

    oldVolumeDimensions_ = numSlices;
}

void OptimizedProxyGeometry::adjustClipPropertiesVisibility() {
    bool clipEnabled = enableClipping_.get();
    setPropertyGroupVisible("clipping", clipEnabled);
    geometryInvalid_ = true;

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }
}

void OptimizedProxyGeometry::updatePropertyVisibility() {
    if (mode_.get() == "boundingbox") {
        tfChannel0_.setVisible(false);
        tfChannel1_.setVisible(false);
        tfChannel2_.setVisible(false);
        tfChannel3_.setVisible(false);
        resolutionMode_.setVisible(false);
        resolution_.setVisible(false);
        resolutionVoxels_.setVisible(false);
        threshold_.setVisible(false);
        enableClipping_.setVisible(true);
        adjustClipPropertiesVisibility();
    }
    else {

        //check number of channels and set transfer functions visible
        tfChannel0_.setVisible(true);

        if (inport_.getData()) {
            size_t numChannels = inport_.getData()->getNumChannels();

            if (numChannels > 1)
                tfChannel1_.setVisible(true);
            else
                tfChannel1_.setVisible(false);

            if (numChannels > 2)
                tfChannel2_.setVisible(true);
            else
                tfChannel2_.setVisible(false);

            if (numChannels > 3)
                tfChannel3_.setVisible(true);
            else
                tfChannel3_.setVisible(false);

            if (numChannels > 4)
                LWARNING("Number of channels in volume is greater than 4, only 4 channels currently supported!");
        }
        else {
            tfChannel1_.setVisible(false);
            tfChannel2_.setVisible(false);
            tfChannel3_.setVisible(false);
        }

        if (!mode_.hasKey("volumeoctree") || (!mode_.isSelected("volumeoctree") && !mode_.isSelected("volumeoctreeouterfaces"))) {
            resolutionMode_.setVisible(true);
            if (resolutionMode_.get() == "voxel") {
                resolution_.setVisible(false);
                resolutionVoxels_.setVisible(true);
            }
            else {
                resolution_.setVisible(true);
                resolutionVoxels_.setVisible(false);
            }
            resolution_.setVisible(true);
        }
        else {
            //volume octree mode does not support resolution
            resolutionMode_.setVisible(false);
            resolution_.setVisible(false);
            resolutionVoxels_.setVisible(false);
        }

        threshold_.setVisible(true);

        enableClipping_.setVisible(true);
        adjustClipPropertiesVisibility();
    }
}

void OptimizedProxyGeometry::adjustClippingToVolumeROI() {
    // adjust clipping sliders to volume ROI, if set
    if (inport_.getData()->hasMetaData("RoiPixelOffset") && inport_.getData()->hasMetaData("RoiPixelLength")) {
        const tgt::ivec3 volumeDim = static_cast<tgt::ivec3>(inport_.getData()->getDimensions());
        tgt::ivec3 roiLlf = inport_.getData()->getMetaDataValue<IVec3MetaData>("RoiPixelOffset", tgt::ivec3(0));
        tgt::ivec3 roiLength = inport_.getData()->getMetaDataValue<IVec3MetaData>("RoiPixelLength", volumeDim);
        if (tgt::hor(tgt::lessThan(roiLlf, tgt::ivec3::zero)) || tgt::hor(tgt::greaterThanEqual(roiLlf, volumeDim))) {
            LWARNING("Invalid ROI offset: " << roiLlf);
        }
        else if (tgt::hor(tgt::lessThanEqual(roiLength, tgt::ivec3::zero))) {
            LWARNING("Invalid ROI length: " << roiLength);
        }
        else {
            tgt::ivec3 roiUrb = tgt::min(roiLlf + roiLength - 1, volumeDim - tgt::ivec3::one);

            LINFO("Applying volume ROI: llf=" << roiLlf << ", urb=" << roiUrb);

            clipRight_.set(static_cast<float>(roiLlf.x));
            clipFront_.set(static_cast<float>(roiLlf.y));
            clipBottom_.set(static_cast<float>(roiLlf.z));

            clipLeft_.set(static_cast<float>(roiUrb.x));
            clipBack_.set(static_cast<float>(roiUrb.y));
            clipTop_.set(static_cast<float>(roiUrb.z));
        }
    }
}

void OptimizedProxyGeometry::onThresholdChange() {
    geometryInvalid_ = true;

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }
}

void OptimizedProxyGeometry::onModeChange() {
    //structureInvalid_ = true;
    geometryInvalid_ = true;

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }

    updatePropertyVisibility();
}

void OptimizedProxyGeometry::onTransFuncChange() {

    geometryInvalid_ = true;

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }

    //transfunc copies are invalid
    for (std::vector<TransFunc*>::iterator i = tfCopies_.begin(); i != tfCopies_.end(); ++i) {
        delete *i;
    }
    tfCopies_.clear();
}

void OptimizedProxyGeometry::onResolutionModeChange() {
    if (resolutionMode_.get() == "voxel") {
        resolution_.setVisible(false);
        resolutionVoxels_.setVisible(true);
    }
    else {
        //have to set the resolution according to the resolution voxel size
        if (inport_.getData()) {
            //get input data
            const VolumeBase* inputVolume = inport_.getData();
            tgt::ivec3 dim = inputVolume->getDimensions();
            //determine shortest side and for this side the number of subdivisions
            int minDim = std::min(std::min(dim.x, dim.y), dim.z);
            int div = static_cast<int>(std::ceil(static_cast<float>(minDim) / static_cast<float>(resolutionVoxels_.get())));
            resolution_.set(div);
        }

        resolution_.setVisible(true);
        resolutionVoxels_.setVisible(false);
    }
}

void OptimizedProxyGeometry::onVolumeChange() {

    //register as new observer
    inport_.getData()->addObserver(this);

    //invalidate data structures
    structureInvalid_ = true;

    // adapt clipping plane properties on volume change
    adjustClipPropertiesRanges();

    // extract ROI from volume and adjust clipping sliders accordingly
    adjustClippingToVolumeROI();

    //clear region structure
    volumeStructure_.clear();

    //adjust the possible values for the resolution
    adjustResolutionPropertyRanges();

    //set the resolution in voxels, if the other mode is presently used
    if (resolutionMode_.get() == "subdivide") {
        //set voxel value according to previous resolution
        if (inport_.getData()) {
            //get input data
            const VolumeBase* inputVolume = inport_.getData();
            tgt::ivec3 dim = inputVolume->getDimensions();
            //determine shortest side and for this side the step size
            int minDim = std::min(std::min(dim.x, dim.y), dim.z);
            int stepSize = std::max(1, tgt::iround(static_cast<float>(minDim) / static_cast<float>(resolution_.get())));
            resolutionVoxels_.set(stepSize);
        }
    }

    size_t numChannels = inport_.getData()->getNumChannels();

    if (!mode_.isSelected("boundingbox")) {
        //check number of channels and set transfer functions visible

        if (numChannels > 1)
            tfChannel1_.setVisible(true);
        else
            tfChannel1_.setVisible(false);

        if (numChannels > 2)
            tfChannel2_.setVisible(true);
        else
            tfChannel2_.setVisible(false);

        if (numChannels > 3)
            tfChannel3_.setVisible(true);
        else
            tfChannel3_.setVisible(false);

        if (numChannels > 4)
            LWARNING("Number of channels in volume is greater than 4, only 4 channels currently supported!");
    }

    tfChannel0_.setVolumeHandle(inport_.getData());

    if (numChannels > 1)
        tfChannel1_.setVolumeHandle(inport_.getData(), 1);

    if (numChannels > 2)
        tfChannel2_.setVolumeHandle(inport_.getData(), 2);

    if (numChannels > 3)
        tfChannel3_.setVolumeHandle(inport_.getData(), 3);
}

void OptimizedProxyGeometry::onResolutionChange() {
    //do not invalidate the structure if the change happened because the resolution mode has been changed
    if (!resolution_.isVisible())
        return;

    //set voxel value according to previous resolution
    if (inport_.getData()) {
        //get input data
        const VolumeBase* inputVolume = inport_.getData();
        tgt::ivec3 dim = inputVolume->getDimensions();
        //determine shortest side and for this side the step size
        int minDim = std::min(std::min(dim.x, dim.y), dim.z);
        int stepSize = std::max(1, tgt::iround(static_cast<float>(minDim) / static_cast<float>(resolution_.get())));
        resolutionVoxels_.set(stepSize);
    }

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }
}

void OptimizedProxyGeometry::onResolutionVoxelChange() {
    //invalidate the structure
    structureInvalid_ = true;

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }
}

bool OptimizedProxyGeometry::structureInvalid() const {
    return structureInvalid_;
}

bool OptimizedProxyGeometry::geometryInvalid() const {
    return geometryInvalid_;
}

void OptimizedProxyGeometry::setStructureInvalid(bool value) {
    structureInvalid_ = value;
}

void OptimizedProxyGeometry::setGeometryInvalid(bool value) {
    geometryInvalid_ = value;
}

void OptimizedProxyGeometry::setVolStructureSize(tgt::ivec3 volStructureSize) {
    volStructureSize_ = volStructureSize;
}

bool OptimizedProxyGeometry::volumeHasChanged() const {
    return volumeHasChanged_;
}

void OptimizedProxyGeometry::setVolumeHasChanged(bool value) {
    volumeHasChanged_ = value;
}


//-------------------------------------------------------------------------------------------------
// background threads

OptimizedProxyGeometryBackgroundThread::OptimizedProxyGeometryBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume,
          std::vector<TransFunc1DKeys*> tfVector, float threshold, TriangleMeshGeometryVec4Vec3* geometry, int stepSize, bool debugOutput,
          bool clippingEnabled, tgt::vec3 clipLlf, tgt::vec3 clipUrb)
        : ProcessorBackgroundThread<OptimizedProxyGeometry>(processor)
        , volume_(volume)
        , tfCopyVector_(tfVector)
        , threshold_(threshold)
        , geometry_(geometry)
        , stepSize_(stepSize)
        , debugOutput_(debugOutput)
        , clippingEnabled_(clippingEnabled)
        , clipLlf_(clipLlf)
        , clipUrb_(clipUrb)
{}

void OptimizedProxyGeometryBackgroundThread::handleInterruption() {
    //nothing to handle
}

bool OptimizedProxyGeometryBackgroundThread::isRegionEmptyPi(float min, float max, const PreIntegrationTable* piTable) const {
    return (piTable->classify(min, max).a <= 0.0001 * static_cast<float>(threshold_));
}


//-------------------------------------------------------------------------------------------------
// StructureProxyGeometryBackgroundThread

StructureProxyGeometryBackgroundThread::StructureProxyGeometryBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume,
          std::vector<TransFunc1DKeys*> tfVector, float threshold, TriangleMeshGeometryVec4Vec3* geometry, std::vector<ProxyGeometryVolumeRegion>* volumeStructure,
          tgt::ivec3 volStructureSize, int stepSize, bool debugOutput, bool clippingEnabled, tgt::vec3 clipLlf, tgt::vec3 clipUrb)
        : OptimizedProxyGeometryBackgroundThread(processor, volume, tfVector, threshold, geometry, stepSize, debugOutput, clippingEnabled, clipLlf, clipUrb)
        , volumeStructure_(volumeStructure)
        , volStructureSize_(volStructureSize)
{}

void StructureProxyGeometryBackgroundThread::computeRegionStructure() {

    interruptionPoint();

    volumeStructure_->clear();

    if (debugOutput_) {
        stopWatch_.reset();
        stopWatch_.start();
    }

    tgt::ivec3 dim = volume_->getDimensions();

    interruptionPoint();

    const VolumeRAM* vol = volume_->getRepresentation<VolumeRAM>();
    RealWorldMapping rwm = volume_->getRealWorldMapping();
    int numChannels = static_cast<int>(volume_->getNumChannels());

    interruptionPoint();

    const tgt::ivec3 step = tgt::ivec3(stepSize_);

    //determine size for this resolution
    const tgt::ivec3 size(
            static_cast<int>(std::ceil(static_cast<float>(dim.x) / static_cast<float>(stepSize_))),
            static_cast<int>(std::ceil(static_cast<float>(dim.y) / static_cast<float>(stepSize_))),
            static_cast<int>(std::ceil(static_cast<float>(dim.z) / static_cast<float>(stepSize_))));

    volStructureSize_ = size;
    processor_->setVolStructureSize(volStructureSize_);

    interruptionPoint();

    tgt::ivec3 pos, llf, urb;

    for (VolumeIterator it(size); !it.outofrange(); it.next()) {

        interruptionPoint();

        pos = it.value();
        llf = pos * step;
        urb = (pos+1) * step - tgt::ivec3(1);

        //since a voxel is the center, add/subtract 0.5 to/from the coordinates to get the bounding box
        tgt::Bounds regionBounds(tgt::vec3(llf) - tgt::vec3(0.5f), tgt::vec3(urb) + tgt::vec3(0.5f));

        interruptionPoint();

        // find min and max intensities for all channels
        float minIntensity[4] = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        float maxIntensity[4] = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
        llf = tgt::max(llf - tgt::ivec3(1),tgt::ivec3(0));
        llf = tgt::min(llf,dim - 1);
        urb = tgt::max(urb + tgt::ivec3(1),tgt::ivec3(0));
        urb = tgt::min(urb,dim - 1);

        //don't use macro because of interruption points within the loops...
        //VRN_FOR_EACH_VOXEL(pos, llf, urb) {
        for (pos = llf; pos.z < urb.z; ++pos.z) {
            //interruption point after each slice
            interruptionPoint();

            for (pos.y = llf.y; pos.y < urb.y; ++pos.y) {
                for (pos.x = llf.x; pos.x < urb.x; ++pos.x) {

                    for (int i = 0; i < numChannels; ++i) {
                        float current = vol->getVoxelNormalized(pos, i);
                        //apply realworld mapping
                        current = rwm.normalizedToRealWorld(current);
                        minIntensity[i] = std::min(minIntensity[i],current);
                        maxIntensity[i] = std::max(maxIntensity[i],current);
                    }
                }
            }
        }

        //create list of min-max intensities
        std::vector<tgt::vec2> minMaxIntensities(numChannels);
        for (int i = 0; i < numChannels; ++i) {
            minMaxIntensities.at(i) = tgt::vec2(minIntensity[i], maxIntensity[i]);
        }

        //add region
        volumeStructure_->push_back(ProxyGeometryVolumeRegion(regionBounds,minMaxIntensities));

    }

    if (debugOutput_) {
        stopWatch_.stop();
        std::cout << "Computing region structure took " << stopWatch_.getRuntime() << " milliseconds" << std::endl;
    }
}

ProxyGeometryVolumeRegion& StructureProxyGeometryBackgroundThread::getVolumeRegion(tgt::ivec3 pos) {
    return volumeStructure_->at(pos.z * (volStructureSize_.x * volStructureSize_.y) + pos.y * volStructureSize_.x + pos.x);
}


//-------------------------------------------------------------------------------------------------
// MinCubeBackgroundThread

MinCubeBackgroundThread::MinCubeBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume, std::vector<TransFunc1DKeys*> tfVector,
                                                 float threshold, TriangleMeshGeometryVec4Vec3* geometry,
                                                 std::vector<ProxyGeometryVolumeRegion>* volumeStructure, tgt::ivec3 volStructureSize,
                                                 int stepSize, bool debugOutput, bool clippingEnabled, tgt::vec3 clipLlf, tgt::vec3 clipUrb)
        : StructureProxyGeometryBackgroundThread(processor, volume, tfVector, threshold, geometry, volumeStructure, volStructureSize, stepSize,
                                                 debugOutput, clippingEnabled, clipLlf, clipUrb)
{}

MinCubeBackgroundThread::~MinCubeBackgroundThread() {
    //wait for internal thread to finish
    join();
}

void MinCubeBackgroundThread::threadMain() {
    computeMinCube();
}

void MinCubeBackgroundThread::computeMinCube() {

    //get volume dimensions
    tgt::ivec3 dim = volume_->getDimensions();

    interruptionPoint();

    if (processor_->structureInvalid()) {
        //invalidate geometry
        processor_->setGeometryInvalid();

        //clear structure
        volumeStructure_->clear();

        interruptionPoint();

        //compute new structure
        computeRegionStructure();
        processor_->setStructureInvalid(false);
    }

    interruptionPoint();

    if (processor_->geometryInvalid()) {

        geometry_->clear();

        interruptionPoint();

        if (debugOutput_) {
            stopWatch_.reset();
            stopWatch_.start();
        }

        //compute pre-integration tables for every TF
        std::vector<const PreIntegrationTable*> piTables;
        for (int i = 0; i < volume_->getNumChannels(); ++i) {
            piTables.push_back(tfCopyVector_.at(i)->getPreIntegrationTable(1.f, 256));
        }

        if (debugOutput_) {
            stopWatch_.stop();
            std::cout << "Fetching PreIntegration tables took " << stopWatch_.getRuntime() << " milliseconds" << std::endl;

            stopWatch_.reset();
            stopWatch_.start();
        }

        interruptionPoint();

        //scan through region structure, classify every region and add the bounds of non-transparent blocks to the minimal bounding box
        tgt::Bounds minBounds;

        std::vector<ProxyGeometryVolumeRegion>::const_iterator i;
        for (i = volumeStructure_->begin(); i != volumeStructure_->end(); ++i) {
            interruptionPoint();
            //check every channel if the region is opaque
            for (int channel = 0; channel < volume_->getNumChannels(); ++channel) {
                //apply tf domain
                float minIntensity = tfCopyVector_.at(channel)->realWorldToNormalized(i->getMinIntensity(channel));
                float maxIntensity = tfCopyVector_.at(channel)->realWorldToNormalized(i->getMaxIntensity(channel));
                if (!isRegionEmptyPi(minIntensity,maxIntensity,piTables.at(channel))) {
                    minBounds.addVolume(i->getBounds());
                    break;
                }
            }
        }

        if (debugOutput_) {
            stopWatch_.stop();
            std::cout << "Determined (approximate) minimal bounding box in " << stopWatch_.getRuntime() << " milliseconds" << std::endl;
        }

        interruptionPoint();

        if (minBounds.isDefined()) {
            //now create the bounding box that has been found
            if (clippingEnabled_) {
                //get clipping planes
                tgt::Bounds clipBounds(clipLlf_, clipUrb_);
                //only add and clip mesh if necessary
                if (clipBounds.containsVolume(minBounds))
                    OptimizedProxyGeometry::addCubeMesh(geometry_, minBounds, dim);
                else if (clipBounds.intersects(minBounds))
                    OptimizedProxyGeometry::addCubeMeshClip(geometry_, minBounds,dim,clipBounds);
            }
            else
                OptimizedProxyGeometry::addCubeMesh(geometry_, minBounds,dim);
        }

        geometry_->setTransformationMatrix(volume_->getTextureToWorldMatrix());
        processor_->setGeometryInvalid(false);

        interruptionPoint();

        if (debugOutput_)
            std::cout << "Created (approximate) minimal bounding box proxy geometry, volume "
                      << ftos(minBounds.volume()) << " (before clipping)" << std::endl;
    }
}

//-------------------------------------------------------------------------------------------------
// VisibleBricksBackgroundThread

VisibleBricksBackgroundThread::VisibleBricksBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume, std::vector<TransFunc1DKeys*> tfVector,
                                                            float threshold, TriangleMeshGeometryVec4Vec3* geometry,
                                                            std::vector<ProxyGeometryVolumeRegion>* volumeStructure,
                                                            tgt::ivec3 volStructureSize, int stepSize, bool debugOutput,
                                                            bool clippingEnabled, tgt::vec3 clipLlf, tgt::vec3 clipUrb)
        : StructureProxyGeometryBackgroundThread(processor, volume, tfVector, threshold, geometry, volumeStructure, volStructureSize, stepSize,
                                                debugOutput, clippingEnabled, clipLlf, clipUrb)
{}

VisibleBricksBackgroundThread::~VisibleBricksBackgroundThread() {
    //wait for internal thread to finish
    join();
}

void VisibleBricksBackgroundThread::threadMain() {
    computeMaximalBricks();
}

void VisibleBricksBackgroundThread::computeMaximalBricks() {

    //get volume dimensions
    tgt::ivec3 dim = volume_->getDimensions();

    interruptionPoint();

    if (processor_->structureInvalid()) {
        //invalidate geometry
        processor_->setGeometryInvalid();

        //clear structure
        volumeStructure_->clear();

        interruptionPoint();

        //compute new structure
        computeRegionStructure();
        processor_->setStructureInvalid(false);
    }

    interruptionPoint();

    if (processor_->geometryInvalid()) {

        geometry_->clear();

        interruptionPoint();

        if (debugOutput_) {
            stopWatch_.reset();
            stopWatch_.start();
        }

        //compute pre-integration tables for every TF
        std::vector<const PreIntegrationTable*> piTables;
        for (int i = 0; i < volume_->getNumChannels(); ++i) {
            piTables.push_back(tfCopyVector_.at(i)->getPreIntegrationTable(1.f, 256));
        }

        if (debugOutput_) {
            stopWatch_.stop();
            std::cout << "Fetching PreIntegration tables took " << stopWatch_.getRuntime() << " milliseconds" << std::endl;
            stopWatch_.reset();
            stopWatch_.start();
        }

        setVolBound(tgt::ivec3(0),volStructureSize_-1,false);

        interruptionPoint();

        int numberOfCubes = 0; //number of created cubes
        float proxyVolume = 0.f;

        tgt::ivec3 pos, urbVol;
        tgt::vec3 llf, urb;
        for (VolumeIterator it(volStructureSize_); !it.outofrange(); it.next()) {
            interruptionPoint();
            pos = it.value();
            if (isVolNotEmptyPiNotBound(pos,tfCopyVector_, piTables)) {
                urbVol = getUrbPi(pos,tfCopyVector_, piTables);
                llf = getVolumeRegion(pos).getBounds().getLLF();
                urb = getVolumeRegion(urbVol).getBounds().getURB();

                interruptionPoint();

                tgt::Bounds cubeBounds(llf,urb);
                if (clippingEnabled_) {
                    tgt::Bounds clipBounds(clipLlf_, clipUrb_);
                    //only add and clip mesh if necessary
                    if (clipBounds.containsVolume(cubeBounds))
                        OptimizedProxyGeometry::addCubeMesh(geometry_, cubeBounds, dim);
                    else if (clipBounds.intersects(cubeBounds))
                        OptimizedProxyGeometry::addCubeMeshClip(geometry_, cubeBounds,dim,clipBounds);
                }
                else
                    OptimizedProxyGeometry::addCubeMesh(geometry_, cubeBounds, dim);

                if (debugOutput_) {
                    proxyVolume += cubeBounds.volume();
                    numberOfCubes++;
                }

            }
        }

        if (debugOutput_) {
            stopWatch_.stop();
            std::cout << "Created maximal cubes proxy geometry in " << stopWatch_.getRuntime() << " milliseconds" << std::endl;
            std::cout << "Created Proxy Geometry consisting of " << numberOfCubes << " cubes using maximal cubes mode," << std::endl;
            std::cout << " volume < " << proxyVolume << " (before clipping)" << std::endl;
        }

        interruptionPoint();

        geometry_->setTransformationMatrix(volume_->getTextureToWorldMatrix());
        processor_->setGeometryInvalid(false);
    }
}


void VisibleBricksBackgroundThread::setVolBound(tgt::ivec3 llf, tgt::ivec3 urb, bool value) {
    for (int z=llf.z; z<=urb.z;z++) {
        interruptionPoint();
        for (int y=llf.y; y<=urb.y; y++) {
            for (int x=llf.x; x<=urb.x; x++) {
                ProxyGeometryVolumeRegion& v = getVolumeRegion(tgt::ivec3(x,y,z));
                v.setBound(value);
            }
        }
    }
}

bool VisibleBricksBackgroundThread::isVolNotEmptyPiNotBound(tgt::ivec3 pos, const std::vector<TransFunc1DKeys*>& tfs, const std::vector<const PreIntegrationTable*>& piTables) {
    ProxyGeometryVolumeRegion& v = getVolumeRegion(pos);
    if (v.isBound())
        return false;

    //check every channel
    bool empty = true;

    for (int channel = 0; channel < piTables.size(); ++channel) {
        //apply tf domain
        float minIntensity = tfs.at(channel)->realWorldToNormalized(v.getMinIntensity(channel));
        float maxIntensity = tfs.at(channel)->realWorldToNormalized(v.getMaxIntensity(channel));
        if (!isRegionEmptyPi(minIntensity,maxIntensity,piTables.at(channel))) {
            empty = false;
            break;
        }
     }

     return (!empty);
}

bool VisibleBricksBackgroundThread::isVolNotEmptyPiNotBound(tgt::ivec3 llf, tgt::ivec3 urb,
        const std::vector<TransFunc1DKeys*>& tfs, const std::vector<const PreIntegrationTable*>& piTables)
{
    for (int z=llf.z; z<=urb.z;z++) {
        interruptionPoint();
        for (int y=llf.y; y<=urb.y; y++) {
            for (int x=llf.x; x<=urb.x; x++) {
                ProxyGeometryVolumeRegion& v = getVolumeRegion(tgt::ivec3(x,y,z));
                if (v.isBound())
                    return false;

                //check every channel
                bool empty = true;

                for (int channel = 0; channel < piTables.size(); ++channel) {
                    //apply tf domain
                    float minIntensity = tfs.at(channel)->realWorldToNormalized(v.getMinIntensity(channel));
                    float maxIntensity = tfs.at(channel)->realWorldToNormalized(v.getMaxIntensity(channel));
                    if (!isRegionEmptyPi(minIntensity,maxIntensity,piTables.at(channel))) {
                        empty = false;
                        break;
                    }
                }

                if (empty)
                    return false;
            }
        }
    }

    return true;
}

tgt::ivec3 VisibleBricksBackgroundThread::getUrbPi(tgt::ivec3 llf, const std::vector<TransFunc1DKeys*>& tfs,
        const std::vector<const PreIntegrationTable*>& piTables)
{

    interruptionPoint();

    const tgt::ivec3 size = volStructureSize_;
    tgt::bvec3 inc(true);
    tgt::ivec3 urb(llf);

    while(inc.x||inc.y||inc.z) {
        interruptionPoint();
        if (inc.x) {
            if (urb.x+1>size.x-1) {
                inc.x=false;
            } else {
                if (isVolNotEmptyPiNotBound(
                    tgt::ivec3(urb.x+1,llf.y,llf.z),
                    tgt::ivec3(urb.x+1,urb.y,urb.z),
                    tfs, piTables))
                {
                    urb.x += 1;
                } else {
                    inc.x=false;
                }
            }
        }
        if (inc.y) {
            if (urb.y+1>size.y-1) {
                inc.y=false;
            } else {
                if (isVolNotEmptyPiNotBound(
                    tgt::ivec3(llf.x,urb.y+1,llf.z),
                    tgt::ivec3(urb.x,urb.y+1,urb.z),
                    tfs, piTables)) {
                    urb.y += 1;
                } else {
                    inc.y=false;
                }
            }
        }
        if (inc.z) {
            if (urb.z+1>size.z-1) {
                inc.z=false;
            } else {
                if (isVolNotEmptyPiNotBound(
                    tgt::ivec3(llf.x,llf.y,urb.z+1),
                    tgt::ivec3(urb.x,urb.y,urb.z+1),
                    tfs, piTables)) {
                    urb.z += 1;
                } else {
                    inc.z=false;
                }
            }
        }
    }

    setVolBound(llf,urb);
    return urb;
}


//-------------------------------------------------------------------------------------------------
// OuterFacesBackgroundThread

OuterFacesBackgroundThread::OuterFacesBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume, std::vector<TransFunc1DKeys*> tfVector,
                                                            float threshold, TriangleMeshGeometryVec4Vec3* geometry,
                                                            std::vector<ProxyGeometryVolumeRegion>* volumeStructure,
                                                            tgt::ivec3 volStructureSize, int stepSize, bool debugOutput,
                                                            bool clippingEnabled, tgt::vec3 clipLlf, tgt::vec3 clipUrb)
        : StructureProxyGeometryBackgroundThread(processor, volume, tfVector, threshold, geometry, volumeStructure, volStructureSize, stepSize,
                                                debugOutput, clippingEnabled, clipLlf, clipUrb)
{}

OuterFacesBackgroundThread::~OuterFacesBackgroundThread() {
    //wait for internal thread to finish
    join();
}

void OuterFacesBackgroundThread::threadMain() {
    computeOuterFaces();
}

void OuterFacesBackgroundThread::computeOuterFaces() {

    //get volume dimensions
    tgt::ivec3 volDim = volume_->getDimensions();

    interruptionPoint();

    if (processor_->structureInvalid()) {
        //invalidate geometry
        processor_->setGeometryInvalid();

        //clear structure
        volumeStructure_->clear();

        interruptionPoint();

        //compute new structure
        computeRegionStructure();
        processor_->setStructureInvalid(false);
    }

    interruptionPoint();

    if (processor_->geometryInvalid()) {

        geometry_->clear();

        interruptionPoint();

        if (debugOutput_) {
            stopWatch_.reset();
            stopWatch_.start();
        }

        //compute pre-integration tables for every TF
        std::vector<const PreIntegrationTable*> piTables;
        for (int i = 0; i < volume_->getNumChannels(); ++i) {
            piTables.push_back(tfCopyVector_.at(i)->getPreIntegrationTable(1.f, 256));
        }

        if (debugOutput_) {
            stopWatch_.stop();
            std::cout << "Fetching PreIntegration table took " << stopWatch_.getRuntime() << " milliseconds" << std::endl;
            stopWatch_.reset();
            stopWatch_.start();
        }

        interruptionPoint();

        //first pass: scan through region structure and classify every region
        std::vector<ProxyGeometryVolumeRegion>::iterator i;
        for (i = volumeStructure_->begin(); i != volumeStructure_->end(); ++i) {
            interruptionPoint();

            //check every channel
            bool empty = true;

            for (int channel = 0; channel < piTables.size(); ++channel) {
                //apply tf domain
                float minIntensity = tfCopyVector_.at(channel)->realWorldToNormalized(i->getMinIntensity(channel));
                float maxIntensity = tfCopyVector_.at(channel)->realWorldToNormalized(i->getMaxIntensity(channel));
                if (!isRegionEmptyPi(minIntensity,maxIntensity,piTables.at(channel))) {
                    empty = false;
                    break;
                }
            }

            i->setOpaque(!empty);
        }

        //second pass: scan through region structure
        //for every opaque region and for every side of the brick check the neighbors and add a face if the neighbor is transparent
        tgt::ivec3 pos;
        for (VolumeIterator it(volStructureSize_); !it.outofrange(); it.next()) {

            interruptionPoint();

            tgt::Bounds clipBounds(clipLlf_, clipUrb_);

            //get position of current brick and a reference to the brick
            pos = it.value();
            ProxyGeometryVolumeRegion& current  = getVolumeRegion(pos);

            //if the current brick is transparent no faces are generated
            if (!current.isOpaque())
                continue;

            //if clipping is enabled and the current brick is completely outside the clipping area no faces are generated
            if (clippingEnabled_ && !clipBounds.containsVolume(current.getBounds()) && !clipBounds.intersects(current.getBounds()))
                continue;

            //outer loop: set sign of the normal from -1 to +1
            for (int sign = -1; sign <= 1; sign += 2) {
                //inner loop: iterate over the three dimensions
                for (int dim = 0; dim < 3; ++dim) {

                    interruptionPoint();

                    if (!clippingEnabled_ || clipBounds.containsVolume(current.getBounds())) {
                        //no clipping
                        //if direct neighbor in direction sign*dim is existing and opaque: continue
                        tgt::ivec3 neighborPos = pos;
                        neighborPos.elem[dim] += sign;

                        if ((neighborPos.elem[dim] >= 0) && (neighborPos.elem[dim] < volStructureSize_.elem[dim]) && getVolumeRegion(neighborPos).isOpaque())
                            continue;

                        //else: create a face in direction sign*dim

                        //get the coordinates of the current block
                        tgt::vec3 llf = current.getBounds().getLLF();
                        tgt::vec3 urb = current.getBounds().getURB();

                        tgt::vec3 coordllf = tgt::max(llf, tgt::vec3(0.f));
                        tgt::vec3 coordurb = tgt::min(urb, tgt::vec3(volDim));

                        tgt::vec3 texllf = coordllf / tgt::vec3(volDim);
                        texllf = tgt::clamp(texllf, 0.f, 1.f);

                        tgt::vec3 texurb = coordurb / tgt::vec3(volDim);
                        texurb = tgt::clamp(texurb, 0.f, 1.f);

                        // four vertices for the quad
                        tgt::vec3 vertices[4];

                        // set the coordinate of the face side according to dim and sign
                        for (int i = 0; i < 4; ++i) {
                            vertices[i].elem[dim] = (sign < 0) ? texllf.elem[dim] : texurb.elem[dim];
                        }

                        //set the other coordinates
                        for (int incr = 1; incr <= 2; ++incr) {
                            int curDim = (dim + incr) % 3;

                            vertices[0].elem[curDim] = texllf.elem[curDim];
                            vertices[1].elem[curDim] = (incr == 1) ? texurb.elem[curDim] : texllf.elem[curDim];
                            vertices[2].elem[curDim] = texurb.elem[curDim];
                            vertices[3].elem[curDim] = (incr == 1) ? texllf.elem[curDim] : texurb.elem[curDim];
                        }

                        VertexVec3 ll(vertices[0], vertices[0]);
                        VertexVec3 lr(vertices[1], vertices[1]);
                        VertexVec3 ur(vertices[2], vertices[2]);
                        VertexVec3 ul(vertices[3], vertices[3]);

                        //counter-clockwise order of vertices
                        if (sign > 0)
                            geometry_->addQuad(ll, lr, ur, ul);
                        else
                            geometry_->addQuad(ll, ul, ur, lr);

                    }
                    else {
                        //clipping needed

                        //if direct neighbor in direction sign*dim is existing and opaque AND is not completely outside the clipping area: continue
                        tgt::ivec3 neighborPos = pos;
                        neighborPos.elem[dim] += sign;

                        if ((neighborPos.elem[dim] >= 0) && (neighborPos.elem[dim] < volStructureSize_.elem[dim]) && getVolumeRegion(neighborPos).isOpaque()
                                && (clipBounds.containsVolume(getVolumeRegion(neighborPos).getBounds()) || clipBounds.intersects(getVolumeRegion(neighborPos).getBounds())))
                            continue;

                        //else: create a face in direction sign*dim

                        //get the coordinates of the current block
                        tgt::vec3 llf = current.getBounds().getLLF();
                        tgt::vec3 urb = current.getBounds().getURB();

                        tgt::vec3 coordllf = tgt::max(llf, tgt::vec3(0.f));
                        tgt::vec3 coordurb = tgt::min(urb, tgt::vec3(volDim));


                        //clip the coordinates
                        coordllf = tgt::max(coordllf, clipLlf_);
                        coordurb = tgt::min(coordurb, clipUrb_);

                        tgt::vec3 texllf = coordllf / tgt::vec3(volDim);
                        texllf = tgt::clamp(texllf, 0.f, 1.f);

                        tgt::vec3 texurb = coordurb / tgt::vec3(volDim);
                        texurb = tgt::clamp(texurb, 0.f, 1.f);

                        // four vertices for the quad
                        tgt::vec3 vertices[4];

                        // set the coordinate of the face side according to dim and sign
                        for (int i = 0; i < 4; ++i) {
                            vertices[i].elem[dim] = (sign < 0) ? texllf.elem[dim] : texurb.elem[dim];
                        }

                        //set the other coordinates
                        for (int incr = 1; incr <= 2; ++incr) {
                            int curDim = (dim + incr) % 3;

                            vertices[0].elem[curDim] = texllf.elem[curDim];
                            vertices[1].elem[curDim] = (incr == 1) ? texurb.elem[curDim] : texllf.elem[curDim];
                            vertices[2].elem[curDim] = texurb.elem[curDim];
                            vertices[3].elem[curDim] = (incr == 1) ? texllf.elem[curDim] : texurb.elem[curDim];
                        }

                        VertexVec3 ll(vertices[0], vertices[0]);
                        VertexVec3 lr(vertices[1], vertices[1]);
                        VertexVec3 ur(vertices[2], vertices[2]);
                        VertexVec3 ul(vertices[3], vertices[3]);

                        //counter-clockwise order of vertices
                        if (sign > 0)
                            geometry_->addQuad(ll, lr, ur, ul);
                        else
                            geometry_->addQuad(ll, ul, ur, lr);
                    }
                }
            }
        }

        if (debugOutput_) {
            stopWatch_.stop();
            std::cout << "Created outer faces proxy geometry in " << stopWatch_.getRuntime() << " milliseconds" << std::endl;
        }

        interruptionPoint();

        geometry_->setTransformationMatrix(volume_->getTextureToWorldMatrix());
        processor_->setGeometryInvalid(false);
    }
}

//-------------------------------------------------------------------------------------------------
// VolumeOctreeBackgroundThread

VolumeOctreeBackgroundThread::VolumeOctreeBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume, std::vector<TransFunc1DKeys*> tfVector,
            float threshold, TriangleMeshGeometryVec4Vec3* geometry, int stepSize, bool clippingEnabled, tgt::vec3 clipLlf, tgt::vec3 clipUrb)
        : OptimizedProxyGeometryBackgroundThread(processor, volume, tfVector, threshold, geometry, stepSize, false, clippingEnabled, clipLlf, clipUrb)
{}

VolumeOctreeBackgroundThread::~VolumeOctreeBackgroundThread() {
    //wait for internal thread to finish
    join();
}

void VolumeOctreeBackgroundThread::threadMain() {
    computeVolumeOctreeGeometry();
}

void VolumeOctreeBackgroundThread::computeVolumeOctreeGeometry() {
    interruptionPoint();

    //TODO: check if computation is necessary?!
    processor_->setGeometryInvalid(true);

    if (processor_->geometryInvalid()) {
        geometry_->clear();

        tgt::svec3 volumeDim = volume_->getDimensions();

        //compute pre-integration tables for every TF
        std::vector<const PreIntegrationTable*> piTables;
        for (int i = 0; i < volume_->getNumChannels(); ++i) {
            piTables.push_back(tfCopyVector_.at(i)->getPreIntegrationTable(1.f, 256));
        }

        interruptionPoint();

        tgt::Bounds clipBounds(clipLlf_, clipUrb_);

        //get VolumeOctree representation and traverse it to create the geometry
        const VolumeOctreeBase* octree = volume_->getRepresentation<VolumeOctreeBase>();
        const VolumeOctreeNode* root = octree->getRootNode();
        int numChannels = static_cast<int>(volume_->getNumChannels());

        tgt::svec3 octreeDim = octree->getOctreeDim();

        traverseOctreeAndCreateGeometry(root, tgt::vec3(0.f), tgt::vec3(octreeDim), tfCopyVector_, numChannels, piTables, clipBounds, volumeDim);

        interruptionPoint();

        geometry_->setTransformationMatrix(volume_->getTextureToWorldMatrix());
        processor_->setGeometryInvalid(false);
    }

    interruptionPoint();
}

void VolumeOctreeBackgroundThread::traverseOctreeAndCreateGeometry(const VolumeOctreeNode* node, const tgt::vec3& nodeLlf, const tgt::vec3& nodeUrb,
            /*TransFunc1DKeys* tf, const PreIntegrationTable* piTable,*/
            std::vector<TransFunc1DKeys*> tfVector, int numVolumeChannels, std::vector<const PreIntegrationTable*> piTables, tgt::Bounds clipBounds,
            tgt::ivec3 volumeDim)
{

    interruptionPoint();

    if (node->isLeaf()) {
        //compute intensity values
        RealWorldMapping rwm = volume_->getRealWorldMapping();

        //check if any of the channels is not transparent
        bool isOpaque = false;
        for (int i = 0; i < numVolumeChannels; ++i) {
            // transform normalized voxel intensities to real-world
            float minRealWorld = rwm.normalizedToRealWorld(node->getMinValue(i) / 65535.f);
            float maxRealWorld = rwm.normalizedToRealWorld(node->getMaxValue(i) / 65535.f);

            float minIntensity = tfVector.at(i)->realWorldToNormalized(minRealWorld);
            float maxIntensity = tfVector.at(i)->realWorldToNormalized(maxRealWorld);

            //check pre-integration heuristic
            if (!isRegionEmptyPi(minIntensity, maxIntensity, piTables.at(i))) {
                isOpaque = true;
                break;
            }
        }

        //do not render if all channels are transparent
        if (!isOpaque)
            return;

        //check if node is outside volume dimensions
        if (tgt::hor(tgt::greaterThanEqual(nodeLlf, tgt::vec3(volumeDim))))
            return;

        //clamp node to volume dimensions
        tgt::vec3 correctNodeUrb = tgt::min(nodeUrb, tgt::vec3(volumeDim));

        //not transparent -> create cube (if necessery: clipping)
        tgt::Bounds nodeBounds(nodeLlf, correctNodeUrb);    //get bounding box

        if (clippingEnabled_) {
            if (clipBounds.containsVolume(nodeBounds))
                OptimizedProxyGeometry::addCubeMesh(geometry_, nodeBounds, volumeDim);
            else if (clipBounds.intersects(nodeBounds))
                OptimizedProxyGeometry::addCubeMeshClip(geometry_, nodeBounds, volumeDim, clipBounds);
         }
         else
            OptimizedProxyGeometry::addCubeMesh(geometry_, nodeBounds, volumeDim);

    } else {

        //for every child: compute bounding box and traverse further
        tgt::vec3 nodeHalfDim = (nodeUrb-nodeLlf) / 2.f;
        for (size_t childID=0; childID<8; childID++) {
            const VolumeOctreeNode* childNode = node->children_[childID];
            if (childNode) {
                tgt::svec3 childCoord = linearCoordToCubic(childID, tgt::svec3::two);
                tgt::vec3 childLlf = nodeLlf + static_cast<tgt::vec3>(childCoord)*nodeHalfDim;
                tgt::vec3 childUrb = childLlf + nodeHalfDim;

                //traverseOctreeAndCreateGeometry(childNode, childLlf, childUrb, tf, piTable, clipBounds, volumeDim);
                traverseOctreeAndCreateGeometry(childNode, childLlf, childUrb, tfVector, numVolumeChannels, piTables, clipBounds, volumeDim);
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
// VolumeOctreeOuterFacesBackgroundThread

VolumeOctreeOuterFacesBackgroundThread::VolumeOctreeOuterFacesBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume, std::vector<TransFunc1DKeys*> tfVector,
            float threshold, TriangleMeshGeometryVec4Vec3* geometry, int stepSize, bool clippingEnabled, tgt::vec3 clipLlf, tgt::vec3 clipUrb)
        : OptimizedProxyGeometryBackgroundThread(processor, volume, tfVector, threshold, geometry, stepSize, false, clippingEnabled, clipLlf, clipUrb)
{}

VolumeOctreeOuterFacesBackgroundThread::~VolumeOctreeOuterFacesBackgroundThread() {
    //wait for internal thread to finish
    join();
}

void VolumeOctreeOuterFacesBackgroundThread::threadMain() {
    computeVolumeOctreeGeometry();
}

void VolumeOctreeOuterFacesBackgroundThread::computeVolumeOctreeGeometry() {
    interruptionPoint();

    //TODO: check if computation is necessary?!
    processor_->setGeometryInvalid(true);

    if (processor_->geometryInvalid()) {
        geometry_->clear();

        tgt::svec3 volumeDim = volume_->getDimensions();

        //compute pre-integration tables for every TF
        std::vector<const PreIntegrationTable*> piTables;
        for (int i = 0; i < volume_->getNumChannels(); ++i) {
            piTables.push_back(tfCopyVector_.at(i)->getPreIntegrationTable(1.f, 256));
        }

        interruptionPoint();

        tgt::Bounds clipBounds(clipLlf_, clipUrb_);

        //get VolumeOctree representation and some meta information
        const VolumeOctreeBase* octree = volume_->getRepresentation<VolumeOctreeBase>();
        const VolumeOctreeNode* root = octree->getRootNode();
        int numChannels = static_cast<int>(volume_->getNumChannels());

        tgt::svec3 octreeDim = octree->getOctreeDim();
        octreeDepth_ = octree->getActualTreeDepth();

        //compute number of bricks in every direction
        brickDim_ = octree->getBrickDim();
        volDim_ = octree->getVolumeDim();
        brickStructureSize_ = volDim_ / brickDim_;
        if (volDim_.x % brickDim_.x != 0)
            brickStructureSize_.x += 1;
        if (volDim_.y % brickDim_.y != 0)
            brickStructureSize_.y += 1;
        if (volDim_.z % brickDim_.z != 0)
            brickStructureSize_.z += 1;

        //now allocate memory for the brick structure
        //TODO: allocate that in the processor, set flags -> do not re-compute the bricks if only the TF has changed?!
        brickStructure_ = std::vector<ProxyGeometryVolumeRegion>(brickStructureSize_.x * brickStructureSize_.y * brickStructureSize_.z,
                ProxyGeometryVolumeRegion());

        //set to zero to mark all bricks as being not opaque
        //std::memset(&brickStructure_, 0, brickStructure_.capacity() * sizeof(ProxyGeometryVolumeRegion));

        //do the actual traversal to create the brick structure
        traverseOctreeAndCreateBrickStructure(root, 0, tgt::svec3((size_t) 0), octreeDim, tfCopyVector_, numChannels, piTables, clipBounds, volumeDim);

        interruptionPoint();

        //create the outer faces geometry from the brick structure
        computeOuterFaces();

        geometry_->setTransformationMatrix(volume_->getTextureToWorldMatrix());
        processor_->setGeometryInvalid(false);
    }

    interruptionPoint();
}

void VolumeOctreeOuterFacesBackgroundThread::traverseOctreeAndCreateBrickStructure (const VolumeOctreeNode* node, size_t level,
        const tgt::svec3& nodeLlf, const tgt::svec3& nodeUrb, std::vector<TransFunc1DKeys*> tfVector, int numVolumeChannels,
        std::vector<const PreIntegrationTable*> piTables, tgt::Bounds clipBounds, tgt::ivec3 volumeDim)
{

    interruptionPoint();

    if (node->isLeaf()) {
        //compute intensity values
        RealWorldMapping rwm = volume_->getRealWorldMapping();

        //check if any of the channels is not transparent
        bool isOpaque = false;
        for (int i = 0; i < numVolumeChannels; ++i) {
            // transform normalized voxel intensities to real-world
            float minRealWorld = rwm.normalizedToRealWorld(node->getMinValue(i) / 65535.f);
            float maxRealWorld = rwm.normalizedToRealWorld(node->getMaxValue(i) / 65535.f);

            float minIntensity = tfVector.at(i)->realWorldToNormalized(minRealWorld);
            float maxIntensity = tfVector.at(i)->realWorldToNormalized(maxRealWorld);

            //check pre-integration heuristic
            if (!isRegionEmptyPi(minIntensity, maxIntensity, piTables.at(i))) {
                isOpaque = true;
                break;
            }
        }

        //do not change the corresponding brick(s) if all channels are transparent
        if (!isOpaque)
            return;

        //check if node is outside volume dimensions and discard it
        if (tgt::hor(tgt::greaterThanEqual(nodeLlf, tgt::svec3(volumeDim))))
            return;

        //set bricks in vector opaque
        tgt::svec3 brickLlf = nodeLlf / brickDim_;
        tgt::svec3 brickUrb = nodeUrb / brickDim_ - tgt::svec3(1);
        setBricksVisible(brickLlf, brickUrb);

        //tgt::Bounds b(tgt::vec3(brickLlf), tgt::vec3(brickUrb));
        //std::cout << brickLlf << " " << brickUrb << std::endl;

    } else {

        //for every child: compute bounding box and traverse further
        tgt::svec3 tmpNodeDim = tgt::svec3(nodeUrb-nodeLlf);
        tgt::svec3 nodeHalfDim = tgt::svec3(tmpNodeDim.x / 2, tmpNodeDim.y / 2, tmpNodeDim.z / 2);
        for (size_t childID=0; childID<8; childID++) {
            const VolumeOctreeNode* childNode = node->children_[childID];
            if (childNode) {
                tgt::svec3 childCoord = linearCoordToCubic(childID, tgt::svec3::two);
                tgt::svec3 childLlf = nodeLlf + childCoord*nodeHalfDim;
                tgt::svec3 childUrb = childLlf + nodeHalfDim;

                //traverseOctreeAndCreateGeometry(childNode, childLlf, childUrb, tf, piTable, clipBounds, volumeDim);
                traverseOctreeAndCreateBrickStructure(childNode, level + 1, childLlf, childUrb, tfVector, numVolumeChannels, piTables, clipBounds, volumeDim);
            }
        }
    }
}

void VolumeOctreeOuterFacesBackgroundThread::setBricksVisible(tgt::svec3 llf, tgt::svec3 urb) {
    for (size_t z=llf.z; z<=urb.z && z < brickStructureSize_.z;z++) {
        interruptionPoint();
        for (size_t y=llf.y; y<=urb.y && y < brickStructureSize_.y; y++) {
            for (size_t x=llf.x; x<=urb.x && x < brickStructureSize_.x; x++) {
                tgt::vec3 rllf = tgt::vec3(brickDim_ * tgt::svec3(x,y,z)) - tgt::vec3(0.5f);
                tgt::vec3 rurb = tgt::vec3((brickDim_ * tgt::svec3(x+1,y+1,z+1)) - tgt::svec3(1)) + tgt::vec3(0.5f);
                tgt::Bounds regionBounds(rllf, rurb);
                //std::cout << rllf << " " << rurb << std::endl;
                std::vector<tgt::vec2> minMaxIntensities;       //<- not needed here, may be empty
                ProxyGeometryVolumeRegion& v = getVolumeRegion(tgt::ivec3((int)x,(int)y,(int)z));
                v = ProxyGeometryVolumeRegion(regionBounds,minMaxIntensities);
            }
        }
    }
}

ProxyGeometryVolumeRegion& VolumeOctreeOuterFacesBackgroundThread::getVolumeRegion(tgt::ivec3 pos) {
    return brickStructure_.at(pos.z * (brickStructureSize_.x * brickStructureSize_.y) + pos.y * brickStructureSize_.x + pos.x);
}

void VolumeOctreeOuterFacesBackgroundThread::computeOuterFaces() {

    //get volume dimensions
    tgt::ivec3 volDim = volume_->getDimensions();

    interruptionPoint();

    //second pass: scan through region structure
    //for every opaque region and for every side of the brick check the neighbors and add a face if the neighbor is transparent
    tgt::ivec3 pos;
    for (VolumeIterator it(brickStructureSize_); !it.outofrange(); it.next()) {

        interruptionPoint();

        tgt::Bounds clipBounds(clipLlf_, clipUrb_);

        //get position of current brick and a reference to the brick
        pos = it.value();
        ProxyGeometryVolumeRegion& current  = getVolumeRegion(pos);

        //if the current brick is transparent no faces are generated
        if (!current.isOpaque())
            continue;

        //if clipping is enabled and the current brick is completely outside the clipping area no faces are generated
        if (clippingEnabled_ && !clipBounds.containsVolume(current.getBounds()) && !clipBounds.intersects(current.getBounds()))
            continue;

        //std::cout << current.getBounds().getLLF() << " " << current.getBounds().getURB() << std::endl;

        //outer loop: set sign of the normal from -1 to +1
        for (int sign = -1; sign <= 1; sign += 2) {
            //inner loop: iterate over the three dimensions
            for (int dim = 0; dim < 3; ++dim) {

                interruptionPoint();

                if (!clippingEnabled_ || clipBounds.containsVolume(current.getBounds())) {
                    //no clipping
                    //if direct neighbor in direction sign*dim is existing and opaque: continue
                    tgt::ivec3 neighborPos = pos;
                    neighborPos.elem[dim] += sign;

                    if ((neighborPos.elem[dim] >= 0) && (neighborPos.elem[dim] < brickStructureSize_.elem[dim]) && getVolumeRegion(neighborPos).isOpaque())
                        continue;

                    //else: create a face in direction sign*dim

                    //get the coordinates of the current block
                    tgt::vec3 llf = current.getBounds().getLLF();
                    tgt::vec3 urb = current.getBounds().getURB();

                    tgt::vec3 coordllf = tgt::max(llf, tgt::vec3(0.f));
                    tgt::vec3 coordurb = tgt::min(urb, tgt::vec3(volDim));

                    tgt::vec3 texllf = coordllf / tgt::vec3(volDim);
                    texllf = tgt::clamp(texllf, 0.f, 1.f);

                    tgt::vec3 texurb = coordurb / tgt::vec3(volDim);
                    texurb = tgt::clamp(texurb, 0.f, 1.f);

                    // four vertices for the quad
                    tgt::vec3 vertices[4];

                    // set the coordinate of the face side according to dim and sign
                    for (int i = 0; i < 4; ++i) {
                        vertices[i].elem[dim] = (sign < 0) ? texllf.elem[dim] : texurb.elem[dim];
                    }

                    //set the other coordinates
                    for (int incr = 1; incr <= 2; ++incr) {
                        int curDim = (dim + incr) % 3;

                        vertices[0].elem[curDim] = texllf.elem[curDim];
                        vertices[1].elem[curDim] = (incr == 1) ? texurb.elem[curDim] : texllf.elem[curDim];
                        vertices[2].elem[curDim] = texurb.elem[curDim];
                        vertices[3].elem[curDim] = (incr == 1) ? texllf.elem[curDim] : texurb.elem[curDim];
                    }

                    VertexVec3 ll(vertices[0], vertices[0]);
                    VertexVec3 lr(vertices[1], vertices[1]);
                    VertexVec3 ur(vertices[2], vertices[2]);
                    VertexVec3 ul(vertices[3], vertices[3]);

                    //counter-clockwise order of vertices
                    if (sign > 0)
                        geometry_->addQuad(ll, lr, ur, ul);
                    else
                        geometry_->addQuad(ll, ul, ur, lr);

                }
                else {
                    //clipping needed

                    //if direct neighbor in direction sign*dim is existing and opaque AND is not completely outside the clipping area: continue
                    tgt::ivec3 neighborPos = pos;
                    neighborPos.elem[dim] += sign;

                    if ((neighborPos.elem[dim] >= 0) && (neighborPos.elem[dim] < brickStructureSize_.elem[dim]) && getVolumeRegion(neighborPos).isOpaque()
                            && (clipBounds.containsVolume(getVolumeRegion(neighborPos).getBounds()) || clipBounds.intersects(getVolumeRegion(neighborPos).getBounds())))
                       continue;

                    //else: create a face in direction sign*dim

                    //get the coordinates of the current block
                    tgt::vec3 llf = current.getBounds().getLLF();
                    tgt::vec3 urb = current.getBounds().getURB();

                    tgt::vec3 coordllf = tgt::max(llf, tgt::vec3(0.f));
                    tgt::vec3 coordurb = tgt::min(urb, tgt::vec3(volDim));


                    //clip the coordinates
                    coordllf = tgt::max(coordllf, clipLlf_);
                    coordurb = tgt::min(coordurb, clipUrb_);

                    tgt::vec3 texllf = coordllf / tgt::vec3(volDim);
                    texllf = tgt::clamp(texllf, 0.f, 1.f);

                    tgt::vec3 texurb = coordurb / tgt::vec3(volDim);
                    texurb = tgt::clamp(texurb, 0.f, 1.f);

                    // four vertices for the quad
                    tgt::vec3 vertices[4];

                    // set the coordinate of the face side according to dim and sign
                    for (int i = 0; i < 4; ++i) {
                        vertices[i].elem[dim] = (sign < 0) ? texllf.elem[dim] : texurb.elem[dim];
                    }

                    //set the other coordinates
                    for (int incr = 1; incr <= 2; ++incr) {
                        int curDim = (dim + incr) % 3;

                        vertices[0].elem[curDim] = texllf.elem[curDim];
                        vertices[1].elem[curDim] = (incr == 1) ? texurb.elem[curDim] : texllf.elem[curDim];
                        vertices[2].elem[curDim] = texurb.elem[curDim];
                        vertices[3].elem[curDim] = (incr == 1) ? texllf.elem[curDim] : texurb.elem[curDim];
                    }

                    VertexVec3 ll(vertices[0], vertices[0]);
                    VertexVec3 lr(vertices[1], vertices[1]);
                    VertexVec3 ur(vertices[2], vertices[2]);
                    VertexVec3 ul(vertices[3], vertices[3]);

                    //counter-clockwise order of vertices
                    if (sign > 0)
                        geometry_->addQuad(ll, lr, ur, ul);
                    else
                        geometry_->addQuad(ll, ul, ur, lr);
                }
            }
        }
    }

    if (debugOutput_) {
        stopWatch_.stop();
        std::cout << "Created outer faces proxy geometry in " << stopWatch_.getRuntime() << " milliseconds" << std::endl;
    }

    interruptionPoint();

    geometry_->setTransformationMatrix(volume_->getTextureToWorldMatrix());
    processor_->setGeometryInvalid(false);
}

} // namespace
