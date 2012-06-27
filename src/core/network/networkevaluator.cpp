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

#include "voreen/core/network/networkevaluator.h"

#include "voreen/core/network/processornetwork.h"
#include "voreen/core/interaction/idmanager.h"
#include "voreen/core/network/networkgraph.h"
#include "voreen/core/processors/canvasrenderer.h"

#include "tgt/textureunit.h"
#include "tgt/framebufferobject.h"

#include <vector>

using std::vector;

namespace voreen {

const std::string NetworkEvaluator::loggerCat_("voreen.NetworkEvaluator");
bool NetworkEvaluator::reuseRenderTargets_ = true;

NetworkEvaluator::NetworkEvaluator(tgt::GLCanvas* sharedContext)
    : network_(0)
    , renderingOrder_()
    , processWrappers_()
    , sharedContext_(sharedContext)
    , networkChanged_(false)
    , locked_(false)
    , processPending_(false)
{

#ifdef VRN_DEBUG
    addProcessWrapper(new CheckOpenGLStateProcessWrapper());
#endif

}

NetworkEvaluator::~NetworkEvaluator() {
#ifdef VRN_DEBUG
    // delete CheckOpenGLStateProcessWrapper
    for (size_t i=0; i<processWrappers_.size(); ++i) {
        if (dynamic_cast<CheckOpenGLStateProcessWrapper*>(processWrappers_[i]))
            delete processWrappers_[i];
    }
#endif

    clearProcessWrappers();
}

void NetworkEvaluator::addProcessWrapper(ProcessWrapper* w) {
    processWrappers_.push_back(w);
}

const std::set<Processor*> NetworkEvaluator::getEndProcessors() const {
    tgtAssert(network_, "No network");
    std::set<Processor*> endProcessors;
    for (size_t i = 0; i < network_->getProcessors().size(); ++i) {
        if (network_->getProcessors()[i]->isEndProcessor() == true)
            endProcessors.insert(network_->getProcessors()[i]);
    }
    return endProcessors;
}

bool NetworkEvaluator::initializeNetwork()  {

    if (isLocked()) {
        LWARNING("initializeNetwork() called on locked evaluator.");
        return false;
    }

    if (!network_) {
        LWARNING("initializeNetwork() called on evaluator without network.");
        return false;
    }

    // prevent parallel execution in multithreaded/event dispatching environments
    lock();

    // Voreen's default depth buffer settings
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    bool failed = false;
    for (size_t i = 0; i < network_->getProcessors().size(); ++i) {
        Processor* processor = network_->getProcessors()[i];
        if (!processor->isInitialized()) {
            try {
                if (sharedContext_)
                    sharedContext_->getGLFocus();
                processor->initialize();
                if (sharedContext_)
                    sharedContext_->getGLFocus();
                LGL_ERROR;
            }
            catch (const VoreenException& e) {
                LERROR("Failed to initialize '" << processor->getName()
                        << "' (" << processor->getClassName() << "): " << e.what());

                // deinitialize processor, in order to make sure that all resources are freed
                LINFO("Deinitializing '" << processor->getName()
                    << "' (" << processor->getClassName() << ") ...");
                if (sharedContext_)
                    sharedContext_->getGLFocus();
                processor->initialized_ = true;
                processor->deinitialize();

                // don't break, try to initialize the other processors even if one failed
                failed = true;
            }
        }
    }

    assignRenderTargets();

    unlock();
    return !failed;
}

bool NetworkEvaluator::deinitializeNetwork() {
    if (isLocked()) {
        LWARNING("deinitializeNetwork() called on locked evaluator.");
        return false;
    }

    if (!network_) {
        LWARNING("deinitializeNetwork() called on evaluator without network.");
        return false;
    }

    // prevent parallel execution in multithreaded/event dispatching environments
    lock();

    bool failed = false;
    for (size_t i = 0; i < network_->getProcessors().size(); ++i) {
        Processor* processor = network_->getProcessors()[i];
        if (processor->isInitialized()) {
            try {
                if (sharedContext_)
                    sharedContext_->getGLFocus();
                processor->deinitialize();
                if (sharedContext_)
                    sharedContext_->getGLFocus();
                LGL_ERROR;
            }
            catch (VoreenException e) {
                LERROR("Failed to deinitialize '" << processor->getName()
                    << "' (" << processor->getClassName() << "): " << e.what());
                // don't break, try to deinitialize the other processors even if one failed
                failed = true;
            }
        }
    }

    unlock();
    return !failed;
}

void NetworkEvaluator::onNetworkChange() {
    if (isLocked()) {
        LWARNING("onNetworkChange() called on locked evaluator.");
        return;
    }

    networkChanged_ = false;

    {
        PROFILING_BLOCK("onNetworkChange");

        renderingOrder_.clear();
        loopPortMap_.clear();

        // nothing more to do, if no network is present
        if (!network_)
            return;

        // define rendering order from network
        {
            PROFILING_BLOCK("defineRenderingOrder");
            defineRenderingOrder();
        }

        // initialize processors
        {
            PROFILING_BLOCK("initialize");
            initializeNetwork();
            LGL_ERROR;
        }
    }
    //performanceRecord_.getLastSample()->print();

}

void NetworkEvaluator::process() {

    if (!network_)
        return;

    if (isLocked()) {
        LDEBUG("process() called on locked evaluator. Scheduling.");
        processPending_ = true;
        return;
    }

    // re-analyze and initialize network, if its topology has changed since last process() call
    if (networkChanged_ || checkForInvalidPorts()) {
        onNetworkChange();
    }

    // prevent parallel execution in multithreaded/event dispatching environments
    lock();

    if (renderingOrder_.empty()) {
        LDEBUG("process(): rendering order is not defined!");
    }

    // mark processed processor for validation
    // note: we need to wait until all processors have been processed
    // before validating them, in order to allow multiple occurrences of
    // processors in the rendering order (loops)
    std::set<Processor*> processed;

    // reset loop ports' iteration counters and progressbars
    for (size_t i = 0; i < renderingOrder_.size(); ++i) {
        Processor* processor = renderingOrder_[i];
        tgtAssert(loopPortMap_.find(processor) != loopPortMap_.end(), "Processor missing in loop port map");
        for (size_t j = 0; j < loopPortMap_[processor].size(); ++j)
            loopPortMap_[processor][j]->setLoopIteration(-1);

        if (!processor->isValid())
            processor->setProgress(0.f);
    }

    // notify process wrappers
    for (size_t j = 0; j < processWrappers_.size(); ++j)
        processWrappers_[j]->beforeNetworkProcess();
    LGL_ERROR;

    // Iterate over processing in rendering order
    for (size_t i = 0; i < renderingOrder_.size(); ++i) {
        Processor* const currentProcessor = renderingOrder_[i];

        // all processors should have been initialized at this point
        if (!currentProcessor->isInitialized()) {
            LWARNING("process(): Skipping uninitialized processor '" << currentProcessor->getName()
                     << "' (" << currentProcessor->getClassName() << ")");
            continue;
        }

        bool needsProcessing = true;
        if (currentProcessor->isValid())
            needsProcessing = false;
        //if (currentProcessor->isEndProcessor())
        if (dynamic_cast<CanvasRenderer*>(currentProcessor))
            needsProcessing = true;

        // run the processor, if it needs processing and is ready
        if (needsProcessing && currentProcessor->isReady()) {

            // increase iteration counters
            for (size_t j=0; j<loopPortMap_[currentProcessor].size(); ++j) {
                Port* port = loopPortMap_[currentProcessor][j];
                // note: modulo is required for nested loops
                port->setLoopIteration((port->getLoopIteration()+1) % port->getNumLoopIterations());
            }

            // notify process wrappers
            for (size_t j=0; j < processWrappers_.size(); ++j)
                processWrappers_[j]->beforeProcess(currentProcessor);
            LGL_ERROR;

            try {
                {
                    ProfilingBlock block(currentProcessor->getName() + ".beforeprocess", currentProcessor->performanceRecord_);
                    currentProcessor->beforeProcess();
                }
                //currentProcessor->performanceRecord_.getLastSample()->print();
                LGL_ERROR;
                {
                    ProfilingBlock block(currentProcessor->getName() + ".process", currentProcessor->performanceRecord_);
                    currentProcessor->process();
                }
                //currentProcessor->performanceRecord_.getLastSample()->print();
                LGL_ERROR;
                {
                    ProfilingBlock block(currentProcessor->getName() + ".afterprocess", currentProcessor->performanceRecord_);
                    currentProcessor->afterProcess();
                }
                //currentProcessor->performanceRecord_.getLastSample()->print();
                LGL_ERROR;
                // mark processor as processed during this rendering pass
                processed.insert(currentProcessor);
            }
            catch (VoreenException& e) {
                LERROR("process(): VoreenException from "
                    << currentProcessor->getClassName()
                    << " (" << currentProcessor->getName() << "): " << e.what());
             }
             catch (std::exception& e) {
                LERROR("process(): Exception from "
                    << currentProcessor->getClassName()
                    << " (" << currentProcessor->getName() << "): " << e.what());
             }

             // notify process wrappers
             for (size_t j = 0; j < processWrappers_.size(); ++j)
                processWrappers_[j]->afterProcess(currentProcessor);
             LGL_ERROR;

             // break loop if network topology has changed (due to changes in loop port configurations)
             if (checkForInvalidPorts()) {
                 unlock();

                 for (size_t j = 0; j < processWrappers_.size(); ++j)
                     processWrappers_[j]->afterNetworkProcess();
                 LGL_ERROR;

                 onNetworkChange();
                 return;
             }
        }

    }   // for (rendering order)

    LGL_ERROR;

    // assumption: a processor is valid after calling process(), except ports or processor itself is invalid
    for (std::set<Processor*>::iterator iter = processed.begin(); iter != processed.end(); ++iter)
        if ((*iter)->getInvalidationLevel() < Processor::INVALID_PORTS)
            (*iter)->setValid();
    LGL_ERROR;

    // notify process wrappers
    for (size_t j = 0; j < processWrappers_.size(); ++j)
        processWrappers_[j]->afterNetworkProcess();
    LGL_ERROR;

    unlock();

    if (processPending_) {
        // make sure that canvases are repainted, if their update has been blocked by the locked evaluator
        processPending_ = false;
        updateCanvases();
    }
}

void NetworkEvaluator::removeProcessWrapper(const ProcessWrapper* w)  {
    std::vector<ProcessWrapper*>::iterator it = std::find(processWrappers_.begin(), processWrappers_.end(), w);
    if (it != processWrappers_.end())
        processWrappers_.erase(it);
}

void NetworkEvaluator::clearProcessWrappers() {
    processWrappers_.clear();
}

void NetworkEvaluator::setProcessorNetwork(ProcessorNetwork* network, bool deinitializeCurrent) {
    if (network_) {
        stopObservation(network_);
        if (deinitializeCurrent)
            deinitializeNetwork();
    }

    network_ = network;

    if (network_)
        network_->addObserver(this);

    onNetworkChange();
}

void NetworkEvaluator::invalidateProcessors(Processor::InvalidationLevel level) {

    if (isLocked()) {
        LWARNING("invalidateProcessors() called on locked evaluator.");
        return;
    }

    if (!network_)
        return;

    for (size_t i=0; i<network_->getProcessors().size(); ++i) {
        network_->getProcessors()[i]->invalidate(level);
    }
}

void NetworkEvaluator::forceUpdate() {

    if (isLocked()) {
        LWARNING("forceUpdate() called on locked evaluator.");
        return;
    }

    invalidateProcessors();
    process();
}


// protected methods
//

void NetworkEvaluator::defineRenderingOrder() {

    tgtAssert(network_, "No processor network");

    if (isLocked()) {
        LWARNING("defineRenderingOrder() called on locked evaluator.");
        return;
    }

    // construct graph: ignore loop ports for graph construction (second parameter), but unroll them afterwards (third parameter)
    NetworkGraph netGraph(network_->getProcessors(), LoopPortTypeCheck(true), LoopPortTypeCheck());

    // generate subset of processors that are predecessors of the end processors
    std::set<Processor*> endProcessors = getEndProcessors();
    std::set<Processor*> predecessors = netGraph.getPredecessors(endProcessors);

    // sort generated subset topologically
    renderingOrder_ = netGraph.sortTopological(predecessors);

    // construct mapping from processors to their loop inports
    // (used for incrementing loop ports' iteration counter)
    loopPortMap_.clear();
    for (size_t i=0; i<renderingOrder_.size(); ++i) {
        Processor* processor = renderingOrder_[i];
        if (loopPortMap_.find(processor) == loopPortMap_.end())
            loopPortMap_[processor] = std::vector<Port*>();
        else
            continue;
        for (size_t j=0; j<processor->getInports().size(); ++j) {
            Port* port = processor->getInports()[j];
            if (port->isLoopPort()) {
                if (port->isConnected())
                    loopPortMap_[processor].push_back(port);
                else
                    port->setLoopIteration(0);
            }
        }
    }

    // reduce processors' invalidation level in order to prevent
    // a continuous re-analysis of the network
    for (size_t i=0; i<network_->getProcessors().size(); ++i) {
        if (network_->getProcessors()[i]->getInvalidationLevel() >= Processor::INVALID_PORTS)
            network_->getProcessors()[i]->invalidationLevel_ = Processor::INVALID_PROGRAM;
    }
}

void NetworkEvaluator::assignRenderTargets() {

    // ports to process (connected outports)
    std::vector<RenderPort*> PORTS;

    // direct successors of each port in the network graph (non-transitive)
    std::map< RenderPort*, std::vector<RenderPort*> > SUCC_GRAPH;

    // successors of each port in the rendering order (transitive)
    std::map< RenderPort*, std::vector<RenderPort*> > SUCC_ORDER;

    // predecessors of each port in the rendering order (transitive)
    std::map< RenderPort*, std::vector<RenderPort*> > PRED_ORDER;

    // collect ports to process
    std::vector<Processor*> processors = network_->getProcessors();
    for (size_t p = 0; p < processors.size(); ++p) {
        if (RenderProcessor* rp = dynamic_cast<RenderProcessor*>(processors[p])) {

            //if (!rp->isInitialized())
            //    continue;

            // outports
            std::vector<Port*> outports = rp->getOutports();
            for (size_t o = 0; o < outports.size(); ++o) {
                if (RenderPort* rport = dynamic_cast<RenderPort*>(outports[o])) {
                    if (rport->isConnected())
                        PORTS.push_back(rport);
                }
            }

            // private ports
            const std::vector<RenderPort*>& privatePorts = rp->getPrivateRenderPorts();
            for (size_t o = 0; o < privatePorts.size(); ++o) {
                if (privatePorts[o]) {
                    PORTS.push_back(privatePorts[o]);
                }
            }
        }
    }

    // collect graph successors
    for (size_t portID = 0; portID < PORTS.size(); ++portID) {
        RenderPort* port = PORTS.at(portID);
        std::vector<RenderPort*> successors;

        // iterate over all connected processors and add their render outports to the successors list
        const std::vector<Port*>& connectedPorts = port->getConnected();
        for (size_t connPortID = 0; connPortID < connectedPorts.size(); ++connPortID) {
            tgtAssert(dynamic_cast<RenderPort*>(connectedPorts.at(connPortID)), "RenderPort connected to Non-RenderPort");
            tgtAssert(connectedPorts.at(connPortID)->getProcessor(), "RenderPort without owner");
            std::vector<Port*> succOutports = connectedPorts.at(connPortID)->getProcessor()->getOutports();
            for (size_t i = 0; i < succOutports.size(); ++i) {
                // add outport to successor list, if it is a RenderPort and element of PORTS
                if (RenderPort* succPort = dynamic_cast<RenderPort*>(succOutports.at(i))) {
                    if (std::find(PORTS.begin(), PORTS.end(), succPort) != PORTS.end())
                        successors.push_back(succPort);
                }
            }
        }

        // add successor list to map
        SUCC_GRAPH.insert(std::pair<RenderPort*, std::vector<RenderPort*> >(port, successors));
    }

}

bool NetworkEvaluator::checkForInvalidPorts() {
    tgtAssert(network_, "no network");

    for (size_t i=0; i<network_->getProcessors().size(); ++i) {
        if (network_->getProcessors()[i]->getInvalidationLevel() >= Processor::INVALID_PORTS) {
            return true;
        }
    }
    return false;
}

const ProcessorNetwork* NetworkEvaluator::getProcessorNetwork() const {
    return network_;
}

std::vector<RenderPort*> NetworkEvaluator::collectRenderPorts() const {

    std::vector<RenderPort*> renderPorts;
    if (isLocked()) {
        LWARNING("collectRenderPorts() called on locked evaluator.");
        return renderPorts;
    }

    if (!network_) {
        //LWARNING("collectRenderPorts() called on evaluator without network.");
        return renderPorts;
    }

    std::vector<Processor*> processors = network_->getProcessors();
    for (size_t p = 0; p < processors.size(); ++p) {
        RenderProcessor* rp = dynamic_cast<RenderProcessor*>(processors[p]);

        if (!rp)
            continue;

        if (!rp->isInitialized())
            continue;

        std::vector<Port*> outports = rp->getOutports();
        for (size_t o = 0; o < outports.size(); ++o) {
            Port* const outport = outports[o];

            RenderPort* rport = dynamic_cast<RenderPort*>(outport);
            if (rport) {
                if (rport->hasRenderTarget())
                    renderPorts.push_back(rport);
            }
        }
        //add private ports:
        const std::vector<RenderPort*>& privatePorts = rp->getPrivateRenderPorts();
        for (size_t o = 0; o < privatePorts.size(); ++o) {
            RenderPort* pport = privatePorts[o];

            if (pport) {
                if (pport->hasRenderTarget())
                    renderPorts.push_back(pport);
            }
        }
    }

    return renderPorts;
}

void NetworkEvaluator::updateCanvases() {

    if (isLocked()) {
        LWARNING("updateCanvases() called on locked evaluator.");
        return;
    }

    if (!network_) {
        LWARNING("updateCanvases() called on evaluator without network.");
        return;
    }

    for (size_t i = 0; i < network_->getProcessors().size(); ++i) {
        CanvasRenderer* canvasRenderer = dynamic_cast<CanvasRenderer*>(network_->getProcessors()[i]);
        if (canvasRenderer)
            canvasRenderer->invalidate();
    }
}

void NetworkEvaluator::lock() {
    locked_ = true;
}

void NetworkEvaluator::unlock() {
    locked_ = false;
}

bool NetworkEvaluator::isLocked() const {
    return locked_;
}

void NetworkEvaluator::networkChanged() {
    networkChanged_ = true;
}

void NetworkEvaluator::processorRenamed(const Processor* /*processor*/, const std::string& /*prevName*/) {
    // ignore
}

void NetworkEvaluator::propertyLinkAdded(const PropertyLink* /*link*/) {
    // ignore
}

void NetworkEvaluator::propertyLinkRemoved(const PropertyLink* /*link*/) {
    // ignore
}

void NetworkEvaluator::processorRemoved(const Processor* processor) {

    if (processor->isInitialized()) {
        try {
            if (sharedContext_)
                sharedContext_->getGLFocus();
            const_cast<Processor*>(processor)->deinitialize();
            if (sharedContext_)
                sharedContext_->getGLFocus();
            LGL_ERROR;
         }
         catch (VoreenException e) {
            LERROR("Failed to deinitialize '" << processor->getName()
                    << "' (" << processor->getClassName() << "): " << e.what());
         }
    }

    networkChanged_ = true;
}


// ----------------------------------------------------------------------------

// processWrapper stuff pending...

namespace {

bool checkGL(GLenum pname, bool value) {
    GLboolean b;
    glGetBooleanv(pname, &b);
    return (static_cast<bool>(b) == value);
}

bool checkGL(GLenum pname, GLint value) {
    GLint i;
    glGetIntegerv(pname, &i);
    return (i == value);
}

bool checkGL(GLenum pname, GLfloat value) {
    GLfloat f;
    glGetFloatv(pname, &f);
    return (f == value);
}

bool checkGL(GLenum pname, const tgt::vec4 value) {
    tgt::vec4 v;
    glGetFloatv(pname, reinterpret_cast<float*>(&v.elem));
    return (v == value);
}

} // namespace

void NetworkEvaluator::CheckOpenGLStateProcessWrapper::afterProcess(Processor* p) {
    checkState(p);
}

void NetworkEvaluator::CheckOpenGLStateProcessWrapper::beforeNetworkProcess() {
    checkState();
}

void NetworkEvaluator::CheckOpenGLStateProcessWrapper::checkState(Processor* p) {

    if (!checkGL(GL_BLEND, false)) {
        glDisable(GL_BLEND);
        warn(p, "GL_BLEND was enabled");
    }

    if (!checkGL(GL_BLEND_SRC, GL_ONE) || !checkGL(GL_BLEND_DST, GL_ZERO)) {
        glBlendFunc(GL_ONE, GL_ZERO);
        warn(p, "Modified BlendFunc");
    }

    if (!checkGL(GL_DEPTH_TEST, true)) {
        glEnable(GL_DEPTH_TEST);
        warn(p, "GL_DEPTH_TEST was not enabled");
    }

    if (!checkGL(GL_CULL_FACE, false)) {
        glDisable(GL_CULL_FACE);
        warn(p, "GL_CULL_FACE was enabled");
    }

    if (!checkGL(GL_COLOR_CLEAR_VALUE, tgt::vec4(0.f))) {
        glClearColor(0.f, 0.f, 0.f, 0.f);
        warn(p, "glClearColor() was not set to all zeroes");
    }

    if (!checkGL(GL_DEPTH_CLEAR_VALUE, 1.f)) {
        glClearDepth(1.0);
        warn(p, "glClearDepth() was not set to 1.0");
    }

    if (!checkGL(GL_LINE_WIDTH, 1.f)) {
        glLineWidth(1.f);
        warn(p, "glLineWidth() was not set to 1.0");
    }

    if (!checkGL(GL_MATRIX_MODE, GL_MODELVIEW)) {
        glMatrixMode(GL_MODELVIEW);
        warn(p, "glMatrixMode was not set to GL_MODELVIEW");
    }

    if (!checkGL(GL_ACTIVE_TEXTURE, GL_TEXTURE0)) {
        glActiveTexture(GL_TEXTURE0);
        warn(p, "glActiveTexture was not set to GL_TEXTURE0");
    }

    if (!checkGL(GL_TEXTURE_1D, false)) {
        glDisable(GL_TEXTURE_1D);
        warn(p, "GL_TEXTURE_1D was enabled");
    }

    if (!checkGL(GL_TEXTURE_2D, false)) {
        glDisable(GL_TEXTURE_2D);
        warn(p, "GL_TEXTURE_2D was enabled");
    }

    if (!checkGL(GL_TEXTURE_3D, false)) {
        glDisable(GL_TEXTURE_3D);
        warn(p, "GL_TEXTURE_3D was enabled");
    }

    if (tgt::Shader::getCurrentProgram() != 0) {
        tgt::Shader::deactivate();
        warn(p, "A shader was active");
    }

    if (tgt::FramebufferObject::getActiveObject() != 0) {
        tgt::FramebufferObject::deactivate();
        warn(p, "A framebuffer object was active (RenderPort::deactivateTarget() missing?)");
    }

    if (!checkGL(GL_DEPTH_FUNC, GL_LESS)) {
        glDepthFunc(GL_LESS);
        warn(p, "glDepthFunc was not set to GL_LESS");
    }

    if (!checkGL(GL_CULL_FACE_MODE, GL_BACK)) {
        glCullFace(GL_BACK);
        warn(p, "glCullFace was not set to GL_BACK");
    }

    if (!tgt::TextureUnit::unused()) {
        // Warn only if units that were initialized non-statically are still active (which probably means
        // they were generated with the new operator and not deleted)
        if (tgt::TextureUnit::numLocalActive() > 0)
            warn(p, "tgt::TextureUnit still had active texture units");
        tgt::TextureUnit::cleanup();
    }

    /*
      Check also:
      identity matrix for GL_MODELVIEW, GL_PROJECTION, GL_TEXTURE
    */
}

void NetworkEvaluator::CheckOpenGLStateProcessWrapper::warn(Processor* p, const std::string& message) {
    if (p) {
        LWARNING(p->getClassName() << " (" << p->getName()
                 << "): invalid OpenGL state after processing: " << message);
    }
    else {
        LWARNING("Invalid OpenGL state before network processing: " << message);
    }
}

} // namespace
