/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/processors/render/proxygeometry.h"
#include "voreen/core/vis/processors/portmapping.h"

#include "voreen/core/vis/messagedistributor.h"

#include "voreen/core/volume/volumegl.h"
#include "voreen/core/volume/volumeseries.h"
#include "voreen/core/volume/volumeset.h"
#include "voreen/core/volume/volumesetcontainer.h"

#include "tgt/plane.h"
#include "tgt/quadric.h"
#include "tgt/glmath.h"

namespace voreen {

using tgt::vec3;
using tgt::vec4;

const Identifier ProxyGeometry::setUseClipping_("set.UseClipping");
const Identifier ProxyGeometry::setLeftClipPlane_("set.LeftClipPlane");
const Identifier ProxyGeometry::setRightClipPlane_("set.RightClipPlane");
const Identifier ProxyGeometry::setTopClipPlane_("set.TopClipPlane");
const Identifier ProxyGeometry::setBottomClipPlane_("set.BottomClipPlane");
const Identifier ProxyGeometry::setFrontClipPlane_("set.FrontClipPlane");
const Identifier ProxyGeometry::setBackClipPlane_("set.BackClipPlane");
const Identifier ProxyGeometry::resetClipPlanes_("reset.clipPlanes");
const Identifier ProxyGeometry::getVolumeSize_("get.volumeSize");

ProxyGeometry::ProxyGeometry()
    : VolumeRenderer(),
      needsBuild_(true),
      volumeSize_(tgt::vec3::zero),
      volume_(0)
{}

int ProxyGeometry::initializeGL() {
    return VRN_OK;
}

void ProxyGeometry::setVolumeHandle(VolumeHandle* const handle) {
    VolumeRenderer::setVolumeHandle(handle);
    if ( currentVolumeHandle_ == 0 ) {
        volume_ = 0;
        return;
    }

    //volume_ = currentVolumeHandle_->getVolume();
	volume_ = currentVolumeHandle_->getVolumeGL()->getVolume();
    if (volume_ != 0) {
        needsBuild_ = true;
        volumeSize_ = volume_->getCubeSize() / 2.f;
    }
}

void ProxyGeometry::process(LocalPortMapping* portMapping) {
    VolumeHandle* volumeHandle = portMapping->getVolumeHandle("volumehandle.volumehandle");

    if (volumeHandle != currentVolumeHandle_)
        setVolumeHandle(volumeHandle);    
}

Message* ProxyGeometry::call(Identifier ident, LocalPortMapping* /*portMapping*/) {
	if (ident == "render") {
		render();
		return 0;
	}
    else if (ident == getVolumeSize_) {
        return new Vec3Msg("", getVolumeSize());
    }
    else
        return 0;
}

tgt::vec3 ProxyGeometry::getVolumeSize() {
	return volumeSize_;
}

//---------------------------------------------------------------------------

//TODO: RPTMERGE: reintroduce setting of useVirtualClipplane (deleted from constructor)

CubeProxyGeometry::CubeProxyGeometry()
  : ProxyGeometry(),
    useClipping_(setUseClipping_, "Use Clipping", true),
    clipLeftX_(setLeftClipPlane_, "Left Clippingplane", 0, 0, 100, true),
    clipRightX_(setRightClipPlane_, "Right Clippingplane", 0, 0, 100, true),
    clipUpY_(setTopClipPlane_, "Top Clippingplane", 0, 0, 100, true),
    clipDownY_(setBottomClipPlane_, "Bottom Clippingplane", 0, 0, 100, true),
    clipFrontZ_(setFrontClipPlane_, "Front Clippingplane", 0, 0, 100, true),
    clipBackZ_(setBackClipPlane_, "Back Clippingplane", 0, 0, 100, true),
    dl_(0),
    useVirtualClipplane_("switch.virtualClipplane", "Use Virtual Clipplane", false),
    clipPlane_("set.virtualClipplane", "Plane equation", vec4(1.f/5.f, 2.f/5.f, 1.f, 0.3f),
               tgt::vec4(-10.f), tgt::vec4(10.f))
{
    setName("Cube-ProxyGeometry");
    addProperty(&useClipping_);
    addProperty(&useVirtualClipplane_);
    addProperty(new ConditionProp("virtualClipPlaneCond", &useVirtualClipplane_));
    clipPlane_.setConditioned("virtualClipPlaneCond", 1);
    addProperty(&clipPlane_);

    addProperty(&clipLeftX_);
    addProperty(&clipRightX_);
    addProperty(&clipDownY_);
    addProperty(&clipUpY_);
    addProperty(&clipFrontZ_);
    addProperty(&clipBackZ_);

    clipplaneGroup_ = new GroupProp("group.clipPlanesSlider","Clipping Planes");
    addProperty(clipplaneGroup_);

    clipLeftX_.setGrouped("group.clipPlanesSlider");
    clipRightX_.setGrouped("group.clipPlanesSlider");
    clipDownY_.setGrouped("group.clipPlanesSlider");
    clipUpY_.setGrouped("group.clipPlanesSlider");
    clipFrontZ_.setGrouped("group.clipPlanesSlider");
    clipBackZ_.setGrouped("group.clipPlanesSlider");

    createInport("volumehandle.volumehandle");
	createCoProcessorOutport("coprocessor.proxygeometry", &Processor::call);

	setIsCoprocessor(true);
}

const std::string CubeProxyGeometry::getProcessorInfo() const {
	return "Provides a simple cube proxy. The resulting geometry depends on the ratio of the values in dim.";
}

void CubeProxyGeometry::setPropertyDestination(Identifier tag) {
    MessageReceiver::setTag(tag);
    clipLeftX_.setMsgDestination(tag);
    clipRightX_.setMsgDestination(tag);
    clipDownY_.setMsgDestination(tag);
    clipUpY_.setMsgDestination(tag);
    clipFrontZ_.setMsgDestination(tag);
    clipBackZ_.setMsgDestination(tag);
    clipPlane_.setMsgDestination(tag);
    useVirtualClipplane_.setMsgDestination(tag);
    clipplaneGroup_->setMsgDestination(tag);
    useClipping_.setMsgDestination(tag);
    MsgDistr.insert(this);
}

CubeProxyGeometry::~CubeProxyGeometry() {
    if (dl_)
        glDeleteLists(dl_, 1);

    if (MessageReceiver::getTag() != Message::all_)
        MsgDistr.remove(this);

    delete clipplaneGroup_;
}

bool CubeProxyGeometry::getUseVirtualClipplane() {
    return useVirtualClipplane_.get();
}

/**
 * Renders the OpenGL list (and creates it, when needed).
 */
void CubeProxyGeometry::render() {
    if (volume_) {
        if (needsBuild_) {
            if (!dl_)
                dl_ = glGenLists(1);
            
            revalidateCubeGeometry();
            needsBuild_ = false;
        }
        glCallList(dl_);
    }
}

tgt::vec3 CubeProxyGeometry::getClipPlaneLDF() {
    return tgt::vec3(static_cast<float>(clipLeftX_.get()),
        static_cast<float>(clipDownY_.get()),
        static_cast<float>(clipFrontZ_.get()) );
}

tgt::vec3 CubeProxyGeometry::getClipPlaneRUB() {
    return tgt::vec3(static_cast<float>(clipRightX_.get()),
        static_cast<float>(clipUpY_.get()),
        static_cast<float>(clipBackZ_.get()) );
}

void CubeProxyGeometry::revalidateCubeGeometry() {
    vec3 geomLlf = -volumeSize_;
    vec3 geomUrb = volumeSize_;

	if (useClipping_.get()) {
		// clipping along the xyz axes
    	geomLlf[0] = geomLlf[0] + clipLeftX_.get()/50.0f*volumeSize_.x;
    	geomLlf[1] = geomLlf[1] + clipDownY_.get()/50.0f*volumeSize_.y;
    	geomLlf[2] = geomLlf[2] + clipFrontZ_.get()/50.0f*volumeSize_.z;

    	geomUrb[0] = geomUrb[0] - clipRightX_.get()/50.0f*volumeSize_.x;
    	geomUrb[1] = geomUrb[1] - clipUpY_.get()/50.0f*volumeSize_.y;
    	geomUrb[2] = geomUrb[2] - clipBackZ_.get()/50.0f*volumeSize_.z;
	}

    // recreate display list
    glNewList(dl_, GL_COMPILE);

    if (useVirtualClipplane_.get()) {
        tgt::plane clipPlane( normalize(vec3(clipPlane_.get().elem)), clipPlane_.get().w);

		std::vector<vec3> clippedPolygon;
		clipPlane.clipAAB(geomLlf, geomUrb, clippedPolygon);

		// draw patch
		glBegin(GL_POLYGON);
        for (size_t i = 0; i < clippedPolygon.size(); ++i)
            glVertex3fv(clippedPolygon[i].elem);
        glEnd();

		// set clip plane in OpenGL
		glEnable(GL_CLIP_PLANE0);
		tgt::setClipPlane(GL_CLIP_PLANE0, clipPlane);
	}

    glBegin(GL_QUADS);
		// back face
        glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
        glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
        glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
        glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);
        // front face
        glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);
        glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);
        glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);
        glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
        // top face
        glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
        glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);
        glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
        glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);
        // bottom face
        glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
        glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);
        glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);
        glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
        // right face
        glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
        glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);
        glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
        glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);
        // left face
        glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);
        glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);
        glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
        glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
	glEnd();

    if (useVirtualClipplane_.get())
        glDisable(GL_CLIP_PLANE0);

    glEndList();

    LGL_ERROR;
}

