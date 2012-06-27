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

#include "voreen/modules/opencl/raytracingentryexitpoints.h"

#include "voreen/core/interaction/camerainteractionhandler.h"

#include "tgt/glmath.h"
#include "tgt/gpucapabilities.h"

using tgt::vec3;
using tgt::mat4;

namespace voreen {

using namespace cl;

const std::string RaytracingEntryExitPoints::loggerCat_("voreen.RaytracingEntryExitPoints");

/*
    constructor
*/
RaytracingEntryExitPoints::RaytracingEntryExitPoints()
    : RenderProcessor(),
      useFloatRenderTargets_("useFloatRenderTargets", "Use float rendertargets", false),
      camera_("camera", "Camera", tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f))),
      opencl_(0),
      context_(0),
      queue_(0),
      prog_(0),
      entryPort_(Port::OUTPORT, "image.entrypoints"),
      exitPort_(Port::OUTPORT, "image.exitpoints"),
      inport_(Port::INPORT, "proxgeometry.geometry")
{
    addProperty(useFloatRenderTargets_);
    addProperty(camera_);

    cameraHandler_ = new CameraInteractionHandler("cameraHandler", "Camera Handler", &camera_);
    addInteractionHandler(cameraHandler_);

    addPort(entryPort_);
    addPort(exitPort_);
    addPort(inport_);
}

std::string RaytracingEntryExitPoints::getProcessorInfo() const {
    return "Raytraces the Proxygeometry to create Entry/Exit points."
           "The generated image color-codes the ray parameters for a subsequent VolumeRaycaster.<br/>"
           "See CubeMeshProxyGeometry.";
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

void RaytracingEntryExitPoints::initialize() throw (VoreenException) {
    //TODO: Central OpenCL init in voreen
    RenderProcessor::initialize();

    opencl_ = new OpenCL();

    const std::vector<Platform>&  platforms = opencl_->getPlatforms();
    if(platforms.size() == 0) {
        LERROR("Found no OpenCL platforms!");
        return;
    }

    const std::vector<Device*>& devices = platforms[0].getDevices();
    if(devices.size() == 0) {
        LERROR("Found no devices in platform!");
        return;
    }

    context_ = new Context(Context::generateGlSharingProperties(), devices[0]);
    queue_ = new CommandQueue(context_, devices.back());

    prog_ = new Program(context_);

    prog_->loadSource("../../src/modules/opencl/raytracingentryexitpoints.cl");

    prog_->build(devices.back());

    initialized_ = true;
}

bool RaytracingEntryExitPoints::isReady() const {
    if(entryPort_.isReady() && exitPort_.isReady() && inport_.isReady()) {
        if(dynamic_cast<MeshListGeometry*>(inport_.getData()))
            return true;
        else {
            LERROR("Geometry of type MeshListGeometry expected.");
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
    float4 a;
    float4 b;
    float4 c;
    float4 a_tc;
    float4 b_tc;
    float4 c_tc;
};

void RaytracingEntryExitPoints::process() {
    MeshListGeometry* inportGeometry = static_cast<MeshListGeometry*>(inport_.getData()); //checked in isReady

    if(prog_) {
        Kernel* k = prog_->getKernel("raytrace");
        if(k) {
            LGL_ERROR;
            glFinish();

            std::vector<triangle> triangles;
            for(size_t i=0; i<inportGeometry->getMeshCount(); i++) {
                MeshGeometry& mesh = inportGeometry->getMesh(i);

                for(size_t j=0; j<mesh.getFaceCount(); ++j) {
                    FaceGeometry& fg = mesh.getFace(j);

                    if((fg.getVertexCount() < 3) || (fg.getVertexCount() > 5)) {
                        LWARNING("Skipping face, unhandled number of vertices: " << fg.getVertexCount());
                        continue;
                    }

                    triangle t;

                    t.a = tgt::vec4(fg.getVertex(0).getCoords(), 0.0f);
                    t.b = tgt::vec4(fg.getVertex(1).getCoords(), 0.0f);
                    t.c = tgt::vec4(fg.getVertex(2).getCoords(), 0.0f);
                    t.a_tc = tgt::vec4(fg.getVertex(0).getTexCoords(), 0.0f);
                    t.b_tc = tgt::vec4(fg.getVertex(1).getTexCoords(), 0.0f);
                    t.c_tc = tgt::vec4(fg.getVertex(2).getTexCoords(), 0.0f);
                    triangles.push_back(t);

                    if(fg.getVertexCount() == 4) {
                        t.a = tgt::vec4(fg.getVertex(2).getCoords(), 0.0f);
                        t.b = tgt::vec4(fg.getVertex(3).getCoords(), 0.0f);
                        t.c = tgt::vec4(fg.getVertex(0).getCoords(), 0.0f);
                        t.a_tc = tgt::vec4(fg.getVertex(2).getTexCoords(), 0.0f);
                        t.b_tc = tgt::vec4(fg.getVertex(3).getTexCoords(), 0.0f);
                        t.c_tc = tgt::vec4(fg.getVertex(0).getTexCoords(), 0.0f);
                        triangles.push_back(t);
                    }

                    if(fg.getVertexCount() == 5) {
                        t.a = tgt::vec4(fg.getVertex(3).getCoords(), 0.0f);
                        t.b = tgt::vec4(fg.getVertex(4).getCoords(), 0.0f);
                        t.c = tgt::vec4(fg.getVertex(0).getCoords(), 0.0f);
                        t.a_tc = tgt::vec4(fg.getVertex(3).getTexCoords(), 0.0f);
                        t.b_tc = tgt::vec4(fg.getVertex(4).getTexCoords(), 0.0f);
                        t.c_tc = tgt::vec4(fg.getVertex(0).getTexCoords(), 0.0f);
                        triangles.push_back(t);
                    }
                }
            }

            SharedTexture entry(context_, CL_MEM_WRITE_ONLY, entryPort_.getColorTexture());
            SharedTexture exit(context_, CL_MEM_WRITE_ONLY, exitPort_.getColorTexture());
            Buffer inBuffer(context_, CL_MEM_READ_ONLY, sizeof(triangle) * triangles.size());

            float ratio = (float)entryPort_.getSize().x / (float)entryPort_.getSize().y;
            float h = tan((camera_.get().getFovy() * 0.5f / 360.0f) * 2.0f * tgt::PIf);
            float w = h * ratio;
            vec3 up = camera_.get().getUpVector() * h;
            vec3 strafe = camera_.get().getStrafe() * w;

            k->setArg(0, entry);
            k->setArg(1, exit);
            k->setArg(2, inBuffer);
            k->setArg(3, static_cast<int>(triangles.size()));
            k->setArg(4, camera_.get().getPosition());
            k->setArg(5, up);
            k->setArg(6, camera_.get().getLook());
            k->setArg(7, strafe);

            queue_->enqueueWrite(&inBuffer, &triangles[0]);
            queue_->enqueueAcquireGLObject(&entry);
            queue_->enqueueAcquireGLObject(&exit);
            queue_->enqueue(k, entryPort_.getSize());
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
