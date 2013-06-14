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
    , transfunc_("transferfunction", "Transfer Function")
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
#ifdef VRN_PROXY_DEBUG
    , setEnclosedOpaque_("enclosedopaque", "Set Enclosed Opaque", false)
    , debugOutput_("debugOutput", "Debug Output")
    , checkHalfNodes_("halfnodes", "Check and render half nodes", true)
#endif
    , geometryInvalid_(true)
    , structureInvalid_(true)
    , volStructureSize_(0,0,0)
#ifdef VRN_PROXY_DEBUG
    , octreeRoot_(0)
    , octreeInvalid_(true)
#endif
    , backgroundThread_(0)
{
    //create mesh list geometry
    geometry_ = new TriangleMeshGeometryVec4Vec3();
    tmpGeometry_ = new TriangleMeshGeometryVec4Vec3();

    addPort(inport_);
    addPort(outport_);

    mode_.addOption("boundingbox",          "Bounding Box");
    mode_.addOption("minboundingbox",       "Minimal Visible Bounding Box");
    mode_.addOption("visiblebricks",        "Visible Bricks");
    mode_.addOption("outerfaces",           "Visible Bricks (Outer Faces)");
#ifdef VRN_MODULE_STAGING
    mode_.addOption("volumeoctree",         "Volume Octree");
#endif
#ifdef VRN_PROXY_DEBUG
    mode_.addOption("octreebricks",         "Visible Bricks (Octree)");
    mode_.addOption("brutemincube", "Brute Force Minimal Bounding Box");
    mode_.addOption("bruteforce", "Brute Force Bricks");
    mode_.addOption("structurecubes", "Bricks");
    mode_.addOption("octree", "Octree Bricks");
#endif
    mode_.set("visiblebricks");
    addProperty(mode_);

    addProperty(transfunc_);
    transfunc_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onTransFuncChange));

    resolutionMode_.addOption("subdivide", "Subdivide Shortest Side");
    resolutionMode_.addOption("voxel", "Subdivide in Voxels");
    addProperty(resolutionMode_);
    resolutionMode_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onResolutionModeChange));

    addProperty(resolution_);

    addProperty(resolutionVoxels_);

    addProperty(threshold_);
    threshold_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onThresholdChange));
#ifdef VRN_PROXY_DEBUG
    addProperty(setEnclosedOpaque_);
    setEnclosedOpaque_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onSetEnclosedOpaqueChange));
    addProperty(checkHalfNodes_);
    checkHalfNodes_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onCheckHalfNodesChange));
#endif

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
#ifdef VRN_PROXY_DEBUG
    debugOutput_.set(false);
    addProperty(debugOutput_);
#endif
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
    transfunc_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onTransFuncChange));
    resolution_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onResolutionChange));
    resolutionVoxels_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onResolutionVoxelChange));

    tfCopy_ = 0;
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
#ifdef VRN_PROXY_DEBUG
    delete octreeRoot_;
#endif
    delete tfCopy_;
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
}

