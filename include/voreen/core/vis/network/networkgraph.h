#ifndef VRN_NETWORKGRAPH_H
#define VRN_NETWORKGRAPH_H

#include <set>
#include <typeinfo>
#include <vector>

#include "voreen/core/vis/processors/ports/allports.h"
#include "voreen/core/vis/processors/graphvisitor.h"

namespace voreen {

class PortTypeCheck {
    public:
        virtual ~PortTypeCheck() {}
        virtual bool isA(Port* p) const = 0;
        virtual bool hasA(Processor* p) const = 0;
};

// ----------------------------------------------------------------------------

template <typename T>
class GenericPortTypeCheck : public PortTypeCheck {
    public:
        virtual bool isA(Port* p) const {
            if(dynamic_cast<T*>(p))
                return true;
            else
                return false;
            //return dynamic_cast<T*>(p);
        }
        bool hasA(Processor* p) const {
            std::vector<Port*> ports = p->getInports();
            ports.insert(ports.end(), p->getOutports().begin(), p->getOutports().end());
            ports.insert(ports.end(), p->getCoProcessorInports().begin(), p->getCoProcessorInports().end());
            ports.insert(ports.end(), p->getCoProcessorOutports().begin(), p->getCoProcessorOutports().end());

            for (size_t i = 0; i < ports.size(); ++i) {
                if (isA(ports[i]))
                    return true;
            }
            return false;
        }

};

// ----------------------------------------------------------------------------

/**
 * Class for representing processor networks as directed graphs. This class performs
 * all kinds of topological analysis, sorting and traversing within the networks.
 * It is primary intended to be used by <code>NetworkEvaluator</code> for
 * determining rendering orders and traversing the networks.
 * The visitor design-pattern is used to perform certain tasks during the graph
 * traversing. See also <code>GraphVisitor</code> class.
 *
 * @author  Dirk Feldmann, July/August 2009
 */
class NetworkGraph {
private:

    /**
     * Class for representing a node within the Graph. The nodes store the related
     * pointer to the <code>Processor</code> object and their direct successors so
     * that they form adjacency lists. Each node yields an ID to identify them with
     * a graph.
     */
    class GraphNode {
    friend class NetworkGraph;
    public:
        /**
         * Comparator class allowing to compare the content of the passed GraphNode
         * pointers instead of only comporing the pointers with each other.
         */
        struct GraphNodeComparator {
            /**
             * Operator returning false if one or more of the passed pointers are NULL
             * or the result of (*n1 < *n2) otherwise.
             */
            bool operator()(const GraphNode* const n1, const GraphNode* const n2) const {
                if ((n1 == 0) || (n2 == 0))
                    return false;
                return (*n1 < *n2);
            }
        };

        //typedef std::pair<GraphNode*, const std::type_info&> Successor;
    public:
        /**
         * Ctor which uses the passed ID for the node.
         * This ctor is primary intended to be used by NetworkGraph class only
         * for transposing graphs.
         *
         * @param   processor   The pointer to the Processor object which shall
         *                      be wrapped by this GraphNode. This pointer should
         *                      not be NULL.
         * @param   id  ID of the node. Make sure that this ID is really unique
         *              within the graph containing this node.
         */
        explicit GraphNode(Processor* const processor, const int id);

        /**
         * Ctor creating a new node for the passed Processor objects. The
         * pointer shall not be NULL and the ID of the new node is set to the
         * invalid value -1.
         * This ctor is primary used by the NetworkGraph to search for nodes
         * by the processor pointer.
         *
         * @param   processor   The pointer to the Processor object which shall
         *                      be wrapped by this GraphNode. This pointer should
         *                      not be NULL.
         */
        explicit GraphNode(Processor* const processor);

        /**
         * Ctor creating a new node which holds only and ID, but no Processor pointer.
         * The ID should be unique in the graph using this node.
         * This ctor is primary used by the NetworkGraph to search for nodes
         * by ID.
         *
         * @param   id  ID of the node.
         */
        explicit GraphNode(int id);

