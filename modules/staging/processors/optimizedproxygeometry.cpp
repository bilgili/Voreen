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

#include "optimizedproxygeometry.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/operators/volumeoperatormagnitude.h"

namespace voreen {
OptimizedProxyGeometry::OptimizedProxyGeometry()
    : Processor()
    , inport_(Port::INPORT, "volumehandle.volumehandle", "Volume Input")
    , inportGradient_(Port::INPORT,"volumehandle.gradientvolumehandle", "Gradient Volume Input")
    , outport_(Port::OUTPORT, "proxygeometry.geometry", "Proxy Geometry Output")
    , mode_("approachString","Approach")
    , transfunc_("transferFunction","Transfer Function")
    , resolution_("proxyGeometryResolution","Proxy Geometry Resolution",8,1,64)
    , threshold_("transparentTheshold","Transparent Threshold %",5.f,0.f,100.f)
    , minmaxtexres_("minMaxTexRes","Min-Max-Texture Resolution",256,1,1024)
    , minmaxtex2DTFxres_("minMaxTex2DTFxres","Min-Max-Texture-2DTF x-Resolution",40,1,64)
    , minmaxtex2DTFyres_("minMaxTex2DTFyres","Min-Max-Texture-2DTF y-Resolution",40,1,64)
    , spacing_("spacing","Spacing",0,0,5)
    , setEnclosedOpaque_("setEnclosedOpaque","Set Enclosed Parts Opaque",true)
    , brefresh_("brefresh","refresh",true)
    , bgeometry_("bgeometry","geometry")
    , bminmaxt_("bminmaxt","minmaxt")
{
    //common gp
    geometry_ = new MeshListGeometry();

    mode_cubeproxygeometry = "cubeproxygeometry";
    mode_bruteforce = "bruteforce";
    mode_bordersonly = "bordersonly";
    mode_convex = "convex";
    mode_octree = "octree";
    mode_minmaxconvex = "minmaxconvex";
    mode_minmaxoctree = "minmaxoctree";

    mode_.addOption(mode_cubeproxygeometry,"CubeProxyGeometry");
    mode_.addOption(mode_bruteforce,"Brute Force");
    mode_.addOption(mode_bordersonly,"Borders Only");
    mode_.addOption(mode_convex,"Convex");
    mode_.addOption(mode_octree,"Octree");
    mode_.addOption(mode_minmaxconvex,"MinMax Convex");
    mode_.addOption(mode_minmaxoctree,"MinMax Octree");

    addPort(inport_);
    addPort(inportGradient_);
    addPort(outport_);

    addProperty(mode_);
    addProperty(transfunc_);
    addProperty(resolution_);
    addProperty(minmaxtexres_);
    addProperty(minmaxtex2DTFxres_);
    addProperty(minmaxtex2DTFyres_);
    addProperty(threshold_);
    addProperty(spacing_);
    addProperty(setEnclosedOpaque_);
    addProperty(brefresh_);
    addProperty(bgeometry_);
    addProperty(bminmaxt_);

    updatePropertyVisibility();

    mode_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onModeChange));
    transfunc_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onTransFuncChange));
    resolution_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onResolutionChange));
    threshold_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onThresholdChange));
    minmaxtexres_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onMinMaxTexResChange));
    minmaxtex2DTFxres_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onMinMaxTexResChange));
    minmaxtex2DTFyres_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onMinMaxTexResChange));
    spacing_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onThresholdChange));
    setEnclosedOpaque_.onChange(CallMemberAction<OptimizedProxyGeometry>(this, &OptimizedProxyGeometry::onThresholdChange));

    pvolmm_ = 0; minmaxtex_ = 0; minmaxoct_ = 0;
    updateMinMaxTex_ = true;
    setupVolPart_ = true;
    setupOctree_ = true;
    updateGeometry_ = false;

    volvox_ = 0; volgradmag_ = 0; inportGradientChanged_ = false;
}

OptimizedProxyGeometry::~OptimizedProxyGeometry() {
    geometry_->clear(); delete geometry_; geometry_ = 0;
    delete minmaxtex_; minmaxtex_ = 0;
    delete pvolmm_; pvolmm_ = 0;
    delete minmaxoct_; minmaxoct_ = 0;
}

Processor* OptimizedProxyGeometry::create() const {
    return new OptimizedProxyGeometry();
}

bool OptimizedProxyGeometry::isReady() const {
    return isInitialized() && inport_.isReady() && outport_.isReady() && (use2DTF_?inportGradient_.isReady():true);
}

void OptimizedProxyGeometry::updatePropertyVisibility() {
    if (mode_.isSelected(mode_cubeproxygeometry)) {
        transfunc_.setVisible(false);
        resolution_.setVisible(false);
        threshold_.setVisible(false);
        minmaxtexres_.setVisible(false);
        minmaxtex2DTFxres_.setVisible(false);
        minmaxtex2DTFyres_.setVisible(false);
    } else {
        transfunc_.setVisible(true);
        resolution_.setVisible(true);
        threshold_.setVisible(true);
    }

    if (mode_.isSelected(mode_minmaxconvex) || mode_.isSelected(mode_minmaxoctree)) {
        if (use2DTF_) {
            minmaxtexres_.setVisible(false);
            minmaxtex2DTFxres_.setVisible(true);
            minmaxtex2DTFyres_.setVisible(true);
        } else {
            minmaxtexres_.setVisible(true);
            minmaxtex2DTFxres_.setVisible(false);
            minmaxtex2DTFyres_.setVisible(false);
        }
    } else {
        minmaxtexres_.setVisible(false);
        minmaxtex2DTFxres_.setVisible(false);
        minmaxtex2DTFyres_.setVisible(false);
    }

    if (mode_.isSelected(mode_convex) || mode_.isSelected(mode_minmaxconvex)) {
        setEnclosedOpaque_.setVisible(true);
    } else {
        setEnclosedOpaque_.setVisible(false);
    }
}

void OptimizedProxyGeometry::onModeChange() {
    updateGeometry_ = true;
    updatePropertyVisibility();
}

void OptimizedProxyGeometry::onTransFuncChange() {
    if (transfunc_.get()->getTexture()->getHeight()>=2) {
        use2DTF_ = true;
        LWARNING("Optimization for 2D TFs is untested!");
    } else {
        use2DTF_ = false;
    }
    updateMinMaxTex_ = true;
    updatePropertyVisibility();
}

void OptimizedProxyGeometry::onResolutionChange() {
    setupVolPart_ = true;
    setupOctree_ = true;
}

void OptimizedProxyGeometry::onThresholdChange() {
    updateMinMaxTex_ = true;
}

