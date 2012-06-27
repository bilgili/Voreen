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

#include "voreen/core/processors/renderprocessor.h"

#include "tgt/glmath.h"
#include "tgt/camera.h"
#include "tgt/shadermanager.h"
#include "tgt/gpucapabilities.h"

#include "voreen/core/network/networkevaluator.h"

#include "voreen/core/properties/cameraproperty.h"

#include <sstream>

using tgt::vec3;
using tgt::vec4;
using tgt::Color;
using std::map;

namespace voreen {

const std::string RenderProcessor::loggerCat_("voreen.RenderProcessor");

RenderProcessor::RenderProcessor()
    : Processor()
    , portResizeVisited_(false)
    , testSizeOriginVisited_(false)
{}

void RenderProcessor::initialize() throw (tgt::Exception) {

    Processor::initialize();

    const std::vector<RenderPort*> pports = getPrivateRenderPorts();
    for (size_t i=0; i<pports.size(); ++i) {
        pports[i]->initialize();
    }
    LGL_ERROR;
}

void RenderProcessor::deinitialize() throw (tgt::Exception) {

    const std::vector<RenderPort*> pports = getPrivateRenderPorts();
    for (size_t i=0; i<pports.size(); ++i) {
        pports[i]->deinitialize();
    }
    LGL_ERROR;

    Processor::deinitialize();
}

void RenderProcessor::invalidate(int inv) {

    Processor::invalidate(inv);

    if (inv == Processor::VALID)
        return;

    if (!isInitialized())
        return;

    // invalidate result of render ports
    for (size_t i=0; i<getOutports().size(); ++i) {
        RenderPort* renderPort = dynamic_cast<RenderPort*>(getOutports()[i]);
        if (renderPort)
            renderPort->invalidateResult();
    }
}

void RenderProcessor::sizeOriginChanged(RenderPort* p) {
    if (!p->getSizeOrigin()) {
        const std::vector<Port*> outports = getOutports();
        for (size_t i=0; i<outports.size(); ++i) {
            RenderPort* rp = dynamic_cast<RenderPort*>(outports[i]);
            if (rp) {
                if (rp->getSizeOrigin())
                    return;
            }
        }
    }

    const std::vector<Port*> inports = getInports();
    for (size_t i=0; i<inports.size(); ++i) {
        RenderPort* rp = dynamic_cast<RenderPort*>(inports[i]);
        if (rp)
            rp->sizeOriginChanged(p->getSizeOrigin());
    }
}

bool RenderProcessor::testSizeOrigin(const RenderPort* p, void* so) const {
    tgtAssert(p->isOutport(), "testSizeOrigin used with inport");

    // cycle prevention
    if (testSizeOriginVisited_)
        return true;
    testSizeOriginVisited_ = true;

    if (so) {
        const std::vector<Port*> outports = getOutports();
        for (size_t i=0; i<outports.size(); ++i) {
            if(p == outports[i])
                continue;
            RenderPort* rp = dynamic_cast<RenderPort*>(outports[i]);
            if (rp) {
                if (rp->getSizeOrigin() && (rp->getSizeOrigin() != so)) {
                    testSizeOriginVisited_ = false;
                    return false;
                }
            }
        }
    }

    const std::vector<Port*> inports = getInports();
    for (size_t i=0; i<inports.size(); ++i) {
        RenderPort* rp = dynamic_cast<RenderPort*>(inports[i]);
        if (rp) {
            if (rp->getSizeOrigin() && (rp->getSizeOrigin() != so) ) {
                testSizeOriginVisited_ = false;
                return false;
            }

            const std::vector<const Port*> connectedOutports = inports[i]->getConnected();
            for (size_t j=0; j<connectedOutports.size(); ++j) {
                const RenderPort* op = static_cast<const RenderPort*>(connectedOutports[j]);

                if (!static_cast<RenderProcessor*>(op->getProcessor())->testSizeOrigin(op, so)) {
                    testSizeOriginVisited_ = false;
                    return false;
                }
            }
        }
    }

    testSizeOriginVisited_ = false;

    return true;
}

void RenderProcessor::beforeProcess() {
    Processor::beforeProcess();

    tgtAssert(isInitialized(), "No initialized");
    manageRenderTargets();
    adjustRenderOutportDimensions();
}

void RenderProcessor::manageRenderTargets() {
    const std::vector<Port*> outports = getOutports();
    for (size_t i=0; i<outports.size(); ++i) {
        RenderPort* rp = dynamic_cast<RenderPort*>(outports[i]);
        if (rp && !rp->getRenderTargetSharing()) {
            if (rp->isConnected()) {
                if (!rp->hasRenderTarget()) {
                    rp->initialize();
                }
            }
            else {
                if (rp->hasRenderTarget()) {
                    rp->deinitialize();
                }
            }
        }
    }
}

void RenderProcessor::adjustRenderOutportDimensions() {

    // detect largest inport dimension
    tgt::ivec2 dim(-1);
    const std::vector<Port*> inports = getInports();
    for (size_t i=0; i<inports.size(); ++i) {
        RenderPort* rp = dynamic_cast<RenderPort*>(inports[i]);
        if (rp && rp->hasRenderTarget() && (rp->getSize().x >= dim.x && rp->getSize().y >= dim.y))
            dim = rp->getSize();
    }
    if (dim == tgt::ivec2(-1))
        return;

    // assign largest inport dimension to all render outports without size origin
    bool assigned = false;
    const std::vector<Port*> outports = getOutports();
    for (size_t i=0; i<outports.size(); ++i) {
        RenderPort* rp = dynamic_cast<RenderPort*>(outports[i]);
        if (rp && rp->isConnected() && (rp->getSizeOrigin() == 0)) {
            rp->resize(dim);
            assigned = true;
        }
    }

    // assign size to private ports
    if (assigned) {
        const std::vector<RenderPort*> privatePorts = getPrivateRenderPorts();
        for (size_t i=0; i<privatePorts.size(); ++i) {
            privatePorts[i]->resize(dim);
        }
    }

}

void RenderProcessor::portResized(RenderPort* /*p*/, tgt::ivec2 newsize) {

    // cycle prevention
    if (portResizeVisited_)
        return;

    portResizeVisited_ = true;

    // propagate to predecessing RenderProcessors
    const std::vector<Port*> inports = getInports();
    for(size_t i=0; i<inports.size(); ++i) {
        RenderPort* rp = dynamic_cast<RenderPort*>(inports[i]);
        if (rp)
            rp->resize(newsize);
    }

    //distribute to outports:
    const std::vector<Port*> outports = getOutports();
    for(size_t i=0; i<outports.size(); ++i) {
        RenderPort* rp = dynamic_cast<RenderPort*>(outports[i]);
        if (rp)
            rp->resize(newsize);
    }

    //distribute to private ports:
    const std::vector<RenderPort*> pports = getPrivateRenderPorts();
    for (size_t i=0; i<pports.size(); ++i) {
        RenderPort* rp = pports[i];
        rp->resize(newsize);
    }

    // notify camera properties about viewport change
    if(newsize != tgt::ivec2(0,0)) {
        const std::vector<Property*> properties = getProperties();
        for (size_t i=0; i<properties.size(); ++i) {
            CameraProperty* cameraProp = dynamic_cast<CameraProperty*>(properties[i]);
            if (cameraProp) {
                cameraProp->viewportChanged(newsize);
            }
        }
    }

    invalidate();

    portResizeVisited_ = false;
}

void RenderProcessor::addPrivateRenderPort(RenderPort* port) {
    port->setProcessor(this);
    privateRenderPorts_.push_back(port);

    map<std::string, Port*>::const_iterator it = portMap_.find(port->getName());
    if (it == portMap_.end())
        portMap_.insert(std::make_pair(port->getName(), port));
    else {
        LERROR("Port with name " << port->getName() << " has already been inserted!");
        tgtAssert(false, std::string("Port with name " + port->getName() + " has already been inserted").c_str());
    }
}

void RenderProcessor::addPrivateRenderPort(RenderPort& port) {
    addPrivateRenderPort(&port);
}

void RenderProcessor::renderQuad() {
    glDepthFunc(GL_ALWAYS);
    glBegin(GL_QUADS);
        glTexCoord2f(0.f, 0.f);
        glVertex2f(-1.f, -1.f);

        glTexCoord2f(1.f, 0.f);
        glVertex2f(1.f, -1.f);

        glTexCoord2f(1.f, 1.f);
        glVertex2f(1.f, 1.f);

        glTexCoord2f(0.f, 1.f);
        glVertex2f(-1.f, 1.f);
    glEnd();
    glDepthFunc(GL_LESS);
}

// Parameters currently set:
// - screenDim_
// - screenDimRCP_
// - cameraPosition_ (camera position in world coordinates)
void RenderProcessor::setGlobalShaderParameters(tgt::Shader* shader, const tgt::Camera* camera, tgt::ivec2 screenDim) {
    shader->setIgnoreUniformLocationError(true);

    if (screenDim == tgt::ivec2(-1)) {
        RenderPort* rp = 0;
        for (size_t i=0; i<getOutports().size(); ++i) {
            rp = dynamic_cast<RenderPort*>(getOutports()[i]);
            if (rp && rp->hasRenderTarget())
                break;
        }
        if (rp) {
            screenDim = rp->getSize();
        }
    }

    shader->setUniform("screenDim_", tgt::vec2(screenDim));
    shader->setUniform("screenDimRCP_", 1.f / tgt::vec2(screenDim));
    LGL_ERROR;

    // camera position in world coordinates, and corresponding transformation matrices
    if (camera) {
        shader->setUniform("cameraPosition_", camera->getPosition());
        shader->setUniform("viewMatrix_", camera->getViewMatrix());
        shader->setUniform("projectionMatrix_", camera->getProjectionMatrix());
        tgt::mat4 viewInvert;
        if(camera->getViewMatrix().invert(viewInvert))
            shader->setUniform("viewMatrixInverse_", viewInvert);
        tgt::mat4 projInvert;
        if(camera->getProjectionMatrix().invert(projInvert))
            shader->setUniform("projectionMatrixInverse_", projInvert);
    }

    shader->setIgnoreUniformLocationError(false);

    LGL_ERROR;
}


std::string RenderProcessor::generateHeader(const tgt::GpuCapabilities::GlVersion* version) {
    if (!tgt::GpuCapabilities::isInited()) {
        LERROR("generateHeader() called before initialization of GpuCapabilities");
        return "";
    }

    using tgt::GpuCapabilities;

    tgt::GpuCapabilities::GlVersion useVersion;

    //use supplied version if available, else use highest available.
    //if no version is supplied, use up tp 1.30 as default.
    if (version && GpuCaps.getShaderVersion() >= *version)
        useVersion = *version;
    else if(GpuCaps.getShaderVersion() > GpuCapabilities::GlVersion::SHADER_VERSION_410)
        useVersion = GpuCapabilities::GlVersion::SHADER_VERSION_410;
    else
        useVersion = GpuCaps.getShaderVersion();

    std::stringstream versionHeader;
    versionHeader << useVersion.major() << useVersion.minor();

    std::string header = "#version " + versionHeader.str();

    if(header.length() < 12)
        header += "0";

    //Run in compability mode to use deprecated functionality
    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_150)
        header += " compatibility";
    else if(useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_140)
        header += "\n#extension GL_ARB_compatibility : enable";