        /**
         * Copy-ctor performing shallow copies (pointer to Processor objects)
         * and does not copy the successors of this node (see comment on copy-
         * ctor of <code>NetworkGraph</code>).
         */
        GraphNode(const GraphNode& other);

        /**
         * Dtor. Has nothing to do.
         */
        ~GraphNode();

        /**
         * Assignment operator performing shallow copies only and does not
         * copy the successors of this node (see comment on copy-
         * ctor of <code>NetworkGraph</code>).
         */
        GraphNode& operator =(const GraphNode& other);

        /**
         * GraphNodes are defined to be "less" than another as follows:
         *
         * 1. If the internal Processor pointers are both not NULL, the
         * result from the comparision of these pointers are used.
         * 2. If at least one internal Processor pointer is NULL, the
         * result form the comparision of the IDs of the nodes are used.
         */
        bool operator <(const GraphNode& other) const;

        /**
         * Adds the passed GraphNode pointer to its successor vector, if
         * it is not NULL. Otherwise nothing will happen.
         * By adding a successor, this method increase the out-degree of this
         * node the in-degree of the successor.
         *
         * @param   successor   Successor to be added. This pointer shall not be NULL.
         */
        void addSuccessor(GraphNode* const successor);

        /**
         * Removes all successors from this node. Its out-degree will be set to 0
         * and the in-degrees of all former successor will be decreased by 1.
         */
        void clearSuccessors();

        /**
         * Return the ID of this node.
         */
        int getID() const { return id_; }

        /**
         * Returns this in-degree of this node. This is the number of direct predecessors
         * and does necessarily correspond to the number of inports to the related
         * Processor object!
         */
        int getInDegree() const { return inDegree_; }

        /**
         * Returns the minimal distance to the root which has bee calculated during
         * the last breadth-first search. The term 'root' herein also referes to that
         * particular root used by the last breadth-first search. The default value
         * is 0, so if no breadth-first search has been performed in the graph containing
         * this node, all nodes will yield zero.
         */
        int getMinDistanceToRoot() const { return minDistanceToRoot_; }

        /**
         * Returns the out-degree of this node. This is the number of direct successors
         * nodes and does not necessarily correspond to the number of outports on the
         * related Processor object!
         */
        int getOutDegree() const { return outDegree_; }

        /**
         * Return the processor object wrapped by this node.
         */
        Processor* getProcessor() const { return processor_; }

        /**
         * Returns all successor nodes of this one.
         */
        const std::vector<GraphNode*>& getSuccessors() const { return successors_; }

        /**
         * Returns the value of an internal bool marker which is used by breadth-first
         * search.
         */
        bool isMarked() const { return marked_; }

        /**
         * Sets the values of the internal bool marker, which is e.g. used by
         * breadth-first search.
         */
        void setMarked(const bool marked) { marked_ = marked; }

        /**
         * Convenience method to set the internal bool marker to true.
         */
        void mark() { marked_ = true; }

        /**
         * Convenience method to set the internal bool marker to false.
         */
        void unmark() { marked_ = false; }

        /**
         * Return the 'timestamp' of discovery of this node during depth-first search.
         */
        int getDiscovered() const { return tDiscovered_; }

        /**
         * Return the 'timestamp' of finalization of this node during depth-first search.
         */
        int getFinished() const { return tFinished_; }

        /**
         * Convenience method to determine whether this node has already been discovered
         * during depth-first search.
         */
        bool isDiscovered() const { return (tDiscovered_ > 0); }

        /**
         * Convenience method to determine whether this node has already been finished
         * during depth-first search.
         */
        bool isFinished() const { return (tFinished_ > 0); }

        /**
         * Sets the timestamp for discovery of this node during depth-first search to
         * the given value. The timestamps need to be strictly monotonic increasing
         * and must be > 0.
         */
        void setDiscovered(const int value) { tDiscovered_ = value; }

        /**
         * Sets the timestamp for discovery of this node during depth-first search to
         * the given value. The timestamps need to be strictly monotonic increasing
         * and must be > 0.
         */
        void setFinished(const int value) { tFinished_ = value; }

