/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#ifndef VRN_POLYGON3D_H
#define VRN_POLYGON3D_H

#ifndef TGT_VECTOR_H
#include "tgt/vector.h"
#endif

#include <set>
#include <list>

#include <iostream>

typedef unsigned long DWORD;

namespace voreen
{

class EdgeVertex3D;
class Edge3D;
class PolygonFace3D;
class Polygon3D;

// ---------------------------------------------------------------------

/**
 * Class for the faces of a Polygon in 3D
 *
 * @author  Dirk Feldmann 
 */
class PolygonFace3D
{
public:
    /**
     * Structure with method for comparing PolygonFace3D-pointers within a set. This ensures, that
     * the pointers are dereferenced and compared. Otherwise the pointers (addresses)
     * would be compared.
     */
    struct FaceComparator
    {
        bool operator()(const PolygonFace3D* const f1, const PolygonFace3D* const f2) const {
            if( (f1 == 0) || (f2 == 0 ) )
                return false;

            return (*f1 < *f2);
        }
    };
    
    /**
     * Faces are ordered by their ids, so that a strict weak order can be performed and
     * a std::set can be used
     */
    typedef std::set<PolygonFace3D*, FaceComparator> FaceSet;

public:
    /**
     * Constructor for a face with the give ID and the given normal
     */
    PolygonFace3D(const DWORD ID, const tgt::vec3& normal);
    
    PolygonFace3D(const PolygonFace3D& face);

    /**
     * Destructor performs deletion of the face and removes its
     * references from the edges in its own edge list.
     */
    ~PolygonFace3D();

    PolygonFace3D& operator=(const PolygonFace3D& face);

    /**
     * Faces are compared by their IDs.
     */
    bool operator==(const PolygonFace3D& f) const;
    bool operator!=(const PolygonFace3D& f) const;
    bool operator<(const PolygonFace3D& f) const;
    bool operator<=(const PolygonFace3D& f) const;
    bool operator>(const PolygonFace3D& f) const;
    bool operator>=(const PolygonFace3D& f) const;

    /**
     * <b>INSERT</b> the given edge in the list of edges at 
     * its correct position.
     *
     * @return  true if the insertion was successull, false if the
     * the edge cannot be inserted or is already contained.
     *
     */
    bool insertEdge(Edge3D* const e);

    /**
     * <b>ADD</b> the given edge at the end of the list of edges.
     * The list of edges will be in arbitrary order and needs
     * to be sorted before <code>getVertices()</code> can be called in order
     * to render the face. Use <code><b>sortEdges()</b></code> to obtain
     * a correct order of the edges which can be rendered by calling
     * <code>glBegin(GL_POLYGON)</code>.
     *
     * @see    sortEdges(), getVertices()
     *
     * @param   e Edge to be added.
     * @return true if the edges was added, false otherwise
     */
    bool addEdge(Edge3D *const e);

    /**
     * Sort the edges so that the vertices can be obtained by calling
     * <code>getVertices()</code> in an order that can be rendered by
     * glBegin(GL_POLYGON).
     */
    void sortEdges();

    /**
     * Returns the edge if it is contained in the edge list
     * and is equal to the given edge.
     *
     * @return  the wanted edge if it is contained in the list or 0 otherwise
     */
    Edge3D* findEdge(Edge3D* const e);

    /**
     * Remove the given edge from the face.
     *
     * @return  true if the edge is contained and removed successfully, false otherwise
     */
    bool removeEdge(Edge3D *const e);

    /**
     * Returns the ID of the the face.
     */
    DWORD getID() const;

    /**
     * Returns the normal of the face.
     */
    const tgt::vec3& getNormal() const;

    /**
     * Returns the edges of the face. The order depends on the previous insertions
     * by <code>insertEdge()</code> or whether a call to <code>sortEdges()</code>
     * has been made or not.
     */
    const std::list<Edge3D*> getEdges() const;

    /**
     * Returns the number of edges in the edge list.
     */
    DWORD getNumEdges() const;