void OptimizedProxyGeometry::volumeChange(const VolumeBase* source) {

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }

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

    const VolumeRAM* volumeRam = inport_.getData()->getRepresentation<VolumeRAM>();
    if (!volumeRam && !mode_.isSelected("boundingbox"))
        LWARNING("VolumeRAM not available. Falling back to bounding box.");

    if (mode_.get() == "boundingbox" || !volumeRam) {
        processCube();
    }
    else if (mode_.isSelected("minboundingbox")) {
        tgtAssert(transfunc_.get(), "no transfunc");

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

            //copy transfer function
            if(!tfCopy_)
                tfCopy_ = transfunc_.get()->clone();

            // determine TF type
            TransFunc1DKeys* tfi = 0;

            if (tfCopy_) {
                tfi = dynamic_cast<TransFunc1DKeys*>(tfCopy_);
                if (tfi == 0) {
                    TransFunc2DPrimitives* tfig = dynamic_cast<TransFunc2DPrimitives*>(tfCopy_);
                    if (tfig == 0) {
                        LERROR("Unsupported transfer function.");
                        return;
                    }
                    LWARNING("2D Transfer Function currently not supported: using bounding box mode.");
                    processCube();
                    return;
                }
            }
            else {
                LERROR("No valid transfer function");
                return;
            }

            //create worker thread that computes min bounding box
            int stepSize = resolutionVoxels_.get();
            tgt::vec3 clipLlf(clipRight_.get(), clipFront_.get(), clipBottom_.get());
            tgt::vec3 clipUrb(clipLeft_.get() + 1.f, clipBack_.get() + 1.f, clipTop_.get() + 1.f);
#ifdef VRN_PROXY_DEBUG
            backgroundThread_ = new MinCubeBackgroundThread(this, inport_.getData(), tfi, static_cast<float>(threshold_.get()),
                geometry_, &volumeStructure_, volStructureSize_, stepSize, debugOutput_.get(), enableClipping_.get(), clipLlf, clipUrb);
#else
            backgroundThread_ = new MinCubeBackgroundThread(this, inport_.getData(), tfi, static_cast<float>(threshold_.get()),
                geometry_, &volumeStructure_, volStructureSize_, stepSize, false, enableClipping_.get(), clipLlf, clipUrb);
#endif

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
#ifdef VRN_MODULE_STAGING
    else if (mode_.isSelected("volumeoctree")) {

        tgtAssert(transfunc_.get(), "no transfunc");

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

            //copy transfer function
            if(!tfCopy_)
                tfCopy_ = transfunc_.get()->clone();

            // determine TF type
            TransFunc1DKeys* tfi = 0;

            if (tfCopy_) {
                tfi = dynamic_cast<TransFunc1DKeys*>(tfCopy_);
                if (tfi == 0) {
                    TransFunc2DPrimitives* tfig = dynamic_cast<TransFunc2DPrimitives*>(tfCopy_);
                    if (tfig == 0) {
                        LERROR("Unsupported transfer function.");
                        return;
                    }
                    LWARNING("2D Transfer Function currently not supported: using bounding box mode.");
                    processCube();
                    return;
                }
            }
            else {
                LERROR("No valid transfer function");
                return;
            }

            int stepSize = resolutionVoxels_.get();
            tgt::vec3 clipLlf(clipRight_.get(), clipFront_.get(), clipBottom_.get());
            tgt::vec3 clipUrb(clipLeft_.get() + 1.f, clipBack_.get() + 1.f, clipTop_.get() + 1.f);

            //select if the volume has an octree representation and start a VolumeOctreeBackgroundThread or use visible bricks as fallback mode
            if (inport_.getData()->hasRepresentation<VolumeOctree>()) {
                LDEBUG("VolumeOctree representation available");

                //create worker thread that computes a proxy geometry based on the volume octree
                backgroundThread_ = new VolumeOctreeBackgroundThread(this, inport_.getData(), tfi, static_cast<float>(threshold_.get()),
                    geometry_, stepSize, enableClipping_.get(), clipLlf, clipUrb);
            }
            else { 
                LWARNING("VolumeOctree representation not available... using Visible Bricks mode as fallback.");

                //create worker thread that computes visible bricks proxy geometry
#ifdef VRN_PROXY_DEBUG
                backgroundThread_ = new VisibleBricksBackgroundThread(this, inport_.getData(), tfi, static_cast<float>(threshold_.get()),
                    geometry_, &volumeStructure_, volStructureSize_, stepSize, debugOutput_.get(), enableClipping_.get(), clipLlf, clipUrb);
#else
                backgroundThread_ = new VisibleBricksBackgroundThread(this, inport_.getData(), tfi, static_cast<float>(threshold_.get()),
                    geometry_, &volumeStructure_, volStructureSize_, stepSize, false, enableClipping_.get(), clipLlf, clipUrb);
#endif
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
#endif
    else if (mode_.isSelected("visiblebricks")) {
        tgtAssert(transfunc_.get(), "no transfunc");
#ifdef VRN_PROXY_DEBUG
        if (setEnclosedOpaque_.get())
            processMaximalCubesSetEnclosedOpaque();
        else {
#endif
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

            //copy transfer function
            if(!tfCopy_)
                tfCopy_ = transfunc_.get()->clone();

            // determine TF type
            TransFunc1DKeys* tfi = 0;

            if (tfCopy_) {
                tfi = dynamic_cast<TransFunc1DKeys*>(tfCopy_);
                if (tfi == 0) {
                    TransFunc2DPrimitives* tfig = dynamic_cast<TransFunc2DPrimitives*>(tfCopy_);
                    if (tfig == 0) {
                        LERROR("Unsupported transfer function.");
                        return;
                    }
                    LWARNING("2D Transfer Function currently not supported: using bounding box mode.");
                    processCube();
                    return;
                }
            }
            else {
                LERROR("No valid transfer function");
                return;
            }

            //create worker thread that computes visible bricks proxy geometry
            int stepSize = resolutionVoxels_.get();
            tgt::vec3 clipLlf(clipRight_.get(), clipFront_.get(), clipBottom_.get());
            tgt::vec3 clipUrb(clipLeft_.get() + 1.f, clipBack_.get() + 1.f, clipTop_.get() + 1.f);
#ifdef VRN_PROXY_DEBUG
            backgroundThread_ = new VisibleBricksBackgroundThread(this, inport_.getData(), tfi, static_cast<float>(threshold_.get()),
                geometry_, &volumeStructure_, volStructureSize_, stepSize, debugOutput_.get(), enableClipping_.get(), clipLlf, clipUrb);
#else
            backgroundThread_ = new VisibleBricksBackgroundThread(this, inport_.getData(), tfi, static_cast<float>(threshold_.get()),
                geometry_, &volumeStructure_, volStructureSize_, stepSize, false, enableClipping_.get(), clipLlf, clipUrb);
#endif

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

#ifdef VRN_PROXY_DEBUG
        }
#endif
    }
    else if (mode_.get() == "outerfaces") {
        tgtAssert(transfunc_.get(), "no transfunc");

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

            //copy transfer function
            if(!tfCopy_)
                tfCopy_ = transfunc_.get()->clone();

            // determine TF type
            TransFunc1DKeys* tfi = 0;

            if (tfCopy_) {
                tfi = dynamic_cast<TransFunc1DKeys*>(tfCopy_);
                if (tfi == 0) {
                    TransFunc2DPrimitives* tfig = dynamic_cast<TransFunc2DPrimitives*>(tfCopy_);
                    if (tfig == 0) {
                        LERROR("Unsupported transfer function.");
                        return;
                    }
                    LWARNING("2D Transfer Function currently not supported: using bounding box mode.");
                    processCube();
                    return;
                }
            }
            else {
                LERROR("No valid transfer function");
                return;
            }

            //create worker thread that computes the outer facves proxy geometry
            int stepSize = resolutionVoxels_.get();
            tgt::vec3 clipLlf(clipRight_.get(), clipFront_.get(), clipBottom_.get());
            tgt::vec3 clipUrb(clipLeft_.get() + 1.f, clipBack_.get() + 1.f, clipTop_.get() + 1.f);
#ifdef VRN_PROXY_DEBUG
            backgroundThread_ = new OuterFacesBackgroundThread(this, inport_.getData(), tfi, static_cast<float>(threshold_.get()),
                geometry_, &volumeStructure_, volStructureSize_, stepSize, debugOutput_.get(), enableClipping_.get(), clipLlf, clipUrb);
#else
            backgroundThread_ = new OuterFacesBackgroundThread(this, inport_.getData(), tfi, static_cast<float>(threshold_.get()),
                geometry_, &volumeStructure_, volStructureSize_, stepSize, false, enableClipping_.get(), clipLlf, clipUrb);
#endif

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
#ifdef VRN_PROXY_DEBUG
    else if (mode_.isSelected("octreebricks")) {
        tgtAssert(transfunc_.get(), "no transfunc");

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

            //copy transfer function
            if(!tfCopy_)
                tfCopy_ = transfunc_.get()->clone();

            // determine TF type
            TransFunc1DKeys* tfi = 0;

            if (tfCopy_) {
                tfi = dynamic_cast<TransFunc1DKeys*>(tfCopy_);
                if (tfi == 0) {
                    TransFunc2DPrimitives* tfig = dynamic_cast<TransFunc2DPrimitives*>(tfCopy_);
                    if (tfig == 0) {
                        LERROR("Unsupported transfer function.");
                        return;
                    }
                    LWARNING("2D Transfer Function currently not supported: using bounding box mode.");
                    processCube();
                    return;
                }
            }
            else {
                LERROR("No valid transfer function");
                return;
            }

            //create worker thread that computes the octree visible bricks proxy geometry
            int stepSize = resolutionVoxels_.get();
            tgt::vec3 clipLlf(clipRight_.get(), clipFront_.get(), clipBottom_.get());
            tgt::vec3 clipUrb(clipLeft_.get() + 1.f, clipBack_.get() + 1.f, clipTop_.get() + 1.f);

            backgroundThread_ = new OctreeBackgroundThread(this, inport_.getData(), tfi, static_cast<float>(threshold_.get()),
                geometry_, &octreeRoot_, checkHalfNodes_.get(), stepSize, debugOutput_.get(), enableClipping_.get(), clipLlf, clipUrb);

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
    else if (mode_.get() == "brutemincube") {
        tgtAssert(transfunc_.get(), "no transfunc");
        processBruteMinCube();
    }
    else if (mode_.get() == "bruteforce") {
        tgtAssert(transfunc_.get(), "no transfunc");
        processBruteForceCubes();
    }
    else if (mode_.get() == "octree") {
        tgtAssert(transfunc_.get(), "no transfunc");
        processOctree();
    }
    else if (mode_.get() == "structurecubes") {
        tgtAssert(transfunc_.get(), "no transfunc");
        processCubes();
    }
#endif

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

#ifdef VRN_PROXY_DEBUG
    //for debug output: compute volume of the proxy geometry before clipping
    float geometryVolume;
    if (debugOutput_.get())
        geometryVolume = tgt::Bounds(inputVolume->getPhysicalToVoxelMatrix() * coordLlf,
                                     inputVolume->getPhysicalToVoxelMatrix() * coordUrb).volume();
#endif

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
#ifdef VRN_PROXY_DEBUG
    if (debugOutput_.get())
        LINFO("Created bounding box proxy geometry with volume " + ftos(geometryVolume) + " (before clipping)");
#endif
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
        transfunc_.setVisible(false);
        resolutionMode_.setVisible(false);
        resolution_.setVisible(false);
        resolutionVoxels_.setVisible(false);
        threshold_.setVisible(false);
        enableClipping_.setVisible(true);
        adjustClipPropertiesVisibility();
#ifdef VRN_PROXY_DEBUG
        checkHalfNodes_.setVisible(false);
        setEnclosedOpaque_.setVisible(false);
        debugOutput_.setVisible(true);
#endif
    }
    else {
        transfunc_.setVisible(true);
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
        threshold_.setVisible(true);

        enableClipping_.setVisible(true);
        adjustClipPropertiesVisibility();
#ifdef VRN_PROXY_DEBUG
        if (mode_.get() == "octreebricks")
           checkHalfNodes_.setVisible(true);
        else
           checkHalfNodes_.setVisible(false);

        debugOutput_.setVisible(true);

        if (mode_.get() == "visiblebricks")
            setEnclosedOpaque_.setVisible(true);
        else
            setEnclosedOpaque_.setVisible(false);
#endif
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

    delete tfCopy_;
    tfCopy_ = 0;
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
#ifdef VRN_PROXY_DEBUG
    octreeInvalid_ = true;
#endif
    // adapt clipping plane properties on volume change
    adjustClipPropertiesRanges();

    // extract ROI from volume and adjust clipping sliders accordingly
    adjustClippingToVolumeROI();
#ifdef VRN_PROXY_DEBUG
    //delete octree
    delete octreeRoot_;
    octreeRoot_ = 0;
#endif
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
#ifdef VRN_PROXY_DEBUG
    octreeInvalid_ = true;
#endif
    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }
}

bool OptimizedProxyGeometry::structureInvalid() const {
    return structureInvalid_;
}

#ifdef VRN_PROXY_DEBUG
bool OptimizedProxyGeometry::octreeInvalid() const {
    return octreeInvalid_;
}
#endif

bool OptimizedProxyGeometry::geometryInvalid() const {
    return geometryInvalid_;
}

void OptimizedProxyGeometry::setStructureInvalid(bool value) {
    structureInvalid_ = value;
}

#ifdef VRN_PROXY_DEBUG
void OptimizedProxyGeometry::setOctreeInvalid(bool value) {
    octreeInvalid_ = value;
}
#endif

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

/*
 * Modes only availabe in debug mode
 *-----------------------------------*/

#ifdef VRN_PROXY_DEBUG

void OptimizedProxyGeometry::onCheckHalfNodesChange() {
    geometryInvalid_ = true;

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }
}

bool OptimizedProxyGeometry::isRegionEmptyPi(float min, float max, const PreIntegrationTable* piTable) const {
    return (piTable->classify(min, max).a <= 0.0001 * static_cast<float>(threshold_.get()));
}

void OptimizedProxyGeometry::computeOctreeRecursively(const VolumeBase* inputVolume) {

    stopWatch_.reset();
    stopWatch_.start();

    tgt::ivec3 dim = inputVolume->getDimensions();
    //const int res = resolution_.get();

    const VolumeRAM* vol = inputVolume->getRepresentation<VolumeRAM>();
    RealWorldMapping rwm = inputVolume->getRealWorldMapping();

    int stepSize = resolutionVoxels_.get();
    const tgt::ivec3 step = tgt::ivec3(stepSize);

    //determine size for this resolution
    const tgt::ivec3 size(
            static_cast<int>(std::ceil(static_cast<float>(dim.x) / static_cast<float>(stepSize))),
            static_cast<int>(std::ceil(static_cast<float>(dim.y) / static_cast<float>(stepSize))),
            static_cast<int>(std::ceil(static_cast<float>(dim.z) / static_cast<float>(stepSize))));

    //create root for new octree
    octreeRoot_ = new ProxyGeometryOctreeNode();

    tgt::ivec3 pos(0);
    subdivideOctreeNodeRecursively(octreeRoot_, pos, size, stepSize, vol, dim, rwm);

    stopWatch_.stop();
    if (debugOutput_.get())
        std::cout << "Recursive octree construction took " << stopWatch_.getRuntime() <<  " milliseconds" << std::endl;
}

void OptimizedProxyGeometry::subdivideOctreeNodeRecursively(ProxyGeometryOctreeNode* current, tgt::ivec3 pos, tgt::ivec3 size, int stepSize,
    const VolumeRAM* vol, tgt::ivec3 dim, RealWorldMapping rwm)
{
    if (!current) {
        LERROR("Encounter node 0 during octree creation...");
        return;
    }

    //if there is no further subdividing possible: make current node leaf and compute min and max values as well as bounds
    if ((size.x == 1) || (size.y == 1) || (size.z == 1)) {
        current->isLeaf_ = true;
        tgt::ivec3 llf = pos * stepSize;
        tgt::ivec3 urb = (pos + size) * stepSize;

        //since a voxel is the center, add/subtract 0.5 to/from the coordinates to get the bounding box
        current->bounds_ = tgt::Bounds(tgt::vec3(llf) - tgt::vec3(0.5f), tgt::vec3(urb) + tgt::vec3(0.5f));

        // find min and max intensities
        float minIntensity = std::numeric_limits<float>::max();
        float maxIntensity = std::numeric_limits<float>::min();
        llf = tgt::max(llf - tgt::ivec3(1),tgt::ivec3(0));
        llf = tgt::min(llf,dim - 1);
        urb = tgt::max(urb + tgt::ivec3(1),tgt::ivec3(0));
        urb = tgt::min(urb,dim - 1);

        //don't use macro because of interruption points within the loops...
        //VRN_FOR_EACH_VOXEL(pos, llf, urb) {
        for (pos = llf; pos.z < urb.z; ++pos.z) {
            //interruption point after each slice

            for (pos.y = llf.y; pos.y < urb.y; ++pos.y) {
                for (pos.x = llf.x; pos.x < urb.x; ++pos.x) {
                    float currentIntensity = vol->getVoxelNormalized(pos);
                    //apply realworld mapping
                    currentIntensity = rwm.normalizedToRealWorld(currentIntensity);
                    minIntensity = std::min(minIntensity,currentIntensity);
                    maxIntensity = std::max(maxIntensity,currentIntensity);
                }
            }
        }

        current->minMaxIntensity_ = tgt::vec2(minIntensity, maxIntensity);
    }
    else {
        //subdivide the current node
        tgt::ivec3 sizeLlf = size / 2;
        tgt::ivec3 sizeUrb = size - sizeLlf;
        tgt::ivec3 posUrb = pos + sizeLlf;

        //recursively compute children
        current->llf_ = new ProxyGeometryOctreeNode();
        subdivideOctreeNodeRecursively(current->llf_, pos, sizeLlf, stepSize, vol, dim, rwm);

        current->lrf_ = new ProxyGeometryOctreeNode();
        subdivideOctreeNodeRecursively(current->lrf_, tgt::ivec3(posUrb.x, pos.y, pos.z),
            tgt::ivec3(sizeUrb.x, sizeLlf.y, sizeLlf.z), stepSize, vol, dim, rwm);

        current->ulf_ = new ProxyGeometryOctreeNode();
        subdivideOctreeNodeRecursively(current->ulf_, tgt::ivec3(pos.x, posUrb.y, pos.z),
            tgt::ivec3(sizeLlf.x, sizeUrb.y, sizeLlf.z), stepSize, vol, dim, rwm);

        current->urf_ = new ProxyGeometryOctreeNode();
        subdivideOctreeNodeRecursively(current->urf_, tgt::ivec3(posUrb.x, posUrb.y, pos.z),
            tgt::ivec3(sizeUrb.x, sizeUrb.y, sizeLlf.z), stepSize, vol, dim, rwm);

        current->llb_ = new ProxyGeometryOctreeNode();
        subdivideOctreeNodeRecursively(current->llb_, tgt::ivec3(pos.x, pos.y, posUrb.z),
            tgt::ivec3(sizeLlf.x, sizeLlf.y, sizeUrb.z), stepSize, vol, dim, rwm);

        current->lrb_ = new ProxyGeometryOctreeNode();
        subdivideOctreeNodeRecursively(current->lrb_, tgt::ivec3(posUrb.x, pos.y, posUrb.z),
            tgt::ivec3(sizeUrb.x, sizeLlf.y, sizeUrb.z), stepSize, vol, dim, rwm);

        current->ulb_ = new ProxyGeometryOctreeNode();
        subdivideOctreeNodeRecursively(current->ulb_, tgt::ivec3(pos.x, posUrb.y, posUrb.z),
            tgt::ivec3(sizeLlf.x, sizeUrb.y, sizeUrb.z), stepSize, vol, dim, rwm);

        current->urb_ = new ProxyGeometryOctreeNode();
        subdivideOctreeNodeRecursively(current->urb_, posUrb, sizeUrb, stepSize, vol, dim, rwm);

        //compute bounds and min/max from child nodes, set parent to child nodes
        current->llf_->parent_ = current;
        current->lrf_->parent_ = current;
        current->ulf_->parent_ = current;
        current->urf_->parent_ = current;
        current->llb_->parent_ = current;
        current->lrb_->parent_ = current;
        current->ulb_->parent_ = current;
        current->urb_->parent_ = current;

        current->bounds_ = current->llf_->bounds_;
        current->bounds_.addVolume(current->lrf_->bounds_);
        current->bounds_.addVolume(current->ulf_->bounds_);
        current->bounds_.addVolume(current->urf_->bounds_);
        current->bounds_.addVolume(current->llb_->bounds_);
        current->bounds_.addVolume(current->lrb_->bounds_);
        current->bounds_.addVolume(current->ulb_->bounds_);
        current->bounds_.addVolume(current->urb_->bounds_);

        float min, max;
        min = current->llf_->minMaxIntensity_.x;
        max = current->llf_->minMaxIntensity_.y;
        min = std::min(min, current->lrf_->minMaxIntensity_.x);
        max = std::max(max, current->lrf_->minMaxIntensity_.y);
        min = std::min(min, current->ulf_->minMaxIntensity_.x);
        max = std::max(max, current->ulf_->minMaxIntensity_.y);
        min = std::min(min, current->urf_->minMaxIntensity_.x);
        max = std::max(max, current->urf_->minMaxIntensity_.y);
        min = std::min(min, current->llb_->minMaxIntensity_.x);
        max = std::max(max, current->llb_->minMaxIntensity_.y);
        min = std::min(min, current->lrb_->minMaxIntensity_.x);
        max = std::max(max, current->lrb_->minMaxIntensity_.y);
        min = std::min(min, current->ulb_->minMaxIntensity_.x);
        max = std::max(max, current->ulb_->minMaxIntensity_.y);
        min = std::min(min, current->urb_->minMaxIntensity_.x);
        max = std::max(max, current->urb_->minMaxIntensity_.y);

        current->minMaxIntensity_ = tgt::vec2(min, max);
    }
}



bool OptimizedProxyGeometry::isRegionEmpty(const VolumeRAM* vol, tgt::ivec3 llf, tgt::ivec3 urb, const tgt::Texture* tfTexture, float threshold) {

    const tgt::ivec3 dim = inport_.getData()->getDimensions();

    llf = tgt::max(llf,tgt::ivec3(0));
    llf = tgt::min(llf,dim - 1);

    urb = tgt::max(urb + tgt::ivec3(1),tgt::ivec3(0));
    urb = tgt::min(urb,dim - 1);

    VRN_FOR_EACH_VOXEL(pos, llf, urb) {
                float current = vol->getVoxelNormalized(pos);
                //apply realworld mapping and TF domain
                current = inport_.getData()->getRealWorldMapping().normalizedToRealWorld(current);
                current = transfunc_.get()->realWorldToNormalized(current);

                //check both intensity values (floor and ceil) to account for linear interpolation
                int widthMinusOne = tfTexture->getWidth()-1;
                tgt::vec4 valueFloor = tgt::vec4(tfTexture->texel<tgt::col4>(static_cast<size_t>(tgt::clamp(static_cast<int>(std::floor(current * widthMinusOne)), 0, widthMinusOne)))) / 255.f;
                tgt::vec4 valueCeil = tgt::vec4(tfTexture->texel<tgt::col4>(static_cast<size_t>(tgt::clamp(static_cast<int>(std::ceil(current * widthMinusOne)), 0, widthMinusOne)))) / 255.f;

               if (std::max(valueFloor.a, valueCeil.a) > threshold)
                  return false;
    }

    return true;
}

tgt::ivec3 OptimizedProxyGeometry::getUrbPi(tgt::ivec3 llf, TransFunc1DKeys* tfi, const PreIntegrationTable* piTable) {
    const tgt::ivec3 size = volStructureSize_;
    tgt::bvec3 inc(true);
    tgt::ivec3 urb(llf);

    while(inc.x||inc.y||inc.z) {
        if (inc.x) {
            if (urb.x+1>size.x-1) {
                inc.x=false;
            } else {
                if (isVolNotEmptyPiNotBound(
                    tgt::ivec3(urb.x+1,llf.y,llf.z),
                    tgt::ivec3(urb.x+1,urb.y,urb.z),
                    tfi, piTable))
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
                    tfi, piTable)) {
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
                    tfi, piTable)) {
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


tgt::ivec3 OptimizedProxyGeometry::getUrbOpaque(tgt::ivec3 llf) {
    const tgt::ivec3 size = volStructureSize_;
    tgt::bvec3 inc(true);
    tgt::ivec3 urb(llf);

    while(inc.x||inc.y||inc.z) {
        if (inc.x) {
            if (urb.x+1>size.x-1) {
                inc.x=false;
            } else {
                if (isVolOpaqueNotBound(
                    tgt::ivec3(urb.x+1,llf.y,llf.z),
                    tgt::ivec3(urb.x+1,urb.y,urb.z)))
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
                if (isVolOpaqueNotBound(
                    tgt::ivec3(llf.x,urb.y+1,llf.z),
                    tgt::ivec3(urb.x,urb.y+1,urb.z))) {
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
                if (isVolOpaqueNotBound(
                    tgt::ivec3(llf.x,llf.y,urb.z+1),
                    tgt::ivec3(urb.x,urb.y,urb.z+1))) {
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

void OptimizedProxyGeometry::setVolBound(tgt::ivec3 llf, tgt::ivec3 urb, bool value) {
    for (int z=llf.z; z<=urb.z;z++) {
        for (int y=llf.y; y<=urb.y; y++) {
            for (int x=llf.x; x<=urb.x; x++) {
                ProxyGeometryVolumeRegion* v = getVolumeRegion(tgt::ivec3(x,y,z));
                v->setBound(value);
            }
        }
    }
}

bool OptimizedProxyGeometry::isVolNotEmptyPiNotBound(tgt::ivec3 llf, tgt::ivec3 urb, TransFunc1DKeys* tfi, const PreIntegrationTable* piTable) {
    for (int z=llf.z; z<=urb.z;z++) {
        for (int y=llf.y; y<=urb.y; y++) {
            for (int x=llf.x; x<=urb.x; x++) {
                ProxyGeometryVolumeRegion* v = getVolumeRegion(tgt::ivec3(x,y,z));
                if (v->isBound())
                    return false;
                if (isRegionEmptyPi(tfi->realWorldToNormalized(v->getMinIntensity()), tfi->realWorldToNormalized(v->getMaxIntensity()), piTable))
                    return false;
            }
        }
    }
    return true;
}

bool OptimizedProxyGeometry::isVolNotEmptyPiNotBound(tgt::ivec3 pos, TransFunc1DKeys* tfi, const PreIntegrationTable* piTable) {
     ProxyGeometryVolumeRegion* v = getVolumeRegion(pos);
     if (v->isBound())
        return false;
     if (isRegionEmptyPi(tfi->realWorldToNormalized(v->getMinIntensity()), tfi->realWorldToNormalized(v->getMaxIntensity()), piTable))
        return false;

     return true;
}

bool OptimizedProxyGeometry::isVolOpaqueNotBound(tgt::ivec3 llf, tgt::ivec3 urb) {
    for (int z=llf.z; z<=urb.z;z++) {
        for (int y=llf.y; y<=urb.y; y++) {
            for (int x=llf.x; x<=urb.x; x++) {
                ProxyGeometryVolumeRegion* v = getVolumeRegion(tgt::ivec3(x,y,z));
                if (v->isBound())
                    return false;
                if (!v->isOpaque())
                    return false;
            }
        }
    }
    return true;
}

bool OptimizedProxyGeometry::isVolOpaqueNotBound(tgt::ivec3 pos) {
     ProxyGeometryVolumeRegion* v = getVolumeRegion(pos);
     if (v->isBound())
        return false;
     if (!v->isOpaque())
        return false;

     return true;
}

ProxyGeometryVolumeRegion* OptimizedProxyGeometry::getVolumeRegion(tgt::ivec3 pos) {
    return &volumeStructure_[pos.z * (volStructureSize_.x * volStructureSize_.y) + pos.y * volStructureSize_.x + pos.x];
}

void OptimizedProxyGeometry::computeRegionStructure(const VolumeBase* inputVolume) {
    volumeStructure_.clear();

    stopWatch_.reset();
    stopWatch_.start();

    tgt::ivec3 dim = inputVolume->getDimensions();
    //const int res = resolution_.get();

    const VolumeRAM* vol = inputVolume->getRepresentation<VolumeRAM>();
    RealWorldMapping rwm = inputVolume->getRealWorldMapping();

    /*
    //determine shortest side and for this side the step size
    int minDim = std::min(std::min(dim.x, dim.y), dim.z);
    int stepSize = std::max(1, tgt::iround(static_cast<float>(minDim) / static_cast<float>(res)));
    const tgt::ivec3 step = tgt::ivec3(stepSize);
    */
    int stepSize = resolutionVoxels_.get();
    const tgt::ivec3 step = tgt::ivec3(stepSize);

    //determine size for this resolution
    const tgt::ivec3 size(
            static_cast<int>(std::ceil(static_cast<float>(dim.x) / static_cast<float>(stepSize))),
            static_cast<int>(std::ceil(static_cast<float>(dim.y) / static_cast<float>(stepSize))),
            static_cast<int>(std::ceil(static_cast<float>(dim.z) / static_cast<float>(stepSize))));

    volStructureSize_ = size;

    tgt::ivec3 pos, llf, urb;

    for (VolumeIterator it(size); !it.outofrange(); it.next()) {
        pos = it.value();
        llf = pos * step;
        urb = (pos+1) * step - tgt::ivec3(1);
        //urb = tgt::min(urb, dim);

        //since a voxel is the center, add/subtract 0.5 to/from the coordinates to get the bounding box
        tgt::Bounds regionBounds(tgt::vec3(llf) - tgt::vec3(0.5f), tgt::vec3(urb) + tgt::vec3(0.5f));

        // find min and max intensities
        float minIntensity = std::numeric_limits<float>::max();
        float maxIntensity = std::numeric_limits<float>::min();
        llf = tgt::max(llf - tgt::ivec3(1),tgt::ivec3(0));
        llf = tgt::min(llf,dim - 1);
        urb = tgt::max(urb + tgt::ivec3(1),tgt::ivec3(0));
        urb = tgt::min(urb,dim - 1);

        VRN_FOR_EACH_VOXEL(pos, llf, urb) {
           float current = vol->getVoxelNormalized(pos);
           //apply realworld mapping
           current = rwm.normalizedToRealWorld(current);
           minIntensity = std::min(minIntensity,current);
           maxIntensity = std::max(maxIntensity,current);
        }

        //add region
        volumeStructure_.push_back(ProxyGeometryVolumeRegion(regionBounds,tgt::vec2(minIntensity,maxIntensity)));

    }

    stopWatch_.stop();
    if (debugOutput_.get())
        std::cout << "Computing region structure took " << stopWatch_.getRuntime() << " milliseconds" << std::endl;
}



float OptimizedProxyGeometry::traverseOctreeAndCreateGeometry(ProxyGeometryOctreeNode* node, TransFunc1DKeys* tfi,
        const PreIntegrationTable* piTable, tgt::ivec3 dim, tgt::Bounds clipBounds) {

    float proxyVolume = 0.f;

    if (!node) {
        LERROR("Encountered 0 pointer instead of valid node during octree traversal...");
        return 0.f;
    }

    // get min and max intensity of this region
    float minIntensity = tfi->realWorldToNormalized(node->minMaxIntensity_.x);
    float maxIntensity = tfi->realWorldToNormalized(node->minMaxIntensity_.y);

    // if region is empty: do not traverse any further
    if (isRegionEmptyPi(minIntensity, maxIntensity, piTable))
        return 0.f;

    //if region is leaf: add geometry
    if (node->isLeaf_) {
        if (enableClipping_.get()) {
            //only add and clip mesh if necessary
            if (clipBounds.containsVolume(node->bounds_))
                addCubeMesh(geometry_, node->bounds_, dim);
            else if (clipBounds.intersects(node->bounds_))
                addCubeMeshClip(geometry_, node->bounds_,dim,clipBounds);
        }
        else
            addCubeMesh(geometry_, node->bounds_,dim/*,inputVolume->getVoxelToPhysicalMatrix()*/);

        proxyVolume += node->bounds_.volume();
    }
    else {
        //recursively traverse
        proxyVolume += traverseOctreeAndCreateGeometry(node->llf_, tfi, piTable, dim, clipBounds);
        proxyVolume += traverseOctreeAndCreateGeometry(node->lrf_, tfi, piTable, dim, clipBounds);
        proxyVolume += traverseOctreeAndCreateGeometry(node->ulf_, tfi, piTable, dim, clipBounds);
        proxyVolume += traverseOctreeAndCreateGeometry(node->urf_, tfi, piTable, dim, clipBounds);
        proxyVolume += traverseOctreeAndCreateGeometry(node->llb_, tfi, piTable, dim, clipBounds);
        proxyVolume += traverseOctreeAndCreateGeometry(node->lrb_, tfi, piTable, dim, clipBounds);
        proxyVolume += traverseOctreeAndCreateGeometry(node->ulb_, tfi, piTable, dim, clipBounds);
        proxyVolume += traverseOctreeAndCreateGeometry(node->urb_, tfi, piTable, dim, clipBounds);
    }

    return proxyVolume;
}

void OptimizedProxyGeometry::setEnclosedOpaque() {
    tgt::ivec3 pos;

    setVolBound(tgt::ivec3(0),volStructureSize_-1,false);

    for (VolumeIterator it(volStructureSize_); !it.outofrange(); it.next()) {
        pos = it.value();
        ProxyGeometryVolumeRegion* current = getVolumeRegion(pos);
        if (!current->isOpaque() && !current->isBound()) {
            if (isEnclosed(pos)) {
                current->setOpaque(true);
            }
        }
    }
}

bool OptimizedProxyGeometry::isEnclosed(tgt::ivec3 pos) {
    const tgt::ivec3 tpos[6] = {tgt::ivec3(0,0,-1)
                         ,tgt::ivec3(0,0,1)
                         ,tgt::ivec3(-1,0,0)
                         ,tgt::ivec3(1,0,0)
                         ,tgt::ivec3(0,-1,0)
                         ,tgt::ivec3(0,1,0)};
    std::queue<tgt::ivec3> q;
    tgt::ivec3 qpos,ipos;
    tgt::bvec3 tless, tgreater;
    bool enclosed = true;

    q.push(pos);
    while (!q.empty()) {
        qpos = q.front(); q.pop();
        ProxyGeometryVolumeRegion* qCurrent = getVolumeRegion(pos);
        if (!qCurrent->isOpaque()) {if (!qCurrent->isBound()) {qCurrent->setBound(true);

        for (int i=0; i<6; i++) {
            ipos = qpos + tpos[i];
            tless = tgt::lessThan(ipos,tgt::ivec3(0));
            tgreater = tgt::greaterThan(ipos,volStructureSize_-1);
            if (tless.x || tless.y || tless.z || tgreater.x || tgreater.y || tgreater.z) enclosed = false;
            else q.push(ipos);
        }

        }}
    }

    return enclosed;
}

void OptimizedProxyGeometry::processMaximalCubesSetEnclosedOpaque() {
    //get input data
    const VolumeBase* inputVolume = inport_.getData();
    tgt::ivec3 dim = inputVolume->getDimensions();
    //const int res = resolution_.get();

    // determine TF type
    TransFunc1DKeys* tfi = 0;
    //TransFunc2DPrimitives* tfig = 0;
    //intensityGradientTF_ = false;

    if (transfunc_.get()) {
        tfi = dynamic_cast<TransFunc1DKeys*>(transfunc_.get());
        if (tfi == 0) {
            TransFunc2DPrimitives* tfig = dynamic_cast<TransFunc2DPrimitives*>(transfunc_.get());
            if (tfig == 0) {
                LWARNING("CPURaycaster::process: unsupported tf");
                return;
            }
            LERROR("2D Transfer Function currently not supported: using bounding box mode.");
            processCube();
            return;
        }
    }

    if (structureInvalid_) {
        //invalidate geometry
        geometryInvalid_ = true;
        //clear structure
        volumeStructure_.clear();
        //compute new structure
        computeRegionStructure(inputVolume);
        structureInvalid_ = false;
    }

    if (geometryInvalid_) {
        geometry_->clear();

        stopWatch_.reset();
        stopWatch_.start();

        const PreIntegrationTable* piTable = tfi->getPreIntegrationTable(1.f, 256);

        stopWatch_.stop();
        if (debugOutput_.get())
            std::cout << "Fetching PreIntegration table took " << stopWatch_.getRuntime() << " milliseconds" << std::endl;

        stopWatch_.reset();
        stopWatch_.start();

        setVolBound(tgt::ivec3(0),volStructureSize_-1,false);

        //for every region: determine opacity
        std::vector<ProxyGeometryVolumeRegion>::iterator regionIterator;
        for (regionIterator = volumeStructure_.begin(); regionIterator != volumeStructure_.end(); ++regionIterator) {
            //apply tf domain
            float minIntensity = tfi->realWorldToNormalized(regionIterator->getMinIntensity());
            float maxIntensity = tfi->realWorldToNormalized(regionIterator->getMaxIntensity());
            if (isRegionEmptyPi(minIntensity,maxIntensity,piTable))
                regionIterator->setOpaque(false);
            else
                regionIterator->setOpaque(true);
        }

        //set enclosed regions opaque
        setEnclosedOpaque();

        //now build maximal cubes
        int numberOfCubes = 0; //number of created cubes
        float proxyVolume = 0.f;

        tgt::ivec3 pos, urbVol;
        tgt::vec3 llf, urb;
        for (VolumeIterator it(volStructureSize_); !it.outofrange(); it.next()) {
            pos = it.value();
            if (isVolOpaqueNotBound(pos)) {
                urbVol = getUrbOpaque(pos);
                llf = getVolumeRegion(pos)->getBounds().getLLF();
                urb = getVolumeRegion(urbVol)->getBounds().getURB();
                tgt::Bounds cubeBounds(llf,urb);
                if (enableClipping_.get()) {
                    //get clipping planes
                    tgt::vec3 clipLlf(clipRight_.get(), clipFront_.get(), clipBottom_.get());
                    tgt::vec3 clipUrb(clipLeft_.get() + 1.f, clipBack_.get() + 1.f, clipTop_.get() + 1.f);
                    tgt::Bounds clipBounds(clipLlf, clipUrb);
                    //only add and clip cube mesh if necessary
                    if (clipBounds.containsVolume(cubeBounds))
                        addCubeMesh(geometry_, cubeBounds, dim);
                    else if (clipBounds.intersects(cubeBounds))
                        addCubeMeshClip(geometry_, cubeBounds,dim,clipBounds);
                }
                else
                    addCubeMesh(geometry_, cubeBounds,dim);

                if (debugOutput_.get()) {
                    proxyVolume += cubeBounds.volume();
                    numberOfCubes++;
                }
            }
        }

        stopWatch_.stop();
        if (debugOutput_.get()) {
            std::cout << "Determined maximal cubes (setting enclosed opaque in " << stopWatch_.getRuntime() << " milliseconds" << std::endl;
            std::cout << "Created Proxy Geometry consisting of " << numberOfCubes
                      << " cubes using maximal cubes mode with setting enclosed regions opaque," << std::endl;
            std::cout << " volume < " << proxyVolume << " (before clipping)" << std::endl;
        }

        //if mesh list is empty: add first region (empty mesh geometry might lead to camera problem)
        if (geometry_->isEmpty() && !volumeStructure_.empty()) {
            ProxyGeometryVolumeRegion& region = volumeStructure_[0];
            addCubeMesh(geometry_, region.getBounds(),dim);
        }

        geometry_->setTransformationMatrix(inputVolume->getTextureToWorldMatrix());
        geometryInvalid_ = false;
    }

}


void OptimizedProxyGeometry::onSetEnclosedOpaqueChange() {
    geometryInvalid_ = true;

    if (backgroundThread_) {
        backgroundThread_->interrupt();
        delete backgroundThread_;
        backgroundThread_ = 0;
    }
}

void OptimizedProxyGeometry::processBruteMinCube() {

    geometry_->clear();

    const VolumeBase* inputVolume = inport_.getData();

    //determine TF type
    TransFunc1DKeys* tfi = 0;
    //TransFunc2DPrimitives* tfig = 0;
    //intensityGradientTF_ = false;

    if (transfunc_.get()) {
        tfi = dynamic_cast<TransFunc1DKeys*>(transfunc_.get());
        if (tfi == 0) {
            TransFunc2DPrimitives* tfig = dynamic_cast<TransFunc2DPrimitives*>(transfunc_.get());
            if (tfig == 0) {
                LWARNING("CPURaycaster::process: unsupported tf");
                return;
            }
            LERROR("2D Transfer Function currently not supported: using cube mode.");
            processCube();
            return;
        }
    }

    // retrieve tf texture
    tgt::Texture* tfTexture = transfunc_.get()->getTexture();
    tfTexture->downloadTexture();

    tgt::ivec3 dim = inputVolume->getDimensions();

    const VolumeRAM* vol = inputVolume->getRepresentation<VolumeRAM>();

    /*
    //determine shortest side and for this side the step size
    int minDim = std::min(std::min(dim.x, dim.y), dim.z);
    int stepSize = std::max(1, tgt::iround(static_cast<float>(minDim) / static_cast<float>(res)));
    const tgt::ivec3 step = tgt::ivec3(stepSize);
    */
    int stepSize = resolutionVoxels_.get();
    const tgt::ivec3 step = tgt::ivec3(stepSize);

    stopWatch_.reset();

    //determine size for this resolution
    const tgt::ivec3 size(
            static_cast<int>(std::ceil(static_cast<float>(dim.x) / static_cast<float>(stepSize))),
            static_cast<int>(std::ceil(static_cast<float>(dim.y) / static_cast<float>(stepSize))),
            static_cast<int>(std::ceil(static_cast<float>(dim.z) / static_cast<float>(stepSize))));

    tgt::ivec3 pos, llf, urb;

    tgt::Bounds bounds; //bounding box
    //find bounds
    stopWatch_.start(); //get runtime for determining volume
    for (VolumeIterator it(size); !it.outofrange(); it.next()) {
        pos = it.value();
        llf = pos * step;
        urb = (pos+1) * step - tgt::ivec3(1);
        //check if the region is empty and if not: include the region
        if (!isRegionEmpty(vol, llf - tgt::ivec3(1),urb + tgt::ivec3(1), tfTexture, static_cast<float>(threshold_.get())*0.001f)) {
            bounds.addVolume(tgt::Bounds(llf,urb));
        }
    }
    stopWatch_.stop();

    //since a voxel is the center, add/subtract 0.5 to/from the coordinates
    bounds = tgt::Bounds(bounds.getLLF() - tgt::vec3(0.5f), bounds.getURB() + tgt::vec3(0.5f));

    if (enableClipping_.get()) {
        tgt::vec3 clipLlf(clipRight_.get(), clipFront_.get(), clipBottom_.get());
        tgt::vec3 clipUrb(clipLeft_.get() + 1.f, clipBack_.get() + 1.f, clipTop_.get() + 1.f);
        tgt::Bounds clipBounds(clipLlf, clipUrb);

        addCubeMeshClip(geometry_, bounds, dim, clipBounds);
    }
    else
        addCubeMesh(geometry_, bounds, dim);

    geometry_->setTransformationMatrix(inputVolume->getTextureToWorldMatrix());

    if (debugOutput_.get()) {
        std::cout << "Created minimal cube proxy geometry with volume " << bounds.volume() << " (before clipping)" << std::endl;
        std::cout << "Determined minimal cube (brute-force) in " << stopWatch_.getRuntime() << " milliseconds" << std::endl;
    }
}

void OptimizedProxyGeometry::processBruteForceCubes() {

    geometry_->clear();

    const VolumeBase* inputVolume = inport_.getData();

    // determine TF type
    TransFunc1DKeys* tfi = 0;
    //TransFunc2DPrimitives* tfig = 0;
    //intensityGradientTF_ = false;

    if (transfunc_.get()) {
        tfi = dynamic_cast<TransFunc1DKeys*>(transfunc_.get());
        if (tfi == 0) {
            TransFunc2DPrimitives* tfig = dynamic_cast<TransFunc2DPrimitives*>(transfunc_.get());
            if (tfig == 0) {
                LWARNING("CPURaycaster::process: unsupported tf");
                return;
            }
            LERROR("2D Transfer Function currently not supported: using cube mode.");
            processCube();
            return;
        }
    }

    // retrieve tf texture
    tgt::Texture* tfTexture = transfunc_.get()->getTexture();
    tfTexture->downloadTexture();

    const VolumeRAM* vol = inputVolume->getRepresentation<VolumeRAM>();

    tgt::ivec3 dim = inputVolume->getDimensions();
    //const int res = resolution_.get();

    /*
    //determine shortest side and for this side the step size
    int minDim = std::min(std::min(dim.x, dim.y), dim.z);
    int stepSize = std::max(1, tgt::iround(static_cast<float>(minDim) / static_cast<float>(res)));
    const tgt::ivec3 step = tgt::ivec3(stepSize);
    */
    int stepSize = resolutionVoxels_.get();
    const tgt::ivec3 step = tgt::ivec3(stepSize);

    //determine size for this resolution
    const tgt::ivec3 size(
            static_cast<int>(std::ceil(static_cast<float>(dim.x) / static_cast<float>(stepSize))),
            static_cast<int>(std::ceil(static_cast<float>(dim.y) / static_cast<float>(stepSize))),
            static_cast<int>(std::ceil(static_cast<float>(dim.z) / static_cast<float>(stepSize))));

    tgt::ivec3 pos, llf, urb;

    int numberOfCubes = 0; //number of created cubes
    float proxyVolume = 0.f; //volume of the created proxy geometry

    stopWatch_.reset();
    stopWatch_.start();
    for (VolumeIterator it(size); !it.outofrange(); it.next()) {
        pos = it.value();
        llf = pos * step;
        urb = (pos+1) * step - tgt::ivec3(1);

        //since a voxel is the center, add/subtract 0.5 to/from the coordinates
        tgt::Bounds regionBounds(tgt::vec3(llf) - tgt::vec3(0.5f), tgt::vec3(urb) + tgt::vec3(0.5f));

        if (!isRegionEmpty(vol, llf - tgt::ivec3(1),urb + tgt::ivec3(1), tfTexture, static_cast<float>(threshold_.get())*0.001f)) {
            if (enableClipping_.get()) {
                //get clipping planes
                tgt::vec3 clipLlf(clipRight_.get(), clipFront_.get(), clipBottom_.get());
                tgt::vec3 clipUrb(clipLeft_.get() + 1.f, clipBack_.get() + 1.f, clipTop_.get() + 1.f);
                tgt::Bounds clipBounds(clipLlf, clipUrb);
                //only add and clip cube mesh if necessary
                if (clipBounds.containsVolume(regionBounds))
                    addCubeMesh(geometry_, regionBounds, dim);
                else if (clipBounds.intersects(regionBounds))
                    addCubeMeshClip(geometry_, regionBounds,dim,clipBounds);
            }
            else
                addCubeMesh(geometry_, regionBounds,dim);

            //for debugging output
            if (debugOutput_.get()) {
                proxyVolume += regionBounds.volume();
                numberOfCubes++;
            }
        }
    }
    stopWatch_.stop();
    if (debugOutput_.get()) {
        std::cout << "Built proxy geometry using brute-force cubes in " << stopWatch_.getRuntime() << " milliseconds" << std::endl;
        std::cout << "Created Proxy Geometry consisting of " << numberOfCubes << " cubes using brute force cube mode," << std::endl;
        std::cout << " volume < " << proxyVolume << " (before clipping)" << std::endl;
    }

    geometry_->setTransformationMatrix(inputVolume->getTextureToWorldMatrix());
}

void OptimizedProxyGeometry::processCubes() {

    //get input data
    const VolumeBase* inputVolume = inport_.getData();
    tgt::ivec3 dim = inputVolume->getDimensions();
    //const int res = resolution_.get();

    // determine TF type
    TransFunc1DKeys* tfi = 0;
    //TransFunc2DPrimitives* tfig = 0;
    //intensityGradientTF_ = false;

    if (transfunc_.get()) {
        tfi = dynamic_cast<TransFunc1DKeys*>(transfunc_.get());
        if (tfi == 0) {
            TransFunc2DPrimitives* tfig = dynamic_cast<TransFunc2DPrimitives*>(transfunc_.get());
            if (tfig == 0) {
                LWARNING("CPURaycaster::process: unsupported tf");
                return;
            }
            LERROR("2D Transfer Function currently not supported: using cube mode.");
            processCube();
            return;
        }
    }

    if (structureInvalid_) {
        //invalidate geometry
        geometryInvalid_ = true;
        //clear structure
        volumeStructure_.clear();
        //compute new structure
        computeRegionStructure(inputVolume);
        structureInvalid_ = false;
    }

    if (geometryInvalid_) {
        geometry_->clear();

        stopWatch_.reset();
        stopWatch_.start();

        const PreIntegrationTable* piTable = tfi->getPreIntegrationTable(1.f, 256);

        stopWatch_.stop();
        if (debugOutput_.get())
            std::cout << "Fetching PreIntegration table took " << stopWatch_.getRuntime() << " milliseconds" << std::endl;

        stopWatch_.reset();
        stopWatch_.start();

        //scan through region structure, classify every region and add the non-transparent blocks to the MeshGeometry
        int numberOfCubes = 0; //number of created cubes
        float proxyVolume = 0.f;

        std::vector<ProxyGeometryVolumeRegion>::const_iterator i;
        for (i = volumeStructure_.begin(); i != volumeStructure_.end(); ++i) {
            //apply tf domain
            float minIntensity = tfi->realWorldToNormalized(i->getMinIntensity());
            float maxIntensity = tfi->realWorldToNormalized(i->getMaxIntensity());
            if (!isRegionEmptyPi(minIntensity,maxIntensity,piTable)) {
                if (enableClipping_.get()) {
                    //get clipping planes
                    tgt::vec3 clipLlf(clipRight_.get(), clipFront_.get(), clipBottom_.get());
                    tgt::vec3 clipUrb(clipLeft_.get() + 1.f, clipBack_.get() + 1.f, clipTop_.get() + 1.f);
                    tgt::Bounds clipBounds(clipLlf, clipUrb);
                    //only add and clip cube mesh if necessary
                    if (clipBounds.containsVolume(i->getBounds()))
                        addCubeMesh(geometry_, i->getBounds(), dim);
                    else if (clipBounds.intersects(i->getBounds()))
                        addCubeMeshClip(geometry_, i->getBounds(),dim,clipBounds);
                }
                else
                    addCubeMesh(geometry_, i->getBounds(),dim/*,inputVolume->getVoxelToPhysicalMatrix()*/);

                if (debugOutput_.get()) {
                    proxyVolume += i->getBounds().volume();
                    numberOfCubes++;
                }
            }
        }

        stopWatch_.stop();

        if (debugOutput_.get()) {
            std::cout << "Created cubes in " << stopWatch_.getRuntime() << " milliseconds" << std::endl;
            std::cout << "Created Proxy Geometry consisting of " << numberOfCubes << " cubes using cube mode," << std::endl;
            std::cout << " volume < " << proxyVolume << " (before clipping)" << std::endl;
        }

        geometry_->setTransformationMatrix(inputVolume->getTextureToWorldMatrix());
        geometryInvalid_ = false;
    }
}

void OptimizedProxyGeometry::processOctree() {
    //get input data
    const VolumeBase* inputVolume = inport_.getData();
    tgt::ivec3 dim = inputVolume->getDimensions();
    //const int res = resolution_.get();

    // determine TF type
    TransFunc1DKeys* tfi = 0;
    //TransFunc2DPrimitives* tfig = 0;
    //intensityGradientTF_ = false;

    if (transfunc_.get()) {
        tfi = dynamic_cast<TransFunc1DKeys*>(transfunc_.get());
        if (tfi == 0) {
            TransFunc2DPrimitives* tfig = dynamic_cast<TransFunc2DPrimitives*>(transfunc_.get());
            if (tfig == 0) {
                LWARNING("CPURaycaster::process: unsupported tf");
                return;
            }
            LERROR("2D Transfer Function currently not supported: using cube mode.");
            processCube();
            return;
        }
    }

    if (octreeInvalid_) {
        delete octreeRoot_;
        octreeRoot_ = 0;
    }

    if (!octreeRoot_) {
        computeOctreeRecursively(inputVolume);
        octreeInvalid_ = false;
        //invalidate geometry
        geometryInvalid_ = true;
    }

    if (geometryInvalid_) {
        geometry_->clear();

        stopWatch_.reset();
        stopWatch_.start();

        const PreIntegrationTable* piTable = tfi->getPreIntegrationTable(1.f, 256);

        stopWatch_.stop();
        if (debugOutput_.get())
            std::cout << "Fetching PreIntegration table took " << stopWatch_.getRuntime() << " milliseconds" << std::endl;

        //get clipping planes
        tgt::vec3 clipLlf(clipRight_.get(), clipFront_.get(), clipBottom_.get());
        tgt::vec3 clipUrb(clipLeft_.get() + 1.f, clipBack_.get() + 1.f, clipTop_.get() + 1.f);
        tgt::Bounds clipBounds(clipLlf, clipUrb);

        stopWatch_.reset();
        stopWatch_.start();

        //traverse octree
        float proxyVolume = traverseOctreeAndCreateGeometry(octreeRoot_, tfi, piTable, dim, clipBounds);

        stopWatch_.stop();
        if (debugOutput_.get()) {
            std::cout << "Octree traversal and geometry creation took " << stopWatch_.getRuntime() << " milliseconds" << std::endl;
            std::cout << "Created Proxy Geometry consisting of " << geometry_->getNumTriangles()
                      << " triangles using octree cubes mode." << std::endl;
            std::cout << "Volume < " << proxyVolume << " (before clipping)" << std::endl;
        }

        //if mesh list is empty: add leftmost node (empty mesh geometry might lead to camera problem)
        if (geometry_->isEmpty() && octreeRoot_) {
            ProxyGeometryOctreeNode* node = octreeRoot_;
            while (!node->isLeaf_)
                node = node->llf_;
            addCubeMesh(geometry_, node->bounds_,dim);
        }

        geometry_->setTransformationMatrix(inputVolume->getTextureToWorldMatrix());
        geometryInvalid_ = false;
    }

}
#endif

//-------------------------------------------------------------------------------------------------
// background threads

OptimizedProxyGeometryBackgroundThread::OptimizedProxyGeometryBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume,
          TransFunc1DKeys* tf, float threshold, TriangleMeshGeometryVec4Vec3* geometry, int stepSize, bool debugOutput,
          bool clippingEnabled, tgt::vec3 clipLlf, tgt::vec3 clipUrb)
        : ProcessorBackgroundThread<OptimizedProxyGeometry>(processor)
        , volume_(volume)
        , tf_(tf)
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
          TransFunc1DKeys* tf, float threshold, TriangleMeshGeometryVec4Vec3* geometry, std::vector<ProxyGeometryVolumeRegion>* volumeStructure,
          tgt::ivec3 volStructureSize, int stepSize, bool debugOutput, bool clippingEnabled, tgt::vec3 clipLlf, tgt::vec3 clipUrb)
        : OptimizedProxyGeometryBackgroundThread(processor, volume, tf, threshold, geometry, stepSize, debugOutput, clippingEnabled, clipLlf, clipUrb)
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

        // find min and max intensities
        float minIntensity = std::numeric_limits<float>::max();
        float maxIntensity = std::numeric_limits<float>::min();
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
                    float current = vol->getVoxelNormalized(pos);
                    //apply realworld mapping
                    current = rwm.normalizedToRealWorld(current);
                    minIntensity = std::min(minIntensity,current);
                    maxIntensity = std::max(maxIntensity,current);
                }
            }
        }

        //add region
        volumeStructure_->push_back(ProxyGeometryVolumeRegion(regionBounds,tgt::vec2(minIntensity,maxIntensity)));

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

MinCubeBackgroundThread::MinCubeBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume, TransFunc1DKeys* tf,
                                                 float threshold, TriangleMeshGeometryVec4Vec3* geometry,
                                                 std::vector<ProxyGeometryVolumeRegion>* volumeStructure, tgt::ivec3 volStructureSize,
                                                 int stepSize, bool debugOutput, bool clippingEnabled, tgt::vec3 clipLlf, tgt::vec3 clipUrb)
        : StructureProxyGeometryBackgroundThread(processor, volume, tf, threshold, geometry, volumeStructure, volStructureSize, stepSize,
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

        const PreIntegrationTable* piTable = tf_->getPreIntegrationTable(1.f, 256);

        if (debugOutput_) {
            stopWatch_.stop();
            std::cout << "Fetching PreIntegration table took " << stopWatch_.getRuntime() << " milliseconds" << std::endl;

            stopWatch_.reset();
            stopWatch_.start();
        }

        interruptionPoint();

        //scan through region structure, classify every region and add the bounds of non-transparent blocks to the minimal bounding box
        tgt::Bounds minBounds;

        std::vector<ProxyGeometryVolumeRegion>::const_iterator i;
        for (i = volumeStructure_->begin(); i != volumeStructure_->end(); ++i) {
            interruptionPoint();
            //apply tf domain
            float minIntensity = tf_->realWorldToNormalized(i->getMinIntensity());
            float maxIntensity = tf_->realWorldToNormalized(i->getMaxIntensity());
            if (!isRegionEmptyPi(minIntensity,maxIntensity,piTable))
                minBounds.addVolume(i->getBounds());
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
        /*else {
            //if mesh list is empty: add first region (empty mesh geometry might lead to camera problem)
            if (!volumeStructure_->empty()) {
                VolumeRegion& region = (*volumeStructure_)[0];
                OptimizedProxyGeometry::addCubeMesh(geometry_, region.getBounds(),dim);
            }
        }*/

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

VisibleBricksBackgroundThread::VisibleBricksBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume, TransFunc1DKeys* tf,
                                                            float threshold, TriangleMeshGeometryVec4Vec3* geometry,
                                                            std::vector<ProxyGeometryVolumeRegion>* volumeStructure,
                                                            tgt::ivec3 volStructureSize, int stepSize, bool debugOutput,
                                                            bool clippingEnabled, tgt::vec3 clipLlf, tgt::vec3 clipUrb)
        : StructureProxyGeometryBackgroundThread(processor, volume, tf, threshold, geometry, volumeStructure, volStructureSize, stepSize,
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

        const PreIntegrationTable* piTable = tf_->getPreIntegrationTable(1.f, 256);

        if (debugOutput_) {
            stopWatch_.stop();
            std::cout << "Fetching PreIntegration table took " << stopWatch_.getRuntime() << " milliseconds" << std::endl;
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
            if (isVolNotEmptyPiNotBound(pos,tf_, piTable)) {
                urbVol = getUrbPi(pos,tf_, piTable);
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
                    OptimizedProxyGeometry::addCubeMesh(geometry_, cubeBounds,dim);

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

        //if mesh list is empty: add first region (empty mesh geometry might lead to camera problem)
        /*if (geometry_->isEmpty() && !volumeStructure_->empty()) {
            VolumeRegion& region = (*volumeStructure_)[0];
            OptimizedProxyGeometry::addCubeMesh(geometry_, region.getBounds(),dim);
        }*/

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

bool VisibleBricksBackgroundThread::isVolNotEmptyPiNotBound(tgt::ivec3 pos, TransFunc1DKeys* tfi, const PreIntegrationTable* piTable) {
     ProxyGeometryVolumeRegion& v = getVolumeRegion(pos);
     if (v.isBound())
        return false;
     if (isRegionEmptyPi(tfi->realWorldToNormalized(v.getMinIntensity()), tfi->realWorldToNormalized(v.getMaxIntensity()), piTable))
        return false;

     return true;
}

bool VisibleBricksBackgroundThread::isVolNotEmptyPiNotBound(tgt::ivec3 llf, tgt::ivec3 urb, TransFunc1DKeys* tfi,
                                                            const PreIntegrationTable* piTable)
{
    for (int z=llf.z; z<=urb.z;z++) {
        interruptionPoint();
        for (int y=llf.y; y<=urb.y; y++) {
            for (int x=llf.x; x<=urb.x; x++) {
                ProxyGeometryVolumeRegion& v = getVolumeRegion(tgt::ivec3(x,y,z));
                if (v.isBound())
                    return false;
                if (isRegionEmptyPi(tfi->realWorldToNormalized(v.getMinIntensity()), tfi->realWorldToNormalized(v.getMaxIntensity()), piTable))
                    return false;
            }
        }
    }
    return true;
}

tgt::ivec3 VisibleBricksBackgroundThread::getUrbPi(tgt::ivec3 llf, TransFunc1DKeys* tfi, const PreIntegrationTable* piTable) {

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
                    tfi, piTable))
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
                    tfi, piTable)) {
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
                    tfi, piTable)) {
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

OuterFacesBackgroundThread::OuterFacesBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume, TransFunc1DKeys* tf,
                                                            float threshold, TriangleMeshGeometryVec4Vec3* geometry,
                                                            std::vector<ProxyGeometryVolumeRegion>* volumeStructure,
                                                            tgt::ivec3 volStructureSize, int stepSize, bool debugOutput,
                                                            bool clippingEnabled, tgt::vec3 clipLlf, tgt::vec3 clipUrb)
        : StructureProxyGeometryBackgroundThread(processor, volume, tf, threshold, geometry, volumeStructure, volStructureSize, stepSize,
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

        const PreIntegrationTable* piTable = tf_->getPreIntegrationTable(1.f, 256);

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
            //apply tf domain
            float minIntensity = tf_->realWorldToNormalized(i->getMinIntensity());
            float maxIntensity = tf_->realWorldToNormalized(i->getMaxIntensity());
            if (!isRegionEmptyPi(minIntensity,maxIntensity,piTable))
                i->setOpaque(true);
            else
                i->setOpaque(false);
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

                        //modify the coordinates according to the neighboring blocks
                        //not necessary anymore as coordinates have already been corrected when building the bricks
                        /*if (sign < 0)
                            llf.elem[dim] -= 0.5;
                        else
                            urb.elem[dim] += 0.5;

                        for (int incr = 1; incr <= 2; ++incr) {
                            int curDim = (dim + incr) % 3;

                            //positively into this direction
                            neighborPos = pos;
                            neighborPos.elem[curDim] += 1;

                            if (neighborPos[curDim] >= volStructureSize_.elem[curDim]) {
                                //no neighbors in this direction
                                urb[curDim] += 0.5f;
                            }
                            else {
                                neighborPos.elem[dim] += sign;

                                if ((neighborPos.elem[dim] >= 0) && (neighborPos.elem[dim] < volStructureSize_.elem[dim]) && getVolumeRegion(neighborPos).isOpaque())
                                    urb[curDim] -= 0.5f;
                                else {
                                    neighborPos.elem[dim] -= sign;
                                    if (getVolumeRegion(neighborPos).isOpaque())
                                        urb[curDim] -= 0.5f;
                                    else    //both transparent
                                        urb[curDim] += 0.5f;
                                }
                            }

                            //negatively into this direction
                            neighborPos = pos;
                            neighborPos.elem[curDim] -= 1;

                            if (neighborPos[curDim] < 0) {
                                //no neighbors in this direction
                                llf[curDim] -= 0.5f;
                            }
                            else {
                                neighborPos.elem[dim] += sign;

                                if ((neighborPos.elem[dim] >= 0) && (neighborPos.elem[dim] < volStructureSize_.elem[dim]) && getVolumeRegion(neighborPos).isOpaque())
                                    llf[curDim] += 0.5f;
                                else {
                                    neighborPos.elem[dim] -= sign;
                                    if (getVolumeRegion(neighborPos).isOpaque())
                                        llf[curDim] += 0.5f;
                                    else    //both transparent
                                        llf[curDim] -= 0.5f;
                                }
                            }
                        }*/

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

#ifdef VRN_MODULE_STAGING
//-------------------------------------------------------------------------------------------------
// VolumeOctreeBackgroundThread

VolumeOctreeBackgroundThread::VolumeOctreeBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume, TransFunc1DKeys* tf,
                                               float threshold, TriangleMeshGeometryVec4Vec3* geometry, int stepSize, bool clippingEnabled, tgt::vec3 clipLlf,
                                               tgt::vec3 clipUrb)
        : OptimizedProxyGeometryBackgroundThread(processor, volume, tf, threshold, geometry, stepSize, false, clippingEnabled, clipLlf, clipUrb)
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

        const PreIntegrationTable* piTable = tf_->getPreIntegrationTable(1.f, 256);

        interruptionPoint();

        tgt::Bounds clipBounds(clipLlf_, clipUrb_);
        
        //get VolumeOctree representation and traverse it to create the geometry
        const VolumeOctree* octree = volume_->getRepresentation<VolumeOctree>();
        const VolumeOctreeNode* root = octree->getRootNode();
    
        tgt::svec3 octreeDim = octree->getOctreeDim();
    
        traverseOctreeAndCreateGeometry(root, tgt::vec3(0.f), tgt::vec3(octreeDim), tf_, piTable, clipBounds, volumeDim);

        interruptionPoint();

        geometry_->setTransformationMatrix(volume_->getTextureToWorldMatrix());
        processor_->setGeometryInvalid(false);
    }

    interruptionPoint();
}

void VolumeOctreeBackgroundThread::traverseOctreeAndCreateGeometry(const VolumeOctreeNode* node, const tgt::vec3& nodeLlf, const tgt::vec3& nodeUrb, TransFunc1DKeys* tf, const PreIntegrationTable* piTable, tgt::Bounds clipBounds, tgt::ivec3 volumeDim) {

    interruptionPoint();
    //TODO: set a maximum level for traversal (do not always traverse all the way down)
    if (node->isLeaf()) {
        //compute intensity values
        RealWorldMapping rwm = volume_->getRealWorldMapping();

        // transform normalized voxel intensities to real-world
        float minRealWorld = rwm.normalizedToRealWorld(node->getMinValue() / 65535.f);
        float maxRealWorld = rwm.normalizedToRealWorld(node->getMaxValue() / 65535.f);
        
        float minIntensity = tf->realWorldToNormalized(minRealWorld);
        float maxIntensity = tf->realWorldToNormalized(maxRealWorld);

        //check pre-integration heuristic
        if (isRegionEmptyPi(minIntensity, maxIntensity, piTable))
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

                traverseOctreeAndCreateGeometry(childNode, childLlf, childUrb, tf, piTable, clipBounds, volumeDim);
            }
        }
    }
}