    private:
        /**
         * Performs internal shallow copying for copy-ctor and operator=.
         * The successors of this node will not be copied. See comment on
         * <code>NetworkGraph::copy()</code>.
         */
        void copy(const GraphNode& other);

    private:
        /** Pointer to the wrapped processor object. */
        Processor* processor_;

        /** ID of this node. Must be unique within graph using it. */
        int id_;

        /** All direct successor nodes of this one. */
        std::vector<GraphNode*> successors_;

        /** Number of direct predecessor nodes. */
        int inDegree_;

        /** Number of direct successor nodes. */
        int outDegree_;

        /** Marker used by breadth-first search. */
        bool marked_;

        /** Minimal distance to the root determined during last breadth-first search. */
        int minDistanceToRoot_;

        /** Timestamp of discovery of this node during depth-first search. */
        int tDiscovered_;

        /** Timestamp of finalization of this node during depth-first search. */
        int tFinished_;
    };  // class GraphNode


public:
    /**
     * Ctor contruction NetworkGraph with according to the given processors and
     * the connections between them.
     */
    explicit NetworkGraph(const std::vector<Processor*>& processors);

    /**
     * Ctor contruction NetworkGraph with according to the given processors and
     * the connections between them, but confines them to processors and connections
     * which use the given PortTypeCheck.
     *
     * @param   type    Port of ports and connections to which the graph shall be
     *                  confined. Acts like a kind of mask.
     * @param   keepNodes   Determines whether processors/nodes which are not affected
     *                      by the given PortTypeCheck shall be kept in the graph or
     *                      disposed.
     */
    explicit NetworkGraph(const std::vector<Processor*>& processors, const PortTypeCheck& type,
        const bool keepNodes = false);

    /**
     * Copy-ctor. This methods calls <code>copy()</code>
     */
    NetworkGraph(const NetworkGraph& other);

    /**
     * Dtor. Calls <code>clearGraph()</code>.
     */
    ~NetworkGraph();

    /**
     * Assignment-operator internally calling <code>copy()</code>.
     */
    NetworkGraph& operator =(const NetworkGraph& other);

    /**
     * Connects the nodes within the graph. The given PortTypeCheck can confine
     * the connections to be made.
     *
     * @param   ptc PortTypeCheck which determines, whether a connection between
     *              the nodes will be established, according to the type of the
     *              port on the internal Processor pointer of the node.
     */
    void connectNodes(const PortTypeCheck& ptc = GenericPortTypeCheck<Port>());

    /**
     * Indicates whether the given processor is containing within this graph.
     *
     * @return  true if the processor is contained, or false otherwise of if
     *          the given pointer is NULL.
     */
    bool containsProcessor(Processor* const processor) const;

    /**
     * Traverses the entire graph in breadth-first order. This means,
     * that every node within the graph is a potential root. The roots
     * for the search are determined by <code>identifyRoots()</code>.
     * Thereby, all connected components will be found. This method
     * internally calls <code>traverseBreadthFirstInternal()</code>.
     * After traversing, all bool marks of the nodes are set to true.
     * Before traversing, this method sets all bool markers of the nodes
     * to false. Furthermore, the breadth-first search determines the
     * minimal distance to the root for this node, for each connected
     * component of the graph.
     *
     * @param   visitor A visitor according to the visitor design-pattern
     *                  which is applied to each node when it is discovered
     *                  by breadth-first search, and if it is not NULL.
     */
    void fullTraverseBreadthFirst(GraphVisitor* const visitor = 0) const;

