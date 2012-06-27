#include "voreen/core/vis/network/networkgraph.h"

#include <algorithm>
#include <deque>
#include <iostream>
#include <stack>

#include "voreen/core/vis/processors/graphvisitor.h"
#include "voreen/core/vis/processors/processor.h"

namespace voreen {

NetworkGraph::GraphNode::GraphNode(Processor* const processor, const int id)
    : processor_(processor),
    id_(id),
    successors_(),
    inDegree_(0),
    outDegree_(0),
    marked_(false),
    minDistanceToRoot_(0),
    tDiscovered_(0),
    tFinished_(0)
{
}

NetworkGraph::GraphNode::GraphNode(Processor* const processor)
    : processor_(processor),
    id_(-1),
    successors_(),
    inDegree_(0),
    outDegree_(0),
    marked_(false),
    minDistanceToRoot_(0),
    tDiscovered_(0),
    tFinished_(0)
{
}

NetworkGraph::GraphNode::GraphNode(const int id)
    : processor_(0),
    id_(id),
    successors_(),
    inDegree_(0),
    outDegree_(0),
    marked_(false),
    minDistanceToRoot_(0),
    tDiscovered_(0),
    tFinished_(0)
{
}

NetworkGraph::GraphNode::GraphNode(const NetworkGraph::GraphNode& other) {
    copy(other);
}

NetworkGraph::GraphNode::~GraphNode() {
}

bool NetworkGraph::GraphNode::operator <(const NetworkGraph::GraphNode& other) const {
    if ((processor_ == 0) || (other.processor_ == 0))
        return (id_ < other.id_);
    return (processor_ < other.processor_); // only compare pointers
}

NetworkGraph::GraphNode& NetworkGraph::GraphNode::operator =(const NetworkGraph::GraphNode& other) {
    copy(other);
    return *this;
}

void NetworkGraph::GraphNode::addSuccessor(NetworkGraph::GraphNode* const successor)
{
    if (successor == 0)
        return;

    successors_.push_back(successor);
    ++(successor->inDegree_);
    ++outDegree_;
}

void NetworkGraph::GraphNode::clearSuccessors() {
    for (size_t i = 0; i < successors_.size(); ++i) {
        if (successors_[i]->inDegree_ > 0)
            --(successors_[i]->inDegree_);
    }
    successors_.clear();
    outDegree_ = 0;
}

// private methods
//

void NetworkGraph::GraphNode::copy(const NetworkGraph::GraphNode& other) {
    processor_ = other.processor_;  // shallow copy only!
    id_ = other.id_;
    successors_ = std::vector<GraphNode*>();    // do not copy pointers to successors!
    inDegree_ = 0;
    outDegree_ = 0;
    marked_ = other.marked_;
    minDistanceToRoot_ = other.minDistanceToRoot_;
    tDiscovered_ = other.tDiscovered_;
    tFinished_= other.tFinished_;
}

// ============================================================================

const std::string NetworkGraph::loggerCat_("voreen.NetworkGraph");

NetworkGraph::NetworkGraph(const std::vector<Processor*>& processors)
    : nodes_(),
    nodeCounter_(0)
{
    buildGraph(processors, GenericPortTypeCheck<Port>(), true);
}

NetworkGraph::NetworkGraph(const std::vector<Processor*>& processors,
                           const PortTypeCheck& type, const bool keepNodes)
    : nodes_(),
    nodeCounter_(0)
{
    buildGraph(processors, type, keepNodes);
}

NetworkGraph::NetworkGraph(const NetworkGraph& other) {
    copy(other);
}

NetworkGraph::~NetworkGraph() {
    clearGraph();
}

NetworkGraph& NetworkGraph::operator =(const NetworkGraph& other) {
    copy(other);
    return *this;
}


bool NetworkGraph::containsProcessor(Processor* const processor) const {
    return (findNode(processor) != 0);
}

void NetworkGraph::fullTraverseBreadthFirst(GraphVisitor* const visitor) const {
    setAllNodesMarked(false);
    std::vector<GraphNode*> roots = identifyRoots();
    for (size_t i = 0; i < roots.size(); ++i) {
        if (roots[i]->isMarked() == false)
            traverseBreadthFirstInternal(roots[i], visitor);
    }
}

void NetworkGraph::fullTraverseDepthFirst(GraphVisitor* const onDiscoveryVisitor,
                                          GraphVisitor* const onFinishVisitor) const
{
    setAllNodesDiscovered(0);
    setAllNodesFinished(0);

    int startTime = 0;      // start timestamp is 0 for first run
    std::vector<GraphNode*> roots = identifyRoots();
    for (size_t i = 0; i < roots.size(); ++i) {
        if (roots[i]->isDiscovered() == false)
            startTime = traverseDepthFirstInternal(roots[i], startTime, onDiscoveryVisitor, onFinishVisitor);
    }
}

std::vector<Processor*> NetworkGraph::getProcessors(const bool sortByID) const {
    std::vector<Processor*> processors;

    // If no sorting is required, simply copy the processor pointers to a vector.
    //
    if (sortByID == false) {
        for (NodeSet::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it)
            processors.push_back((*it)->getProcessor());
    } else {
        // Insert all nodes into a vector
        //
        std::vector<GraphNode*> nodes;
        for (NodeSet::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it)
            nodes.push_back((*it));

        // Sort that vector according to the IDs of the nodes.
        //
        std::sort(nodes.begin(), nodes.end(), NodeIDComparator());

        // Take the processors from the nodes in the sorted order.
        //
        for (size_t i = 0; i < nodes.size(); ++i)
            processors.push_back(nodes[i]->getProcessor());
    }
    return processors;
}

NetworkGraph NetworkGraph::getSubGraph(const PortTypeCheck& type, const bool keepNodes) {
    NetworkGraph subGraph;
    subGraph.buildGraph(getProcessors(), type, keepNodes);
    return subGraph;
}


NetworkGraph* NetworkGraph::getTransposed() const {
    // Copy all nodes to a new graph. The successors will not be set for
    // new nodes yet.
    //
    NetworkGraph* transposed = new NetworkGraph(nodes_);
    for (NodeSet::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it) {
        const GraphNode* const n = *it;
        const std::vector<GraphNode*>& successors = n->getSuccessors();

        // Find that current node in the transposed graph and add itself as
        // successor to all of its succesors...
        //
        GraphNode* const nt = transposed->findNode(n->getProcessor());
        if (nt == 0)
            continue;

        for (size_t s = 0; s < successors.size(); ++s) {
            const GraphNode* succ = successors[s];
            GraphNode* const st = transposed->findNode(succ->getProcessor());
            if (st == 0)
                continue;
            st->addSuccessor(nt);
        }
    }
    return transposed;
}

std::vector<Processor*> NetworkGraph::sortTopological(const std::set<Processor*> processorSubset) const {
    // Perfrom depth-first search at first
    //
    fullTraverseDepthFirst();

    // Copy the nodes to a vector so that it can be sorted.
    //
    std::vector<GraphNode*> nodesVector;
    for (NodeSet::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it)
        nodesVector.push_back(*it);

    // Sort the nodes according to their 'finish' timestep in descending order.
    //
    std::sort(nodesVector.begin(), nodesVector.end(), NodeSortTopologicalComparator());

    std::vector<Processor*> processors;

    // If the processor subset is not empty, perform some addtional tests.
    //
    if (processorSubset.empty() == false) {
        for (size_t i = 0; i < nodesVector.size(); ++i) {
            // Find the processor of the current node within the subset. If it is present,
            // add that processor to the vector which is about to be returned.
            //
            std::set<Processor*>::const_iterator it = processorSubset.find(nodesVector[i]->getProcessor());
            if (it != processorSubset.end())
                processors.push_back(nodesVector[i]->getProcessor());
        }
    } else {
        for (size_t i = 0; i < nodesVector.size(); ++i)
            processors.push_back(nodesVector[i]->getProcessor());
    }
    return processors;
}

bool NetworkGraph::traverseBreadthFirst(Processor* const processor, GraphVisitor* const visitor) const
{
    if (processor == 0)
        return false;

    GraphNode* root = findNode(processor);
    if (root == 0)
        return false;

    setAllNodesMarked(false);
    traverseBreadthFirstInternal(root, visitor);
    return true;
}

bool NetworkGraph::traverseDepthFirst(Processor* const processor,
                                      GraphVisitor* const onDiscoveryVisitor,
                                      GraphVisitor* const onFinishVisitor) const
{
    if (processor == 0)
        return false;

    GraphNode* root = findNode(processor);
    if (root == 0)
        return false;

    setAllNodesDiscovered(0);
    setAllNodesFinished(0);
    traverseDepthFirstInternal(root, 0, onDiscoveryVisitor, onFinishVisitor);
    return true;
}

// private methods
//

NetworkGraph::NetworkGraph()
    : nodes_(),
    nodeCounter_(0)
{
}

NetworkGraph::NetworkGraph(const NodeSet& nodes)
    : nodes_(),
    nodeCounter_(0)
{
    for (NodeSet::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
        // Copy that node in the node set by making a 'deep' copy. Note that
        // the new node will have no successors set yet!
        //
        const GraphNode* const n = *it;
        std::pair<NodeSet::iterator, bool> res =
            nodes_.insert(new GraphNode(n->getProcessor(), n->getID()));

        // Increase counter if a new node was created.
        //
        if (res.second == true)
            ++nodeCounter_;
    }
}

NetworkGraph::GraphNode* NetworkGraph::addNode(Processor* const processor, bool* const added)
{
    if (processor == 0) {
        if (added != 0)
            *added = false;
        return 0;
    }

    GraphNode* node = new GraphNode(processor, nodeCounter_++);
    std::pair<NodeSet::iterator, bool> res = nodes_.insert(node);

    if (added != 0)
        *added = res.second;

    // If that node was already present in this graph, delete this new
    // and superfluous node.
    //
    if (res.second == false) {
        --nodeCounter_;
        delete node;
        return (*(res.first));
    }

    return node;
}

void NetworkGraph::connectNodes(const PortTypeCheck& ptc) {
    for (NodeSet::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it) {
        GraphNode* const n1 = *it;
        n1->clearSuccessors();
        n1->inDegree_ = 0;

        Processor* const p1 = n1->getProcessor();

        // Get all outport, including co-processor outports.
        //
        std::vector<Port*> outports = p1->getOutports();
        const std::vector<CoProcessorPort*>& coOutports = p1->getCoProcessorOutports();
        outports.insert(outports.end(), coOutports.begin(), coOutports.end());

        int outportCounter = 0;
        for (size_t p = 0; p < outports.size(); ++p) {
            if (!ptc.isA(outports[p]))
                continue;

            ++outportCounter;
            const std::vector<Port*>& connectedPorts = outports[p]->getConnected();
            for (size_t c = 0; c < connectedPorts.size(); ++c) {
                Port* const outport = connectedPorts[c];
                if (!ptc.isA(outport))
                    continue;

                // Find the corresponding node in the graph for that succeeding processor
                //
                Processor* p2 = outport->getProcessor();
                GraphNode* n2 = findNode(p2);
                if (n2 == 0)
                    LERROR("buildGraph(): processor '" << p2->getName() << "' should already have a node, too!");
                else
                    n1->addSuccessor(n2);
            }   // for (c
        }   // for (p
    }   // for (it  (processors)
}
void NetworkGraph::buildGraph(const std::vector<Processor*>& processors, const PortTypeCheck& ptc,
                              const bool keepNodes)
{
    clearGraph();

    // Add new nodes for all given processors to the graph
    //
    for (size_t i = 0; i < processors.size(); ++i) {
        if ((keepNodes == false) && !ptc.hasA(processors[i]))
            continue;

        addNode(processors[i]);
    }

    connectNodes(ptc);
}

void NetworkGraph::clearGraph() {
    for (NodeSet::iterator it = nodes_.begin(); it != nodes_.end(); ++it)
        delete (*it);
    nodes_.clear();
    nodeCounter_ = 0;
}

void NetworkGraph::copy(const NetworkGraph& other) {
    clearGraph();

    for (NodeSet::const_iterator it = other.nodes_.begin(); it != other.nodes_.end(); ++it)
        nodes_.insert( new GraphNode(*(*it)) );

    // The successors are NOT copied by GraphNode, because they would have the address
    // in the other graph. So the successors must be re-inserted with the addresses they
    // have obtained by being copied to the new graph.
    //
    for (NodeSet::const_iterator it = other.nodes_.begin(); it != other.nodes_.end(); ++it) {
        const std::vector<GraphNode*>& successors = (*it)->getSuccessors();
        GraphNode* n = findNode((*it)->getProcessor());
        if (n == 0)
            continue;
        for (size_t i = 0; i < successors.size(); ++i) {
            const GraphNode* succ = successors[i];
            GraphNode* s = findNode(succ->getProcessor());
            if (s != 0)
                n->addSuccessor(s);
        }
    }
    nodeCounter_ = other.nodeCounter_;
}

NetworkGraph::GraphNode* NetworkGraph::findNode(Processor* const processor) const {
    if (processor == 0)
        return 0;

    GraphNode tmp(processor);   // node with invalid ID
    NodeSet::const_iterator it = nodes_.find(&tmp);
    if (it == nodes_.end())
        return 0;
    return (*it);
}

NetworkGraph::GraphNode* NetworkGraph::findNode(const int id) const {
    if (id < 0)
        return 0;

    GraphNode tmp(id);      // node with no processor, i.e. NULL pointer
    NodeSet::const_iterator it = nodes_.find(&tmp);
    if (it == nodes_.end())
        return 0;
    return (*it);
}

std::vector<NetworkGraph::GraphNode*> NetworkGraph::identifyRoots() const {
    std::vector<GraphNode*> roots;
    for (NodeSet::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it) {
        if ((*it)->getInDegree() == 0)
            roots.push_back(*it);
    }
    return roots;
}

void NetworkGraph::setAllNodesMarked(const bool marked) const {
    for (NodeSet::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it) {
        GraphNode* const node = const_cast<GraphNode*>(*it);
        node->setMarked(marked);
    }
}

void NetworkGraph::setAllNodesDiscovered(const int value) const {
    for (NodeSet::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it) {
        GraphNode* const node = const_cast<GraphNode*>(*it);
        node->setDiscovered(value);
    }
}

void NetworkGraph::setAllNodesFinished(const int value) const {
    for (NodeSet::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it) {
        GraphNode* const node = const_cast<GraphNode*>(*it);
        node->setFinished(value);
    }
}

void NetworkGraph::traverseBreadthFirstInternal(GraphNode* const root,
                                                GraphVisitor* const visitor) const
{
    if (root == 0)
        return;

    std::deque<GraphNode*> q;
    root->mark();
    q.push_back(root);

    while (q.empty() == false) {
        GraphNode* const n = q.front();
        q.pop_front();

        // Apply visitor if present
        //
        if (visitor != 0)
            visitor->visit(n->getProcessor());

        const std::vector<GraphNode*>& successors = n->getSuccessors();
        const int level = n->minDistanceToRoot_;
        for (size_t i = 0; i < successors.size(); ++i) {
            GraphNode* const n = successors[i];
            if (n->isMarked() == false) {
                n->mark();
                q.push_back(n);
                n->minDistanceToRoot_ = (level + 1);
            }
        }
    }   // while
}

int NetworkGraph::traverseDepthFirstInternal(GraphNode* const root, const int startTime,
                                             GraphVisitor* const onDiscoveryVisitor,
                                             GraphVisitor* const onFinishVisitor) const
{
    int time = startTime;
    std::stack<GraphNode*> stack;
    stack.push(root);
    root->setDiscovered(++time);
    if (onDiscoveryVisitor != 0)
        onDiscoveryVisitor->visit(root->getProcessor());

    while (stack.empty() == false) {
        GraphNode* const n = stack.top();
        const std::vector<GraphNode*>& successors = n->getSuccessors();
        bool newDiscovery = false;
        for (size_t j = 0; j < successors.size(); ++j) {
            GraphNode* succ = successors[j];
            if (succ->isDiscovered() == false) {
                succ->setDiscovered(++time);

                // Apply discovery visitor, if present.
                //
                if (onDiscoveryVisitor != 0)
                    onDiscoveryVisitor->visit((succ)->getProcessor());

                stack.push(succ);
                newDiscovery = true;
                break;
            }
        }

        if (newDiscovery == false) {
            stack.pop();
            n->setFinished(++time);

            // Apply finish visitor, if present.
            //
            if (onFinishVisitor != 0)
                onFinishVisitor->visit(n->getProcessor());
        }
    }   // while
    return time;
}

}   // namespace
