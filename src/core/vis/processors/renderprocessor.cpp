/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/processors/renderprocessor.h"

#include "tgt/glmath.h"
#include "tgt/camera.h"
#include "tgt/shadermanager.h"
#include "tgt/gpucapabilities.h"

#include "voreen/core/vis/network/networkevaluator.h"

#include <sstream>

using tgt::vec3;
using tgt::vec4;
using tgt::Color;
using std::map;

namespace voreen {

const std::string RenderProcessor::loggerCat_("voreen.RenderProcessor");

const std::string RenderProcessor::XmlElementName_("RenderProcessor");

RenderProcessor::RenderProcessor()
    : Processor()
    , backgroundColor_("backgroundColor", "Background color", tgt::Color(0.0f, 0.0f, 0.0f, 0.0f))
{}

RenderProcessor::~RenderProcessor() {
}

void RenderProcessor::initialize() throw (VoreenException) {
    const std::vector<Port*> outports = getOutports();
    for (size_t i=0; i<outports.size(); ++i) {
        RenderPort* rp = dynamic_cast<RenderPort*>(outports[i]);
        if (rp) {
            rp->initialize();
        }
    }
    const std::vector<RenderPort*> pports = getPrivateRenderPorts();
    for (size_t i=0; i<pports.size(); ++i) {
        pports[i]->initialize();
    }

    Processor::initialize();
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

    if (so) {
        const std::vector<Port*> outports = getOutports();
        for (size_t i=0; i<outports.size(); ++i) {
            if(p == outports[i])
                continue;
            RenderPort* rp = dynamic_cast<RenderPort*>(outports[i]);
            if (rp) {
                if (rp->getSizeOrigin() && (rp->getSizeOrigin() != so))
                    return false;
            }
        }
    }

    const std::vector<Port*> inports = getInports();
    for (size_t i=0; i<inports.size(); ++i) {
        RenderPort* rp = dynamic_cast<RenderPort*>(inports[i]);
        if (rp) {
            if (rp->getSizeOrigin() && (rp->getSizeOrigin() != so) )
                return false;

            const std::vector<Port*>& connectedOutports = inports[i]->getConnected();
            for (size_t j=0; j<connectedOutports.size(); ++j) {
                RenderPort* op = static_cast<RenderPort*>(connectedOutports[j]);

                if (!static_cast<RenderProcessor*>(op->getProcessor())->testSizeOrigin(op, so))
                    return false;
            }
        }
    }

    return true;
}

void RenderProcessor::portResized(RenderPort* /*p*/, tgt::ivec2 newsize) {
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
}

void RenderProcessor::addPrivateRenderPort(RenderPort* port) {
    port->setProcessor(this);
    privateRenderPorts_.push_back(port);

    map<std::string, Port*>::const_iterator it = portMap_.find(port->getName());
    if (it == portMap_.end())
        portMap_.insert(std::make_pair(port->getName(), port));
    else
        LERROR("Port with name " << port->getName() << " has already been inserted!");
}

void RenderProcessor::addPrivateRenderPort(RenderPort& port) {
    addPrivateRenderPort(&port);
}

void RenderProcessor::renderQuad() {
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
}

void RenderProcessor::renderQuadWithTexCoords(const RenderPort& port) {

    if (!port.isReady()) {
        LWARNING("RenderPort not ready");
        return;
    }

    if (port.getColorTexture()->getType() == GL_TEXTURE_2D) {
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
    }
    else if (port.getColorTexture()->getType() == GL_TEXTURE_RECTANGLE_ARB){
        // use integer texcoords in case of texture rectangles
        glBegin(GL_QUADS);
            glTexCoord2i(0, 0);
            glVertex2f(-1.f, -1.f);

            glTexCoord2i(port.getSize().x, 0);
            glVertex2f(1.f, -1.f);

            glTexCoord2i(port.getSize().x, port.getSize().y);
            glVertex2f(1.f, 1.f);

            glTexCoord2i(0, port.getSize().y);
            glVertex2f(-1.f, 1.f);
        glEnd();
    }
    else {
        LERROR("Unknown texture type: " << port.getColorTexture()->getType());
    }

}

// Parameters currently set:
// - screenDim_
// - screenDimRCP_
// - cameraPosition_ (camera position in world coordinates)
void RenderProcessor::setGlobalShaderParameters(tgt::Shader* shader, tgt::Camera* camera) {
    shader->setIgnoreUniformLocationError(true);

    //HACK:
    RenderPort* rp = 0;
    for (size_t i=0; i<getOutports().size(); ++i) {
        rp = dynamic_cast<RenderPort*>(getOutports()[i]);
        if(rp)
            break;
    }
    if (rp) {
        shader->setUniform("screenDim_", tgt::vec2(rp->getSize()));
        shader->setUniform("screenDimRCP_", 1.f / tgt::vec2(rp->getSize()));
    }

    // camera position in world coordinates
    if (camera)
        shader->setUniform("cameraPosition_", camera->getPosition());

    shader->setIgnoreUniformLocationError(false);
}


std::string RenderProcessor::generateHeader() {

    if (!isInitialized()) {
        LWARNING("generateHeader() called on an uninitialized processor");
    }

    std::string header;

    // use highest available shading language version up to version 1.30
    using tgt::GpuCapabilities;
    if (GpuCaps.getShaderVersion() >= GpuCapabilities::GlVersion::SHADER_VERSION_130) {
        header += "#version 130\n";
        header += "precision highp float;\n";
    }
    else if (GpuCaps.getShaderVersion() == GpuCapabilities::GlVersion::SHADER_VERSION_120)
        header += "#version 120\n";
    else if (GpuCaps.getShaderVersion() == GpuCapabilities::GlVersion::SHADER_VERSION_110)
        header += "#version 110\n";

    //HACK:
    if (!GpuCaps.isNpotSupported())
        header += "#define VRN_TEXTURE_RECTANGLE\n";
    else
        header += "#define VRN_TEXTURE_2D\n";

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

    return header;
}

const std::vector<RenderPort*>& RenderProcessor::getPrivateRenderPorts() const {
    return privateRenderPorts_;
}

} // namespace voreen