    /**
     * Returns the vertices of the edges. Each vertex is returned only once.
     * If the edges and their vertices are unordered, rendering polygons will probably
     * fail. Use <code>sortEdges()</code> to ensure an order of vertices which is
     * suited for rendering the face by calling <code>glBegin(GL_POLYGON)</code>.
     * Before returning the list, rebuildVertexList_ is evaluated and the list is
     * rebuilt if necessary. This is inevitable if changes on this face have been made.
     *
     * <b>NOTE:</b>
     * The order is either clockwise or counter-clockwise. If a face is not
     * rendered, try turning of face-culling. Use the face's normal and the
     * view-vector from the camara to determine visibility of the face.
     */
    const std::list<tgt::vec3>& getVertices();

protected:
    /**
     * The face's ID
     */
    DWORD id_;

    /**
     * The face's normal. Used for visibility and lighting calculations if desired.
     */
    tgt::vec3 normal_;

    /**
     * List of edges which belong to this face.
     */
    std::list<Edge3D*> edges_;

    /**
     * Indicates whether the vertex list vertices_ has to be rebuild
     */
    bool rebuildVertexList_;

    /**
     * List of vertices being used for rendering this face. The list
     * is only build if changes have been made on this face and held
     * for efficency. You can obtain this list by calling
     * <code>getVertices()</code>. Before the list is return, it is
     * checked wheter a rebuild is necessary or not.
     */
    std::list<tgt::vec3> vertices_;

private:
    void clear();
    void clone(const PolygonFace3D& face);

    /**
     * Creates the list of vertices in vertices_ which is used for rendering
     * and which is returned by calling <code>getVertices()</code>.
     * The list has to be rebuild if changes on the face have been made and
     * rebuildVertexList_ is therefore "true".
     */
    void buildVertexList();
};

// ---------------------------------------------------------------------

/**
 * Class for an edge in a polygon in 3D. An edge contains 2 vertices and is
 * usually shared by serveral faces.
 *
 * <b>NOTE:</b>
 *
 * Use this class very carefully, as it is tricky! Read comments on <code>dtor()</code>,
 * <code>ctor()</code>, <code>createEdge()</code> and <code>disconnect()</code>!
 * Avoid deleting Edge3D object directly! Instead, delete vertices.
 * Ensure that you know what you do on setting vertices to NULL!
 *
 * @author  Dirk Feldmann
 */
class Edge3D
{
public:
    class EdgeSet : public std::list<Edge3D*>
    {
        public:
            EdgeSet::iterator find(Edge3D* const e) {
                if( e == 0 )
                    return end();

                EdgeSet::iterator it = begin();
                for( ; it != end(); it++ ) {
                    if( (*it != 0) && (*(*it) == *e) )
                        return it;
                }
                return end();
            }

            EdgeSet::const_iterator find(Edge3D* const e) const {
                if( e == 0 )
                    return end();

                EdgeSet::const_iterator it = begin();
                for( ; it != end(); it++ ) {
                    if( (*it != 0) && (*(*it) == *e) )
                        return it;
                }
                return end();
            }

            EdgeSet::iterator erase(EdgeSet::iterator itWhere) {
                if( itWhere == end() )
                    return end();

                return std::list<Edge3D*>::erase(itWhere);
            }

            size_t erase(Edge3D* const e) {
                if( e == 0 )
                    return 0;

                size_t count = 0;
                EdgeSet::iterator it = begin();
                while( it != end() ) {
                    if( (*it != 0) && (*(*it) == *e) ) {
                        it = std::list<Edge3D*>::erase(it);
                        count++;
                    } else {
                        it++;
                    }
                }
                return count;
            }

            std::pair<EdgeSet::iterator, bool> insert(Edge3D* const e) {
                const EdgeSet::iterator& itPos = this->find(e);
                if( itPos == this->end() ) {
                    this->push_back(e);
                    return std::pair<EdgeSet::iterator, bool>(this->end(), true);
                }
                return std::pair<EdgeSet::iterator, bool>(itPos, false);
            }
    };

public:
    Edge3D(const Edge3D& edge);

