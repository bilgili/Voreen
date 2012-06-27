/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "voreen/core/processors/processor.h"
#include "voreen/core/ports/allports.h"
#include "voreen/core/datastructures/geometry/meshlistgeometry.h"
#include "voreen/core/datastructures/transfunc/transfunc.h"
#include "minmaxtexture.h"
#include "minmaxoctree.h"
#include <queue>

#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/floatproperty.h"

namespace voreen {

class OptimizedProxyGeometry : public Processor {

public:
    OptimizedProxyGeometry();
    virtual ~OptimizedProxyGeometry();
    virtual Processor* create() const;
    virtual std::string getCategory() const { return "Volume Proxy Geometry"; }
    virtual std::string getClassName() const { return "OptimizedProxyGeometry"; }
    virtual Processor::CodeState getCodeState() const { return CODE_STATE_TESTING; }

protected:
    virtual void setDescriptions() {
        setDescription("Generates an optimized proxy-geometry, allowing for empty-space skipping.");
    }

    virtual void process();
    virtual bool isReady() const ;
    virtual void onModeChange();
    virtual void onTransFuncChange();
    virtual void onResolutionChange();
    virtual void onThresholdChange();
    virtual void onMinMaxTexResChange();
    virtual void updatePropertyVisibility();

    VolumePort inport_;
    VolumePort inportGradient_;
    GeometryPort outport_;

    MeshListGeometry* geometry_;

    StringOptionProperty mode_;
    TransFuncProperty transfunc_;
    IntProperty resolution_;
    FloatProperty threshold_;
    IntProperty minmaxtexres_;
    IntProperty minmaxtex2DTFxres_;
    IntProperty minmaxtex2DTFyres_;
    IntProperty spacing_;
    BoolProperty setEnclosedOpaque_;
    // begin debug
    BoolProperty brefresh_;
    BoolProperty bgeometry_;
    BoolProperty bminmaxt_;
    // end debug

    std::string mode_cubeproxygeometry;
    std::string mode_bruteforce;
    std::string mode_bordersonly;
    std::string mode_convex;
    std::string mode_octree;
    std::string mode_minmaxconvex;
    std::string mode_minmaxoctree;

    /**
    * Begin Common
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
            if (pos_.x>urb_.x) {
                pos_.x=llf_.x;
                pos_.y++;
                if (pos_.y>urb_.y) {
                    pos_.y=llf_.y;
                    pos_.z++;
                }
            }
        }
        bool hasnext() {return pos_.x<urb_.x || pos_.y<urb_.y || pos_.z<urb_.z;}
        tgt::ivec3 value() {return pos_;}
        bool outofrange() {return pos_.z>urb_.z;}
        tgt::ivec3 getnext() {next(); return value();}
    };

    class PropertyVolume {
    private:
        struct VolPart {
            float minvox;
            float maxvox;
            float mingrad;
            float maxgrad;
            bool opaque;
            bool bound;
            inline bool opaquenotbound() {return opaque && !bound;}
        };
        VolPart* vpart_;
        tgt::ivec3 size_;
        VolPart* getVolPart(tgt::ivec3 pos) {return &vpart_[pos.x*(size_.y*size_.z)+pos.y*size_.z+pos.z];}
    public:
        PropertyVolume(tgt::ivec3 size)
            : vpart_(0)
            , size_(size)
        {
            vpart_ = new VolPart[size_.x*size_.y*size_.z];
        }
        ~PropertyVolume() {
            delete[] vpart_;
        }

        tgt::ivec3 getSize() {return size_;}
        float getMinVox(tgt::ivec3 pos) {return getVolPart(pos)->minvox;}
        float getMaxVox(tgt::ivec3 pos) {return getVolPart(pos)->maxvox;}
        float getMinGrad(tgt::ivec3 pos) {return getVolPart(pos)->mingrad;}
        float getMaxGrad(tgt::ivec3 pos) {return getVolPart(pos)->maxgrad;}
        bool getOpaque(tgt::ivec3 pos) {return getVolPart(pos)->opaque;}
        bool getBound(tgt::ivec3 pos) {return getVolPart(pos)->bound;}

        void setMinVox(tgt::ivec3 pos, float value) {getVolPart(pos)->minvox = value;}
        void setMaxVox(tgt::ivec3 pos, float value) {getVolPart(pos)->maxvox = value;}
        void setMinGrad(tgt::ivec3 pos, float value) {getVolPart(pos)->mingrad = value;}
        void setMaxGrad(tgt::ivec3 pos, float value) {getVolPart(pos)->maxgrad = value;}
        void setOpaque(tgt::ivec3 pos, bool value) {getVolPart(pos)->opaque = value;}
        void setBound(tgt::ivec3 pos, bool value) {getVolPart(pos)->bound = value;}

        bool getOpaqueNotBound(tgt::ivec3 pos) {return getVolPart(pos)->opaquenotbound();}
    };

    virtual tgt::vec4 applyTransferFunction(tgt::ivec3 vox);
    virtual void setVolBound(PropertyVolume* pvol, tgt::ivec3 llf, tgt::ivec3 urb, bool value = true);
    virtual bool isVolOpaqueNotBound(PropertyVolume* pvol, tgt::ivec3 llf, tgt::ivec3 urb);
    virtual tgt::ivec3 getUrb(PropertyVolume* pvol, tgt::ivec3 llf);
    virtual MeshGeometry createCubeMesh(tgt::vec3 sizing, tgt::ivec3 dim, tgt::ivec3 step, tgt::ivec3 llf, tgt::ivec3 urb);
    virtual inline bool isVoxelTransparent(tgt::ivec3 vox);
    virtual bool isVolumeTransparent(tgt::ivec3 llf, tgt::ivec3 urb);

    const VolumeBase* volvox_;
    Volume* volgradmag_;
    tgt::Texture* textf_;
    bool inportGradientChanged_;

    /**
    * End Common
    */