void CubeProxyGeometry::resetClippingPlanes() {
    clipLeftX_.set(0);
    clipRightX_.set(0);
    clipFrontZ_.set(0);
    clipBackZ_.set(0);
    clipDownY_.set(0);
    clipUpY_.set(0);
}

void CubeProxyGeometry::processMessage(Message *msg, const Identifier& dest) {
    ProxyGeometry::processMessage(msg, dest);

    if (msg->id_ == setUseClipping_) {
		useClipping_.set(msg->getValue<bool>());
		needsBuild_ = true;
        invalidate();
	}
    else if (msg->id_ == setLeftClipPlane_) {
        clipLeftX_.set(msg->getValue<int>());
        if ((clipLeftX_.get() + clipRightX_.get()) >= 100) {
            clipRightX_.set(99-clipLeftX_.get());
        }
        needsBuild_ = true;
        invalidate();
    }
    else if (msg->id_ == setRightClipPlane_) {
        clipRightX_.set(msg->getValue<int>());
        if ((clipLeftX_.get() + clipRightX_.get()) >= 100) {
            clipLeftX_.set(99-clipRightX_.get());
        }
        needsBuild_ = true;
        invalidate();
    }
    else if (msg->id_ == setTopClipPlane_) {
        clipUpY_.set(msg->getValue<int>());
        if ((clipUpY_.get() + clipDownY_.get()) >= 100) {
            clipDownY_.set(99-clipUpY_.get());
        }
        needsBuild_ = true;
        invalidate();
    }
    else if (msg->id_ == setBottomClipPlane_) {
        clipDownY_.set(msg->getValue<int>());
        if ((clipUpY_.get() + clipDownY_.get()) >= 100) {
            clipUpY_.set(99-clipDownY_.get());
        }
        needsBuild_ = true;
        invalidate();
    }
    else if (msg->id_ == setFrontClipPlane_) {
        clipFrontZ_.set(msg->getValue<int>());
        if ((clipFrontZ_.get() + clipBackZ_.get()) >= 100) {
            clipBackZ_.set(99-clipFrontZ_.get());
        }
        needsBuild_ = true;
        invalidate();
    }
    else if (msg->id_ == setBackClipPlane_) {
        clipBackZ_.set(msg->getValue<int>());
        if ((clipBackZ_.get() + clipFrontZ_.get()) >= 100) {
            clipFrontZ_.set(99-clipBackZ_.get());
        }
        needsBuild_ = true;
        invalidate();
    }
    else if (msg->id_ == "switch.virtualClipplane") {
        useVirtualClipplane_.set(msg->getValue<bool>());
        needsBuild_ = true;
        invalidate();
    }
    else if (msg->id_ == "set.virtualClipplane") {
        clipPlane_.set(msg->getValue<vec4>());
        needsBuild_ = true;
        invalidate();
    }
    else if (msg->id_ == resetClipPlanes_) {
        resetClippingPlanes();
        needsBuild_ = true;
        invalidate();
    }
}