    /**
     * ctor contructing an edge by using the given EdgeVertex3D pointers. The new created
     * edge <b>DOES NOT REGISTER</b> itself at the vertices!
     * Use the static method <code>createEdge()</code> to ensure registration at the vertices
     * or call <code>addEdge()</code> on the vertices afterwards.
     * This is necessary as edges might only be used temporary and would destroy vertex-connections
     * on their deletion.
     * 
     * @param   parent  Polygon which holds a list this edge is contained it. <code>disconnect()</code>
     *                  makes use of this member to remove this vertex from the parent polygons list.
     *                  Therefore this parameter should not be NULL, except for temporary edges.
     */
    Edge3D(EdgeVertex3D* const first, EdgeVertex3D* const second, Polygon3D* const parent);

    /**
     * dtor removes this edge from all polygon faces in its list, but does not neither remove itself from
     * any vertex' edge list nor from its parent polygons list! Direct deletion of edges should by
     * avoided. Instead, deleting the vertices with a proceeding call to <code>disconnect()</code>
     * is recommended.
     */
    ~Edge3D();

    Edge3D& operator=(const Edge3D& edge);

    /**
     * Return first vertex.
     */
    EdgeVertex3D* getFirst() const;

    /**
     * Return second vertex.
     */
    EdgeVertex3D* getSecond() const;

    /**
     * Return faces which contain this edge.
     */
    const PolygonFace3D::FaceSet& getPolygons() const;

    /**
     * Set first vertex.
     */
    void setFirst(EdgeVertex3D* const v);

    /**
     * Set second vertex.
     */
    void setSecond(EdgeVertex3D* const v);

    /**
     * Return the direction of this edge. Used for clipping and splitting
     * edges.
     */
    tgt::vec3 getEdgeDirection() const;

    /**
     * Return the length of this edge.
     */
    float getEdgeLength() const;

    /**
     * Edges are defined to be equal, when both vertices are identical.
     */
    bool operator==(const Edge3D& e) const;
    bool operator!=(const Edge3D& e) const;

    /**
     * Creates a new edge containing the given vertices and adds the given Polygon3D pointer as
     * its parent. This method also registers the new edge at the given vertices! Use this
     * method if you want an edge to be part of a polygon or a face.
     */
    static Edge3D* createEdge(EdgeVertex3D* const first, EdgeVertex3D* const second, Polygon3D* const parent);

    /**
     * Add the given face to the edges list of faces which share this face.
     *
     * <b>NOTE:</b>
     * This method does not register the edge for the face and should not be called directly!
     * Instead, call <code>addEdge()</code> on the PolygonFace3D object. It will call
     * this method and ensure proper registrations between this edge and the face.
     * This is necessary for preventing these two methods to call each other circularly.
     */
    bool addFace(PolygonFace3D* const p);

    /**
     * Removes the given face from this edge's list of faces it is part of.
     * This method does not unregister this edge from the PolygonFace3D object!
     * It is only called by PolygonFace3D's dtor in order to ensure consistency.
     * You should not call this method directly either.
     */
    void removeFace(PolygonFace3D* const p);

    /**
     * Decides, whether the given vertex is <b>SOMEWHERE</b> on this
     * edge.
     *
     * @return  true, if the vertex is placed on this edge, false otherwise
     */
    bool containsVertex(EdgeVertex3D* const v) const;

    /**
     * Decides, whether the given vertex is <b>SOMEWHERE</b> on this
     * edge. This method internally calls the version for EdgeVertex3D pointers.
     *
     * @return  true, if the vertex is placed on this edge, false otherwise
     */
    bool containsVertex(const tgt::vec3& v) const;

    /**
     * Invalidates the edge, use with caution! The following steps are performed:
     *
     * - this edge is unregistred at the vertex which is different from v
     * - the edge is removed from its parent polygon's edge list
     * - the edge is removed from all faces' lists containing it.
     * - first_ and second_ are set to NULL
     *
     * The edge becomes degenereated and will be usually deleted directly after
     * calling this method. It is called by EdgeVertex3D's dtor and should always be
     * called on all edge BEFORE deleting them.
     * Usually it is sufficient to delete vertices, as all adjacent edges will be
     * deleted automatically then.
     */
    void disconnect(const EdgeVertex3D* const v);