#endif

#ifdef VRN_PROXY_DEBUG
//-------------------------------------------------------------------------------------------------
// OctreeBackgroundThread

OctreeBackgroundThread::OctreeBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume, TransFunc1DKeys* tf,
                                               float threshold, TriangleMeshGeometryVec4Vec3* geometry, ProxyGeometryOctreeNode** octreeRoot,
                                               bool checkHalfNodes, int stepSize, bool debugOutput, bool clippingEnabled, tgt::vec3 clipLlf,
                                               tgt::vec3 clipUrb)
        : OptimizedProxyGeometryBackgroundThread(processor, volume, tf, threshold, geometry, stepSize, debugOutput, clippingEnabled, clipLlf, clipUrb)
        , octreeRoot_(octreeRoot)
        , checkHalfNodes_(checkHalfNodes)
{}

OctreeBackgroundThread::~OctreeBackgroundThread() {
    //wait for internal thread to finish
    join();
}

void OctreeBackgroundThread::threadMain() {
    computeOctreeMaxBricks();
}

void OctreeBackgroundThread::computeOctreeMaxBricks() {

    interruptionPoint();

    tgt::ivec3 dim = volume_->getDimensions();

    if (processor_->octreeInvalid()) {
        delete *octreeRoot_;
        *octreeRoot_ = 0;
    }

    if (!(*octreeRoot_)) {
        computeOctreeRecursively(volume_);
        processor_->setOctreeInvalid(false);
        //invalidate geometry
        processor_->setGeometryInvalid();
    }

    if (processor_->geometryInvalid()) {
        geometry_->clear();

        if (debugOutput_) {
            stopWatch_.reset();
            stopWatch_.start();
        }

        const PreIntegrationTable* piTable;

        interruptionPoint();

        piTable = tf_->getPreIntegrationTable(1.f, 256);

        if (debugOutput_) {
            stopWatch_.stop();
            std::cout << "Fetching PreIntegration table took " << stopWatch_.getRuntime() << " milliseconds" << std::endl;
        }

        tgt::Bounds clipBounds(clipLlf_, clipUrb_);

        if (debugOutput_) {
            stopWatch_.reset();
            stopWatch_.start();
        }

        //traverse octree for the first time and set visibility for each node
        traverseOctreeAndSetVisibility(*octreeRoot_, tf_, piTable);

        interruptionPoint();

        //traverse octree for the second time and create proxy geometry
        float proxyVolume = traverseOctreeAndCreateMaxCubeGeometry(*octreeRoot_, dim, clipBounds);

        if (debugOutput_) {
            stopWatch_.stop();
            std::cout << "Traversing octree to create maximal cubes took " << stopWatch_.getRuntime() << " milliseconds" << std::endl;
            std::cout << "Created Proxy Geometry consisting of " << geometry_->getNumTriangles()
                      << " triangles using octree max cubes mode." << std::endl;
            std::cout << "Volume < " << proxyVolume << " (before clipping)" << std::endl;
        }

        interruptionPoint();

        //if mesh list is empty: add leftmost node (empty mesh geometry might lead to camera problem)
        /*if (geometry_->isEmpty() && *octreeRoot_) {
            OctreeNode* node = *octreeRoot_;
            while (!node->isLeaf_)
                node = node->llf_;
            OptimizedProxyGeometry::addCubeMesh(geometry_, node->bounds_,dim);
        }*/

        geometry_->setTransformationMatrix(volume_->getTextureToWorldMatrix());
        processor_->setGeometryInvalid(false);
    }

    interruptionPoint();
}

