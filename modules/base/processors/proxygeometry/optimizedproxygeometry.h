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

#ifndef VRN_OPTIMIZEDPROXYGEOMETRY_H
#define VRN_OPTIMIZEDPROXYGEOMETRY_H

//define for debug mode: debugging output and some more modes
//#define VRN_PROXY_DEBUG

#include "voreen/core/processors/processor.h"

#include "voreen/core/ports/volumeport.h"
#include "voreen/core/ports/geometryport.h"

#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/buttonproperty.h"

#include "voreen/core/datastructures/transfunc/preintegrationtable.h"
#include "voreen/core/datastructures/transfunc/transfunc2dprimitives.h"
#include "voreen/core/properties/transfuncproperty.h"

#include "voreen/core/datastructures/volume/volume.h"

#include "tgt/stopwatch.h"

#include "voreen/core/utils/backgroundthread.h"

#ifdef VRN_MODULE_STAGING
#include "../../../staging/octree/datastructures/volumeoctree.h"
#include "modules/staging/octree/datastructures/octreeutils.h"
#endif

namespace voreen {

class TriangleMeshGeometryVec4Vec3;

/**
 * A volume region (brick) used for spatial subdivision.
 */
class ProxyGeometryVolumeRegion {
public:
    ProxyGeometryVolumeRegion(tgt::Bounds bounds, tgt::vec2 minMaxIntensity) {
        bounds_ = bounds;
        minMaxIntensity_ = minMaxIntensity;
        isBound_ = false;
    }

    tgt::Bounds getBounds() const { return bounds_; }
    float getMinIntensity() const { return minMaxIntensity_.x; }
    float getMaxIntensity() const { return minMaxIntensity_.y; }
    bool isBound() const { return isBound_;}
    void setBound(bool value) {isBound_ = value;}
    bool isOpaque() const { return isOpaque_;}
    void setOpaque(bool value) {isOpaque_ = value;}

private:
    tgt::Bounds bounds_;        ///< bounding box in voxel coordinates
    tgt::vec2 minMaxIntensity_; ///< contains min and max intensity values within the bounding box
    bool isBound_;              ///< is this region already bound by a maximal cube
    bool isOpaque_;             ///< is this region opaque? (depends on TF)
};

#ifdef VRN_PROXY_DEBUG
//-----------------------------------------------------------------------------

/**
 * Node of an octree data structure for spatial subdivision.
 */
struct ProxyGeometryOctreeNode {

    /// Enum for octree node visibility
    enum NodeOpacity {
        OPAQUE_NODE,
        TRANSPARENT_NODE,
        PARTOPAQUE_NODE
    };

    tgt::Bounds bounds_;        ///< bounding box in voxel coordinates
    tgt::vec2 minMaxIntensity_; ///< contains min and max intensity values within the bounding box
    bool isLeaf_;               ///< true if this node is a leaf node in the octree
    NodeOpacity opacity_;       ///< is this node completely opaque, transparent, or do the chil nodes differ in their opacity?

    ProxyGeometryOctreeNode* parent_;
    ProxyGeometryOctreeNode* llf_;
    ProxyGeometryOctreeNode* lrf_;
    ProxyGeometryOctreeNode* ulf_;
    ProxyGeometryOctreeNode* urf_;
    ProxyGeometryOctreeNode* llb_;
    ProxyGeometryOctreeNode* lrb_;
    ProxyGeometryOctreeNode* ulb_;
    ProxyGeometryOctreeNode* urb_;

    ProxyGeometryOctreeNode() : parent_(0), llf_(0), lrf_(0), ulf_(0), urf_(0), llb_(0), lrb_(0), ulb_(0), urb_(0), isLeaf_(false) {}
    ~ProxyGeometryOctreeNode() { delete llf_; delete lrf_; delete ulf_; delete urf_; delete llb_; delete lrb_; delete ulb_; delete urb_; }

};
#endif

//-------------------------------------------------------------------------------------------------

/**
 * Generates an optimized proxy geometry, allowing for empty-space leaping.
 *
 * @see EntryExitPoints
 * @see MeshClipping
 */
class OptimizedProxyGeometry : public Processor, public VolumeObserver {