//---------------------------------------------------------------------------

CubeCutProxyGeometry::CubeCutProxyGeometry()
  : ProxyGeometry(),
    cutCube_("switch.cutCube", "Render cut cube", false),
    cubeSize_("set.cutCubeSize", "Cutted cube size", vec3(25.f, 50.f, 75.f), vec3(0.f), vec3(100.f)),
    dl_(0)
{
    setName("CubeCut-ProxyGeometry");
    addProperty(&cutCube_);
    addProperty(&cubeSize_);

	createInport("volumehandle.volumehandle");
	createCoProcessorOutport("coprocessor.proxygeometry",&Processor::call);

	setIsCoprocessor(true);
}

const std::string CubeCutProxyGeometry::getProcessorInfo() const {
	return "Provides a simple clipping cube proxy.";
}

Message* CubeCutProxyGeometry::call(Identifier ident, LocalPortMapping* /*portMapping*/) {
	if (ident == "render") {
        needsBuild_ = true;
        render();
	}
	return 0;
}

void CubeCutProxyGeometry::setPropertyDestination(Identifier tag) {
    MessageReceiver::setTag(tag);
    cutCube_.setMsgDestination(tag);
    cubeSize_.setMsgDestination(tag);
    MsgDistr.insert(this);
}

CubeCutProxyGeometry::~CubeCutProxyGeometry() {
    if (dl_)
        glDeleteLists(dl_, 1);

    if (MessageReceiver::getTag() != Message::all_)
        MsgDistr.remove(this);
}

