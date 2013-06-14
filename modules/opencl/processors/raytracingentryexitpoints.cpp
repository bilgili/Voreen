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

#include "raytracingentryexitpoints.h"

#include "voreen/core/interaction/camerainteractionhandler.h"
#include "voreen/core/voreenapplication.h"

#include "tgt/glmath.h"
#include "tgt/gpucapabilities.h"

using tgt::vec3;
using tgt::mat4;

namespace voreen {

using namespace cl;

const std::string RaytracingEntryExitPoints::loggerCat_("voreen.RaytracingEntryExitPoints");

RaytracingEntryExitPoints::RaytracingEntryExitPoints()
    : RenderProcessor(),
      useFloatRenderTargets_("useFloatRenderTargets", "Use float rendertargets", false),
      camera_("camera", "Camera", tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f))),
      opencl_(0),
      context_(0),
      queue_(0),
      prog_(0),
      entryPort_(Port::OUTPORT, "image.entrypoints", "Entry-points Output",  true, Processor::INVALID_PROGRAM, RenderPort::RENDERSIZE_RECEIVER),
      exitPort_(Port::OUTPORT, "image.exitpoints", "Exit-points Output",  true, Processor::INVALID_PROGRAM, RenderPort::RENDERSIZE_RECEIVER),
      inport_(Port::INPORT, "proxgeometry.geometry", "Proxy-geometry Input")
{
    addProperty(useFloatRenderTargets_);
    addProperty(camera_);

    cameraHandler_ = new CameraInteractionHandler("cameraHandler", "Camera Handler", &camera_);
    addInteractionHandler(cameraHandler_);

    addPort(entryPort_);
    addPort(exitPort_);
    addPort(inport_);
}

RaytracingEntryExitPoints::~RaytracingEntryExitPoints() {
    delete prog_;
    delete queue_;
    delete context_;
    delete opencl_;

    delete cameraHandler_;
}

Processor* RaytracingEntryExitPoints::create() const {
    return new RaytracingEntryExitPoints();
}

void RaytracingEntryExitPoints::initialize() throw (tgt::Exception) {
    //TODO: Central OpenCL init in voreen
    RenderProcessor::initialize();

    opencl_ = new OpenCL();

    const std::vector<Platform>&  platforms = opencl_->getPlatforms();
    if(platforms.size() == 0) {
        LERROR("Found no OpenCL platforms!");
        return;
    }

    const std::vector<Device>& devices = platforms[0].getDevices();
    if(devices.size() == 0) {
        LERROR("Found no devices in platform!");
        return;
    }

    context_ = new Context(Context::generateGlSharingProperties(), devices[0]);
    queue_ = new CommandQueue(context_, devices.back());

    prog_ = new Program(context_);

    prog_->loadSource(VoreenApplication::app()->getModulePath("opencl") + "/cl/raytracingentryexitpoints.cl");

    prog_->build(devices.back());

    processorState_ = PROCESSOR_STATE_NOT_READY;
}

bool RaytracingEntryExitPoints::isReady() const {
    if(entryPort_.isReady() && exitPort_.isReady() && inport_.isReady()) {
        if(dynamic_cast<const TriangleMeshGeometryVec3*>(inport_.getData()))
            return true;
        else {
            LERROR("Geometry of type TriangleMeshGeometryVec3 expected.");
        }
    }
    return false;
}

void RaytracingEntryExitPoints::beforeProcess() {
    RenderProcessor::beforeProcess();

    if(useFloatRenderTargets_.get()) {
        if(entryPort_.getRenderTarget()->getColorTexture()->getDataType() != GL_FLOAT) {
            entryPort_.changeFormat(GL_RGBA16F_ARB);
            exitPort_.changeFormat(GL_RGBA16F_ARB);
        }
    }
    else {
        if(entryPort_.getRenderTarget()->getColorTexture()->getDataType() == GL_FLOAT) {
            entryPort_.changeFormat(GL_RGBA16);
            exitPort_.changeFormat(GL_RGBA16);
        }
    }
}

struct triangle {
    cl_float4 a;
    cl_float4 b;
    cl_float4 c;
    cl_float4 a_tc;
    cl_float4 b_tc;
    cl_float4 c_tc;
};