    friend class OptimizedProxyGeometryBackgroundThread;
    friend class StructureProxyGeometryBackgroundThread;
    friend class MinCubeBackgroundThread;
    friend class VisibleBricksBackgroundThread;
#ifdef VRN_PROXY_DEBUG
    friend class OctreeBackgroundThread;
#endif
    friend class OuterFacesBackgroundThread;
#ifdef VRN_MODULE_STAGING
    friend class VolumeOctreeBackgroundThread;
#endif

public:
    OptimizedProxyGeometry();
    virtual ~OptimizedProxyGeometry();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "OptimizedProxyGeometry"; }
    virtual std::string getCategory() const  { return "Volume Proxy Geometry";  }
    virtual CodeState getCodeState() const   { return CODE_STATE_TESTING;       }

    virtual void volumeDelete(const VolumeBase* source);
    virtual void volumeChange(const VolumeBase* source);

protected:

    virtual void setDescriptions() {
        setDescription("Generates an optimized proxy geometry, allowing for empty-space leaping."
                       "<p>See also: <a href=\"http://voreen.uni-muenster.de/?q=optimized-proxy-geometry\">http://voreen.uni-muenster.de/?q=optimized-proxy-geometry</a></p>"
                       "<b>Note</b>: The optimization uses the transfer function to determine the currently visible parts of the volume. "
                       "Therefore, the transfer function has to be linked with the volume renderer that uses the generated proxy geometry!");

        //Modes, pre-integration heuristic
        mode_.setDescription("Selects how the proxy geometry is built. There are four modes available: \
                <ul> \
                <li> <b> Bounding Box </b><br> \
                This mode simply uses a cube containing the whole volume. No empty space leaping or optimization is applied here. </li> \
                <li> <b> Minimal Visible Bounding Box </b><br> \
                This mode subdivides the volume into several bricks, where the size of the bricks is adjustable. Afterwards an approximate minimal bounding box is computed by adding all the bricks that are not entirely transparent to an initially empty bounding box. Transparency is determined by a pre-integration heuristic that compares the opacity of the pre-integration table entry with the minimum and maximum intensity values within the brick to a threshold as described by [Knoll et al., 2011].</li> \
                <li> <b> Visible Bricks </b><br> \
                This mode subdivides the volume into several bricks, where the size of the bricks is adjustable. Afterwards the visibility of the data within the bricks is determined (ie. some bricks are marked as transparent regions by the pre-integration heuristic described above). Then a proxy geometry is built by combining neighboring bricks to larger boxes. </li>  \
                <li> <b> Visible Bricks (Outer Faces) </b><br> \
                This mode subdivides the volume into several bricks, where the size of the bricks is adjustable. Afterwards the visibility of the data within the bricks is determined (ie. some bricks are marked as transparent regions by the pre-integration heuristic described above). Then a proxy geometry is built by examining every non-transparent brick and adding each of its faces that has a transparent brick as its direct neighbor.  </li>  \
                <li> <b> Volume Octree </b><br> \
                This mode recursively traverses an octree representation of the volume (which is constructed by an OctreeCreator processor in the network) and builds a proxy geometry by determining the visibility of the leaf nodes by the pre-integration heuristic described above. If no octree representation of the volume is available, the visible bricks mode is used.</li></ul> \
               Note that the data structures that are built for the second, third and fourth mode only have to be newly computed once the volume or the resolution (ie. the size of the bricks) changes. The proxy geometry computed by these modes is built by a background thread. During background computation, a cube proxy geometry is temporarily used.");
        transfunc_.setDescription("The transfer function that is used to identify transparent regions within the volume.");
        resolutionMode_.setDescription("Determines how the resolution for subdividing the volume into bricks is set, either by subdivisions of the shortest edge of the volume or by the edge length of the bricks in voxels.");
        resolution_.setDescription("Determines the size of the bricks by dividing the size of the shortest edge of the volume.");
        resolutionVoxels_.setDescription("Determines the edge length of the bricks in voxels.");
        threshold_.setDescription("Determines the threshold for classifying bricks as empty (multiplied by 10^(-4)). Increasing this value might lead to better performance, but choosing the visibility threshold too high might remove features of the data set that should be visible.");
        //checkHalfNodes_.setDescription("When selected, octree traversal checks if half of the children of a partially opaque octree node (ie. upper/lower, right/left, back/front children) are all opaque and may therefore be rendered directly instead of traversing further (might be faster).");
        enableClipping_.setDescription("Selects if axis-aligned clipping is enabled for the proxy geometry.");
        waitForOptimization_.setDescription("If selected, no temporary cube is used while computing the optimized proxy geometry, but the processor waits for the computation to finish. Does not have an effect in Cube mode.");
    }