    virtual void processMinMaxOctree();
    virtual void processMinMaxConvex();
    virtual void processOctree();
    virtual void processConvex();
    virtual void processBordersOnly();
    virtual void processBruteForce();
    virtual void processCubeProxyGeometry();

    /**
    * Begin MinMaxOctree
    */

    OctreeNode* minmaxoct_;

    virtual void updateOpaque(OctreeNode* node, MinMaxTexture* mmt);
    virtual MeshGeometry createCubeMesh(tgt::vec3 sizing, tgt::vec3 llf, tgt::vec3 urb);
    virtual void setUpOctreeLeaf(OctreeNode* node, tgt::ivec3 llf, tgt::ivec3 urb);
    virtual void setUpOctreeNode(OctreeNode* node, OctreeLevel* uplevel, tgt::ivec3 llf);
    virtual OctreeNode* createMinMaxOctree(int res);
    virtual OctreeNode* createOctree(int res);
    virtual void createGeometry(MeshListGeometry* mlg, OctreeNode* node, tgt::vec3 sizing);

    bool setupOctree_;

    /**
    * End MinMaxOctree
    */

    /**
    * Begin MinMaxConvex
    */

    virtual void updateVolPartOpaque(PropertyVolume* pvol, tgt::ivec3 pos, tgt::ivec3 llf, tgt::ivec3 urb);
    virtual void setupVolPart(PropertyVolume* pvol, tgt::ivec3 pos, tgt::ivec3 llf, tgt::ivec3 urb);

    PropertyVolume* pvolmm_;
    MinMaxTexture* minmaxtex_;
    bool updateMinMaxTex_;
    bool setupVolPart_;
    bool updateGeometry_;
    bool use2DTF_;

    /**
    * End MinMaxConvex
    */

    /**
    * Begin Convex
    */

    virtual void setEnclosedOpaque(PropertyVolume* pvol);
    virtual bool isEnclosed(PropertyVolume* pvol, tgt::ivec3 pos);
    virtual void setOpaque(PropertyVolume* pvol, tgt::ivec3 pos);

    /**
    * End Convex
    */

    /**
    * Begin BordersOnly
    */

    enum FaceDirection {Left,Right,Bottom,Top,Front,Back};
    virtual VertexGeometry createVertexGeometry(tgt::vec3 sizing, tgt::vec3 voxcoords);
    virtual FaceGeometry createFaceGeometry(tgt::vec3 sizing, tgt::ivec3 dim, FaceDirection dir, tgt::ivec3 llf, tgt::ivec3 urb);

    /**
    * End BordersOnly
    */

};

} // namespace

#endif // VRN_OPTIMIZEDPROXYGEOMETRY_H