void OctreeBackgroundThread::traverseOctreeAndSetVisibility(ProxyGeometryOctreeNode* node, TransFunc1DKeys* tfi, const PreIntegrationTable* piTable) {
    if (!node) {
        if (debugOutput_)
            std::cout << "Encountered 0 pointer instead of valid node during octree traversal..." << std::endl;
        return;
    }

    interruptionPoint();

    //leaf: determine visibility
    if (node->isLeaf_) {
        // get min and max intensity of this region
        float minIntensity = tfi->realWorldToNormalized(node->minMaxIntensity_.x);
        float maxIntensity = tfi->realWorldToNormalized(node->minMaxIntensity_.y);

        // set opacity
        node->opacity_ = isRegionEmptyPi(minIntensity, maxIntensity, piTable) ?
            ProxyGeometryOctreeNode::TRANSPARENT_NODE : ProxyGeometryOctreeNode::OPAQUE_NODE;
        return;
    }

    //not a leaf: traverse further
    traverseOctreeAndSetVisibility(node->llf_, tfi, piTable);
    traverseOctreeAndSetVisibility(node->lrf_, tfi, piTable);
    traverseOctreeAndSetVisibility(node->ulf_, tfi, piTable);
    traverseOctreeAndSetVisibility(node->urf_, tfi, piTable);
    traverseOctreeAndSetVisibility(node->llb_, tfi, piTable);
    traverseOctreeAndSetVisibility(node->lrb_, tfi, piTable);
    traverseOctreeAndSetVisibility(node->ulb_, tfi, piTable);
    traverseOctreeAndSetVisibility(node->urb_, tfi, piTable);

    // set opacity according to the child nodes
    if ((node->llf_->opacity_ == ProxyGeometryOctreeNode::TRANSPARENT_NODE) &&
        (node->lrf_->opacity_ == ProxyGeometryOctreeNode::TRANSPARENT_NODE) &&
        (node->ulf_->opacity_ == ProxyGeometryOctreeNode::TRANSPARENT_NODE) &&
        (node->urf_->opacity_ == ProxyGeometryOctreeNode::TRANSPARENT_NODE) &&
        (node->llb_->opacity_ == ProxyGeometryOctreeNode::TRANSPARENT_NODE) &&
        (node->lrb_->opacity_ == ProxyGeometryOctreeNode::TRANSPARENT_NODE) &&
        (node->ulb_->opacity_ == ProxyGeometryOctreeNode::TRANSPARENT_NODE) &&
        (node->urb_->opacity_ == ProxyGeometryOctreeNode::TRANSPARENT_NODE))
    {
        node->opacity_ = ProxyGeometryOctreeNode::TRANSPARENT_NODE;
        if ((node->llf_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
            (node->lrf_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
            (node->ulf_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
            (node->urf_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
            (node->llb_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
            (node->lrb_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
            (node->ulb_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
            (node->urb_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE))
        {
            node->opacity_ = ProxyGeometryOctreeNode::OPAQUE_NODE;
        }
    }
    else
       node->opacity_ = ProxyGeometryOctreeNode::PARTOPAQUE_NODE;
}

float OctreeBackgroundThread::traverseOctreeAndCreateMaxCubeGeometry(ProxyGeometryOctreeNode* node, tgt::ivec3 dim, tgt::Bounds clipBounds) {

    if (!node) {

        if (debugOutput_)
            std::cout << "Encountered 0 pointer instead of valid node during octree traversal...";

        return 0.f;
    }

    interruptionPoint();

    if (node->opacity_ == ProxyGeometryOctreeNode::TRANSPARENT_NODE) {
        return 0.f;
    }
    else if (node->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) {
        if (clippingEnabled_) {
            if (clipBounds.containsVolume(node->bounds_))
                OptimizedProxyGeometry::addCubeMesh(geometry_, node->bounds_, dim);
            else if (clipBounds.intersects(node->bounds_))
                OptimizedProxyGeometry::addCubeMeshClip(geometry_, node->bounds_,dim,clipBounds);
         }
         else
            OptimizedProxyGeometry::addCubeMesh(geometry_, node->bounds_,dim);

         return node->bounds_.volume();
    }
    else {

        float proxyVolume = 0.f;

        //leaf: traversal is at end (shouldn't be happening, leaf cannot be partially opaque)
        if (node->isLeaf_)
            return 0.f;

        if (checkHalfNodes_) {
            //check if the half of this node sould be rendered
            if ((node->llf_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                (node->lrf_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                (node->ulf_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                (node->urf_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE))
            {
                //visualize front
                tgt::Bounds regionBounds = node->llf_->bounds_;
                regionBounds.addVolume(node->lrf_->bounds_);
                regionBounds.addVolume(node->ulf_->bounds_);
                regionBounds.addVolume(node->urf_->bounds_);

                if (clippingEnabled_) {
                //only add and clip mesh if necessary
                if (clipBounds.containsVolume(regionBounds))
                    OptimizedProxyGeometry::addCubeMesh(geometry_, regionBounds, dim);
                else if (clipBounds.intersects(regionBounds))
                    OptimizedProxyGeometry::addCubeMeshClip(geometry_, regionBounds,dim,clipBounds);
                }
                else
                    OptimizedProxyGeometry::addCubeMesh(geometry_, regionBounds,dim);

                proxyVolume += regionBounds.volume();

                //traverse rest
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->llb_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->lrb_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->ulb_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->urb_, dim, clipBounds);
            }
            else if ((node->llb_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                    (node->lrb_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                    (node->ulb_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                    (node->urb_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE))
            {
                //visualize back
                tgt::Bounds regionBounds = node->llb_->bounds_;
                regionBounds.addVolume(node->lrb_->bounds_);
                regionBounds.addVolume(node->ulb_->bounds_);
                regionBounds.addVolume(node->urb_->bounds_);

                if (clippingEnabled_) {
                    //only add and clip mesh if necessary
                    if (clipBounds.containsVolume(regionBounds))
                        OptimizedProxyGeometry::addCubeMesh(geometry_, regionBounds, dim);
                    else if (clipBounds.intersects(regionBounds))
                        OptimizedProxyGeometry::addCubeMeshClip(geometry_, regionBounds,dim,clipBounds);
                }
                else
                    OptimizedProxyGeometry::addCubeMesh(geometry_, regionBounds,dim);

                proxyVolume += regionBounds.volume();

                //traverse rest
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->llf_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->lrf_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->ulf_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->urf_, dim, clipBounds);
            }
            else if ((node->llf_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                     (node->lrf_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                     (node->llb_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                     (node->lrb_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE))
            {
                //visualize lower half
                tgt::Bounds regionBounds = node->llf_->bounds_;
                regionBounds.addVolume(node->lrf_->bounds_);
                regionBounds.addVolume(node->llb_->bounds_);
                regionBounds.addVolume(node->lrb_->bounds_);

                if (clippingEnabled_) {
                    //only add and clip mesh if necessary
                    if (clipBounds.containsVolume(regionBounds))
                        OptimizedProxyGeometry::addCubeMesh(geometry_, regionBounds, dim);
                    else if (clipBounds.intersects(regionBounds))
                        OptimizedProxyGeometry::addCubeMeshClip(geometry_, regionBounds,dim,clipBounds);
                }
                else
                    OptimizedProxyGeometry::addCubeMesh(geometry_, regionBounds,dim);

                proxyVolume += regionBounds.volume();

                //traverse rest
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->ulb_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->urb_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->ulf_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->urf_, dim, clipBounds);
            }
            else if ((node->ulf_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                     (node->urf_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                     (node->ulb_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                     (node->urb_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE))
            {
                //visualize upper half
                tgt::Bounds regionBounds = node->ulf_->bounds_;
                regionBounds.addVolume(node->urf_->bounds_);
                regionBounds.addVolume(node->ulb_->bounds_);
                regionBounds.addVolume(node->urb_->bounds_);

                if (clippingEnabled_) {
                    //only add and clip mesh if necessary
                    if (clipBounds.containsVolume(regionBounds))
                        OptimizedProxyGeometry::addCubeMesh(geometry_, regionBounds, dim);
                    else if (clipBounds.intersects(regionBounds))
                        OptimizedProxyGeometry::addCubeMeshClip(geometry_, regionBounds,dim,clipBounds);
                }
                else
                    OptimizedProxyGeometry::addCubeMesh(geometry_, regionBounds,dim);

                proxyVolume += regionBounds.volume();

                //traverse rest
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->llf_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->llb_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->lrf_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->lrb_, dim, clipBounds);
            }
            else if ((node->llf_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                     (node->ulf_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                     (node->llb_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                     (node->ulb_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE))
            {
                //visualize left half
                tgt::Bounds regionBounds = node->llf_->bounds_;
                regionBounds.addVolume(node->ulf_->bounds_);
                regionBounds.addVolume(node->llb_->bounds_);
                regionBounds.addVolume(node->ulb_->bounds_);

                if (clippingEnabled_) {
                    //only add and clip mesh if necessary
                    if (clipBounds.containsVolume(regionBounds))
                        OptimizedProxyGeometry::addCubeMesh(geometry_, regionBounds, dim);
                    else if (clipBounds.intersects(regionBounds))
                        OptimizedProxyGeometry::addCubeMeshClip(geometry_, regionBounds,dim,clipBounds);
                }
                else
                    OptimizedProxyGeometry::addCubeMesh(geometry_, regionBounds,dim);

                proxyVolume += regionBounds.volume();

                //traverse rest
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->lrf_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->urf_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->lrb_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->urb_, dim, clipBounds);
            }
            else if ((node->lrf_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                     (node->urf_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                     (node->lrb_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE) &&
                     (node->urb_->opacity_ == ProxyGeometryOctreeNode::OPAQUE_NODE))
            {
                //visualize right half
                tgt::Bounds regionBounds = node->lrf_->bounds_;
                regionBounds.addVolume(node->urf_->bounds_);
                regionBounds.addVolume(node->lrb_->bounds_);
                regionBounds.addVolume(node->urb_->bounds_);

                if (clippingEnabled_) {
                    //only add and clip mesh if necessary
                    if (clipBounds.containsVolume(regionBounds))
                        OptimizedProxyGeometry::addCubeMesh(geometry_, regionBounds, dim);
                    else if (clipBounds.intersects(regionBounds))
                        OptimizedProxyGeometry::addCubeMeshClip(geometry_, regionBounds,dim,clipBounds);
                }
                else
                    OptimizedProxyGeometry::addCubeMesh(geometry_, regionBounds,dim);

                proxyVolume += regionBounds.volume();

                //traverse rest
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->llf_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->llb_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->ulf_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->ulb_, dim, clipBounds);
            }
            else {
                //else: traverse further
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->llf_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->lrf_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->ulf_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->urf_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->llb_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->lrb_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->ulb_, dim, clipBounds);
                proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->urb_, dim, clipBounds);
            }
        }
        else {
            //traverse further
            proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->llf_, dim, clipBounds);
            proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->lrf_, dim, clipBounds);
            proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->ulf_, dim, clipBounds);
            proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->urf_, dim, clipBounds);
            proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->llb_, dim, clipBounds);
            proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->lrb_, dim, clipBounds);
            proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->ulb_, dim, clipBounds);
            proxyVolume += traverseOctreeAndCreateMaxCubeGeometry(node->urb_, dim, clipBounds);
        }

        return proxyVolume;
    }
}

void OctreeBackgroundThread::computeOctreeRecursively(const VolumeBase* inputVolume) {

    if (debugOutput_) {
        stopWatch_.reset();
        stopWatch_.start();
    }

    interruptionPoint();

    tgt::ivec3 dim = inputVolume->getDimensions();

    const VolumeRAM* vol = inputVolume->getRepresentation<VolumeRAM>();
    RealWorldMapping rwm = inputVolume->getRealWorldMapping();

    const tgt::ivec3 step = tgt::ivec3(stepSize_);

    //determine size for this resolution
    const tgt::ivec3 size(
            static_cast<int>(std::ceil(static_cast<float>(dim.x) / static_cast<float>(stepSize_))),
            static_cast<int>(std::ceil(static_cast<float>(dim.y) / static_cast<float>(stepSize_))),
            static_cast<int>(std::ceil(static_cast<float>(dim.z) / static_cast<float>(stepSize_))));

    interruptionPoint();

    //create root for new octree
    *octreeRoot_ = new ProxyGeometryOctreeNode();

    tgt::ivec3 pos(0);
    subdivideOctreeNodeRecursively(*octreeRoot_, pos, size, stepSize_, vol, dim, rwm);

    if (debugOutput_) {
        stopWatch_.stop();
        std::cout << "Recursive octree construction took " << stopWatch_.getRuntime() <<  " milliseconds" << std::endl;
    }
}

void OctreeBackgroundThread::subdivideOctreeNodeRecursively(ProxyGeometryOctreeNode* current, tgt::ivec3 pos, tgt::ivec3 size, int stepSize,
                                                            const VolumeRAM* vol, tgt::ivec3 dim, RealWorldMapping rwm)
{
    if (!current) {

        if (debugOutput_)
            std::cout << "Encounter node 0 during octree creation..." << std::endl;

        return;
    }

    interruptionPoint();

    //if there is no further subdividing possible: make current node leaf and compute min and max values as well as bounds
    if ((size.x == 1) || (size.y == 1) || (size.z == 1)) {
        current->isLeaf_ = true;
        tgt::ivec3 llf = pos * stepSize;
        tgt::ivec3 urb = (pos + size) * stepSize;

        //since a voxel is the center, add/subtract 0.5 to/from the coordinates to get the bounding box
        current->bounds_ = tgt::Bounds(tgt::vec3(llf) - tgt::vec3(0.5f), tgt::vec3(urb) + tgt::vec3(0.5f));

        // find min and max intensities
        float minIntensity = std::numeric_limits<float>::max();
        float maxIntensity = std::numeric_limits<float>::min();
        llf = tgt::max(llf - tgt::ivec3(1),tgt::ivec3(0));
        llf = tgt::min(llf,dim - 1);
        urb = tgt::max(urb + tgt::ivec3(1),tgt::ivec3(0));
        urb = tgt::min(urb,dim - 1);

        interruptionPoint();

        //don't use macro because of interruption points within the loops...
        //VRN_FOR_EACH_VOXEL(pos, llf, urb) {
        for (pos = llf; pos.z < urb.z; ++pos.z) {
            //interruption point after each slice
            interruptionPoint();

            for (pos.y = llf.y; pos.y < urb.y; ++pos.y) {
                for (pos.x = llf.x; pos.x < urb.x; ++pos.x) {
                    float currentIntensity = vol->getVoxelNormalized(pos);
                    //apply realworld mapping
                    currentIntensity = rwm.normalizedToRealWorld(currentIntensity);
                    minIntensity = std::min(minIntensity,currentIntensity);
                    maxIntensity = std::max(maxIntensity,currentIntensity);
                }
            }
        }

        current->minMaxIntensity_ = tgt::vec2(minIntensity, maxIntensity);
    }
    else {
        //subdivide the current node
        tgt::ivec3 sizeLlf = size / 2;
        tgt::ivec3 sizeUrb = size - sizeLlf;
        tgt::ivec3 posUrb = pos + sizeLlf;

        interruptionPoint();

        //recursively compute children
        current->llf_ = new ProxyGeometryOctreeNode();
        subdivideOctreeNodeRecursively(current->llf_, pos, sizeLlf, stepSize, vol, dim, rwm);

        current->lrf_ = new ProxyGeometryOctreeNode();
        subdivideOctreeNodeRecursively(current->lrf_, tgt::ivec3(posUrb.x, pos.y, pos.z),
            tgt::ivec3(sizeUrb.x, sizeLlf.y, sizeLlf.z), stepSize, vol, dim, rwm);

        current->ulf_ = new ProxyGeometryOctreeNode();
        subdivideOctreeNodeRecursively(current->ulf_, tgt::ivec3(pos.x, posUrb.y, pos.z),
            tgt::ivec3(sizeLlf.x, sizeUrb.y, sizeLlf.z), stepSize, vol, dim, rwm);

        current->urf_ = new ProxyGeometryOctreeNode();
        subdivideOctreeNodeRecursively(current->urf_, tgt::ivec3(posUrb.x, posUrb.y, pos.z),
            tgt::ivec3(sizeUrb.x, sizeUrb.y, sizeLlf.z), stepSize, vol, dim, rwm);

        current->llb_ = new ProxyGeometryOctreeNode();
        subdivideOctreeNodeRecursively(current->llb_, tgt::ivec3(pos.x, pos.y, posUrb.z),
            tgt::ivec3(sizeLlf.x, sizeLlf.y, sizeUrb.z), stepSize, vol, dim, rwm);

        current->lrb_ = new ProxyGeometryOctreeNode();
        subdivideOctreeNodeRecursively(current->lrb_, tgt::ivec3(posUrb.x, pos.y, posUrb.z),
            tgt::ivec3(sizeUrb.x, sizeLlf.y, sizeUrb.z), stepSize, vol, dim, rwm);

        current->ulb_ = new ProxyGeometryOctreeNode();
        subdivideOctreeNodeRecursively(current->ulb_, tgt::ivec3(pos.x, posUrb.y, posUrb.z),
            tgt::ivec3(sizeLlf.x, sizeUrb.y, sizeUrb.z), stepSize, vol, dim, rwm);

        current->urb_ = new ProxyGeometryOctreeNode();
        subdivideOctreeNodeRecursively(current->urb_, posUrb, sizeUrb, stepSize, vol, dim, rwm);

        interruptionPoint();

        //compute bounds and min/max from child nodes, set parent to child nodes
        current->llf_->parent_ = current;
        current->lrf_->parent_ = current;
        current->ulf_->parent_ = current;
        current->urf_->parent_ = current;
        current->llb_->parent_ = current;
        current->lrb_->parent_ = current;
        current->ulb_->parent_ = current;
        current->urb_->parent_ = current;

        interruptionPoint();

        current->bounds_ = current->llf_->bounds_;
        current->bounds_.addVolume(current->lrf_->bounds_);
        current->bounds_.addVolume(current->ulf_->bounds_);
        current->bounds_.addVolume(current->urf_->bounds_);
        current->bounds_.addVolume(current->llb_->bounds_);
        current->bounds_.addVolume(current->lrb_->bounds_);
        current->bounds_.addVolume(current->ulb_->bounds_);
        current->bounds_.addVolume(current->urb_->bounds_);

        interruptionPoint();

        float min, max;
        min = current->llf_->minMaxIntensity_.x;
        max = current->llf_->minMaxIntensity_.y;
        min = std::min(min, current->lrf_->minMaxIntensity_.x);
        max = std::max(max, current->lrf_->minMaxIntensity_.y);
        min = std::min(min, current->ulf_->minMaxIntensity_.x);
        max = std::max(max, current->ulf_->minMaxIntensity_.y);
        min = std::min(min, current->urf_->minMaxIntensity_.x);
        max = std::max(max, current->urf_->minMaxIntensity_.y);
        min = std::min(min, current->llb_->minMaxIntensity_.x);
        max = std::max(max, current->llb_->minMaxIntensity_.y);
        min = std::min(min, current->lrb_->minMaxIntensity_.x);
        max = std::max(max, current->lrb_->minMaxIntensity_.y);
        min = std::min(min, current->ulb_->minMaxIntensity_.x);
        max = std::max(max, current->ulb_->minMaxIntensity_.y);
        min = std::min(min, current->urb_->minMaxIntensity_.x);
        max = std::max(max, current->urb_->minMaxIntensity_.y);

        current->minMaxIntensity_ = tgt::vec2(min, max);
    }
}
#endif

} // namespace