void RaytracingEntryExitPoints::process() {
    //const MeshListGeometry* inportGeometry = static_cast<const MeshListGeometry*>(inport_.getData());
    const TriangleMeshGeometryVec3* inportGeometry = dynamic_cast<const TriangleMeshGeometryVec3*> (inport_.getData()); //checked in isReady

    if(prog_) {
        Kernel* k = prog_->getKernel("raytrace");
        if(k) {
            LGL_ERROR;
            glFinish();

            std::vector<triangle> triangles;
            for(size_t i=0; i<inportGeometry->getNumTriangles(); i++) {
                triangle t;

                t.a.s[0] = inportGeometry->getTriangle(i).v_[0].pos_.x;
                t.a.s[1] = inportGeometry->getTriangle(i).v_[0].pos_.y;
                t.a.s[2] = inportGeometry->getTriangle(i).v_[0].pos_.z;
                t.a.s[3] = 1.0f;

                t.b.s[0] = inportGeometry->getTriangle(i).v_[1].pos_.x;
                t.b.s[1] = inportGeometry->getTriangle(i).v_[1].pos_.y;
                t.b.s[2] = inportGeometry->getTriangle(i).v_[1].pos_.z;
                t.b.s[3] = 1.0f;

                t.c.s[0] = inportGeometry->getTriangle(i).v_[2].pos_.x;
                t.c.s[1] = inportGeometry->getTriangle(i).v_[2].pos_.y;
                t.c.s[2] = inportGeometry->getTriangle(i).v_[2].pos_.z;
                t.c.s[3] = 1.0f;

                t.a_tc.s[0] = inportGeometry->getTriangle(i).v_[0].attr1_.x;
                t.a_tc.s[1] = inportGeometry->getTriangle(i).v_[0].attr1_.y;
                t.a_tc.s[2] = inportGeometry->getTriangle(i).v_[0].attr1_.z;
                t.a_tc.s[3] = 1.0f;

                t.b_tc.s[0] = inportGeometry->getTriangle(i).v_[1].attr1_.x;
                t.b_tc.s[1] = inportGeometry->getTriangle(i).v_[1].attr1_.y;
                t.b_tc.s[2] = inportGeometry->getTriangle(i).v_[1].attr1_.z;
                t.b_tc.s[3] = 1.0f;

                t.c_tc.s[0] = inportGeometry->getTriangle(i).v_[2].attr1_.x;
                t.c_tc.s[1] = inportGeometry->getTriangle(i).v_[2].attr1_.y;
                t.c_tc.s[2] = inportGeometry->getTriangle(i).v_[2].attr1_.z;
                t.c_tc.s[3] = 1.0f;

                triangles.push_back(t);
            }

            SharedTexture entry(context_, CL_MEM_WRITE_ONLY, entryPort_.getColorTexture());
            SharedTexture exit(context_, CL_MEM_WRITE_ONLY, exitPort_.getColorTexture());
            Buffer inBuffer(context_, CL_MEM_READ_ONLY, sizeof(triangle) * triangles.size());
            Buffer transformationMatrixBuffer (context_, CL_MEM_READ_ONLY, sizeof(tgt::mat4));

            float ratio = (float)entryPort_.getSize().x / (float)entryPort_.getSize().y;
            float h = tan((camera_.get().getFovy() * 0.5f / 360.0f) * 2.0f * tgt::PIf);
            float w = h * ratio;
            vec3 up = camera_.get().getUpVector() * h;
            vec3 strafe = camera_.get().getStrafe() * w;
            tgt::mat4 tM = inportGeometry->getTransformationMatrix();


            k->setArg(0, entry);
            k->setArg(1, exit);
            k->setArg(2, inBuffer);
            k->setArg(3, static_cast<int>(triangles.size()));
            k->setArg(4, camera_.get().getPosition());
            k->setArg(5, up);
            k->setArg(6, camera_.get().getLook());
            k->setArg(7, strafe);
            k->setArg(8, transformationMatrixBuffer);

            queue_->enqueueWriteBuffer(&transformationMatrixBuffer, &tM);
            queue_->enqueueWriteBuffer(&inBuffer, &triangles[0]);
            queue_->enqueueAcquireGLObject(&entry);
            queue_->enqueueAcquireGLObject(&exit);
            queue_->enqueue(k, exitPort_.getSize());
            queue_->enqueueReleaseGLObject(&exit);
            queue_->enqueueReleaseGLObject(&entry);

            queue_->finish();

            entryPort_.validateResult();
            exitPort_.validateResult();
            LGL_ERROR;
        }
    }
}

} // namespace voreen