void OptimizedProxyGeometry::onMinMaxTexResChange() {
    updateMinMaxTex_ = true;
}

/**
* Begin Process
*/

//MeshGeometry testCreateCubeMesh(tgt::vec3 sizing, tgt::ivec3 dim, tgt::ivec3 step, tgt::ivec3 llf, tgt::ivec3 urb, tgt::vec3 col) {
//    const tgt::vec3 spacing = tgt::vec3(0.5f);
//    tgt::vec3 coordllf;
//    tgt::vec3 coordurb;
//    tgt::vec3 colllf;
//    tgt::vec3 colurb;
//
//    coordllf = (tgt::vec3(llf*step)-spacing)/tgt::vec3(dim);
//    coordllf = tgt::max(coordllf,tgt::vec3(0.f));
//    coordllf = tgt::min(coordllf,tgt::vec3(1.f));
//    colllf = col;
//    coordllf = 2.f * coordllf - tgt::vec3(1.f);
//
//    coordurb = (tgt::vec3((urb+tgt::ivec3(0))*step)+spacing)/tgt::vec3(dim);
//    coordurb = tgt::max(coordurb,tgt::vec3(0.f));
//    coordurb = tgt::min(coordurb,tgt::vec3(1.f));
//    colurb = col;
//    coordurb = 2.f * coordurb - tgt::vec3(1.f);
//
//    coordllf *= sizing;
//    coordurb *= sizing;
//
//    return MeshGeometry::createCube(coordllf, coordurb, colllf, colurb, colllf, colurb);
//}

void OptimizedProxyGeometry::process() {

    tgtAssert(inport_.getData()->getRepresentation<VolumeRAM>(), "no volume");

    transfunc_.setVolumeHandle(inport_.getData());

    volvox_ = inport_.getData();

    if (inportGradient_.hasChanged()) inportGradientChanged_=true;
    if (use2DTF_ && inportGradientChanged_) {
        inportGradientChanged_ = false;
        delete volgradmag_; volgradmag_ = 0;
        const VolumeRAM* vol = inportGradient_.getData()->getRepresentation<VolumeRAM>();
        if (dynamic_cast<const VolumeRAM_3xUInt8*>(vol)) {
            const VolumeRAM_3xUInt8* vol3xuint8 = static_cast<const VolumeRAM_3xUInt8*>(vol);
            if (!vol3xuint8) {
                LWARNING("gradient volume not supported");
                return;
            }
            VolumeOperatorMagnitude voOpMa;
            volgradmag_ = voOpMa.apply<uint8_t>(inportGradient_.getData());
        } else if (dynamic_cast<const VolumeRAM_3xUInt16*>(vol)) {
            const VolumeRAM_3xUInt16* vol3xuint16 = static_cast<const VolumeRAM_3xUInt16*>(vol);
            if (!vol3xuint16) {
                LWARNING("gradient volume not supported");
                return;
            }
            VolumeOperatorMagnitude voOpMa;
            volgradmag_ = voOpMa.apply<uint16_t>(inportGradient_.getData());
        } else {
            LWARNING("gradient volume bits stored " << vol->getBitsAllocated() << " not supported");
            return;
        }
        if (!volgradmag_) {
            LWARNING("could not calculate gradient magnitudes");
            return;
        }
    }

    textf_ = transfunc_.get()->getTexture();
    if (use2DTF_) {
        if (textf_->getBpp()!=sizeof(tgt::vec4)) {
            LWARNING("Bpp " <<  sizeof(tgt::vec4) << " of TF not supported");
            return;
        }
        if (textf_->getType()!=GL_TEXTURE_2D) {
            LWARNING("TF must be a Texture of Type GL_TEXTURE_2D");
            return;
        }
    } else {
        if (textf_->getBpp()!=sizeof(tgt::col4)) {
            LWARNING("Bpp " << sizeof(tgt::vec4) << " of TF not supported");
            return;
        }
    }

    if (inport_.hasChanged()) {
        setupVolPart_ = true;
        setupOctree_ = true;
    }

    // begin debug
    if (!brefresh_.get()) return;

    if (bgeometry_.get()) {
        if (mode_.isSelected(mode_minmaxconvex)) setupVolPart_ = true;
        if (mode_.isSelected(mode_minmaxoctree)) setupOctree_ = true;
    }
    if (bminmaxt_.get()) updateMinMaxTex_ = true;
    //end debug

    if (mode_.isSelected(mode_minmaxoctree)) processMinMaxOctree();
    if (mode_.isSelected(mode_minmaxconvex)) processMinMaxConvex();
    if (mode_.isSelected(mode_octree)) processOctree();
    if (mode_.isSelected(mode_convex)) processConvex();
    if (mode_.isSelected(mode_bordersonly)) processBordersOnly();
    if (mode_.isSelected(mode_bruteforce)) processBruteForce();
    if (mode_.isSelected(mode_cubeproxygeometry)) processCubeProxyGeometry();

    geometry_->transform(inport_.getData()->getPhysicalToWorldMatrix());
    outport_.setData(geometry_, false);
}

void OptimizedProxyGeometry::processMinMaxOctree() {
    if (!minmaxoct_) setupOctree_ = true;

    const tgt::vec3 sizing = volvox_->getCubeSize() / 2.f;

    tgt::ivec3 llf, urb;

    if (updateMinMaxTex_) {
        delete minmaxtex_;
        if (use2DTF_)
            minmaxtex_ = static_cast<MinMaxTexture*>(MinMaxTexture2DTF::create(transfunc_.get()->getTexture(),minmaxtex2DTFxres_.get(),minmaxtex2DTFyres_.get()));
        else
            minmaxtex_ = static_cast<MinMaxTexture*>(MinMaxTexture1DTF::create(transfunc_.get()->getTexture(),minmaxtexres_.get()));
    }

    if (setupOctree_) {
        delete minmaxoct_;
        minmaxoct_ = createMinMaxOctree(resolution_.get());
    }

    if (updateMinMaxTex_ || setupOctree_) {
        updateGeometry_ = true;
    }

    if (updateGeometry_) {
        geometry_->clear();
        updateOpaque(minmaxoct_,minmaxtex_);
        createGeometry(geometry_,minmaxoct_,sizing);
    }

    updateMinMaxTex_ = false;
    setupOctree_ = false;
    updateGeometry_ = false;
}

