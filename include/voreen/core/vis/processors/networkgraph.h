#ifndef VRN_NETWORKGRAPH_H
#define VRN_NETWORKGRAPH_H

#include <ostream>
#include <set>
#include <typeinfo>
#include <vector>

#include "voreen/core/vis/processors/port.h"

namespace voreen {

class GraphVisitor;

class NetworkGraph {
private:
    class GraphNode {
    friend class NetworkGraph;
    public:
        struct GraphNodeComparator {
            bool operator()(const GraphNode* const n1, const GraphNode* const n2) const {
                if ((n1 == 0) || (n2 == 0))
                    return false;
                return (*n1 < *n2);
            }
        };

        typedef std::pair<GraphNode*, Port::PortType> Successor;
    public:
        explicit GraphNode(Processor* const processor, const int id);
        explicit GraphNode(Processor* const processor);
        explicit GraphNode(int id);
        GraphNode(const GraphNode& other);
        ~GraphNode();

        GraphNode& operator =(const GraphNode& other);
        bool operator <(const GraphNode& other) const;

        void addSuccessor(GraphNode* const successor, 
            const Port::PortType& type = Port::PORT_TYPE_UNSPECIFIED);

        int getID() const { return id_; }
        int getInDegree() const { return inDegree_; }
        int getMinDistanceToRoot() const { return minDistanceToRoot_; }
        int getOutDegree() const { return outDegree_; }
        Processor* getProcessor() const { return processor_; }
        const std::vector<Successor>& getSuccessors() const { return successors_; }

        bool isMarked() const { return marked_; }
        void setMarked(const bool marked) { marked_ = marked; }
        void mark() { marked_ = true; }
        void unmark() { marked_ = false; }

        bool isDiscovered() const { return (tDiscovered_ > 0); }
        bool isFinished() const { return (tFinished_ > 0); }
        int getDiscovered() const { return tDiscovered_; }
        int getFinished() const { return tFinished_; }
        void setDiscovered(const int value) { tDiscovered_ = value; }
        void setFinished(const int value) { tFinished_ = value; }

    private:
        void copy(const GraphNode& other);

    private:
        Processor* processor_;
        int id_;
        std::vector<Successor> successors_;
        int inDegree_;
        int outDegree_;
        bool marked_;
        int minDistanceToRoot_;
        int tDiscovered_;
        int tFinished_;
        int connectionType_;
    };  // class GraphNode

friend std::ostream& operator <<(std::ostream& os, const NetworkGraph::GraphNode& n);

public:
    NetworkGraph(const NetworkGraph& other);
    explicit NetworkGraph(const std::vector<Processor*>& processors);
    explicit NetworkGraph(const std::vector<Processor*>& processors, const int edgeMask, 
        const bool keepNodes = false);
    ~NetworkGraph();

    bool hasLeaves() const;

    void printGraph() const;

    void fullTraverseBreadthFirst(GraphVisitor* const visitor = 0) const;
    void fullTraverseDepthFirst(GraphVisitor* const onDiscoveryVisitor = 0,
        GraphVisitor* const onFinishVisitor = 0) const;

    std::vector<Processor*> getProcessors(const bool sortByID = false) const;
    NetworkGraph getSubGraph(const int edgeMask = 0, const bool keepNodes = false);
    NetworkGraph* getTransposed() const;
    std::vector<Processor*> sortTopological() const;
    bool traverseBreadthFirst(Processor* const root, GraphVisitor* const visitor = 0) const;
    bool traverseDepthFirst(Processor* const root, GraphVisitor* const onDiscoveryVisitor = 0,
        GraphVisitor* const onFinishVisitor = 0) const;

    NetworkGraph& operator =(const NetworkGraph& other);

private:
    typedef std::set<GraphNode*, GraphNode::GraphNodeComparator> NodeSet;

    struct NodeSortTopologicalComparator {
        bool operator ()(const GraphNode* const n1, const GraphNode* const n2) const {
            if ((n1 == 0) || (n2 == 0))
                return false;
            return (n1->getFinished() >= n2->getFinished());
        }
    };

    struct NodeIDComparator {
        bool operator ()(const GraphNode* const n1, const GraphNode* const n2) const {
            if ((n1 == 0) || (n2 == 0))
                return false;
            return (n1->getID() < n2->getID());
        }
    };

private:
    NetworkGraph();
    NetworkGraph(const NodeSet& nodes);

    void buildGraph(const std::vector<Processor*>& processors, int edgeMask = 0, 
        const bool keepNodes = true);
    void clearGraph();
    void copy(const NetworkGraph& other);

    GraphNode* addNode(Processor* const processor, bool* const added = 0);
    GraphNode* findNode(Processor* const processor) const;
    GraphNode* findNode(const int id) const;
    std::vector<GraphNode*> identifyRoots() const;

    void setAllNodesDiscovered(const int value) const;
    void setAllNodesFinished(const int value) const;
    void setAllNodesMarked(const bool marked) const;

    void traverseBreadthFirstInternal(GraphNode* const root, GraphVisitor* const visitor = 0) const;
    int traverseDepthFirstInternal(GraphNode* const root, const int startTime = 0,
        GraphVisitor* const onDiscoveryVisitor = 0, GraphVisitor* const onFinishVisitor = 0) const;

private:
    NodeSet nodes_;
    int nodeCounter_;
    bool hasLeaves_;

    static const std::string loggerCat_;
};

}   // namespace

#endif