    header += "\n";

    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_410)
        header += "#define GLSL_VERSION_410\n";
    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_400)
        header += "#define GLSL_VERSION_400\n";
    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_330)
        header += "#define GLSL_VERSION_330\n";
    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_150)
        header += "#define GLSL_VERSION_150\n";
    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_140)
        header += "#define GLSL_VERSION_140\n";
    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_130) {
        header += "#define GLSL_VERSION_130\n";
        header += "precision highp float;\n";
    }
    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_120)
        header += "#define GLSL_VERSION_120\n";
    if (useVersion >= GpuCapabilities::GlVersion::SHADER_VERSION_110)
        header += "#define GLSL_VERSION_110\n";

    if (GLEW_NV_fragment_program2) {
        GLint i = -1;
        glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_MAX_PROGRAM_LOOP_COUNT_NV, &i);
        if (i > 0) {
            std::ostringstream o;
            o << i;
            header += "#define VRN_MAX_PROGRAM_LOOP_COUNT " + o.str() + "\n";
        }
    }

    //
    // add some defines needed for workarounds in the shader code
    //
    if (GLEW_ARB_draw_buffers)
        header += "#define VRN_GLEW_ARB_draw_buffers\n";

    #ifdef __APPLE__
        header += "#define VRN_OS_APPLE\n";
        if (GpuCaps.getVendor() == GpuCaps.GPU_VENDOR_ATI)
            header += "#define VRN_VENDOR_ATI\n";
    #endif

    if (GpuCaps.getShaderVersion() >= GpuCapabilities::GlVersion::SHADER_VERSION_130) {
        // define output for single render target
        header += "//$ @name = \"outport0\"\n";
        header += "out vec4 FragData0;\n";
    }
    else {
        header += "#define FragData0 gl_FragData[0]\n";
    }

    return header;
}

const std::vector<RenderPort*>& RenderProcessor::getPrivateRenderPorts() const {
    return privateRenderPorts_;
}

} // namespace voreen