void OptimizedProxyGeometry::processMinMaxConvex() {
    if (!pvolmm_) setupVolPart_ = true;

    tgt::vec3 sizing = volvox_->getCubeSize() / 2.f;
    tgt::ivec3 dim = volvox_->getDimensions();
    tgt::ivec3 step = tgt::max(tgt::ivec3(1), tgt::ivec3(tgt::round(tgt::vec3(dim)/(float)resolution_.get())));
    tgt::ivec3 size = tgt::ivec3(tgt::ceil(tgt::vec3(dim)/tgt::vec3(step)));

    tgt::ivec3 pos;
    tgt::ivec3 llf, urb;

    if (updateMinMaxTex_) {
        delete minmaxtex_;
        if (use2DTF_)
            minmaxtex_ = static_cast<MinMaxTexture*>(MinMaxTexture2DTF::create(transfunc_.get()->getTexture(),minmaxtex2DTFxres_.get(),minmaxtex2DTFyres_.get()));
        else
            minmaxtex_ = static_cast<MinMaxTexture*>(MinMaxTexture1DTF::create(transfunc_.get()->getTexture(),minmaxtexres_.get()));
    }

    if (setupVolPart_) {
        delete pvolmm_;
        pvolmm_ = new PropertyVolume(size);
        for (VolumeIterator it(pvolmm_->getSize()); !it.outofrange(); it.next()) {
            pos = it.value();
            llf = pos * step;
            urb = (pos+1) * step - 1;
            setupVolPart(pvolmm_,pos,llf,urb);
        }
    }

    if (updateMinMaxTex_ || setupVolPart_) updateGeometry_ = true;

    if (updateGeometry_) {
        geometry_->clear();

        for (VolumeIterator it(pvolmm_->getSize()); !it.outofrange(); it.next()) {
            pos = it.value();
            llf = pos * step;
            urb = (pos+1) * step - 1;
            updateVolPartOpaque(pvolmm_,pos,llf,urb);
        }

        if (setEnclosedOpaque_.get()) setEnclosedOpaque(pvolmm_);

        setVolBound(pvolmm_,tgt::ivec3(0),pvolmm_->getSize()-1,false);

        for (VolumeIterator it(pvolmm_->getSize()); !it.outofrange(); it.next()) {
            pos = it.value();
            if (pvolmm_->getOpaqueNotBound(pos)) {
                llf = pos;
                urb = getUrb(pvolmm_,llf);
                geometry_->addMesh(createCubeMesh(sizing,dim,step,llf,urb));
            }
        }
    }

    updateMinMaxTex_ = false;
    setupVolPart_ = false;
    updateGeometry_ = false;
}

void OptimizedProxyGeometry::processOctree() {
    const tgt::vec3 sizing = volvox_->getCubeSize() / 2.f;
    OctreeNode* root = createOctree(resolution_.get());
    geometry_->clear();
    createGeometry(geometry_,root,sizing);
    delete root;
}

void OptimizedProxyGeometry::processConvex() {
    tgt::vec3 sizing = volvox_->getCubeSize() / 2.f;
    tgt::ivec3 dim = volvox_->getDimensions();
    const int res = resolution_.get();
    const tgt::ivec3 step = tgt::max(tgt::ivec3(1),
        tgt::ivec3((int)tgt::round((float)dim.x/res),
        (int)tgt::round((float)dim.y/res),
        (int)tgt::round((float)dim.z/res)));
    const tgt::ivec3 size((int)ceil((float)dim.x/step.x),(int)ceil((float)dim.y/step.y),(int)ceil((float)dim.z/step.z));

    PropertyVolume* pvol = new PropertyVolume(size);

    tgt::ivec3 pos;
    tgt::ivec3 llf, urb;

    for (VolumeIterator it(pvol->getSize()); !it.outofrange(); it.next()) {
        pos = it.value();
        llf = pos * step;
        urb = (pos+1) * step - tgt::ivec3(1);
        pvol->setOpaque(pos,!isVolumeTransparent(llf,urb));
    }

    if (setEnclosedOpaque_.get()) setEnclosedOpaque(pvol);

    setVolBound(pvol,tgt::ivec3(0),pvol->getSize()-1,false);

    geometry_->clear();

    /*int count = 0;
    tgt::vec3 coltable[10] = {
        tgt::vec3(0.f,0.f,0.f),
        tgt::vec3(1.f,0.f,0.f),
        tgt::vec3(0.f,1.f,0.f),
        tgt::vec3(0.f,0.f,1.f),
        tgt::vec3(0.f,1.f,1.f),
        tgt::vec3(1.f,1.f,0.f),
        tgt::vec3(0.8f,0.8f,0.8f),
        tgt::vec3(0.3f,0.3f,0.3f),
        tgt::vec3(1.f,0.f,1.f),
        tgt::vec3(0.5f,1.f,0.f)
    };*/

    for (VolumeIterator it(pvol->getSize()); !it.outofrange(); it.next()) {
        pos = it.value();
        if (pvol->getOpaqueNotBound(pos)) {
            llf = pos;
            urb = getUrb(pvol,llf);
            geometry_->addMesh(createCubeMesh(sizing,dim,step,llf,urb));
            //if (applyTransferFunction(llf).a!=0.f) geometry_->addMesh(testCreateCubeMesh(sizing,dim,step,llf,llf,applyTransferFunction(llf).xyz()));
            //if (count<10) {
                /*geometry_->addMesh(testCreateCubeMesh(sizing,dim,step,llf,llf,coltable[count%10]));
                geometry_->addMesh(testCreateCubeMesh(sizing,dim,step,urb,urb,coltable[count%10]));
                count++;*/
            //}
        }
    }

    delete pvol;
}