/**
 * Renders the OpenGL list (and creates it, when needed).
 */
void CubeCutProxyGeometry::render() {
    if (volume_) {
        if (needsBuild_) {
            if (!dl_)
                dl_ = glGenLists(1);
            revalidateCubeGeometry();
            needsBuild_ = false;
        }
        glCallList(dl_);
    }
}

void CubeCutProxyGeometry::renderCubeWithCutting() {
    vec3 geomLlf = -volumeSize_;
    vec3 geomUrb = volumeSize_;
    vec3 cubeVec = (cubeSize_.get()/vec3(50.f)) - vec3(1.f);
    glNewList(dl_, GL_COMPILE);
    glBegin(GL_QUADS);
		// back face
        glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
        glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
        glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
        glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);
        // front face
        //o---oo---o   y
        //! 2 !!   !   !
        //o---o! 1 !   !
        //     !   !   0---- x
        //     o---o
        glVertex3f(cubeVec[0], geomLlf[1], geomLlf[2]);
        glVertex3f(cubeVec[0], geomUrb[1], geomLlf[2]);
        glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
        glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);
     
        glVertex3f(geomLlf[0], cubeVec[1], geomLlf[2]);
        glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);
        glVertex3f(cubeVec[0], geomUrb[1], geomLlf[2]);
        glVertex3f(cubeVec[0], cubeVec[1], geomLlf[2]);

		glVertex3f(geomLlf[0], geomLlf[1], cubeVec[2]);
        glVertex3f(geomLlf[0], cubeVec[1], cubeVec[2]);
        glVertex3f(cubeVec[0], cubeVec[1], cubeVec[2]);
        glVertex3f(cubeVec[0], geomLlf[1], cubeVec[2]);

        // top face
        glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
        glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);
        glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
        glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);

        // bottom face
        //      o---oo---o   0--- x
        //     / 2 //   /   /
        //    o---o/ 1 /   /
        //        /   /   z
        //       o---o
        glVertex3f(cubeVec[0], geomLlf[1], geomUrb[2]);
        glVertex3f(cubeVec[0], geomLlf[1], geomLlf[2]);
        glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);
        glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
        
        glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
        glVertex3f(geomLlf[0], geomLlf[1], cubeVec[2]);
        glVertex3f(cubeVec[0], geomLlf[1], cubeVec[2]);
        glVertex3f(cubeVec[0], geomLlf[1], geomUrb[2]);

		glVertex3f(geomLlf[0], cubeVec[1], cubeVec[2]);
        glVertex3f(geomLlf[0], cubeVec[1], geomLlf[2]); 
        glVertex3f(cubeVec[0], cubeVec[1], geomLlf[2]);
        glVertex3f(cubeVec[0], cubeVec[1], cubeVec[2]);
        
        // right face
        glVertex3f(geomUrb[0], geomLlf[1], geomUrb[2]);
        glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]);
        glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]);
        glVertex3f(geomUrb[0], geomUrb[1], geomUrb[2]);

        // left face
        //         o
        //        /!
        //       / !         y
        //      o  !         !
        //     o!1 !         !
        //    /!!  o         !
        //   / !!  /         0
        //  /  o! /         /
        // o 2/ !/         /
        // ! /  o         /
        // !/            z  
        // o
        glVertex3f(geomLlf[0], cubeVec[1], geomUrb[2]);
        glVertex3f(geomLlf[0], geomUrb[1], geomUrb[2]);
        glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]);
        glVertex3f(geomLlf[0], cubeVec[1], geomLlf[2]);
        
        glVertex3f(geomLlf[0], geomLlf[1], cubeVec[2]);
        glVertex3f(geomLlf[0], geomLlf[1], geomUrb[2]);
        glVertex3f(geomLlf[0], cubeVec[1], geomUrb[2]);
        glVertex3f(geomLlf[0], cubeVec[1], cubeVec[2]);

		glVertex3f(cubeVec[0], geomLlf[1], geomLlf[2]);
        glVertex3f(cubeVec[0], geomLlf[1], cubeVec[2]);
        glVertex3f(cubeVec[0], cubeVec[1], cubeVec[2]);
        glVertex3f(cubeVec[0], cubeVec[1], geomLlf[2]);
        
	glEnd();
    glEndList();
}

