#include "voreen/core/vis/processors/networkgraph.h"

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

std::ostream& operator <<(std::ostream& os, const NetworkGraph::GraphNode& n) {
    return (os << "[" << n.getID() << "] (" 
        << ((n.getProcessor() != 0) ? n.getProcessor()->getName() : "NULL") << ")");
}

void NetworkGraph::GraphNode::addSuccessor(NetworkGraph::GraphNode* const successor, 
                                           const Port::PortType& type)
{
    if (successor == 0)
        return;

    successors_.push_back(Successor(successor, type));
    ++(successor->inDegree_);
    ++outDegree_;
}

// private methods
//

void NetworkGraph::GraphNode::copy(const NetworkGraph::GraphNode& other) {
    processor_ = other.processor_;  // shallow copy only!
    id_ = other.id_;
    successors_ = std::vector<Successor>();
    inDegree_ = 0;
    outDegree_ = 0;
    marked_ = other.marked_;
    minDistanceToRoot_ = other.minDistanceToRoot_;
    tDiscovered_ = other.tDiscovered_;
    tFinished_= other.tFinished_;
    connectionType_ = other.connectionType_;
}

// ============================================================================

const std::string NetworkGraph::loggerCat_("voreen.NetworkGraph");

NetworkGraph::NetworkGraph(const NetworkGraph& other) {
    copy(other);
}

NetworkGraph::NetworkGraph(const std::vector<Processor*>& processors)
    : nodes_(),
    nodeCounter_(0),
    hasLeaves_(false)
{
    buildGraph(processors, 0, true);
}

NetworkGraph::NetworkGraph(const std::vector<Processor*>& processors,
                           const int edgeMask, const bool keepNodes)
    : nodes_(),
    nodeCounter_(0),
    hasLeaves_(false)
{
    buildGraph(processors, edgeMask, keepNodes);
}

NetworkGraph::~NetworkGraph() {
    clearGraph();
}

NetworkGraph& NetworkGraph::operator =(const NetworkGraph& other) {
    copy(other);
    return *this;
}

void NetworkGraph::printGraph() const {
    std::cout << "nodes (" << nodes_.size() << "):\n";
    std::cout << "-----------\n";
    for (NodeSet::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it) {
        std::cout << *(*it) << "\n";
        std::cout << "Connections:\n";
        const std::vector<GraphNode::Successor>& successors = (*it)->getSuccessors();
        for (size_t s = 0; s < successors.size(); ++s) {
            std::cout << "\t" << *(successors[s].first) << ", type = ";
            std::cout << static_cast<int>(successors[s].second) << "\n";
        }
    }
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
    int startTime = 0;
    std::vector<GraphNode*> roots = identifyRoots();
    for (size_t i = 0; i < roots.size(); ++i) {
        if (roots[i]->isDiscovered() == false)
            startTime = traverseDepthFirstInternal(roots[i], startTime, onDiscoveryVisitor, onFinishVisitor);
    }
}

std::vector<Processor*> NetworkGraph::getProcessors(const bool sortByID) const {
//std::cout << "getProcessors():\n";
    std::vector<Processor*> processors;
    if (sortByID == false) {
        for (NodeSet::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it) {
//std::cout << "\tnode = " << *(*it) << "\n";
            processors.push_back((*it)->getProcessor());
        }
    } else {
        std::vector<GraphNode*> nodes;
        for (NodeSet::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it)
            nodes.push_back((*it));

        std::sort(nodes.begin(), nodes.end(), NodeIDComparator());        

        for (size_t i = 0; i < nodes.size(); ++i) {
//std::cout << "\tnode = " << *nodes[i] << "\n";
            processors.push_back(nodes[i]->getProcessor());
        }
    }
    return processors;
}

NetworkGraph NetworkGraph::getSubGraph(const int edgeMask, const bool keepNodes) {
    NetworkGraph subGraph;
    subGraph.buildGraph(getProcessors(), edgeMask, keepNodes);
    return subGraph;
}