void OptimizedProxyGeometry::processBordersOnly() {
    tgt::vec3 sizing = volvox_->getCubeSize() / 2.f;
    tgt::ivec3 dim = volvox_->getDimensions();
    const int res = resolution_.get();
    const tgt::ivec3 step = tgt::max(tgt::ivec3(1),
            tgt::ivec3((int)tgt::round((float)dim.x/res),
            (int)tgt::round((float)dim.y/res),
            (int)tgt::round((float)dim.z/res)));
    const tgt::ivec3 size((int)ceil((float)dim.x/step.x),(int)ceil((float)dim.y/step.y),(int)ceil((float)dim.z/step.z));

    bool* vtrans = new bool[size.x*size.y*size.z];

    tgt::ivec3 pos, llf, urb;

    for (VolumeIterator it(size); !it.outofrange(); it.next()) {
        pos = it.value();
        llf = pos * step;
        urb = (pos+1) * step - tgt::ivec3(1);
        vtrans[pos.x*(size.y*size.z)+pos.y*size.z+pos.z] = isVolumeTransparent(llf,urb);
    }

    geometry_->clear();

    MeshGeometry* mesh;

    int x,y,z;

    for (VolumeIterator it(size); !it.outofrange(); it.next()) {
        pos = it.value(); x=pos.x; y=pos.y; z=pos.z;
        if (!vtrans[x*(size.y*size.z)+y*size.z+z]) {
            mesh = new MeshGeometry();
            llf = pos * step;
            urb = (pos+1) * step;
            if (x==0 || vtrans[(x-1)*(size.y*size.z)+y*size.z+z]){
                mesh->addFace(createFaceGeometry(sizing,dim,Left,llf,urb));
            }

            if (x==size.x-1 || vtrans[(x+1)*(size.y*size.z)+y*size.z+z]){
                mesh->addFace(createFaceGeometry(sizing,dim,Right,llf,urb));
            }

            if (y==0 || vtrans[x*(size.y*size.z)+(y-1)*size.z+z]){
                mesh->addFace(createFaceGeometry(sizing,dim,Bottom,llf,urb));
            }

            if (y==size.y-1 || vtrans[x*(size.y*size.z)+(y+1)*size.z+z]){
                mesh->addFace(createFaceGeometry(sizing,dim,Top,llf,urb));
            }

            if (z==0 || vtrans[x*(size.y*size.z)+y*size.z+(z-1)]){
                mesh->addFace(createFaceGeometry(sizing,dim,Front,llf,urb));
            }

            if (z==size.z-1 || vtrans[x*(size.y*size.z)+y*size.z+(z+1)]){
                mesh->addFace(createFaceGeometry(sizing,dim,Back,llf,urb));
            }

            if (mesh->getFaceCount()!=0)
                geometry_->addMesh(*mesh);
        }
    }

    delete[] vtrans;
}

void OptimizedProxyGeometry::processBruteForce() {
    tgt::vec3 sizing = volvox_->getCubeSize() / 2.f;
    tgt::ivec3 dim = volvox_->getDimensions();
    const int res = resolution_.get();
    const tgt::ivec3 step = tgt::max(tgt::ivec3(1),
        tgt::ivec3((int)tgt::round((float)dim.x/res),
        (int)tgt::round((float)dim.y/res),
        (int)tgt::round((float)dim.z/res)));
    const tgt::ivec3 size((int)ceil((float)dim.x/step.x),(int)ceil((float)dim.y/step.y),(int)ceil((float)dim.z/step.z));

    tgt::ivec3 pos, llf, urb;

    geometry_->clear();

    for (VolumeIterator it(size); !it.outofrange(); it.next()) {
        pos = it.value();
        llf = pos * step;
        urb = (pos+1) * step - tgt::ivec3(1);
        if (!isVolumeTransparent(llf,urb)) geometry_->addMesh(createCubeMesh(sizing,dim,step,pos,pos));
    }
}

void OptimizedProxyGeometry::processCubeProxyGeometry() {
    tgt::vec3 sizing = volvox_->getCubeSize() / 2.f;
    tgt::ivec3 dim = volvox_->getDimensions();
    const int res = 1;
    const tgt::ivec3 step = tgt::max(tgt::ivec3(1),
        tgt::ivec3((int)tgt::round((float)dim.x/res),
        (int)tgt::round((float)dim.y/res),
        (int)tgt::round((float)dim.z/res)));
    const tgt::ivec3 size((int)ceil((float)dim.x/step.x),(int)ceil((float)dim.y/step.y),(int)ceil((float)dim.z/step.z));

    geometry_->clear();
    geometry_->addMesh(createCubeMesh(sizing,tgt::vec3(0.f),tgt::vec3(1.f)));
}

/**
* End Process
*/

/**
* Begin Common
*/

tgt::vec4 OptimizedProxyGeometry::applyTransferFunction(tgt::ivec3 vox) {
    if (use2DTF_) {
        // FIXME: RWM, TF domains...
        //linear interpolation
        float x,y;
        size_t x0,x1,y0,y1;
        x = volvox_->getRepresentation<VolumeRAM>()->getVoxelNormalized(vox)*(textf_->getWidth()-1);
        y = volgradmag_->getRepresentation<VolumeRAM>()->getVoxelNormalized(vox)*(textf_->getHeight()-1);
        x0 = (size_t)floor(x);
        x1 = (size_t)ceil(x);
        y0 = (size_t)floor(y);
        y1 = (size_t)ceil(y);
        if (x0==x1) {
            if (y0==y1) {
                return textf_->texel<tgt::vec4>(x0,y0);
            } else {
                tgt::vec4 f0 = textf_->texel<tgt::vec4>(x0,y0);
                tgt::vec4 f1 = textf_->texel<tgt::vec4>(x0,y1);
                return f0 + (f1-f0)/(float)(y1-y0)*(float)(y-y0);
            }
        } else {
            if (y0==y1) {
                tgt::vec4 f0 = textf_->texel<tgt::vec4>(x0,y0);
                tgt::vec4 f1 = textf_->texel<tgt::vec4>(x1,y0);
                return f0 + (f1-f0)/(float)(x1-x0)*(float)(x-x0);
            } else {
                tgt::vec4 f00 = textf_->texel<tgt::vec4>(x0,y0);
                tgt::vec4 f01 = textf_->texel<tgt::vec4>(x0,y1);
                tgt::vec4 f10 = textf_->texel<tgt::vec4>(x1,y0);
                tgt::vec4 f11 = textf_->texel<tgt::vec4>(x1,y1);
                return (x1-x)*((y1-y)*f00+(y-y0)*f01)+(x-x0)*((y1-y)*f10+(y-y0)*f11);
            }
        }
    } else {
        //linear interpolation
        float value = volvox_->getRepresentation<VolumeRAM>()->getVoxelNormalized(vox);
        value = volvox_->getRealWorldMapping().normalizedToRealWorld(value);
        value = transfunc_.get()->realWorldToNormalized(value);
        float x = value*(textf_->getWidth()-1);
        size_t x0 = (size_t)floor(x);
        size_t x1 = (size_t)ceil(x);
        if (x0==x1) {
            return tgt::vec4(textf_->texel<tgt::col4>(x0))/255.f;
        } else {
            tgt::vec4 f0 = textf_->texel<tgt::col4>(x0);
            tgt::vec4 f1 = textf_->texel<tgt::col4>(x1);
            return tgt::vec4(f0 + (f1-f0)/(float)(x1-x0)*(float)(x-x0))/255.f;
        }
    }
}

