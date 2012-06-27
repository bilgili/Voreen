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

#include "voreen/core/vis/network/networkevaluator.h"

#include "voreen/core/vis/network/processornetwork.h"
#include "voreen/core/vis/idmanager.h"
#include "voreen/core/vis/network/networkgraph.h"
#include "voreen/core/vis/processors/volume/volumesourceprocessor.h"
#include "voreen/core/vis/processors/image/canvasrenderer.h"

namespace voreen {

CollectSuccessorsVisitor::CollectSuccessorsVisitor() {}

CollectSuccessorsVisitor::~CollectSuccessorsVisitor() {}

const std::vector<Processor*>& CollectSuccessorsVisitor::getSuccessors() const {
    return successors_;
}

bool CollectSuccessorsVisitor::visit(Processor* const processor) {
    if (processor != 0) {
        successors_.push_back(processor);
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------

EndProcessorVisitor::EndProcessorVisitor()
    : GraphVisitor(),
    active_(false)
{
}

EndProcessorVisitor::~EndProcessorVisitor() {}

const std::set<Processor*>& EndProcessorVisitor::getPredecessors() const {
    return predecessors_;
}

bool EndProcessorVisitor::visit(Processor* const processor) {
    if (processor == 0)
        return false;

    predecessors_.insert(processor);
    return true;
}

// ----------------------------------------------------------------------------

const std::string NetworkEvaluator::loggerCat_("voreen.NetworkEvaluator");
bool NetworkEvaluator::reuseRenderTargets_ = true;

NetworkEvaluator::NetworkEvaluator(tgt::GLCanvas* sharedContext) :
    network_(0),
    netGraph_(0),
    netGraphTransposed_(0),
    renderingOrder_(),
    stateMap_(),
    volumeCache_(),
    processWrappers_(),
    sharedContext_(sharedContext),
    initialized_(false),
    locked_(false)
{

#ifndef VRN_WITH_VOLUMECACHING
    volumeCache_.setEnabled(false);
#endif

#ifdef VRN_DEBUG
    addProcessWrapper(new CheckOpenGLStateProcessWrapper());
#endif

}

NetworkEvaluator::~NetworkEvaluator() {

    delete netGraph_;
    delete netGraphTransposed_;

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

const std::vector<Processor*> NetworkEvaluator::getEndProcessors() const {
    std::vector<Processor*> endProcessors;
    if (network_ != 0) {
        for (size_t i = 0; i < network_->getProcessors().size(); ++i) {
            if (network_->getProcessors()[i]->isEndProcessor() == true)
                endProcessors.push_back(network_->getProcessors()[i]);
        }
    }
    return endProcessors;
}

bool NetworkEvaluator::initialize() throw (VoreenException) {

    if (isLocked()) {
        LWARNING("initialize() called while evaluator is locked. Ignored.");
        return false;
    }

    if (!network_) {
        LWARNING("initialize() called before network has been set");
        return false;
    }

    if (sharedContext_) {
        LDEBUG("initialize(): Activating shared context");
        sharedContext_->getGLFocus();
    }
    else {
        LDEBUG("initialize(): No shared context");
    }

    // Voreen's default depth buffer settings
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    bool failed = false;
    for (size_t i = 0; i < network_->getProcessors().size(); ++i) {
        if (!network_->getProcessors()[i]->isInitialized()) {
            try {
                network_->getProcessors()[i]->initialize();
            }
            catch (VoreenException e) {
                LERROR("initialize failed for " << network_->getProcessors()[i]->getClassName()
                        << " (" << network_->getProcessors()[i]->getName() << ")");
                LERROR("VoreenException: " << e.what());
                // don't break, try to initialize the other processors even if one failed
                failed = true;
            }
        }
    }

    assignRenderTargets();

    initialized_ = !failed;

    if (!failed)
        invalidateProcessors();

    return initialized_;
}

bool NetworkEvaluator::isInitialized() const {
    return initialized_;
}

void NetworkEvaluator::onNetworkChange() {

    if (isLocked()) {
        LDEBUG("onNetworkChange() called while evaluator is locked. Ignored.");
        return;
    }

    // Delete old transposed graph and the subgraph containing
    // on the processors and connections concerned with image
    // ports. Those will be re-created on demand, so;
    //
    delete netGraph_;
    netGraph_ = 0;
    delete netGraphTransposed_;
    netGraphTransposed_ = 0;

    renderingOrder_.clear();

    renderTargets_.clear();

    if (!network_)
        return;

    if (network_->getProcessors().empty() == false) {
        netGraph_ = new NetworkGraph(network_->getProcessors());
        defineRenderingOrder();
    }

    // clear old processor state maps
    stateMap_.clear();

    for (size_t i = 0; i < network_->getProcessors().size(); ++i) {
        Processor* const processor = network_->getProcessors()[i];
        CanvasRenderer* const cvr = dynamic_cast<CanvasRenderer* const>(processor);

        if (cvr) {
            stateMap_.insert(std::make_pair(cvr, ""));
        }

        // initialize the StateMap for the processors with an empty state.
        // this will ensure, that the cache is queried on first evaluation
        // of the network.
        //
        stateMap_.insert(std::make_pair(processor, ""));
    }

    initialize();
}

void NetworkEvaluator::onNetworkConnectionsChange() {

    if (isLocked()) {
        LDEBUG("onNetworkConnectionsChange() called while evaluator is locked. Ignored.");
        return;
    }

    if (network_ == 0)
        return;

    // re-connect the nodes of the "normal" NetworkGraph, if present
    // (all types of port, mask == 0)
    //
    if (netGraph_)
        netGraph_->connectNodes();

    delete netGraphTransposed_;
    netGraphTransposed_ = 0;

    initialize();

    defineRenderingOrder();
}

void NetworkEvaluator::process() {

    if (isLocked()) {
        LWARNING("process() called while evaluator is locked. Ignored.");
        return;
    }

    if (!initialized_) {
        LDEBUG("process(): processors have not been initialized");
        return;
    }

    if (renderingOrder_.empty() == true) {
        LDEBUG("process(): rendering order is not defined!");
    }

    for (size_t j = 0; j < processWrappers_.size(); ++j)
        processWrappers_[j]->beforeNetworkProcess();

    LGL_ERROR;

    // Indicates whether successors have been invalidated (see comment below).
    for (size_t i = 0; i < renderingOrder_.size(); ++i) {
        Processor* const currentProcessor = renderingOrder_[i];

        bool needsProcessing = true;

        // The volume caching is not affected by the invalidation, so if the current processor
        // is also not affected by the RenderTargetCache, but by the VolumeCache, it may not
        // be necessary to call process().
        CacheState volumeCacheState = CACHE_STATE_UNKNOWN;
        needsProcessing &= needsProcessingForVolumeCache(currentProcessor, volumeCacheState);

        if (currentProcessor->isValid())
            needsProcessing = false;
        if (dynamic_cast<CanvasRenderer*>(currentProcessor))
            needsProcessing = true;

        if (needsProcessing == true) {

            try {

                for (size_t j = 0; j < processWrappers_.size(); ++j)
                    processWrappers_[j]->beforeProcess(currentProcessor);

                // Run the processor when its initialized correctly
                if (currentProcessor->isInitialized()) {
                    std::vector<Port*> outports = currentProcessor->getOutports();
                    for (size_t o = 0; o < outports.size(); ++o) {
                        Port* const outport = outports[o];

                        if (!outport->isConnected())
                            continue;

                        RenderPort* rport = dynamic_cast<RenderPort*>(outport);
                        if (!rport)
                            continue;

                        rport->invalidateResult();
                    }
                    if(currentProcessor->isReady()) {
                        currentProcessor->process();

                        // assumption: a processor is valid after calling process()
                        // TODO: does this work with loops in the network?
                        currentProcessor->setValid();
                    }
                }
                else
                    LWARNING("Uninitialized Processor: " << currentProcessor->getName());

                for (size_t j = 0; j < processWrappers_.size(); ++j)
                    processWrappers_[j]->afterProcess(currentProcessor);

                LGL_ERROR;

            }
            catch (VoreenException& e) {
                LERROR("NetworkEvaluator::process(): VoreenException from "
                        << currentProcessor->getClassName()
                        << " (" << currentProcessor->getName() << "): " << e.what());
            }
            catch (std::exception& e) {
                LERROR("NetworkEvaluator::process(): Exception from "
                        << currentProcessor->getClassName()
                        << " (" << currentProcessor->getName() << "): " << e.what());
            }
        }

        // Depending on the cache states, the caches an the state maps may need to be updated
        // for this processor.
        updateVolumeCache(currentProcessor, volumeCacheState);

    }   // for (i... (loop for processors)

    LGL_ERROR;

    for (size_t j = 0; j < processWrappers_.size(); ++j)
        processWrappers_[j]->afterNetworkProcess();

    LGL_ERROR;

}

void NetworkEvaluator::removeProcessWrapper(const ProcessWrapper* w)  {
    std::vector<ProcessWrapper*>::iterator it = std::find(processWrappers_.begin(), processWrappers_.end(), w);
    if (it != processWrappers_.end())
        processWrappers_.erase(it);
}

void NetworkEvaluator::clearProcessWrappers() {
    processWrappers_.clear();
}

void NetworkEvaluator::setProcessorNetwork(ProcessorNetwork* const network){
    network_ = network;
    initialized_ = false;
    onNetworkChange();
}

void NetworkEvaluator::invalidateProcessors(Processor::InvalidationLevel level) {

    if (isLocked()) {
        LWARNING("invalidateProcessors() called while evaluator is locked. Ignored.");
        return;
    }

    if (!network_ || !isInitialized()) {
        LWARNING("invalidateProcessors() was called on an uninitialized NetworkEvaluator");
        return;
    }

    for (size_t i=0; i<network_->getProcessors().size(); ++i) {
        network_->getProcessors()[i]->invalidate(level);
    }
}

void NetworkEvaluator::forceUpdate() {

    if (isLocked()) {
        LWARNING("forceUpdate() called while evaluator is locked. Ignored.");
        return;
    }

    if (!network_ || !isInitialized()) {
        LWARNING("forceUpdate() was called on an uninitialized NetworkEvaluator");
        return;
    }

    invalidateProcessors();
    process();

}


// protected methods
//

void NetworkEvaluator::assignRenderTargets() {
    renderTargets_.clear();

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

            //if (!outport->isConnected()) {
                //continue;
            //}

            RenderPort* rport = dynamic_cast<RenderPort*>(outport);
            if (rport) {
                if(rport->getData())
                    renderTargets_.push_back(rport->getData());
            }
        }
        //add private ports:
        const std::vector<RenderPort*>& privatePorts = rp->getPrivateRenderPorts();
        for (size_t o = 0; o < privatePorts.size(); ++o) {
            RenderPort* pport = privatePorts[o];

            if (pport) {
                if(pport->getData())
                    renderTargets_.push_back(pport->getData());
            }
        }
    }


}

void NetworkEvaluator::defineRenderingOrder() {

    if (isLocked()) {
        LWARNING("defineRenderingOrder() called while evaluator is locked. Ignored.");
        return;
    }

    if (netGraph_ == 0)
        return;

    if (netGraphTransposed_ == 0)
        netGraphTransposed_ = netGraph_->getTransposed();

    EndProcessorVisitor epv;
    std::vector<Processor*> endProcessors = getEndProcessors();
    for (size_t i = 0; i < endProcessors.size(); ++i)
        netGraphTransposed_->traverseBreadthFirst(endProcessors[i], &epv);
    renderingOrder_ = netGraph_->sortTopological(epv.getPredecessors());
}

bool NetworkEvaluator::needsProcessingForVolumeCache(Processor* const processor, CacheState& state) {
    if (volumeCache_.isEnabled() == false) {
        state = CACHE_STATE_NO_CACHING;
        return true;
    }

    state = getCacheState(processor, volumeCache_);

    bool needsProcessing = true;
    switch (state) {
        case CACHE_STATE_NEED_UPDATE:
            needsProcessing = ! injectCachedVolumeData(processor);
            if (needsProcessing == false)
                state = CACHE_STATE_STATE_UPDATE_NEEDED;
            break;
        case CACHE_STATE_OK:
            needsProcessing = false;
            break;
        default:
            break;
    }
    return needsProcessing;
}

void NetworkEvaluator::updateVolumeCache(Processor* const processor, const CacheState state) {
    if ((volumeCache_.isEnabled() == false) || (processor == 0))
        return;

    switch (state) {
        case CACHE_STATE_NEED_UPDATE:
            volumeCache_.update(processor);
            // no break here
        case CACHE_STATE_STATE_UPDATE_NEEDED:
            {
            // The processor's inport configuration has to be treated as part of
            // its internal state to ensure notifications, when the inport data
            // change as this involves the cache...
            //
            std::string inportConfigVol = volumeCache_.getProcessorsInportConfig(processor);
            //std::string inportConfigRT = renderTargetCache_.getProcessorsInportConfig(processor);
            std::string state = processor->getState();
            state += ",inports=" + inportConfigVol/* + "," + inportConfigRT*/;

            std::pair<StateMap::iterator, bool> res =
                stateMap_.insert(std::make_pair(processor, state));
            if (res.second == false)
                (res.first)->second = state;
            }
            break;
        default:
            break;
    }
}

// private methods
//

NetworkEvaluator::CacheState NetworkEvaluator::getCacheState(Processor* const processor,
                                                             const CacheBase& cache)
{
    // at first test for the two simplest cases
    //
    if (processor == 0)
        return CACHE_STATE_ERROR_PROCESSOR_INVALID;

    if (stateMap_.empty())
        return CACHE_STATE_ERROR_STATEMAP_EMPTY;

    if (cache.isCompatible(processor) == false)
        return CACHE_STATE_INCOMPATIBLE_PROCESSOR;

    // try to find the processor's state in the StateMap
    //
    StateMap::const_iterator it = stateMap_.find(processor);
    if (it == stateMap_.end())
        return CACHE_STATE_ERROR_STATE_MISSING;
    const std::string& prevState = it->second;

    // compare previous state with current one:
    // if the states are equal, the cache value from the cache has
    // already been taken and no further operation is required.
    //
    std::string inportConfigVol = volumeCache_.getProcessorsInportConfig(processor);
    //std::string inportConfigRT = renderTargetCache_.getProcessorsInportConfig(processor);
    std::string currentState = processor->getState();
    currentState += ",inports=" + inportConfigVol /*+ "," + inportConfigRT*/;
    if (currentState == prevState)
        return CACHE_STATE_OK;

    return CACHE_STATE_NEED_UPDATE;
}

bool NetworkEvaluator::injectCachedVolumeData(Processor* const processor) {
    if (processor == 0)
        return false;

    std::vector<Port*> outports = volumeCache_.getCacheConcernedOutports(processor);
    bool result = true;

    for (size_t i = 0; i < outports.size(); ++i) {
        VolumeHandle* handle = volumeCache_.find(processor, outports[i]);
        result = result && (handle != 0);
        if (result == false)
            break;

        // Using this construction, the VolumeHandle* in handle is directly 'injected' to
        // the processor's attribute holding usually its VolumeHandle which is usually set
        // by calling process().
        // This work-around permits to not deal with the PortMapping again. But as VolumeHandle
        // ports ALWAYS take the ADDRESS OF A POINTER to a VolumeHandle, this is no real hack.
        // doPortMapping() basically does the same, but it takes the VolumeHandle** rather from
        // the processor directly than from the cache. (dirk)
        //
        VolumePort* vp = dynamic_cast<VolumePort*>(outports[i]);
        VolumeHandle* handleAddr = vp->getData();
        if (handleAddr != 0)
            vp->setData(handle);
            //*handleAddr = handle;
    }
    return result;
}

const ProcessorNetwork* NetworkEvaluator::getProcessorNetwork() const {
    return network_;
}

const std::vector<RenderTarget*>& NetworkEvaluator::getCurrentRenderTargets() const {
    return renderTargets_;
}

void NetworkEvaluator::updateCanvases() {

    if (isLocked()) {
        LWARNING("updateCanvases() called while evaluator is locked. Ignored.");
        return;
    }

    if (!network_)
        return;

    for (size_t i = 0; i < network_->getProcessors().size(); ++i) {
        CanvasRenderer* canvasRenderer = dynamic_cast<CanvasRenderer*>(network_->getProcessors()[i]);
        if (canvasRenderer)
            canvasRenderer->invalidate();
    }

    process();
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

    if (!checkGL(GL_DEPTH_FUNC, GL_LESS)) {
        glDepthFunc(GL_LESS);
        warn(p, "glDepthFunc was not set to GL_LESS");
    }

    if (!checkGL(GL_CULL_FACE_MODE, GL_BACK)) {
        glCullFace(GL_BACK);
        warn(p, "glCullFace was not set to GL_BACK");
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


}   // namespace