    /**
     * Traverses the entire graph in depth-first order. This means, that
     * every node within the graph is a potential root. The roots for the
     * search are determined by <code>identifyRoots()</code>. Thereby,
     * all connected components will be found. This method internally calls
     * <code>traverseBreadthFirstInternal()</code>.
     * After traversing, all discovery timestamps and all finalization
     * timestamps are > 0. Before traversing, all these timestamps are set
     * to 0 again.
     *
     * @param   onDiscoveryVisitor  A vistor according to the visitor desgin-
     * pattern which will be applied to all nodes when they are discovered by
     * the depth-first search, and if it is not NULL.
     * @param   onFinishVisitor  A vistor according to the visitor desgin-
     * pattern which will be applied to all nodes when they are finished by
     * the depth-first search, and if it is not NULL.
     */
    void fullTraverseDepthFirst(GraphVisitor* const onDiscoveryVisitor = 0,
        GraphVisitor* const onFinishVisitor = 0) const;

    /**
     * Returns all processors from all nodes in this graph. The order may be
     * chosen to be unsorted or the processors can be ordered according to the
     * ID of their nodes. The ID of the nodes corresponds to the order of insertion
     * into the graph.
     *
     * @param   sortByID    Determines whether to sort the processor according to
     *                      the IDs of their corresponding nodes or not.
     * @return  Vector containing pointer to all Processor objects which are
     *          represented by nodes within this graph.
     */
    std::vector<Processor*> getProcessors(const bool sortByID = false) const;

    /**
     * Returns a subset of this graph containing only processors with ports and connections
     * between them, which match the given PortTypeCheck criteria. All other processors
     * having different ports and connections can be chosen to be disposed.
     *
     * @param   type    PortTypeCheck specifying, which type of port and connections shall
     *                  be maintained in the new sub-graph.
     * @param   keepNodes   Determines whether to keep nodes, which do not match the
     *                      criteria specified via the given PortTypeCheck.
     * @return  A new NetworkGraph which contains only nodes with processors having ports
     *          and connections of the type specified by the parameter type.
     */
    NetworkGraph getSubGraph(const PortTypeCheck& type = GenericPortTypeCheck<Port>(),
        const bool keepNodes = false);

    /**
     * Returns the transposed graph. A transposed directed graph is a graph
     * where all edges (u, v) are replaced by the edges (v, u). This is done by
     * inverting the role of predecessor and successor for all nodes within
     * this graph.
     * The caller has to take care of deleting the returned graph via delete.
     *
     * @return  Pointer to the transposed graph. Caller has to free it via
     *          delete.
     */
    NetworkGraph* getTransposed() const;

    /**
     * Sorts the graph topolocial. The topological sorting is used to determine the
     * order of evaluation for the processors by the NetworkEvaluator.
     *
     * NOTE: Topological sorting works fine in acyclic directed graphs, but I don't know
     * whether this will work in all cases, if cycles are present. No check is for cycles
     * in the graph is performed, because they might be wanted.
     *
     * @param   processorSubset If the given set is not empty, only the processors which are
     * are also in this set will be included in the returned vector. Otherwise all processors
     * will be included.
     * @return  Topological sorting of processors which can be used as rendering order.
     */
    std::vector<Processor*> sortTopological(const std::set<Processor*> processorSubset = std::set<Processor*>()) const;

    /**
     * Performs breadth-first search in the graph, starting at the node which
     * contains the given processor, if present. Otherwise, nothing will happen.
     * This method internally calls <code>traverseBreadthFirstInternal()</code>.
     * After traversing, all bool marks of the nodes are set to true.
     * Before traversing, this method sets all bool markers of the nodes
     * to false. Furthermore, the breadth-first search determines the
     * minimal distance to the given root for this node.
     *
     * @param   root    The node containing this processor will act as the root
     *                  for the breadth-first search.
     * @param   visitor A visitor according to the visitor design-pattern
     *                  which is applied to each node when it is discovered
     *                  by breadth-first search, and if it is not NULL.
     * @return  true if the search was perfomed, false if it was not, e.g. if
     *          no node with the given processor could be found.
     */
    bool traverseBreadthFirst(Processor* const root, GraphVisitor* const visitor = 0) const;