void OptimizedProxyGeometry::setVolBound(PropertyVolume* pvol, tgt::ivec3 llf, tgt::ivec3 urb, bool value) {
    for (int z=llf.z; z<=urb.z;z++) {
        for (int y=llf.y; y<=urb.y; y++) {
            for (int x=llf.x; x<=urb.x; x++) {
                pvol->setBound(tgt::ivec3(x,y,z),value);
            }
        }
    }
}

bool OptimizedProxyGeometry::isVolOpaqueNotBound(PropertyVolume* pvol, tgt::ivec3 llf, tgt::ivec3 urb) {
    for (int z=llf.z; z<=urb.z;z++) {
        for (int y=llf.y; y<=urb.y; y++) {
            for (int x=llf.x; x<=urb.x; x++) {
                if (!pvol->getOpaqueNotBound(tgt::ivec3(x,y,z))) return false;
            }
        }
    }
    return true;
}

//tgt::ivec3 OptimizedProxyGeometry::getUrb(PropertyVolume* pvol, tgt::ivec3 llf) {
//    const tgt::ivec3 size = pvol->getSize();
//    tgt::bvec3 inc(true);
//    tgt::ivec3 urb(llf);
//    tgt::ivec3 tmpllf,tmpurb;
//
//    setVolBound(pvol,llf,llf);
//
//    while(inc.x||inc.y||inc.z) {
//        if (inc.x) {
//            if (urb.x+1>size.x-1) {
//                inc.x=false;
//            } else {
//                tmpllf = tgt::ivec3(urb.x+1,llf.y,llf.z);
//                tmpurb = tgt::ivec3(urb.x+1,urb.y,urb.z);
//                if (isVolOpaqueNotBound(pvol,tmpllf,tmpurb)) {
//                    setVolBound(pvol,tmpllf,tmpurb);
//                    urb.x += 1;
//                } else {
//                    inc.x=false;
//                }
//            }
//        }
//        if (inc.y) {
//            if (urb.y+1>size.y-1) {
//                inc.y=false;
//            } else {
//                tmpllf = tgt::ivec3(llf.x,urb.y+1,llf.z);
//                tmpurb = tgt::ivec3(urb.x,urb.y+1,urb.z);
//                if (isVolOpaqueNotBound(pvol,tmpllf,tmpurb)) {
//                    setVolBound(pvol,tmpllf,tmpurb);
//                    urb.y += 1;
//                } else {
//                    inc.y=false;
//                }
//            }
//        }
//        if (inc.z) {
//            if (urb.z+1>size.z-1) {
//                inc.z=false;
//            } else {
//                tmpllf = tgt::ivec3(llf.x,llf.y,urb.z+1);
//                tmpurb = tgt::ivec3(urb.x,urb.y,urb.z+1);
//                if (isVolOpaqueNotBound(pvol,tmpllf,tmpurb)) {
//                    setVolBound(pvol,tmpllf,tmpurb);
//                    urb.z += 1;
//                } else {
//                    inc.z=false;
//                }
//            }
//        }
//    }
//    return urb;
//}