    virtual void process();

    bool structureInvalid() const;  ///< is the volume region structure invalid?
#ifdef VRN_PROXY_DEBUG
    bool octreeInvalid() const;     ///< is the octree invalid?
#endif
    bool geometryInvalid() const;   ///< is the mesh list geometry invalid?

    void setStructureInvalid(bool value = true);
#ifdef VRN_PROXY_DEBUG
    void setOctreeInvalid(bool value = true);
#endif
    void setGeometryInvalid(bool value = true);

    bool volumeHasChanged() const;
    void setVolumeHasChanged(bool value = true);

    void setVolStructureSize(tgt::ivec3 size);

    /// creates a cube mesh given the llf and urb in voxel coordinates
    static void addCubeMesh(TriangleMeshGeometryVec4Vec3* mesh, tgt::Bounds bounds, tgt::ivec3 dim);

    /// analogously to addCubeMesh, but takes into account the clipping parameters
    static void addCubeMeshClip(TriangleMeshGeometryVec4Vec3* mesh, tgt::Bounds bounds, tgt::ivec3 dim, tgt::Bounds clipBounds);

private:

    /// is called from process() to create a cube proxy geometry without optimization
    void processCube();

    /// while a background thread is computing the proxy geometry, this sets a temporary cube geometry
    void processTmpCube();

    /// Ensure useful clipping planes on clip right property change.
    void onClipRightChange();

    /// Ensure useful clipping planes on clip left property change.
    void onClipLeftChange();

    /// Ensure useful clipping planes on clip front property change.
    void onClipFrontChange();

    /// Ensure useful clipping planes on clip back property change.
    void onClipBackChange();

    /// Ensure useful clipping planes on clip bottom property change.
    void onClipBottomChange();

    /// Ensure useful clipping planes on clip top property change.
    void onClipTopChange();

    /// Resets clipping plane parameters to extremal values.
    void resetClipPlanes();

    /// Adapt ranges of clip plane properties to the input volume's dimensions.
    void adjustClipPropertiesRanges();

    /// Adjust the resolution property ranges according to the volume dimensions
    void adjustResolutionPropertyRanges();

    /// Adjust visibility of the clipping plane properties according to enableClipping_ property.
    void adjustClipPropertiesVisibility();

    /// Adjust visibility of the properties according to the mode
    void updatePropertyVisibility();

    /// Extracts ROI from volume (if present) and adjusts clipping slider accordingly.
    void adjustClippingToVolumeROI();

    virtual void onThresholdChange();           ///< called when threshold_ has changed
    virtual void onVolumeChange();              ///< called when inport_ has changed
    virtual void onModeChange();                ///< called when mode has been changed
    virtual void onTransFuncChange();           ///< called when transfer function has been changed
    virtual void onResolutionChange();          ///< called when resolution has been changed
    virtual void onResolutionVoxelChange();     ///< called when the resolution in voxels has been changed
    virtual void onResolutionModeChange();      ///< changes resolution_ property according to the mode

#ifdef VRN_PROXY_DEBUG

    virtual void onCheckHalfNodesChange();

    /// is called from process() to create a minimal cube proxy geometry using the brute force approach
    void processBruteMinCube();

    /// is called from process() to subdivide the volume into several cube regions using the brute force approach
    void processBruteForceCubes();

    /// is called from process() to subdivide the volume into several cube regions using a VolumeRegion datastructure
    void processCubes();

    /// uses an octree datastructure that is traversed to create the proxy geometry
    void processOctree();

    /// maximal cubes, but transparent regions that are enclosed by opaque regions are regarded as being opaque
    void processMaximalCubesSetEnclosedOpaque();

    /// this recursively computes an octree structure for the current volume:A
    void computeOctreeRecursively(const VolumeBase* inputVolume);

    /// subdivides an octree node recursively until no subdivision is possible anymore
    void subdivideOctreeNodeRecursively(ProxyGeometryOctreeNode* current, tgt::ivec3 pos, tgt::ivec3 size, int stepSize,
        const VolumeRAM* vol, tgt::ivec3 dim, RealWorldMapping rwm);

    /// This heuristically checks if a region of the volume can be treated as empty by testing the opacity
    /// of the pre-integration table (called with the min and max intensity of the region) to threshold_*1e-3
    bool isRegionEmptyPi(float min, float max, const PreIntegrationTable* piTable) const;