    /**
     * Performs breadth-first search in the graph, starting at the node which
     * contains the given processor, if present. Otherwise, nothing will happen.
     * This method internally calls <code>traverseBreadthFirstInternal()</code>.
     * After traversing, all discovery timestamps and all finalization
     * timestamps are > 0. Before traversing, all these timestamps are set
     * to 0 again.
     *
     * @param   root    The node containing this processor will act as the root
     *                  for the breadth-first search.
     * @param   onDiscoveryVisitor  A vistor according to the visitor desgin-
     *      pattern which will be applied to all nodes when they are discovered by
     *      the depth-first search, and if it is not NULL.
     * @param   onFinishVisitor  A vistor according to the visitor desgin-
     *      pattern which will be applied to all nodes when they are finished by
     *      the depth-first search, and if it is not NULL.
     * @return  true if the search was perfomed, false if it was not, e.g. if
     *          no node with the given processor could be found.
     */
    bool traverseDepthFirst(Processor* const root, GraphVisitor* const onDiscoveryVisitor = 0,
        GraphVisitor* const onFinishVisitor = 0) const;

private:
    typedef std::set<GraphNode*, GraphNode::GraphNodeComparator> NodeSet;

    /**
     * Comparator used to sort the nodes topological. This requires a previous
     * depth-first search for the affected nodes.
     */
    struct NodeSortTopologicalComparator {
        /**
         * This operator returns the result from the comparion of the 'finish' timestamps
         * of the nodes. The nodes have to sorted in descending order of their 'finish'
         * timestamps.
         */
        bool operator ()(const GraphNode* const n1, const GraphNode* const n2) const {
            if ((n1 == 0) || (n2 == 0))
                return false;
            return (n1->getFinished() >= n2->getFinished());
        }
    };

    /**
     * Comparator class to compare pointers to GraphNode objects by their ID only, rather
     * than taking into account the pointers to the internal Processor objects as well.
     */
    struct NodeIDComparator {
        /**
         * Operator which defines n1 to be smaller than n2 if the ID of n1 is less
         * than the one of n2.
         */
        bool operator ()(const GraphNode* const n1, const GraphNode* const n2) const {
            if ((n1 == 0) || (n2 == 0))
                return false;
            return (n1->getID() < n2->getID());
        }
    };

private:
    /**
     * Ctor creating an empty graph.
     */
    NetworkGraph();

    /**
     * Ctor construction a graph with deep copies of the nodes in the given set, but
     * without making any connection between the nodes and without making any
     * determination of successors for the nodes.
     * This is used for instance by creating transposed graphs.
     */
    NetworkGraph(const NodeSet& nodes);

    /**
     * Adds a new GraphNode fot the given processor to this graph and returns it.
     * The new GraphNode objects gets an ID which corresponds to its number on
     * insertion. If the node already exists in the graph, the existing node will
     * be returned and no new node will be created.
     *
     * @param   processor   Pointer to the Processor object for which a node shall be
     *                      added. This pointer must not be NULL.
     * @param   added   Pointer to a bool which may hold the information, if a new node
     *                  has been created or not. If this pointer is NULL, the information
     *                  about possible new creations will not be stored to it.
     * @return  The new GraphNode object, if it was created newly, or the exisiting one,
     *          containing the given processor.
     */
    GraphNode* addNode(Processor* const processor, bool* const added = 0);

    /**
     * Acutally creates the graph from the given processors. The PortTypeCheck may be
     * used to determine what kind of processor and connections may be contained in
     * the graph.
     * Before creating the graph, the previous graph will be cleared by a call to
     * <code>clearGraph()</code>.
     *
     * @param   processors  Vector of all processors which shall be contained.
     * @param   ptc PortTypeCheck to confine the nodes/processors and connections
     *              to those, which fulfill its criterion.
     * @param   keepNodes   Determines whether nodes for processors not matching the
     *                      the criterion by parameter ptc shall be kept in the graph
     *                      or not.
     */
    void buildGraph(const std::vector<Processor*>& processors, const PortTypeCheck& ptc,
        const bool keepNodes = true);

    /**
     * Clears the entire graph be deleting all nodes and resetting the node counter.
     */
    void clearGraph();