tgt::ivec3 OptimizedProxyGeometry::getUrb(PropertyVolume* pvol, tgt::ivec3 llf) {
    const tgt::ivec3 size = pvol->getSize();
    tgt::bvec3 inc(true);
    tgt::ivec3 urb(llf);

    while(inc.x||inc.y||inc.z) {
        if (inc.x) {
            if (urb.x+1>size.x-1) {
                inc.x=false;
            } else {
                if (isVolOpaqueNotBound(pvol,
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
                if (isVolOpaqueNotBound(pvol,
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
                if (isVolOpaqueNotBound(pvol,
                    tgt::ivec3(llf.x,llf.y,urb.z+1),
                    tgt::ivec3(urb.x,urb.y,urb.z+1))) {
                    urb.z += 1;
                } else {
                    inc.z=false;
                }
            }
        }
    }

    setVolBound(pvol,llf,urb);
    return urb;
}

MeshGeometry OptimizedProxyGeometry::createCubeMesh(tgt::vec3 sizing, tgt::ivec3 dim, tgt::ivec3 step, tgt::ivec3 llf, tgt::ivec3 urb) {
    const float spacing = 0.5f;
    tgt::vec3 coordllf;
    tgt::vec3 coordurb;
    tgt::vec3 colllf;
    tgt::vec3 colurb;

    coordllf = (tgt::vec3(llf*step)-spacing-(float)spacing_.get())/tgt::vec3(dim);
    coordllf = tgt::max(coordllf,tgt::vec3(0.f));
    coordllf = tgt::min(coordllf,tgt::vec3(1.f));
    colllf = coordllf;
    coordllf = 2.f * coordllf - tgt::vec3(1.f);

    coordurb = (tgt::vec3((urb+tgt::ivec3(1))*step)-spacing+(float)spacing_.get())/tgt::vec3(dim);
    coordurb = tgt::max(coordurb,tgt::vec3(0.f));
    coordurb = tgt::min(coordurb,tgt::vec3(1.f));
    colurb = coordurb;
    coordurb = 2.f * coordurb - tgt::vec3(1.f);

    coordllf *= sizing;
    coordurb *= sizing;

    return MeshGeometry::createCube(coordllf, coordurb, colllf, colurb, colllf, colurb);
}

inline bool OptimizedProxyGeometry::isVoxelTransparent(tgt::ivec3 vox) {
    return (applyTransferFunction(vox).a <= threshold_.get()/100.f);
}

bool OptimizedProxyGeometry::isVolumeTransparent(tgt::ivec3 llf, tgt::ivec3 urb) {
    const tgt::ivec3 dim = volvox_->getDimensions();

    llf = tgt::max(llf,tgt::ivec3(0));
    llf = tgt::min(llf,dim - 1);

    urb = tgt::max(urb,tgt::ivec3(0));
    urb = tgt::min(urb,dim - 1);

    for(int x=llf.x; x<=urb.x; x++) {
        for(int y=llf.y; y<=urb.y; y++) {
            for(int z=llf.z; z<=urb.z; z++) {
                if (!isVoxelTransparent(tgt::ivec3(x,y,z)))
                    return false;
            }
        }
    }
    return true;
}

/**
* End Common
*/

/**
* Begin MinMaxOctree
*/

void OptimizedProxyGeometry::setUpOctreeLeaf(OctreeNode* node, tgt::ivec3 llf, tgt::ivec3 urb) {
    const tgt::ivec3 dim = volvox_->getDimensions();
    const tgt::vec3 spacing = tgt::vec3(0.5f);
    float val;

    llf = tgt::max(llf,tgt::ivec3(0));
    llf = tgt::min(llf,dim - 1);

    urb = tgt::max(urb,tgt::ivec3(0));
    urb = tgt::min(urb,dim - 1);

    node->coordllf = (tgt::vec3(llf)-spacing-(float)spacing_.get())/tgt::vec3(dim);
    node->coordurb = (tgt::vec3(urb+1)-spacing+(float)spacing_.get())/tgt::vec3(dim);

    node->minvox = 1.f;
    node->maxvox = 0.f;
    node->mingrad = 1.f;
    node->maxgrad = 0.f;

    for(int x=llf.x; x<=urb.x; x++) {
        for(int y=llf.y; y<=urb.y; y++) {
            for(int z=llf.z; z<=urb.z; z++) {
                val = volvox_->getRepresentation<VolumeRAM>()->getVoxelNormalized(tgt::ivec3(x,y,z));
                val = volvox_->getRealWorldMapping().normalizedToRealWorld(val);

                if (val<node->minvox)
                    node->minvox = val;
                if (val>node->maxvox)
                    node->maxvox = val;
                if (use2DTF_) {
                    val = volgradmag_->getRepresentation<VolumeRAM>()->getVoxelNormalized(tgt::ivec3(x,y,z));
                    if (val<node->mingrad) node->mingrad = val;
                    if (val>node->maxgrad) node->maxgrad = val;
                }
            }
        }
    }
}

void OptimizedProxyGeometry::setUpOctreeNode(OctreeNode* node, OctreeLevel* uplevel, tgt::ivec3 llf) {
    const tgt::ivec3 urb = llf + 1;
    OctreeNode* valnode;
    float valmin,valmax;

    node->coordllf = uplevel->getNode(llf)->coordllf;
    node->coordurb = uplevel->getNode(urb)->coordurb;


    node->minvox = 1.f;
    node->maxvox = 0.f;
    node->mingrad = 1.f;
    node->maxgrad = 0.f;

    node->llf = uplevel->getNode(llf + tgt::ivec3(0,0,0));
    node->lrf = uplevel->getNode(llf + tgt::ivec3(1,0,0));
    node->ulf = uplevel->getNode(llf + tgt::ivec3(0,1,0));
    node->urf = uplevel->getNode(llf + tgt::ivec3(1,1,0));
    node->llb = uplevel->getNode(llf + tgt::ivec3(0,0,1));
    node->lrb = uplevel->getNode(llf + tgt::ivec3(1,0,1));
    node->ulb = uplevel->getNode(llf + tgt::ivec3(0,1,1));
    node->urb = uplevel->getNode(llf + tgt::ivec3(1,1,1));

    for(int x=llf.x; x<=urb.x; x++) {
        for(int y=llf.y; y<=urb.y; y++) {
            for(int z=llf.z; z<=urb.z; z++) {
                valnode = uplevel->getNode(tgt::ivec3(x,y,z));
                valnode->parent = node;
                valmin = valnode->minvox;
                valmax = valnode->maxvox;
                if (valmin<node->minvox) node->minvox = valmin;
                if (valmax>node->maxvox) node->maxvox = valmax;
                if (use2DTF_) {
                    valmin = valnode->mingrad;
                    valmax = valnode->maxgrad;
                    if (valmin<node->mingrad) node->mingrad = valmin;
                    if (valmax>node->maxgrad) node->maxgrad = valmax;
                }
            }
        }
    }
}

OctreeNode* OptimizedProxyGeometry::createMinMaxOctree(int res) {
    const tgt::ivec3 dim = volvox_->getDimensions();
    const int numlevels = (int) tgt::round(log((float)res)/log(2.f));
    const tgt::ivec3 size = tgt::ivec3((int)pow(2.f,numlevels));
    const tgt::ivec3 step = tgt::max(tgt::ivec3(1), tgt::ivec3(tgt::ceil(tgt::vec3(dim)/tgt::vec3(size))));

    tgt::ivec3 pos, llf,urb;
    OctreeLevel** otlevel = new OctreeLevel*[numlevels+1];
    OctreeLevel* vpart;

    vpart = new OctreeLevel(size);
    otlevel[numlevels] = vpart;

    for (VolumeIterator it(size); !it.outofrange(); it.next()) {
        pos = it.value();
        llf = pos * step;
        urb = (pos+1) * step - tgt::ivec3(1);
        setUpOctreeLeaf(vpart->getNode(pos),llf,urb);
    }

    tgt::ivec3 sizei = size;

    for (int i=numlevels-1; i>=0; i--) {
        sizei /= 2;
        vpart = new OctreeLevel(sizei);
        otlevel[i] = vpart;
        for (VolumeIterator it(sizei); !it.outofrange(); it.next()) {
            pos = it.value();
            llf = pos * tgt::ivec3(2);
            setUpOctreeNode(vpart->getNode(pos),otlevel[i+1],llf);
        }
    }

    OctreeNode* root = otlevel[0]->getNode(tgt::ivec3(0,0,0));

    for (int i=0; i<=numlevels; i++) delete otlevel[i];
    delete[] otlevel;

    return root;
}

OctreeNode* OptimizedProxyGeometry::createOctree(int res) {
    const tgt::ivec3 dim = volvox_->getDimensions();
    const int numlevels = (int) tgt::round(log((float)res)/log(2.f));
    const tgt::ivec3 size = tgt::ivec3((int)pow(2.f,numlevels));
    const tgt::ivec3 step = tgt::max(tgt::ivec3(1), tgt::ivec3(tgt::ceil(tgt::vec3(dim)/tgt::vec3(size))));
    const tgt::vec3 spacing = tgt::vec3(0.5f);

    tgt::ivec3 pos, llf,urb;
    OctreeLevel** otlevel = new OctreeLevel*[numlevels+1];
    OctreeLevel* vpart;
    OctreeNode* node;

    vpart = new OctreeLevel(size);
    otlevel[numlevels] = vpart;

    for (VolumeIterator it(size); !it.outofrange(); it.next()) {
        pos = it.value();
        llf = pos * step;
        urb = (pos+1) * step - tgt::ivec3(1);

        llf = tgt::max(llf,tgt::ivec3(0));
        llf = tgt::min(llf,dim - 1);

        urb = tgt::max(urb,tgt::ivec3(0));
        urb = tgt::min(urb,dim - 1);

        node = vpart->getNode(pos);

        node->coordllf = (tgt::vec3(llf)-spacing-(float)spacing_.get())/tgt::vec3(dim);
        node->coordurb = (tgt::vec3(urb+1)-spacing+(float)spacing_.get())/tgt::vec3(dim);

        node->opaque = !isVolumeTransparent(llf,urb);
    }

    tgt::ivec3 sizei = size;
    OctreeLevel* uplevel;

    for (int i=numlevels-1; i>=0; i--) {
        sizei /= 2;
        vpart = new OctreeLevel(sizei);
        otlevel[i] = vpart;
        for (VolumeIterator it(sizei); !it.outofrange(); it.next()) {
            pos = it.value();
            llf = pos * tgt::ivec3(2);
            node = vpart->getNode(pos);
            uplevel = otlevel[i+1];
            node->coordllf = uplevel->getNode(llf)->coordllf;
            node->coordurb = uplevel->getNode(llf+1)->coordurb;
            node->llf = uplevel->getNode(llf + tgt::ivec3(0,0,0));
            node->lrf = uplevel->getNode(llf + tgt::ivec3(1,0,0));
            node->ulf = uplevel->getNode(llf + tgt::ivec3(0,1,0));
            node->urf = uplevel->getNode(llf + tgt::ivec3(1,1,0));
            node->llb = uplevel->getNode(llf + tgt::ivec3(0,0,1));
            node->lrb = uplevel->getNode(llf + tgt::ivec3(1,0,1));
            node->ulb = uplevel->getNode(llf + tgt::ivec3(0,1,1));
            node->urb = uplevel->getNode(llf + tgt::ivec3(1,1,1));
            if (node->llf->opaque && node->lrf->opaque && node->ulf->opaque && node->urf->opaque &&
                node->llb->opaque && node->lrb->opaque && node->ulb->opaque && node->urb->opaque)
            {
                node->opaque = true;
            } else {
                node->opaque = false;
            }
        }
    }

    OctreeNode* root = otlevel[0]->getNode(tgt::ivec3(0,0,0));

    for (int i=0; i<=numlevels; i++) delete otlevel[i];
    delete[] otlevel;

    return root;
}

MeshGeometry OptimizedProxyGeometry::createCubeMesh(tgt::vec3 sizing, tgt::vec3 llf, tgt::vec3 urb) {
    tgt::vec3 coordllf;
    tgt::vec3 coordurb;
    tgt::vec3 colllf;
    tgt::vec3 colurb;

    coordllf = llf;
    coordllf = tgt::max(coordllf,tgt::vec3(0.f));
    coordllf = tgt::min(coordllf,tgt::vec3(1.f));
    colllf = coordllf;
    coordllf = 2.f * coordllf - tgt::vec3(1.f);

    coordurb = urb;
    coordurb = tgt::max(coordurb,tgt::vec3(0.f));
    coordurb = tgt::min(coordurb,tgt::vec3(1.f));
    colurb = coordurb;
    coordurb = 2.f * coordurb - tgt::vec3(1.f);

    coordllf *= sizing;
    coordurb *= sizing;

    return MeshGeometry::createCube(coordllf, coordurb, colllf, colurb, colllf, colurb);
}

void OptimizedProxyGeometry::updateOpaque(voreen::OctreeNode *node, voreen::MinMaxTexture *mmt) {
    if (!node->llf || !node->lrf || !node->ulf || !node->urf ||
        !node->llb || !node->lrb || !node->ulb || !node->urb)
    {
        if (mmt->getMax(node->minvox,node->maxvox,node->mingrad,node->maxgrad)>threshold_.get()/100.f) {
            node->opaque=true;
        } else {
            node->opaque=false;
        }
    } else {
        updateOpaque(node->llf, mmt);
        updateOpaque(node->lrf, mmt);
        updateOpaque(node->ulf, mmt);
        updateOpaque(node->urf, mmt);
        updateOpaque(node->llb, mmt);
        updateOpaque(node->lrb, mmt);
        updateOpaque(node->ulb, mmt);
        updateOpaque(node->urb, mmt);
        if (node->llf->opaque && node->lrf->opaque && node->ulf->opaque && node->urf->opaque &&
            node->llb->opaque && node->lrb->opaque && node->ulb->opaque && node->urb->opaque)
        {
            node->opaque=true;
        } else {
            node->opaque=false;
        }
    }
}

void OptimizedProxyGeometry::createGeometry(MeshListGeometry* mlg, OctreeNode* node, tgt::vec3 sizing) {
    if (node->opaque) {
        mlg->addMesh(createCubeMesh(sizing,node->coordllf,node->coordurb));
    } else if (node->llf && node->lrf && node->ulf && node->urf &&
        node->llb && node->lrb && node->ulb && node->urb)
    {
        createGeometry(mlg, node->llf, sizing);
        createGeometry(mlg, node->lrf, sizing);
        createGeometry(mlg, node->ulf, sizing);
        createGeometry(mlg, node->urf, sizing);
        createGeometry(mlg, node->llb, sizing);
        createGeometry(mlg, node->lrb, sizing);
        createGeometry(mlg, node->ulb, sizing);
        createGeometry(mlg, node->urb, sizing);
    }
}

/**
* End MinMaxOctree
*/

/**
* Begin MinMaxConvex
*/

void OptimizedProxyGeometry::updateVolPartOpaque(PropertyVolume* pvol, tgt::ivec3 pos, tgt::ivec3 /*llf*/, tgt::ivec3 /*urb*/) {
    if (minmaxtex_->getMax(pvol->getMinVox(pos),pvol->getMaxVox(pos),pvol->getMinGrad(pos),pvol->getMaxGrad(pos))<=threshold_.get()/100.f) {
        pvol->setOpaque(pos,false);
    } else {
        pvol->setOpaque(pos,true);
    }
}

void OptimizedProxyGeometry::setupVolPart(PropertyVolume* pvol, tgt::ivec3 pos, tgt::ivec3 llf, tgt::ivec3 urb) {
    const tgt::ivec3 dim = volvox_->getDimensions();
    float val;

    llf = tgt::max(llf,tgt::ivec3(0));
    llf = tgt::min(llf,dim - 1);

    urb = tgt::max(urb,tgt::ivec3(0));
    urb = tgt::min(urb,dim - 1);

    pvol->setBound(pos,false);
    pvol->setOpaque(pos,true);
    pvol->setMinVox(pos,1.f);
    pvol->setMaxVox(pos,0.f);
    pvol->setMinGrad(pos,1.f);
    pvol->setMaxGrad(pos,0.f);

    for(int x=llf.x; x<=urb.x; x++) {
        for(int y=llf.y; y<=urb.y; y++) {
            for(int z=llf.z; z<=urb.z; z++) {
                val = volvox_->getRepresentation<VolumeRAM>()->getVoxelNormalized(tgt::ivec3(x,y,z));
                if (val<pvol->getMinVox(pos)) pvol->setMinVox(pos,val);
                if (val>pvol->getMaxVox(pos)) pvol->setMaxVox(pos,val);
                if (use2DTF_) {
                    val = volgradmag_->getRepresentation<VolumeRAM>()->getVoxelNormalized(tgt::ivec3(x,y,z));
                    if (val<pvol->getMinGrad(pos)) pvol->setMinGrad(pos,val);
                    if (val>pvol->getMaxGrad(pos)) pvol->setMaxGrad(pos,val);
                }
            }
        }
    }
}

/**
* End MinMaxConvex
*/

/**
* Begin Convex
*/

void OptimizedProxyGeometry::setEnclosedOpaque(PropertyVolume* pvol) {
    const tgt::ivec3 size = pvol->getSize();
    tgt::ivec3 pos;

    setVolBound(pvol,tgt::ivec3(0),pvol->getSize()-1,false);

    for (VolumeIterator it(size); !it.outofrange(); it.next()) {
        pos = it.value();
        if (!pvol->getOpaque(pos) && !pvol->getBound(pos)) {
            if (isEnclosed(pvol,pos)) {
                setOpaque(pvol,pos);
            }
        }
    }
}

bool OptimizedProxyGeometry::isEnclosed(PropertyVolume* pvol, tgt::ivec3 pos) {
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
        if (!pvol->getOpaque(qpos)) {if (!pvol->getBound(qpos)) {pvol->setBound(qpos,true);

        for (int i=0; i<6; i++) {
            ipos = qpos + tpos[i];
            tless = tgt::lessThan(ipos,tgt::ivec3(0));
            tgreater = tgt::greaterThan(ipos,pvol->getSize()-1);
            if (tless.x || tless.y || tless.z || tgreater.x || tgreater.y || tgreater.z) enclosed = false;
            else q.push(ipos);
        }

        }}
    }

    return enclosed;
}

void OptimizedProxyGeometry::setOpaque(PropertyVolume *pvol, tgt::ivec3 pos) {
    const tgt::ivec3 tpos[6] = {tgt::ivec3(0,0,-1)
                         ,tgt::ivec3(0,0,1)
                         ,tgt::ivec3(-1,0,0)
                         ,tgt::ivec3(1,0,0)
                         ,tgt::ivec3(0,-1,0)
                         ,tgt::ivec3(0,1,0)};

    std::queue<tgt::ivec3> q;
    tgt::ivec3 qpos;

    q.push(pos);
    while (!q.empty()) {
        qpos = q.front(); q.pop();
        if (!pvol->getOpaque(qpos)) {

        pvol->setOpaque(qpos,true);
        for (int i=0; i<6; i++) {q.push(qpos + tpos[i]);}

        }
    }
}

/**
* End Convex
*/

/**
* Begin BordersOnly
*/

VertexGeometry OptimizedProxyGeometry::createVertexGeometry(tgt::vec3 sizing, tgt::vec3 voxcoords) {
    tgt::vec3 glcoords;
    tgt::vec3 glcolors;

    voxcoords = tgt::max(voxcoords,tgt::vec3(0.f));
    voxcoords = tgt::min(voxcoords,tgt::vec3(1.f));

    glcoords = 2.f * voxcoords - tgt::vec3(1.f);
    //glcoords.z = - glcoords.z;

    glcolors = voxcoords;
    //glcolors.z = 1.f - glcolors.z;

    glcoords *= sizing;

    VertexGeometry vg(glcoords,glcolors,tgt::vec4(glcolors,1.f));

    return vg;
}

FaceGeometry OptimizedProxyGeometry::createFaceGeometry(tgt::vec3 sizing, tgt::ivec3 dim, FaceDirection dir, tgt::ivec3 llf, tgt::ivec3 urb) {
    tgt::vec3 llfs,urbs;
    tgt::vec3 ll,lr,ur,ul;
    const tgt::vec3 vec3dim = tgt::vec3(dim);
    const tgt::vec3 spacing(0.5f);
    FaceGeometry face;

    llfs = tgt::vec3(llf) - spacing - (float)spacing_.get();
    urbs = tgt::vec3(urb) - spacing + (float)spacing_.get();

    switch(dir) {
        case Left:
            ll = tgt::vec3(llfs.x,llfs.y,urbs.z);
            lr = tgt::vec3(llfs.x,llfs.y,llfs.z);
            ur = tgt::vec3(llfs.x,urbs.y,llfs.z);
            ul = tgt::vec3(llfs.x,urbs.y,urbs.z);
            break;
        case Right:
            ll = tgt::vec3(urbs.x,llfs.y,llfs.z);
            lr = tgt::vec3(urbs.x,llfs.y,urbs.z);
            ur = tgt::vec3(urbs.x,urbs.y,urbs.z);
            ul = tgt::vec3(urbs.x,urbs.y,llfs.z);
            break;
        case Bottom:
            ll = tgt::vec3(llfs.x,llfs.y,urbs.z);
            lr = tgt::vec3(urbs.x,llfs.y,urbs.z);
            ur = tgt::vec3(urbs.x,llfs.y,llfs.z);
            ul = tgt::vec3(llfs.x,llfs.y,llfs.z);
            break;
        case Top:
            ll = tgt::vec3(llfs.x,urbs.y,llfs.z);
            lr = tgt::vec3(urbs.x,urbs.y,llfs.z);
            ur = tgt::vec3(urbs.x,urbs.y,urbs.z);
            ul = tgt::vec3(llfs.x,urbs.y,urbs.z);
            break;
        case Front:
            ll = tgt::vec3(llfs.x,llfs.y,llfs.z);
            lr = tgt::vec3(urbs.x,llfs.y,llfs.z);
            ur = tgt::vec3(urbs.x,urbs.y,llfs.z);
            ul = tgt::vec3(llfs.x,urbs.y,llfs.z);
            break;
        case Back:
            ll = tgt::vec3(urbs.x,llfs.y,urbs.z);
            lr = tgt::vec3(llfs.x,llfs.y,urbs.z);
            ur = tgt::vec3(llfs.x,urbs.y,urbs.z);
            ul = tgt::vec3(urbs.x,urbs.y,urbs.z);
            break;
    }

    face.addVertex(createVertexGeometry(sizing,ul/vec3dim));
    face.addVertex(createVertexGeometry(sizing,ur/vec3dim));
    face.addVertex(createVertexGeometry(sizing,lr/vec3dim));
    face.addVertex(createVertexGeometry(sizing,ll/vec3dim));

    return face;
}

/**
* End BordersOnly
*/

} // namespace