    /**
     * Decides, whether one of this edge's vertices is identical to the 
     * given vertx v.
     *
     * @return  true, if v is identical to one of the vertices of this edge, false otherwise
     */
    bool isPartOf(const EdgeVertex3D* v) const;

    /**
     * Splits this edge at the given point into 2 edges, both containing the split vertex,
     * if it is not one of the vertices of this edge.
     * The new created edge will be added to all polygons which contain this edge, but
     * it will not be added to the parent polygons list because of clipping issues.
     * If splitting fails or the split vertex is one of this edge's vertices, NULL is
     * returned and no new edge will be created.
     * The second vertex of this edge and the first vertex of the new edge are identical
     * the split vertex afterwards. The first vertex of this edge remains unchanged, and
     * the second vertex of the new edge is the previous second vertex of this.
     *
     * @return  0 if splitting fails or a pointer to a Edge3D object containing
     *              the second part of the splitted edge.
     */
    Edge3D* split(EdgeVertex3D& v);

    /**
     * Wrapper-method for <code>split(EdgeVertex3D& v)</code> which is called internally.
     */
    Edge3D* split(const tgt::vec3& v);

protected:
    /**
     * first vertex of edge
     */
    EdgeVertex3D* first_;

    /**
     * second vertex of edge
     */
    EdgeVertex3D* second_;

    /**
     * list of polygons containing this edge
     */
    PolygonFace3D::FaceSet polygons_;

    /**
     * Polygon3D which contains this edge in its edge-list
     */
    Polygon3D* polyParent_;

private:
    void clear();
    void clone(const Edge3D& edge);
};

// ---------------------------------------------------------------------

/** Class for Vertices belonging to an edge. A vertex can be part of more than one edge.
 *
 * @author  Dirk Feldmann
 */
class EdgeVertex3D
{
public:
    struct VertexComparator
    {
        bool operator()(const EdgeVertex3D* const v1, const EdgeVertex3D* const v2) const {
            if( (v1 == 0) || (v2 == 0) )
                return false;

            return (*v1 < *v2);
        }
    };
    typedef std::set<EdgeVertex3D*, VertexComparator> VertexSet;

public:
    EdgeVertex3D(const EdgeVertex3D& vertex);

    /**
     * Contstuctor using the given vector as vertex but
     * does not add the vertex to an edge
     */
    EdgeVertex3D(const tgt::vec3& v);

    /**
     * Constructor using the given vector as vertex and
     * add it to the given edge
     */
    EdgeVertex3D(const tgt::vec3& v, Edge3D* const edge);

    /**
     * Destructor performing the deletion of the vertex AND
     * edges which are adjacent to this vertex.
     */
    ~EdgeVertex3D();

    EdgeVertex3D& operator=(const EdgeVertex3D& vertex);
    
    /**
     * Vertices are definied to be equal if the vectors
     * are component-wise equal.
     */
    bool operator==(const EdgeVertex3D& v) const;
    bool operator!=(const EdgeVertex3D& v) const;

    /**
     * Defines a strict weak order for vectors in R^3 (not really ;))
     * You can say that v1 < v2 when a "lower" component (ordered as "x, y, z")
     * of v1 is less than the one of v2.
     */
    bool operator<(const EdgeVertex3D& v) const;
    bool operator>(const EdgeVertex3D& v) const;

    /**
     * returns the edges, this vertex is adjacent to
     */
    const Edge3D::EdgeSet& getEdges() const;

    /**
     * returns the vector (point) of the vertex
     */
    tgt::vec3 getVertex() const;

    /**
     * set the vector of the vertex
     */
    void setVertex(const tgt::vec3& v);

    /**
     * add the given edge to this vertex' edge list
     */
    bool addEdge(Edge3D* e);

    /**
     * remove the given edge from this vertex' edge list
     */
    void removeEdge(Edge3D* const e);