    ProxyGeometryVolumeRegion* getVolumeRegion(tgt::ivec3 pos);

    /// This method checks, if the part of the volume can be treated as empty by comparing the alpha values of all the voxels to the threshold
    bool isRegionEmpty(const VolumeRAM* vol, tgt::ivec3 llb, tgt::ivec3 urb, const tgt::Texture* tfTexture, float threshold = 0.f);

    /// get the urb of a volume region by adding its non-empty (determined by isRegionEmptyPi method)
    /// and not already bound neighbor regions, constructing a maximal cube
    tgt::ivec3 getUrbPi(tgt::ivec3 llf, TransFunc1DKeys* tfi, const PreIntegrationTable* piTable);

    /// get the urb of a volume region by adding its opaque and not already bound neighbor regions, constructing a maximal cube
    tgt::ivec3 getUrbOpaque(tgt::ivec3 llf);

    ///set volume regions attribute bound_
    void setVolBound(tgt::ivec3 llf, tgt::ivec3 urb, bool value = true);

    ///check if the volume regions are opaque and not bound
    bool isVolOpaqueNotBound(tgt::ivec3 llf, tgt::ivec3 urb);

    ///check if one volume region is opaque and not bound
    bool isVolOpaqueNotBound(tgt::ivec3 pos);

    ///check if the volume regions are not empty (according to the pre-integration heuristic) and not bound
    bool isVolNotEmptyPiNotBound(tgt::ivec3 llf, tgt::ivec3 urb, TransFunc1DKeys* tfi, const PreIntegrationTable* piTable);

    ///check if one volume region is not empty (according to the pre-integration heuristic) and not bound
    bool isVolNotEmptyPiNotBound(tgt::ivec3 pos, TransFunc1DKeys* tfi, const PreIntegrationTable* piTable);

    /// this computes the subdivision of the volume into several volume regions (stored in volumeStructure_)
    void computeRegionStructure(const VolumeBase* inputVolume);

    /// traverses the octree structure and creates the geometry, determining transparent regions
    /// according to the Pre-Integration heuristic, returns the (approximative) volume of the created proxy geometry
    float traverseOctreeAndCreateGeometry(ProxyGeometryOctreeNode* node, TransFunc1DKeys* tfi, const PreIntegrationTable* piTable,
        tgt::ivec3 dim, tgt::Bounds clipBounds);

    /// helper method that looks for enclosed regions and marks them as being opaque
    void setEnclosedOpaque();

    /// helper method that checks if a region is enclosed by opaque regions
    bool isEnclosed(tgt::ivec3 pos);

    virtual void onSetEnclosedOpaqueChange();   ///< Called when setEnclosedOpaque_ has changed
#endif

    VolumePort inport_;                     ///< Inport for the dataset the proxy is generated for
    GeometryPort outport_;                  ///< Outport for the generated proxy geometry

    StringOptionProperty mode_;             ///< Mode for building the proxy geometry
    TransFuncProperty transfunc_;           ///< Transfer function for determining opacity of regions
    StringOptionProperty resolutionMode_;   ///< Determines, if resolution_ sets a) the number of subdivisions of the shortest side, or b) the edge length of the bricks in voxels
    IntProperty resolution_;                ///< determines often the shortest edge of the volume should be subdivided to compute the edge length of the bricks
    IntProperty resolutionVoxels_;          ///< determines the edge length of the bricks in voxels

    IntProperty threshold_;                 ///< determine the threshold for classifying regions as empty (used with pre-integration heuristic)

    BoolProperty enableClipping_;           ///< Clipping enable / disable property.
    FloatProperty clipRight_;               ///< Right clipping plane position (x).
    FloatProperty clipLeft_;                ///< Left clipping plane position (-x).
    FloatProperty clipFront_;               ///< Front clipping plane position (y).
    FloatProperty clipBack_;                ///< Back clipping plane position property (-y).
    FloatProperty clipBottom_;              ///< Bottom clipping plane position property (z).
    FloatProperty clipTop_;                 ///< Top clipping plane position property (-z).
    ButtonProperty resetClipPlanes_;        ///< Reset clipping plane parameters to extremal values.
    BoolProperty waitForOptimization_;      ///< if set, no temporary cube is used while computing the optimized proxy geometry, but the processor waits for the computation to finish

    tgt::ivec3 oldVolumeDimensions_;