void CubeCutProxyGeometry::renderCuttedCube() {
    vec3 geomLlf = -volumeSize_;
    vec3 geomUrb = volumeSize_;
    vec3 cubeVec = (cubeSize_.get()/vec3(50.f)) - vec3(1.f);
    glNewList(dl_, GL_COMPILE);
    glBegin(GL_QUADS);
		// back face
	
        glVertex3f(geomLlf[0], cubeVec[1], cubeVec[2]);
        glVertex3f(geomLlf[0], geomLlf[1], cubeVec[2]);
        glVertex3f(cubeVec[0], geomLlf[1], cubeVec[2]);
        glVertex3f(cubeVec[0], cubeVec[1], cubeVec[2]);
        // front face
        glVertex3f(cubeVec[0], geomLlf[1], geomLlf[2]);
        glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);
        glVertex3f(geomLlf[0], cubeVec[1], geomLlf[2]);
        glVertex3f(cubeVec[0], cubeVec[1], geomLlf[2]);
        // top face
        glVertex3f(cubeVec[0], cubeVec[1], geomLlf[2]);
        glVertex3f(geomLlf[0], cubeVec[1], geomLlf[2]);
        glVertex3f(geomLlf[0], cubeVec[1], cubeVec[2]);
        glVertex3f(cubeVec[0], cubeVec[1], cubeVec[2]);
        // bottom face
        glVertex3f(geomLlf[0], geomLlf[1], cubeVec[2]);
        glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);
        glVertex3f(cubeVec[0], geomLlf[1], geomLlf[2]);
        glVertex3f(cubeVec[0], geomLlf[1], cubeVec[2]);
        // right face
        glVertex3f(cubeVec[0], geomLlf[1], cubeVec[2]);
        glVertex3f(cubeVec[0], geomLlf[1], geomLlf[2]);
        glVertex3f(cubeVec[0], cubeVec[1], geomLlf[2]);
        glVertex3f(cubeVec[0], cubeVec[1], cubeVec[2]);
        // left face
        glVertex3f(geomLlf[0], cubeVec[1], geomLlf[2]);
        glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]);
        glVertex3f(geomLlf[0], geomLlf[1], cubeVec[2]);
        glVertex3f(geomLlf[0], cubeVec[1], cubeVec[2]);
	glEnd();
	glEndList();
}

void CubeCutProxyGeometry::revalidateCubeGeometry() {
    if (cutCube_.get())
        renderCuttedCube();
    else
        renderCubeWithCutting();
	LGL_ERROR;

}

void CubeCutProxyGeometry::processMessage(Message *msg, const Identifier& dest) {
    ProxyGeometry::processMessage(msg, dest);
	if (msg->id_ == "set.cutCubeSize") {
		cubeSize_.set(msg->getValue<vec3>());
		needsBuild_ = true;
	}
	else if (msg->id_ == "switch.cutCube") {
		cutCube_.set(msg->getValue<bool>());
		needsBuild_ = true;
	}
}

} // namespace voreen