    /**
     * check if the given edge is adjacent to this vertex
     */
    bool isAdjacentToEdge(Edge3D* e) const;

protected:
    /**
     * The vector holding the position in space.
     */
    tgt::vec3 v_;

    /**
     * (hash_)set of edges for which this vertex is adjacent to
     */
    Edge3D::EdgeSet edges_;

private:
    void clear();
    void clone(const EdgeVertex3D& vertex);
};

// ---------------------------------------------------------------------

/**
 * Class for an (arbitrary?) polygon in 3D. The polygon consists of a set of
 * unique vertices, edges and faces.
 * An edge consists of 2 vertices. 3 or more edges form a face and one edge can
 * be shared by multiple faces.
 * The polygon will probably be rendered face-wise, but the vertices and edges can
 * be accessed, too.
 * This polgon-class was especially designed for being clipped against a plane. There
 * shoulb be no limitations for the plane's position and successive clipping is possible.
 * Application is GPU-based raycasting for volume-rendering: arbitrary cuts through a
 * cube-shaped proxy-geometry can be made. This allows slices of any orientation and thickness
 * to be derived from a cube and rendered via gpu-raycasting.
 *
 * @author  Dirk Feldmann
 */
class Polygon3D
{
public:
    Polygon3D();
    Polygon3D(const Polygon3D& poly);

    /**
     * dtor which deletes all vertices, faces and edges contained in this polygon in this order.
     * <b>NOTE:</b>
     * As deleting the vertices causes the adjacent edges to become deleted, too, the
     * iteration over edges should not delete any further edges. Otherwise your polygonal
     * object was probably defect...
     */
    ~Polygon3D();

    Polygon3D& operator=(const Polygon3D& poly);

    /**
     * Returns the edge stored in the polygon's edge list which is identical to the given one.
     * If no such edge exists, a NULL-pointer is returned.
     */
    Edge3D* findEdge(Edge3D* const e);

    /**
     * Returns the edge stored in the polygon's edge list which is identical an
     * edge which consists of the given vertices.
     * If no such edge exists, a NULL-pointer is returned.
     */
    Edge3D* findEdge(EdgeVertex3D* const v1, EdgeVertex3D* const v2);
    
    /**
     * Removes the given edge from the polygons edge list, but does not 
     * delete it.
     */
    bool removeEdge(Edge3D* const e);

    /**
     * By passing an array of vertices and an array of indices for the vertices, including their
     * sizes, a face can be defined. The defined face will be inserted to the polygons face-set
     * if it consists of at least 3 different vertices. The vertices are connected in the order of
     * of their indexing but clockwise or counter-clockwise orientations may mess.
     *
     * @param   vertices    Pointer to an array of vertices which belong to the face. If a vertex 
     *                      alread exisits within the polygon, no dupplication will be made.
     * @param   numVertices Size of the vertex-array > 1. Must not be greater than the 
     *                      actual array but may be less.
     * @param   indices     Pointer to an array of indices for the vertex-array. The order of the indices
     *                      predefines a clockwise or counter-clockwise orientation for the face.
     * @param   numIndices  Size of the index-array. Must not be greater than the actual array but less
     *                      and must be even and suitable for the vertices!!!
     * @param   id          ID for the new face. The greatest ID is stored. You cann obtain a new face-id
     *                      by calling <code>getMaxFace()</code> and adding 1.
     * @param   normal      The normal for the face. This parameter should always be given and valid,
     *                      as visibility-checks could not performed without a valid face-normal.
     */
    bool addFace(const tgt::vec3* vertices, const DWORD numVertices, const DWORD* indices, const DWORD numIndices,
        const DWORD id, const tgt::vec3& faceNormal);

    /**
     * Returns true if a face within the polygons face-set with the given ID exists
     * or false otherwise. This method internally calls <code>findFace()</code>.
     */
    bool containsFace(const DWORD id);

    /**
     * Returns the pointer to the face with the given ID if one exists within the
     * polygons face-set.
     */
    PolygonFace3D* findFace(const DWORD id);

    /**
     * Returns the normal of the face with the given ID or a NULL-vector if no
     * such face exisits within the polygons face-set.
     */
    tgt::vec3 getFaceNormal(const DWORD id) const;