    bool structureInvalid_;  ///< determines if the spatial subdivision of the volume has to be updated (e.g. volume or resolution changed)
#ifdef VRN_PROXY_DEBUG
    bool octreeInvalid_;     ///< flag is set to indicate that the octree has to be updated
#endif
    bool geometryInvalid_;   ///< determines if the mesh geometry has to be updated, e.g. because of a transfer function change

    bool volumeHasChanged_;  ///< is set to true if the input volume changes or has been deleted

    /// Proxy geometry generated by this processor.
    TriangleMeshGeometryVec4Vec3* geometry_;

    TriangleMeshGeometryVec4Vec3* tmpGeometry_; ///< temporary geometry that is used while background thread computations are not finished yet

    /// data structure for spatial subdivision
    std::vector<ProxyGeometryVolumeRegion> volumeStructure_;

    /// size of VolumeRegion structure (ie. number of bricks in every direction)
    tgt::ivec3 volStructureSize_;
#ifdef VRN_PROXY_DEBUG
    /// the root of the octree structure
    ProxyGeometryOctreeNode* octreeRoot_;
#endif
    /// Category used for logging.
    static const std::string loggerCat_;

    ProcessorBackgroundThread<OptimizedProxyGeometry>* backgroundThread_; ///< background thread for proxy geometry computations

    TransFunc* tfCopy_;             ///< copy of the TF for background thread computations

#ifdef VRN_PROXY_DEBUG
    BoolProperty setEnclosedOpaque_; ///< determines if transparent regions should be treated as opaque if they are enclosed by opaque regions (only relevant for maximal cubes)

    tgt::Stopwatch stopWatch_;       ///< stop watch for determining time for datastructure construction (e.g. octree)

    BoolProperty debugOutput_;       ///< if selected, background threads use std::cout to print out information that might be useful for debugging purposes

    BoolProperty checkHalfNodes_;    ///< if selected, octree traversal will test if half of the children of a partially opaque inner node (ie. upper/lower children, left/right children, front/back children) are all opaque and can therefore be rendered as one brick without further traversal

#endif

}; // OptimizedProxyGeometry


//-------------------------------------------------------------------------------------------------
// Background threads performing the actual construction of the proxy geometry

/**
 * Abstract class derived from ProcessorBackgroundThread used as base class for proxy geometry background computations.
 */
class OptimizedProxyGeometryBackgroundThread : public ProcessorBackgroundThread<OptimizedProxyGeometry> {

public:
    OptimizedProxyGeometryBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume,
        TransFunc1DKeys* tf, float threshold, TriangleMeshGeometryVec4Vec3* geometry, int stepSize,
        bool debugOutput, bool clippingEnabled, tgt::vec3 clipLlf = tgt::vec3(0.f), tgt::vec3 clipUrb = tgt::vec3(1.f));

protected:
    virtual void handleInterruption();

    /**
     * This heuristically checks if a region of the volume can be treated as empty by comparing the opacity of the
     * pre-integration table (called with the min and max intensity of the region) to threshold_*1e-3
     */
    bool isRegionEmptyPi(float min, float max, const PreIntegrationTable* piTable) const;

    const VolumeBase* volume_;      ///< the volume for which the proxy geometry should be computed

    TransFunc1DKeys* tf_;           ///< transfer function for identifying transparent regions

    float threshold_;               ///< threshold for the pre-integration heuristic (used by method isRegionEmptyPi)

    TriangleMeshGeometryVec4Vec3* geometry_;    ///< geometry of the processor

    int stepSize_;                  ///< step size (in voxels) for subdividing the volume into bricks

    bool clippingEnabled_;          ///< use axis-aligned clipping?

    tgt::vec3 clipLlf_;             ///< clipping region boundaries (in voxel space)
    tgt::vec3 clipUrb_;             ///< clipping region boundaries (in voxel space)

    bool debugOutput_;              ///< print out debug information?
    tgt::Stopwatch stopWatch_;      ///< for determining the building time of the data structures
};

//-------------------------------------------------------------------------------------------------

/**
 * Abstract class derived from ProxyGeometryBackgroundThread, used as base class for proxy geometry
 * background computations that use the volume region structure.
 */
