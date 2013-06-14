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

#ifndef VRN_SINGLEOCTREERAYCASTERCPU_H
#define VRN_SINGLEOCTREERAYCASTERCPU_H

#include "voreen/core/processors/renderprocessor.h"

#include "voreen/core/ports/volumeport.h"
#include "voreen/core/ports/geometryport.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/transfuncproperty.h"

#include "voreen/core/datastructures/meta/realworldmappingmetadata.h"

#include "modules/staging/octree/datastructures/volumeoctreeport.h"

namespace voreen {

class VolumeOctreeBase;
class VolumeOctreeNode;
class MeshListGeometry;
class MeshGeometry;
class TransFunc1DKeys;

class VRN_CORE_API SingleOctreeRaycasterCPU : public RenderProcessor {
public:
    SingleOctreeRaycasterCPU();
    virtual ~SingleOctreeRaycasterCPU();
    virtual Processor* create() const;

    virtual std::string getClassName() const  { return "SingleOctreeRaycasterCPU";  }
    virtual std::string getCategory() const   { return "Octree/Rendering";          }
    virtual CodeState getCodeState() const    { return CODE_STATE_EXPERIMENTAL;     }

    virtual bool usesExpensiveComputation() const { return true; }
    virtual bool isReady() const;

protected:
    virtual void setDescriptions() {
        setDescription("Performs an out-of-core octree-based volume rendering on the CPU "
            "(very slow, only for demonstration purposes!). "
            "<p>See also: OctreeCreator, SingleOctreeRaycasterCL</p>");
    }

    virtual void process();

private:
    /// Debug struct used for tracking nodes that have been passed by a ray
    struct RayNode {
        size_t depth_;  ///< depth of the node
        tgt::vec3 llf_; ///< lower left front coordinate
        tgt::vec3 urb_; ///< upper right back coordinate
    };

    void clearLevelVolume();

    void renderVolume();
    tgt::vec4 traverseRay(const tgt::vec3& entry, const tgt::vec3& exit,
        const tgt::vec3& camPos, const tgt::mat4& projectionMatrix, const tgt::ivec2& viewport,
        const RealWorldMapping& realWorldMapping, TransFunc1DKeys* transFunc, const float samplingStepSize,
        std::vector<RayNode>* passedNodes = 0);

    tgt::vec4 applyTransFunc(tgt::Texture* tfTexture, float intensity);

    /// Converts the passed node path to a MeshListGeometry.
    MeshListGeometry* generateRayNodePathGeometry(const std::vector<RayNode>& rayNodes);

    VolumePort octreeInport_;
    RenderPort entryPointsInport_;
    RenderPort exitPointsInport_;

    RenderPort renderOutport_;

    GeometryPort octreeGeometryOutport_;
    VolumePort levelVolumeOutport_;
    GeometryPort rayNodeGeometryPort_;
    VolumePort testVolumeOutport_;

    IntProperty outputVolumeLevel_;

    CameraProperty cameraProperty_;
    TransFuncProperty transferFunc_;
    FloatProperty samplingRate_;  ///< specified relative to the size of one voxel

    FloatVec2Property rayPixelCoordsRel_;

    static const std::string loggerCat_;
};

} //namespace

#endif