    /**
     * Returns the polygons face-set. Call this method for rendering. From the
     * faces you can obtain the vertices by calling <code>getVertices()</code>.
     */
    const PolygonFace3D::FaceSet& getFaces() const;

    /**
     * Returns a list of all vertices contained by the face with the given ID
     * if such a face exists. Otherwise the list will be empty.
     */
    std::list<tgt::vec3> getFaceVertices(const DWORD ID);

    /**
     * Returns the greatest currently used face-ID. Call this method and add 1 to
     * to its result to obtain an ID for a new face.
     */
    DWORD getMaxFace() const;

    /**
     * Returns the number of faces within this polygons face-set.
     *
     * <b>NOTE:</b>
     * maxFace_ is probably greater than the number of faces.
     */
    DWORD getNumFaces() const;        

    /**
     * Connects all vertices on the edges which share the given vertex.
     * A connection is only made if the other vertices which are different from the
     * give one on an edge share a face. The new edge is added to the shared face and
     * to the polygons edge-set.
     *
     * Example (polygon may be a clipped cube in 3D):
     * ----------------------------------------------
     *
     *       /  2             /  2
     *   B  +             B  +
     *     / \ C            / \  C
     *  E +   +----  =>  E +---+----
     *    |3 /             |3 /|
     *    | /    1         | / |   1
     *    |/   D           |/  | D
     *  A +---+----      A +---+----
     *
     * For vertex A, the vertices (E,C) and (C,D) would be connected as E and C share the face #3
     * and C and D share face #1. (E,D) will not be connected, as the edge (A-D) containig D is part
     * of face #1 but not of face #3 of which the edge (A-E) is part of. The new edge (E-C) will be
     * added to face #3 and (C-D) to face #1.
     */
    Edge3D::EdgeSet connectAdjacentVertices(EdgeVertex3D* const v);

    /**
     * Returns the vertex from this polygon if it contains a vertex with the given position.
     * If no such vertex is contained, a NULL-pointer is returned.
     */
    EdgeVertex3D* findVertex(const tgt::vec3& v) const;

    /**
     * Returns the vertex-set of this polygon. This can be used for only rendered the vertices for example.
     *
     * <b>NOTE:</b>
     * The returned vertices are completely unordered. Rendering a polygon from it via calling
     * <code>glBegin(GL_POLYGON)</code> will unlikely succeed.
     */
    const EdgeVertex3D::VertexSet& getVertices() const;
    
    /**
     * Clip the Polygon against a plane represented by a point on it and its
     * normal.
     * The clipping results in a "deformation" of this polygon and a new face for the intersection
     * will be added to this polygon it contains at least 3 edges (this should alway be true).
     * During clipping, temporary face are calculated and removed afterwards. The vertices laying
     * "outside" (on the side of the plane's normal) are deleted completely and removed from all
     * lists.
     * The plane's normal is the normal for the new face generated by the intersection.
     */
    void clip(const tgt::vec3& n, const tgt::vec3&p);

protected:
    /**
     * hash_set for vertex-pointers (cannot be ordred).
     * <b>ATTENTION:</b> uses special comparison-function!
     */
    EdgeVertex3D::VertexSet vertices_;
    
    /**
     * hash_set for edge-pointers (cannot be orderd).
     * <b>ATTENTION:</b> uses special comparison-function!
     */
    Edge3D::EdgeSet edges_;

    /**
     * set for face-pointers (can be ordered by their IDs).
     * <b>ATTENTION:</b> uses special comparison-function!
     */
    PolygonFace3D::FaceSet faces_;

    /**
     * The greatest currently used face-ID.
     */
    DWORD maxFace_;

private:
    /**
     * Performs face-reduction after intersecting with a plane.
     */
    void reduceFaces(const DWORD firstID, const DWORD lastID, PolygonFace3D* clippedFace);

    /**
     * Rounds the given float to six digits righthandside of the decimal point.
     */
    float roundFloat(const float f) const;

    void clear();
    void clone(const Polygon3D& poly);
};

}   // namespace
#endif