class StructureProxyGeometryBackgroundThread : public OptimizedProxyGeometryBackgroundThread {

public:
    StructureProxyGeometryBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume,
        TransFunc1DKeys* tf, float threshold, TriangleMeshGeometryVec4Vec3* geometry, std::vector<ProxyGeometryVolumeRegion>* volumeStructure,
        tgt::ivec3 volStructureSize, int stepSize, bool debugOutput, bool clippingEnabled,
        tgt::vec3 clipLlf = tgt::vec3(0.f), tgt::vec3 clipUrb = tgt::vec3(1.f));

protected:
    void computeRegionStructure();               ///< determine the volume region data structurei

    ///get the volume region at a given position within the structure
    ProxyGeometryVolumeRegion& getVolumeRegion(tgt::ivec3 pos);

    std::vector<ProxyGeometryVolumeRegion>* volumeStructure_; ///< data structure for spatial subdivision

    tgt::ivec3 volStructureSize_;                ///< size of the spatial subdivision (ie. number of regions in every direction)
};

//-------------------------------------------------------------------------------------------------

/**
 * Background thread computing an approximate minimal bounding box as a proxy geometry
 */
class MinCubeBackgroundThread : public StructureProxyGeometryBackgroundThread {

public:
    MinCubeBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume,
        TransFunc1DKeys* tf, float threshold, TriangleMeshGeometryVec4Vec3* geometry, std::vector<ProxyGeometryVolumeRegion>* volumeStructure,
        tgt::ivec3 volStructureSize, int stepSize, bool debugOutput, bool clippingEnabled,
        tgt::vec3 clipLlf = tgt::vec3(0.f), tgt::vec3 clipUrb = tgt::vec3(1.f));

    ~MinCubeBackgroundThread();

protected:
    void threadMain();
    void computeMinCube();

};

//-------------------------------------------------------------------------------------------------

/**
 * Background thread computing a proxy geometry of (maximal) bricks using the volume region structure.
 */
class VisibleBricksBackgroundThread : public StructureProxyGeometryBackgroundThread {

public:

    VisibleBricksBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume, TransFunc1DKeys* tf,
        float threshold, TriangleMeshGeometryVec4Vec3* geometry, std::vector<ProxyGeometryVolumeRegion>* volumeStructure,
        tgt::ivec3 volStructureSize, int stepSize, bool debugOutput, bool clippingEnabled,
        tgt::vec3 clipLlf = tgt::vec3(0.f), tgt::vec3 clipUrb = tgt::vec3(1.f));

    ~VisibleBricksBackgroundThread();

protected:

    void threadMain();

    void computeMaximalBricks();

    ///set volume regions attribute bound_
    void setVolBound(tgt::ivec3 llf, tgt::ivec3 urb, bool value = true);

    ///check if one volume region is not empty (according to the pre-integration heuristic) and not bound
    bool isVolNotEmptyPiNotBound(tgt::ivec3 pos, TransFunc1DKeys* tfi,  const PreIntegrationTable* piTable);

    ///check if the volume regions are not empty (according to the pre-integration heuristic) and not bound
    bool isVolNotEmptyPiNotBound(tgt::ivec3 llf, tgt::ivec3 urb, TransFunc1DKeys* tfi, const PreIntegrationTable* piTable);

    /// get the urb of a volume region by adding its non-empty (determinded by isRegionEmptyPi method)
    /// and not already bound neighbor regions, constructing a maximal cube
    tgt::ivec3 getUrbPi(tgt::ivec3 llf, TransFunc1DKeys* tfi, const PreIntegrationTable* piTable);

};

//-------------------------------------------------------------------------------------------------

/**
 * Background thread computing a proxy geometry of the outer faces of opaque neighbors in the volume region structure
 */
class OuterFacesBackgroundThread : public StructureProxyGeometryBackgroundThread {

public:

    OuterFacesBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume, TransFunc1DKeys* tf,
        float threshold, TriangleMeshGeometryVec4Vec3* geometry, std::vector<ProxyGeometryVolumeRegion>* volumeStructure,
        tgt::ivec3 volStructureSize, int stepSize, bool debugOutput, bool clippingEnabled,
        tgt::vec3 clipLlf = tgt::vec3(0.f), tgt::vec3 clipUrb = tgt::vec3(1.f));

    ~OuterFacesBackgroundThread();