NetworkGraph* NetworkGraph::getTransposed() const {
    // Copy all nodes to a new graph. The successors will not be set for
    // new nodes yet.
    //
    NetworkGraph* transposed = new NetworkGraph(nodes_);
//std::cout << "NetworkGraph::transpose()...\n";
    for (NodeSet::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it) {
        const GraphNode* const n = *it;
        const std::vector<GraphNode::Successor>& successors = n->getSuccessors();

        // find that current node in the transposed graph and add itself as
        // successor to all of its succesors...
        //
        GraphNode* const nt = transposed->findNode(n->getProcessor());
        if (nt == 0)
            continue;

//std::cout << "\ttransposing edges on " << *nt << "\n";
        for (size_t s = 0; s < successors.size(); ++s) {
            const GraphNode::Successor& succ = successors[s];
            GraphNode* const st = transposed->findNode(succ.first->getProcessor());
            if (st == 0)
                continue;
//std::cout << "\t\tcurrent successor st is " << *st << "\n";
            st->addSuccessor(nt, succ.second);
        }
    }
    return transposed;
}

bool NetworkGraph::hasLeaves() const {
    return hasLeaves_;
}

std::vector<Processor*> NetworkGraph::sortTopological() const {
    fullTraverseDepthFirst();
//std::cout << "sortTopological()...\n";
    
    std::vector<GraphNode*> nodesVector;
    for (NodeSet::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it)
        nodesVector.push_back(*it);

    std::sort(nodesVector.begin(), nodesVector.end(), NodeSortTopologicalComparator());

    std::vector<Processor*> processors;
    for (std::vector<GraphNode*>::iterator it = nodesVector.begin(); nodesVector.empty() == false;
        it = nodesVector.begin()) {
        const GraphNode* const node = *it;
//std::cout << "\tnode = " << *node << "\n";
        processors.push_back(node->getProcessor());
        nodesVector.erase(it);
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
    nodeCounter_(0),
    hasLeaves_(false)
{
}

NetworkGraph::NetworkGraph(const NodeSet& nodes) 
    : nodes_(),
    nodeCounter_(0),
    hasLeaves_(false)
{
    for (NodeSet::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
        const GraphNode* const n = *it;
        std::pair<NodeSet::iterator, bool> res = 
            nodes_.insert(new GraphNode(n->getProcessor(), n->getID()));
        if (res.second == true)
            ++nodeCounter_;
    }
}

NetworkGraph::GraphNode* NetworkGraph::addNode(Processor* const processor, bool* const added)
{
    GraphNode* node = new GraphNode(processor, nodeCounter_++);
    std::pair<NodeSet::iterator, bool> res = nodes_.insert(node);

    if (added != 0)
        *added = res.second;

    if (res.second == false) {
        --nodeCounter_;
        delete node;
        return (*(res.first));
    }

    return node;
}

void NetworkGraph::buildGraph(const std::vector<Processor*>& processors, int edgeMask, 
                              const bool keepNodes)
{
//std::cout << "buildGraph():\n";
    clearGraph();

    if (edgeMask == 0)
        edgeMask = ~0;

    // Add new nodes for all given processors to the graph
    //
    for (size_t i = 0; i < processors.size(); ++i) {
        if ((keepNodes == false) && (processors[i]->hasPortOfCertainType(edgeMask) == false))
            continue;

        addNode(processors[i]);
//GraphNode* n = addNode(processors[i]);
//std::cout << "add processor '" << processors[i]->getName() << "' to node " << *n << "\n";
    }

    for (NodeSet::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it) {
        GraphNode* const n1 = *it;
        Processor* const p1 = n1->getProcessor();

        // Get all outport, including co-processor outports.
        //
        std::vector<Port*> outports = p1->getOutports();
        const std::vector<Port*>& coOutports = p1->getCoProcessorOutports();
        outports.insert(outports.end(), coOutports.begin(), coOutports.end());

        int outportCounter = 0;
        for (size_t p = 0; p < outports.size(); ++p) {
            int outportType = static_cast<int>(outports[p]->getType());
            if ((edgeMask & outportType) == 0)
                continue;

            ++outportCounter;
            const std::vector<Port*>& connectedPorts = outports[p]->getConnected();
            for (size_t c = 0; c < connectedPorts.size(); ++c) {
                Port* const outport = connectedPorts[c];
                int inportType = static_cast<int>(outport->getType());
                if ((edgeMask & inportType) == 0)
                    continue;

                // Find the corresponding node in the graph for that succeeding processor
                //
                Processor* p2 = outport->getProcessor();
                GraphNode* n2 = findNode(p2);
                if (n2 == 0)
                    LERROR("buildGraph(): processor '" << p2->getName() << "' should already have a node, too!");
                else
                    n1->addSuccessor(n2, outport->getType());
            }   // for (c
        }   // for (p

        if (outportCounter > 0)
            hasLeaves_ = true;
    }   // for (it  (processors)
}

void NetworkGraph::clearGraph() {
    for (NodeSet::iterator it = nodes_.begin(); it != nodes_.end(); ++it)
        delete (*it);
    nodes_.clear();
    hasLeaves_ = false;
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
        const std::vector<GraphNode::Successor>& successors = (*it)->getSuccessors();
        GraphNode* n = findNode((*it)->getProcessor());
        if (n == 0)
            continue;
        for (size_t i = 0; i < successors.size(); ++i) {
            const GraphNode::Successor& succ = successors[i];
            GraphNode* s = findNode(succ.first->getProcessor());
            if (s != 0)
                n->addSuccessor(s, succ.second);
        }
    }
    nodeCounter_ = other.nodeCounter_;
}

NetworkGraph::GraphNode* NetworkGraph::findNode(Processor* const processor) const {
    if (processor == 0)
        return 0;

    GraphNode tmp(processor);
    NodeSet::const_iterator it = nodes_.find(&tmp);
    if (it == nodes_.end())
        return 0;
    return (*it);
}

NetworkGraph::GraphNode* NetworkGraph::findNode(const int id) const {
    if (id < 0)
        return 0;

    GraphNode tmp(id);
    NodeSet::const_iterator it = nodes_.find(&tmp);
    if (it == nodes_.end())
        return 0;
    return (*it);
}

std::vector<NetworkGraph::GraphNode*> NetworkGraph::identifyRoots() const {
//std::cout << "NetworkGraph::identifyRoots():\n";
    std::vector<GraphNode*> roots;
    for (NodeSet::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it) {
        if ((*it)->getInDegree() == 0) {
            roots.push_back(*it);
//std::cout << "\tnode " << *(*it) << " is root...\n";
        }
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
//std::cout << "traverseBreadthFirstInternal():\n";
    root->mark();
    std::deque<GraphNode*> q;
    q.push_back(root);
    while (q.empty() == false) {
        GraphNode* const n = q.front();
        q.pop_front();

        if (visitor != 0)
            visitor->visit(n->getProcessor());
//std::cout << "\tnode = " << *n << "\n";
        const std::vector<GraphNode::Successor>& successors = n->getSuccessors();
        const int level = n->minDistanceToRoot_;
        for (size_t i = 0; i < successors.size(); ++i) {
            const GraphNode::Successor& succ = successors[i];
            if (succ.first->isMarked() == false) {
                succ.first->mark();
                q.push_back(succ.first);
                succ.first->minDistanceToRoot_ = (level + 1);
            }
        }
    }   // while
}

int NetworkGraph::traverseDepthFirstInternal(GraphNode* const root, const int startTime,
                                             GraphVisitor* const onDiscoveryVisitor, 
                                             GraphVisitor* const onFinishVisitor) const
{
//std::cout << "traverseDepthFirstInternal():\n";
    int time = startTime;
    std::stack<GraphNode*> stack;
    stack.push(root);
    root->setDiscovered(++time);
    if (onDiscoveryVisitor != 0)
        onDiscoveryVisitor->visit(root->getProcessor());

    while (stack.empty() == false) {
        GraphNode* const n = stack.top();
        const std::vector<GraphNode::Successor>& successors = n->getSuccessors();
        bool newDiscovery = false;
        for (size_t j = 0; j < successors.size(); ++j) {
            const GraphNode::Successor& succ = successors[j];
            if (succ.first->isDiscovered() == false) {
                succ.first->setDiscovered(++time);
                if (onDiscoveryVisitor != 0)
                    onDiscoveryVisitor->visit((succ.first)->getProcessor());

                stack.push(succ.first);
                newDiscovery = true;
                break;
            }
        }

        if (newDiscovery == false) {
            stack.pop();
            n->setFinished(++time);
//std::cout << "\tnode = " << *n << "\n";
            if (onFinishVisitor != 0)
                onFinishVisitor->visit(n->getProcessor());
        }
    }   // while
    return time;
}

}   // namespace
