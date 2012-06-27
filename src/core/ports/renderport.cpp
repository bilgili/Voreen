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

#include "voreen/core/ports/renderport.h"
#include "voreen/core/datastructures/rendertarget.h"
#include "voreen/core/processors/renderprocessor.h"

#ifdef VRN_WITH_DEVIL
    #include <IL/il.h>
#endif

namespace voreen {

const std::string RenderPort::loggerCat_("voreen.RenderPort");

RenderPort::RenderPort(PortDirection direction, const std::string& name,
                       bool allowMultipleConnections, Processor::InvalidationLevel invalidationLevel,
                       GLint internalColorFormat, GLint internalDepthFormat)
    : GenericPort<RenderTarget>(direction, name, allowMultipleConnections, invalidationLevel)
    , validResult_(false)
    , size_(128,128)
    , sizeOrigin_(0)
    , internalColorFormat_(internalColorFormat)
    , internalDepthFormat_(internalDepthFormat)
{
    portData_ = 0;
}

RenderPort::~RenderPort() {
    if (portData_) {
        LERRORC("voreen.RenderPort", "~RenderPort(): '" << getName() << "' has not been deinitialized before destruction");
    }
}

void RenderPort::setProcessor(Processor* p) {
    Port::setProcessor(p);

    RenderProcessor* rp = dynamic_cast<RenderProcessor*>(p);
    tgtAssert(rp, "RenderPort attached to processor of wrong type");
    if (!rp)
        LERRORC("voreen.RenderPort", "RenderPort attached to processor of wrong type"
                << p->getName() << "|" << getName());
}

void RenderPort::initialize() throw (VoreenException) {

    GenericPort<RenderTarget>::initialize();

    if (!isOutport())
        return;

    setData(new RenderTarget());

#ifdef VRN_USE_FLOAT32_RENDER_TARGETS
    getData()->initialize(GL_RGBA32F_ARB);
#else
    // getData()->initialize(GL_RGBA); //< 8 bit
    //getData()->initialize(GL_RGBA16);  //< 16 bit
    //getData()->initialize(GL_RGBA16F_ARB); //< 16 bit float
    getData()->initialize(internalColorFormat_, internalDepthFormat_);
#endif
    tgtAssert(processor_, "Not attached to processor!");
    getData()->setDebugLabel(processor_->getName()+ "::" + getName());
    getData()->resize(size_);
    validResult_ = false;
    LGL_ERROR;
}

void RenderPort::deinitialize() throw (VoreenException) {
    if (isOutport() && portData_) {
        portData_->deinitialize();
        delete portData_;
        portData_ = 0;
    }
    LGL_ERROR;

    GenericPort<RenderTarget>::deinitialize();
}

void RenderPort::changeFormat(GLint internalColorFormat, GLint internalDepthFormat) {
    tgt::ivec2 s = getSize();

    if (getData()) {
        getData()->deinitialize();
        delete getData();
    }
    setData(new RenderTarget());
    getData()->initialize(internalColorFormat, internalDepthFormat);
    getData()->resize(s);
}

bool RenderPort::hasValidResult() const {
    if (isOutport())
        return validResult_;
    else {
        if (!isConnected())
            return false;

        if (!getData())
            return false;

        //if (!fbo_)
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
    if (hasData())
        return getData()->getSize();
    else
        return tgt::ivec2(0);
}

void RenderPort::invalidateResult() {
    if (isOutport())
        validResult_ = false;
}

bool RenderPort::doesSizeOriginConnectFailWithPort(Port* inport) const {
    tgtAssert(inport, "passed null pointer");

    RenderPort* rin = dynamic_cast<RenderPort*>(inport);
    if (!rin)
        return false;

    bool unEqual = this != rin;
    bool outIsOutport = isOutport();
    bool inIsInport = rin->isInport();
    bool processorUnEqual = getProcessor() != rin->getProcessor();
    bool isNotConnected = !isConnectedTo(rin);
    bool thisIsConnected = rin->isConnected();
    bool thisAllowsMultiple = rin->allowMultipleConnections();

    return rin && unEqual && outIsOutport && inIsInport && processorUnEqual && isNotConnected && (!thisIsConnected || thisAllowsMultiple);
}

bool RenderPort::isReady() const {
    return (isConnected() && (isOutport() || hasValidResult()));
}

void RenderPort::activateTarget(const std::string& debugLabel) {
    if (getData()) {
        getData()->activateTarget(processor_->getName()+ "::" + getName()
                                  + (debugLabel.empty() ? "" : ": " + debugLabel));
        validateResult();
    }
    else
        LERROR("Trying to activate RenderPort with NULL RenderTarget");
}

void RenderPort::deactivateTarget() {
    if (getData())
        getData()->deactivateTarget();
}

void RenderPort::setTextureParameters(tgt::Shader* shader, std::string uniform) {
    if (hasData()) {
        bool oldIgnoreError = shader->getIgnoreUniformLocationError();
        shader->setIgnoreUniformLocationError(true);
        shader->setUniform(uniform + ".dimensions_", tgt::vec2(getSize()));
        shader->setUniform(uniform + ".dimensionsRCP_", tgt::vec2(1.0f) / tgt::vec2(getSize()));
        shader->setIgnoreUniformLocationError(oldIgnoreError);
    }
}

bool RenderPort::connect(Port* inport) {
    if (Port::connect(inport)) {
        RenderPort* rp = static_cast<RenderPort*>(inport);
        sizeOriginChanged(rp->getSizeOrigin());
        if (rp->getSizeOrigin()) {
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
            if (static_cast<RenderPort*>(getConnected()[i])->getSizeOrigin())
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

    if (rp->getSizeOrigin() == 0)
        return true;

    if (rp->getSizeOrigin() == getSizeOrigin())
        return true;

    return static_cast<RenderProcessor*>(getProcessor())->testSizeOrigin(this, rp->getSizeOrigin());
}

void RenderPort::disconnect(Port* other) {
    Port::disconnect(other);

    RenderPort* rp = static_cast<RenderPort*>(other);
    if (isOutport()) {
        if (getSizeOrigin() != rp->getSizeOrigin())
            static_cast<RenderProcessor*>(getProcessor())->sizeOriginChanged(this);

        other->invalidate();
    }
}

void RenderPort::sizeOriginChanged(void* so) {
    if (isOutport()) {
        static_cast<RenderProcessor*>(getProcessor())->sizeOriginChanged(this);
        validResult_ = false;
    }
    else {
        if (sizeOrigin_ == so)
            return;
        sizeOrigin_ = so;
        for (size_t i = 0; i < connectedPorts_.size(); ++i) {
            RenderPort* rp = static_cast<RenderPort*>(connectedPorts_[i]);
            rp->sizeOriginChanged(so);
            if (so)
                rp->resize(size_);
        }
    }
}

void RenderPort::resize(const tgt::ivec2& newsize) {
    if (isOutport()) {
        if (size_ == newsize)
            return;
        if (newsize == tgt::ivec2(0)) {
            LWARNING("resize(): invalid size " << newsize);
            return;
        }
        if (getData()) {
            getData()->resize(newsize);
        }
        validResult_ = false;
        size_ = newsize;
    }
    else {
        size_ = newsize;
        if (!getSizeOrigin())
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
    if (hasData())
        return getData()->getColorTexture();
    else
        return 0;
}

tgt::Texture* RenderPort::getDepthTexture() const {
    if (hasData())
        return getData()->getDepthTexture();
    else
        return 0;
}

#ifdef VRN_WITH_DEVIL

void RenderPort::saveToImage(const std::string& filename) throw (VoreenException) {

    // get color buffer content
    tgt::col4* colorBuffer = readColorBuffer();
    tgt::ivec2 size = getSize();

    // create Devil image from image data and write it to file
    ILuint img;
    ilGenImages(1, &img);
    ilBindImage(img);
    // put pixels into IL-Image
    ilTexImage(size.x, size.y, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, colorBuffer);
    ilEnable(IL_FILE_OVERWRITE);
    ILboolean success = ilSaveImage(const_cast<char*>(filename.c_str()));
    ilDeleteImages(1, &img);

    delete[] colorBuffer;

    if (!success) {
        if (ilGetError() == IL_COULD_NOT_OPEN_FILE)
            throw VoreenException("Unable to open file " + filename + " for writing");
        else if (ilGetError() == IL_INVALID_EXTENSION)
            throw VoreenException("Invalid image file extension: " + filename);
        else
            throw VoreenException("Could not save rendering to file " + filename);
    }
}

#else

void RenderPort::saveToImage(const std::string& /*filename*/) throw (VoreenException) {
    throw VoreenException("Unable to write rendering to file: Voreen was compiled without Devil support.");
}

#endif // VRN_WITH_DEVIL

tgt::col4* RenderPort::readColorBuffer() const throw (VoreenException) {

    if (!getColorTexture()) {
        throw VoreenException("RenderPort::readColorBuffer() called on an empty render port");
    }

    if (getColorTexture()->getFormat() != GL_RGBA) {
        throw VoreenException("RenderPort::readColorBuffer(): Saving render port to file currently only supported for GL_RGBA format");
    }

    // read pixel data in original data format
    GLubyte* pixels = 0;
    try {
        pixels = getColorTexture()->downloadTextureToBuffer();
    }
    catch (std::bad_alloc&) {
        throw VoreenException("RenderPort::readColorBuffer(): bad allocation");
    }

    // convert pixel data to bytes for image output
    tgt::ivec2 size = getSize();
    tgt::col4* pixels_b = 0;
    if (getColorTexture()->getDataType() == GL_UNSIGNED_BYTE) {
        // data type matches
        pixels_b = reinterpret_cast<tgt::col4*>(pixels);
    }
    else if (getColorTexture()->getDataType() == GL_UNSIGNED_SHORT) {
        // convert 16 bit integer to 8 bit integer
        try {
            pixels_b = new tgt::col4[size.x * size.y];
        }
        catch (std::bad_alloc&) {
            delete[] pixels;
            throw VoreenException("RenderPort::readColorBuffer(): bad allocation");
        }
        for (int i=0; i < size.x * size.y; i++) {
            pixels_b[i].x = reinterpret_cast<tgt::Vector4<GLushort>*>(pixels)[i].x >> 8;
            pixels_b[i].y = reinterpret_cast<tgt::Vector4<GLushort>*>(pixels)[i].y >> 8;
            pixels_b[i].z = reinterpret_cast<tgt::Vector4<GLushort>*>(pixels)[i].z >> 8;
            pixels_b[i].a = reinterpret_cast<tgt::Vector4<GLushort>*>(pixels)[i].a >> 8;
        }
        delete[] pixels;
    }
    else if (getColorTexture()->getDataType() == GL_FLOAT) {
        // convert float to 8 bit integer
        try {
            pixels_b = new tgt::col4[size.x * size.y];
        }
        catch (std::bad_alloc&) {
            delete[] pixels;
            throw VoreenException("RenderPort::readColorBuffer(): bad allocation");
        }
        for (int i=0; i < size.x * size.y; i++)
            pixels_b[i] = tgt::clamp(reinterpret_cast<tgt::Color*>(pixels)[i], 0.f, 1.f) * 255.f;
        delete[] pixels;
    }
    else {
        delete[] pixels;
        throw VoreenException("RenderPort::readColorBuffer(): unknown data type");
    }
    tgtAssert(pixels_b, "Pixel buffer empty");

    return pixels_b;
}


//-----------------------------------------------------------------------------

PortGroup::PortGroup(bool ignoreConnectivity) : fbo_(0),
    ignoreConnectivity_(ignoreConnectivity)
{

}

PortGroup::~PortGroup() {
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

void PortGroup::deinitialize() {
    delete fbo_;
    fbo_ = 0;
}

//void PortGroup::removePort(RenderPort* [>rp<]) {
    ////TODO: implement
    ////reattachTargets();
//}

void PortGroup::activateTargets(const std::string& debugLabel) {
    fbo_->activate();
    if (ports_.empty())
        return;

    GLenum* buffers = new GLenum[ports_.size()];

    int count=0;
    for (size_t i=0; i < ports_.size();++i) {
        if (ignoreConnectivity_ || ports_[i]->isConnected()) {
            buffers[count] = GL_COLOR_ATTACHMENT0_EXT+i;
            ports_[i]->validateResult();
            ports_[i]->getData()->setDebugLabel(ports_[i]->getProcessor()->getName() + "::"
                                                + ports_[i]->getName() + (debugLabel.empty() ? "" : ": " + debugLabel));
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

    LGL_ERROR;

    if (hasDepth_)
        fbo_->isComplete();
}

void PortGroup::resize(const tgt::ivec2& newsize) {
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