    /**
     * Copies the graph. This is done by making deep copies of the nodes from the other
     * graph, but without successors. When all nodes are copied, all connections for
     * the successor in the other graph will be made for the copies. This is necessary
     * because if the successors would have been copied from the other graph, the nodes
     * in this new graph would know their memory addresses in the other graph! But a
     * corresponding successor S for a node N in this graph has a diffrent address S' in
     * the other graph.
     */
    void copy(const NetworkGraph& other);

    /**
     * Returns the pointer to the GraphNode object containing the given processor, if
     * existing, or NULL otherwise.
     */
    GraphNode* findNode(Processor* const processor) const;

    /**
     * Returns the pointer to the GraphNode object with the given ID, if
     * existing, or NULL otherwise.
     */
    GraphNode* findNode(const int id) const;

    /**
     * Identifies roots for full travering the graph via breadth-first or depth-first
     * search. A node is assumed to be a root, if it has a in-degree of 0. Note that
     * this does not reflect the role of the processor related to that node. Processors
     * acting as roots/sources in the network are identified by the return value of
     * <code>Processor::isRootProcessor()</code>.
     */
    std::vector<GraphNode*> identifyRoots() const;

    /**
     * Sets the 'discovery' timestamps of all nodes to the given value. This is used to
     * intialize depth-first search.
     */
    void setAllNodesDiscovered(const int value) const;

    /**
     * Sets the 'finish' timestamps of all nodes to the given value. This is used to
     * intialize depth-first search.
     */
    void setAllNodesFinished(const int value) const;

    /**
     * Sets the bool mark of all nodes to the given value. This is used to
     * intialize breadth-first search.
     */
    void setAllNodesMarked(const bool marked) const;

    /**
     * Performs the actual breadth-first rraversin in this graph by
     * starting at the given GraphNode.
     * After traversing, all bool marks of the nodes which were found
     * during the search are set to true.
     * Before traversing, this method sets all bool markers of the nodes
     * to false. Furthermore, the breadth-first search determines the
     * minimal distance to the given root for this node.
     *
     * @param   root    The node which will act as the root for the
     *                  breadth-first search.
     * @param   visitor A visitor according to the visitor design-pattern
     *                  which is applied to each node when it is discovered
     *                  by breadth-first search, and if it is not NULL.
     */
    void traverseBreadthFirstInternal(GraphNode* const root, GraphVisitor* const visitor = 0) const;

    /**
     * After the transversing, every node which coudl be reaced from the given root
     * has a timestamp for its discovery and a timestamp for its finalization,
     * > 0 each and strictly monotic increasing.
     * The timestamps indicate the order of traversal. A node is finished, if the
     * depth-first search has visited all nodes which are reachable form that node
     * and it is about to 'backtrack' to its predecessor.
     *
     * @param   root    The node which will act as the root for the depth-first search.
     * @param   startTime   Timestamp at which the depth-first search starts. This
     *      should be 0 for the first connected component, and T1 for the second,
     *      with T1 denoting the timestamp returned by this method from the for run,
     *      etc..
     * @param   onDiscoveryVisitor  A vistor according to the visitor desgin-
     *      pattern which will be applied to all nodes when they are discovered by
     *      the depth-first search, and if it is not NULL.
     * @param   onFinishVisitor  A vistor according to the visitor desgin-
     *      pattern which will be applied to all nodes when they are finished by
     *      the depth-first search, and if it is not NULL.
     * @return  Return the timestamp of the finalization of the root. This value
     *      is shall be used for a successive call to this method as startTime when
     *      traversing another connected component of this graph with a different root.
     */
    int traverseDepthFirstInternal(GraphNode* const root, const int startTime = 0,
        GraphVisitor* const onDiscoveryVisitor = 0, GraphVisitor* const onFinishVisitor = 0) const;

private:
    /** The set containing all nodes of this graph. */
    NodeSet nodes_;

    /**
     * Counter for the nodes. This counter is used to assign IDs to new nodes
     * according to their order of insertion.
     */
    int nodeCounter_;

    static const std::string loggerCat_;
};

}   // namespace

#endif