protected:

    void threadMain();

    void computeOuterFaces();

    ///set volume regions attribute bound_
    //void setVolBound(tgt::ivec3 llf, tgt::ivec3 urb, bool value = true);

    ///check if one volume region is not empty (according to the pre-integration heuristic) and not bound
    //bool isVolNotEmptyPiNotBound(tgt::ivec3 pos, TransFunc1DKeys* tfi,  const PreIntegrationTable* piTable);

    ///check if the volume regions are not empty (according to the pre-integration heuristic) and not bound
    //bool isVolNotEmptyPiNotBound(tgt::ivec3 llf, tgt::ivec3 urb, TransFunc1DKeys* tfi, const PreIntegrationTable* piTable);

    /// get the urb of a volume region by adding its non-empty (determinded by isRegionEmptyPi method)
    /// and not already bound neighbor regions, constructing a maximal cube
    //tgt::ivec3 getUrbPi(tgt::ivec3 llf, TransFunc1DKeys* tfi, const PreIntegrationTable* piTable);

};
//-------------------------------------------------------------------------------------------------

#ifdef VRN_MODULE_STAGING
/**
 * Background thread computing a proxy geomtry using the VolumeOctree volume representation (if available).
 * If this representation does not exist, the visible bricks mode is used as a fallback mode.
 */
class VolumeOctreeBackgroundThread : public OptimizedProxyGeometryBackgroundThread {

public:

    VolumeOctreeBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume, TransFunc1DKeys* tf,
        float threshold, TriangleMeshGeometryVec4Vec3* geometry, int stepSize, bool clippingEnabled,
        tgt::vec3 clipLlf = tgt::vec3(0.f), tgt::vec3 clipUrb = tgt::vec3(1.f));

    ~VolumeOctreeBackgroundThread();

protected:

    /// compute an octree data structure and traverse it to create the proxy geometry
    void threadMain();

    /// uses an octree to create a proxygeometry of maximal cubes
    void computeVolumeOctreeGeometry();

    /// recursively traverses the octree and creates the geometry
    void traverseOctreeAndCreateGeometry(const VolumeOctreeNode* node, const tgt::vec3& nodeLlf, const tgt::vec3& nodeUrb, TransFunc1DKeys* tf, const PreIntegrationTable* piTable, tgt::Bounds clipBounds, tgt::ivec3 volumeDim);

};
#endif


#ifdef VRN_PROXY_DEBUG
//-------------------------------------------------------------------------------------------------

/**
 * Background thread computing a proxy geometry using an octree data structure for spatial subdivision
 */
class OctreeBackgroundThread : public OptimizedProxyGeometryBackgroundThread {

public:

    OctreeBackgroundThread(OptimizedProxyGeometry* processor, const VolumeBase* volume, TransFunc1DKeys* tf,
        float threshold, TriangleMeshGeometryVec4Vec3* geometry, ProxyGeometryOctreeNode** octreeRoot,
        bool checkHalfNodes, int stepSize, bool debugOutput, bool clippingEnabled,
        tgt::vec3 clipLlf = tgt::vec3(0.f), tgt::vec3 clipUrb = tgt::vec3(1.f));

    ~OctreeBackgroundThread();

protected:

    /// compute an octree data structure and traverse it to create the proxy geometry
    void threadMain();

    /// uses an octree to create a proxygeometry of maximal cubes
    void computeOctreeMaxBricks();

    /// recursively determines opacity of each node by using pre-integration heuristic
    void traverseOctreeAndSetVisibility(ProxyGeometryOctreeNode* node, TransFunc1DKeys* tfi, const PreIntegrationTable* piTable);

    /// recursively traverses the octree structure and creates geometry, depending on the opacity that has been set for each node,
    /// returns the (approximative) volume of the created proxy geometry
    float traverseOctreeAndCreateMaxCubeGeometry(ProxyGeometryOctreeNode* node, tgt::ivec3 dim, tgt::Bounds clipBounds);

    /// this recursively computes an octree structure for the current volume:A
    void computeOctreeRecursively(const VolumeBase* inputVolume);

    /// subdivides an octree node recursively until no subdivision is possible anymore
    void subdivideOctreeNodeRecursively(ProxyGeometryOctreeNode* current, tgt::ivec3 pos, tgt::ivec3 size, int stepSize,
        const VolumeRAM* vol, tgt::ivec3 dim, RealWorldMapping rwm);

private:
    ProxyGeometryOctreeNode** octreeRoot_;

    /// If true, during traversal it is checked if half of a node (in one direction) is opaque
    /// and the other half is transparent. The opaque part is then directly rendered without further traversal
    bool checkHalfNodes_;
};
#endif

} // namespace

#endif // VRN_OPTIMIZEDPROXYGEOMETRY_H
