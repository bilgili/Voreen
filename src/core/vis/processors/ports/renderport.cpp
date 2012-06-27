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

#include "voreen/core/vis/processors/ports/renderport.h"
#include "voreen/core/vis/rendertarget.h"
#include "voreen/core/vis/processors/renderprocessor.h"

#ifdef VRN_WITH_DEVIL
    #include <IL/il.h>
#endif

namespace voreen {

const std::string RenderPort::loggerCat_("voreen.RenderPort");

RenderPort::RenderPort(PortDirection direction, const std::string& name,
        bool allowMultipleConnections, Processor::InvalidationLevel invalidationLevel)
: GenericPort<RenderTarget>(direction, name, allowMultipleConnections, invalidationLevel),
  validResult_(false),
  size_(128,128),
  sizeOrigin_(0)
{
    if(isOutport())
        setData(new RenderTarget());
}

RenderPort::~RenderPort() {
    if (portData_) {
        delete portData_;
        portData_ = 0;
    }
}

void RenderPort::setProcessor(Processor* p) {
    Port::setProcessor(p);

    RenderProcessor* rp = dynamic_cast<RenderProcessor*>(p);
    tgtAssert(rp, "RenderPort attached to processor of wrong type");
    if (!rp)
        LERRORC("voreen.renderport", "RenderPort attached to processor of wrong type" << p->getName() << "|" << getName());
}

void RenderPort::initialize() {
    if (isInport()) {
        LWARNING("Trying to initialize inport!");
        return;
    }

    getData()->initialize();
    tgtAssert(processor_, "Not attached to processor!");
    getData()->setDebugLabel(processor_->getName()+ "::" + getName());
    getData()->resize(size_);
    validResult_ = false;
}

bool RenderPort::hasValidResult() const {
    if (isOutport())
        return validResult_;
    else {
        if (!isConnected())
            return false;

        if (!getData())
            return false;

        //if(!fbo_)
            //return false;

        //TODO: multiple connected ports?
        return static_cast<RenderPort*>(connectedPorts_[0])->hasValidResult();
    }
}

void RenderPort::validateResult() {
    if (isOutport()) {
        validResult_ = true;
        getData()->increaseNumUpdates();
    }
}

tgt::ivec2 RenderPort::getSize() const {
   return getData()->getSize();
}

void RenderPort::invalidateResult() {
    if (isOutport())
        validResult_ = false;
}

bool RenderPort::isReady() const {
    if( isConnected() && getData() && ((isOutport() && getSizeOrigin()) || hasValidResult()) )
        return true;
    return false;
}

void RenderPort::activateTarget(const std::string& debugLabel) {
    if (getData()) {
        getData()->activateTarget(processor_->getName()+ "::" + getName() + (debugLabel.empty() ? "" : ": " + debugLabel));
        validateResult();
    }
    else
        LERROR("Trying to activate RenderPort with NULL RenderTarget");
}

void RenderPort::deactivateTarget() {
    //TODO
}

void RenderPort::setTextureParameters(tgt::Shader* shader, std::string uniform) {
    shader->setIgnoreUniformLocationError(true);
    shader->setUniform(uniform + ".dimensions_", tgt::vec2(getSize()));
    shader->setUniform(uniform + ".dimensionsRCP_", tgt::vec2(1.0f) / tgt::vec2(getSize()));
    shader->setIgnoreUniformLocationError(false);
}

bool RenderPort::connect(Port* inport) {
    if (Port::connect(inport)) {
        RenderPort* rp = static_cast<RenderPort*>(inport);
        sizeOriginChanged(rp->getSizeOrigin());
        if(rp->getSizeOrigin()) {
            static_cast<RenderProcessor*>(getProcessor())->portResized(this, rp->size_);
            //LINFO("size: " << rp->size_ << rp->getName());
        }
        return true;
    }
    return false;
}

void* RenderPort::getSizeOrigin() const {
    if (isOutport()) {
        for (size_t i=0; i<getNumConnections(); ++i) {
            if(static_cast<RenderPort*>(getConnected()[i])->getSizeOrigin())
                return static_cast<RenderPort*>(getConnected()[i])->getSizeOrigin();
        }
        return 0;
    }
    else
        return sizeOrigin_;
}


bool RenderPort::testConnectivity(const Port* inport) const {
    if (!Port::testConnectivity(inport))
        return false;

    const RenderPort* rp = static_cast<const RenderPort*>(inport);

    if(rp->getSizeOrigin() == 0)
        return true;

    if(rp->getSizeOrigin() == getSizeOrigin())
        return true;

    return static_cast<RenderProcessor*>(getProcessor())->testSizeOrigin(this, rp->getSizeOrigin());
}

void RenderPort::disconnect(Port* other) {
    Port::disconnect(other);

    RenderPort* rp = static_cast<RenderPort*>(other);
    if(isOutport()) {
        if(getSizeOrigin() != rp->getSizeOrigin())
            static_cast<RenderProcessor*>(getProcessor())->sizeOriginChanged(this);

        other->invalidate();
    }
}

void RenderPort::sizeOriginChanged(void* so) {
    if (isOutport()) {
        static_cast<RenderProcessor*>(getProcessor())->sizeOriginChanged(this);
        validResult_ = false;
    } else {
        if(sizeOrigin_ == so)
            return;
        sizeOrigin_ = so;
        for (size_t i = 0; i < connectedPorts_.size(); ++i) {
            RenderPort* rp = static_cast<RenderPort*>(connectedPorts_[i]);
            rp->sizeOriginChanged(so);
            if(so)
                rp->resize(size_);
        }
    }
}

void RenderPort::resize(tgt::ivec2 newsize) {
    if (isOutport()) {
		if(size_ == newsize)
			return;
        if(getData()) {
            getData()->resize(newsize);
        }
        validResult_ = false;
        size_ = newsize;
    } else {
        size_ = newsize;
        if(!getSizeOrigin())
            return;
        for (size_t i = 0; i < connectedPorts_.size(); ++i) {
            RenderPort* rp = static_cast<RenderPort*>(connectedPorts_[i]);
            static_cast<RenderProcessor*>(rp->getProcessor())->portResized(rp, newsize);
        }
    }
}

void RenderPort::bindColorTexture() const {
    if (getData())
        getData()->bindColorTexture();
}

void RenderPort::bindColorTexture(GLint texUnit) const {
    if (getData())
        getData()->bindColorTexture(texUnit);
}

void RenderPort::bindDepthTexture() const {
    if (getData())
        getData()->bindDepthTexture();
}

void RenderPort::bindDepthTexture(GLint texUnit) const {
    if (getData())
        getData()->bindDepthTexture(texUnit);
}

void RenderPort::bindTextures(GLint colorUnit, GLint depthUnit) const {
    bindColorTexture(colorUnit);
    bindDepthTexture(depthUnit);
}

tgt::Texture* RenderPort::getColorTexture() const {
       return getData()->getColorTexture();
}

tgt::Texture* RenderPort::getDepthTexture() const {
       return getData()->getDepthTexture();
}

#ifdef VRN_WITH_DEVIL

void RenderPort::saveToImage(const std::string& filename) {
    if (!getData() || !getData()->getColorTexture()) {
        LWARNING("saveToImage() called on an empty render port. Aborting.");
        return;
    }

    // read pixel data as floats and convert them to bytes
    tgt::vec4* pixels_f = reinterpret_cast<tgt::vec4*>(getColorTexture()->downloadTextureToBuffer());
    //tgt::vec4* pixels_f = reinterpret_cast<tgt::vec4*>(getTargetAsFloats());
    tgt::ivec2 size = getSize();
    tgt::col4* pixels_b = new tgt::col4[size.x * size.y];

    for (int i=0; i < size.x * size.y; i++)
        pixels_b[i] = tgt::clamp(pixels_f[i], 0.f, 1.f) * 255.f;

    // create Devil image from image data and write it to file
    ILuint img;
    ilGenImages(1, &img);
    ilBindImage(img);
    // put pixels into IL-Image
    ilTexImage(size.x, size.y, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, pixels_b);
    ilEnable(IL_FILE_OVERWRITE);
    if (ilSaveImage(const_cast<char*>(filename.c_str()))) {
        LINFO("Wrote rendering with dimensions " << size << " to file: " << filename);
    }
    else {
        LERROR("Could not save rendering to file: " + filename);
    }
    ilDeleteImages(1, &img);

    delete[] pixels_f;
    delete[] pixels_b;

}

#else

void RenderPort::saveToImage(const std::string& /*filename*/) {
    LWARNING("Unable to write rendering to file: Voreen was compiled without Devil support.");
}

#endif // VRN_WITH_DEVIL

//-----------------------------------------------------------------------------

PortGroup::PortGroup(bool ignoreConnectivity) : fbo_(0),
    ignoreConnectivity_(ignoreConnectivity)
{

}

void PortGroup::addPort(RenderPort* rp) {
    ports_.push_back(rp);
	reattachTargets();
}

void PortGroup::addPort(RenderPort& rp) {
    addPort(&rp);
}

void PortGroup::initialize() {
    if (fbo_)
        return;

    fbo_ = new tgt::FramebufferObject();
}

//void PortGroup::removePort(RenderPort* [>rp<]) {
    ////TODO: implement
    ////reattachTargets();
//}

void PortGroup::activateTargets(const std::string& debugLabel) {
    fbo_->activate();
    if(ports_.empty())
        return;

    GLenum* buffers = new GLenum[ports_.size()];

    int count=0;
    for (size_t i=0; i < ports_.size();++i) {
        if(ignoreConnectivity_ || ports_[i]->isConnected()) {
            buffers[count] = GL_COLOR_ATTACHMENT0_EXT+i;
            ports_[i]->validateResult();
            ports_[i]->getData()->setDebugLabel(ports_[i]->getProcessor()->getName() + "::" + ports_[i]->getName() + (debugLabel.empty() ? "" : ": " + debugLabel));
            count++;
        }
    }

    glDrawBuffers(count, buffers);
    glViewport(0, 0, ports_[0]->getSize().x, ports_[0]->getSize().y);
    delete[] buffers;
}

void PortGroup::reattachTargets() {
    if (!fbo_)
        return;

    fbo_->activate();
    fbo_->detachAll();

    if (ports_.empty())
        return;

    bool hasDepth_ = false;

    for (size_t i=0; i<ports_.size(); ++i) {
        RenderPort* p = ports_[i];

        if (!ignoreConnectivity_ && !p->isConnected())
            continue;

        fbo_->attachTexture(p->getColorTexture(), GL_COLOR_ATTACHMENT0_EXT+i);
        if (!hasDepth_) {
            hasDepth_ = true;
            fbo_->attachTexture(p->getDepthTexture(), GL_DEPTH_ATTACHMENT_EXT);
        }
    }
	if(hasDepth_)
		fbo_->isComplete();
}

void PortGroup::resize(tgt::ivec2 newsize) {
    for (size_t i=0; i<ports_.size(); ++i) {
        ports_[i]->resize(newsize);
    }
}

std::string PortGroup::generateHeader() {
    // map ports to render targets
    std::string headerSource;
    int targetidx = 0;
    for (size_t i=0; i<ports_.size(); ++i) {
        std::ostringstream op, num;
        op << i;
        num << targetidx;

        if (ignoreConnectivity_ || ports_[i]->isConnected()) {
            headerSource += "#define OP" + op.str() + " " + num.str() + "\n";
            targetidx++;
        }
    }

    return headerSource;
}

} // namespace
