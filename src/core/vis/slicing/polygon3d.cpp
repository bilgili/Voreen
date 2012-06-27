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
#include "voreen/core/vis/slicing/polygon3d.h"

#include <queue>
using std::queue;
using std::list;

namespace voreen
{

// public methods
//

Polygon3D::Polygon3D() : maxFace_(0) {
}

Polygon3D::Polygon3D(const Polygon3D& poly) {
    clone(poly);
}

Polygon3D::~Polygon3D() {
    clear();
}

Polygon3D& Polygon3D::operator=(const Polygon3D& poly) {
    clear();
    clone(poly);
    return *this;
}

const PolygonFace3D::FaceSet& Polygon3D::getFaces() const {
    return faces_;
}

DWORD Polygon3D::getMaxFace() const {
    return maxFace_;
}

DWORD Polygon3D::getNumFaces() const {
    return static_cast<DWORD>(faces_.size());
}

tgt::vec3 Polygon3D::getFaceNormal(const DWORD face) const {
    PolygonFace3D::FaceSet::const_iterator it = faces_.begin();
    for( ; it != faces_.end(); it++ ) {
        if( (*it != 0) && ((*it)->getID() == face) )
            return (*it)->getNormal();
    }
    return tgt::vec3(0.0f, 0.0f, 0.0f);
}

list<tgt::vec3> Polygon3D::getFaceVertices(const DWORD id) {
    PolygonFace3D* face = findFace(id);
    if( face == 0)
        return list<tgt::vec3>();

    return face->getVertices();
}

Edge3D* Polygon3D::findEdge(EdgeVertex3D* const v1, EdgeVertex3D* const v2) {
    if( (v1 == 0) || (v2 == 0) )
        return 0;

    Edge3D::EdgeSet::iterator it = edges_.begin();
    for( ; it != edges_.end(); it++ ) {
        Edge3D* e = *it;
        if( (e != 0) && (e->isPartOf(v1) == true) && (e->isPartOf(v2) == true) )
            return *it;
    }

    return 0;
}

Edge3D* Polygon3D::findEdge(Edge3D* const e) {
    if( (e == 0) )
        return 0;

    Edge3D::EdgeSet::iterator it = edges_.find(e);
    if( it != edges_.end() )
        return *it;
    
    return 0;
}

bool Polygon3D::removeEdge(Edge3D* const e) {
    if( e == 0 )
        return false;

    return (edges_.erase(e) > 0);
}

bool Polygon3D::addFace(const tgt::vec3* vertices, const DWORD numVertices, const DWORD* indices, 
                                const DWORD numIndices, const DWORD id, const tgt::vec3& faceNormal)
{
    // an edge is defined by 2 indices. ensure that there is an even amount of indices
    // and that a face consists of at least two different vertices!
    //
    if( (numVertices <= 1) || (numIndices <= 1) || ((numIndices % 2) != 0) )
        return false;

    if( (numVertices == 2) && (vertices[0] == vertices[1]) )
        return false;

    // ensure, that all the indices are less than the number of vertices
    //
    for( DWORD k = 0; k < numIndices; k++ ) {
        if( indices[k] >= numVertices )
            return false;
    }

    PolygonFace3D* face = new PolygonFace3D(id, faceNormal);
    std::pair<PolygonFace3D::FaceSet::iterator, bool> prF = faces_.insert(face);
    if( prF.second == false ) // if found, no insertion was made, so delete created one
    {
        delete face;
        face = *(prF.first);
    }
    maxFace_ = (id > maxFace_) ? id : maxFace_; // update the maximum face id

    queue<Edge3D*> remaining;   // used for adding edges to polygon face
    for( DWORD ii = 0; ii < numIndices; ii += 2 )  // ii = indices index
    {
        const tgt::vec3& v1 = vertices[indices[ii]];
        const tgt::vec3& v2 = vertices[indices[ii + 1]];

        // ensure, that both vertices of the edge are different. elsewise omit
        // the edge
        //
        if( v1 == v2 )
            continue;

        // try to find the vertices within the local list
        //
        EdgeVertex3D* ev1 = new EdgeVertex3D(v1);
        std::pair<EdgeVertex3D::VertexSet::iterator, bool> prV = vertices_.insert(ev1);
        if( prV.second == false )  // if found, no insertion was made, so delete created one
        {
            delete ev1;
            ev1 = *(prV.first);
        }

        EdgeVertex3D* ev2 = new EdgeVertex3D(v2);
        prV = vertices_.insert(ev2);
        if( prV.second == false )  // if found, no insertion was made, so delete created one
        {
            delete ev2;
            ev2 = *(prV.first);
        }

        Edge3D* e = Edge3D::createEdge(ev1, ev2, this);
        std::pair<Edge3D::EdgeSet::const_iterator, bool> prE = edges_.insert(e);
        if( prE.second == false ) // if found, no insertion was made, so delete created one
        {
            delete e;
            e = *(prE.first);
        }
    
        // insert the new edge at the correct position in the face
        //
        face->insertEdge(e);
    }
    return true;
}

PolygonFace3D* Polygon3D::findFace(const DWORD id) {
    PolygonFace3D face(id, tgt::vec3());
    PolygonFace3D::FaceSet::iterator it = faces_.find(&face);
    if( it != faces_.end() )
        return *it;

    return 0;
}

bool Polygon3D::containsFace(const DWORD id) {
    return (findFace(id) != 0);
}

EdgeVertex3D* Polygon3D::findVertex(const tgt::vec3& v) const {
    if( vertices_.size() == 0 )
        return 0;
    
    EdgeVertex3D ev(v, 0);
    EdgeVertex3D::VertexSet::const_iterator it = vertices_.find(&ev);
    if( it != vertices_.end() )
        return *it;

    return 0;
}

Edge3D::EdgeSet Polygon3D::connectAdjacentVertices(EdgeVertex3D* const v)
{
    Edge3D::EdgeSet newEdges;
    if( v == 0 )
        return newEdges;

    const Edge3D::EdgeSet& edges = v->getEdges();
    if( edges.size() <= 1 )
        return newEdges;  // return empty list if there is only one adjacent edge or less

    // compare edges pair-wise and
    //
    for( Edge3D::EdgeSet::const_iterator itE1 = edges.begin(); itE1 != edges.end(); itE1++ ) {
        // get first edge and the vertex which is not this vertex
        //
        Edge3D* e1 = *itE1;
        EdgeVertex3D* v1 = ( *(e1->getFirst()) == *v ) ? e1->getSecond() : e1->getFirst();

        Edge3D::EdgeSet::const_iterator itE2 = itE1;
        itE2++; // next edge
        for( ; itE2 != edges.end(); itE2++ ) {
            // get a second edge and the other vertex which is not this one either
            //
            Edge3D* e2 = *itE2;
            EdgeVertex3D* v2 = ( *(e2->getFirst()) == *v ) ? e2->getSecond() : e2->getFirst();

            // get the face of both edges, and compare them also pairwise
            //
            const PolygonFace3D::FaceSet& ps1 = e1->getPolygons();
            const PolygonFace3D::FaceSet& ps2 = e2->getPolygons();
            PolygonFace3D::FaceSet::const_iterator itF1 = ps1.begin();
            PolygonFace3D::FaceSet::const_iterator itF2;
            for( ; itF1 != ps1.end(); itF1++ ) {
                for( itF2 = ps2.begin(); itF2 != ps2.end(); itF2++ ) {
                    // if two edges share a polygon face, create a new edge
                    // between the both points which are not this one
                    // 
                    if( *(*itF2) == *(*itF1) ) {
                        // create new edge, if it does not alread exist
                        // and add it to the shared face and the output list
                        //
                        Edge3D* newEdge = this->findEdge(v1, v2);
                        if( newEdge == 0 ) {
                            newEdge = Edge3D::createEdge(v1, v2, this);
                            edges_.insert(newEdge);
                        }

                        (*itF1)->insertEdge(newEdge);
                        newEdges.insert(newEdge);
                    }
                }   // for( itF2 ...
            }   // for( itGF1 ...
        }   // for( itE2 ...
    }   // for( itE1 ...

    return newEdges; // return the new created edges
}

const EdgeVertex3D::VertexSet& Polygon3D::getVertices() const {
    return vertices_;
}

void Polygon3D::clip(const tgt::vec3 &n, const tgt::vec3 &p) {
    // iterate over all edges
    //
    queue<Edge3D*> newEdges;    // temporary queue for inserting new edges
    Edge3D::EdgeSet::iterator itE = edges_.begin();
    for( ; itE != edges_.end(); itE++ ) {
        // for each edge calculate possible intersections
        // with the plane
        //
        Edge3D* e = *itE;
        const tgt::vec3& d = e->getEdgeDirection();  // edge's direction
        float b = dot(d, n);

        // if direction and normal are nearly orthogonal, no intersection can occur
        //
        if( ( (b < 0.0f) ? -b : b) <= 0.000001f )
            continue;

        // calculate the intersection point of ray on edge and the plane
        //
        const tgt::vec3& pe = e->getFirst()->getVertex();    // point on edge
        tgt::vec3 i(pe); // intersection point
        float a = dot((p - pe), n);
        i += d * (a / b);
   
        // round up the intersection point in order to prevent problems with
        // exisiting point which are nearly identical
        //
        i.x = roundFloat(i.x); i.y = roundFloat(i.y); i.z = roundFloat(i.z);

        // check if the intersection point lies on the edge. if so, split
        // the edge at this point into 2 edges! the new edge has to be stored
        // in a queue and will be processed after all alread existing edges
        // have been tested.
        //
        if( e->containsVertex(i) == true ) {
            // the first vertex of the new edge generated by split() is
            // the split vertex and therefore the intersection point
            //
            Edge3D* eNew = e->split(i);
            if( eNew != 0 )
                newEdges.push(eNew);

            vertices_.insert(e->getSecond());
        }
    }

    // add the new edges to polygons edge list
    //
    while( newEdges.empty() == false ) {
        Edge3D* e = newEdges.front();
        newEdges.pop();
        edges_.insert(e);
    }
    
    // first ID of a new temporary face, required for face reduction
    //
    DWORD startFace = (maxFace_ + 1);

    // check for all vertices if they are on the plane's front, but not
    // within the plane.
    //
    EdgeVertex3D::VertexSet::iterator itV = vertices_.begin();
    while( (itV != vertices_.end()) ) {
        const tgt::vec3& v = (*itV)->getVertex();
        float f = dot((v - p), n);

        // if the vertex is outside, remove it and obtain the
        // new edges which are created by the vertex which is about to be
        // removed
        //
        if( f > 0.00001f ) {
            // connect the vertices which share an edge with this vertex
            // and return the new created edges. create a new face with the new edges
            //
            Edge3D::EdgeSet faceEdges = connectAdjacentVertices(*itV);
            if( faceEdges.empty() == false ) {
                PolygonFace3D* face = new PolygonFace3D(++maxFace_, n);
                for( itE = faceEdges.begin(); itE != faceEdges.end(); itE++ ) {
                    face->insertEdge(*itE);
                }
                faces_.insert(face);
            }

            // delete the vertex and remove it from the list. deleting a
            // vertex causes its adjacent edges to become degenrated and
            // they will be deleted from all lists.
            //
            delete *itV;
            vertices_.erase(itV++);
        } else {
            ++itV;
        }
    }   // while
    DWORD endFace = maxFace_;   // required for face-reduction

    // create a new face for the intersection of the polygon with the plane and
    // reduce the above created faces to one single face
    //
    PolygonFace3D* clippedFace = new PolygonFace3D(startFace, n);
    reduceFaces(startFace, endFace, clippedFace);
}

void Polygon3D::reduceFaces(const DWORD firstID, const DWORD lastID, PolygonFace3D* clippedFace) {
    if( clippedFace == 0 )
        return;

    // iterate over all faces which have been inserted during clipping and remove
    // them. add the edges, which have at least one of the old faces in common
    // to the face for the clipped area
    //
    PolygonFace3D::FaceSet::iterator itF1 = faces_.begin();
    while( ((faces_.empty() == false) && (itF1 != faces_.end())) ) {
        PolygonFace3D* face1 = *itF1;

        // only faces which have been inserted recently are of interest
        //
        if( (face1->getID() > firstID) && (face1->getID() <= lastID) ) {
            // consider all edges on the face and find those which are shared
            // with old faces
            //
            const list<Edge3D*>& edges = face1->getEdges();
            for( list<Edge3D*>::const_iterator itE = edges.begin(); itE != edges.end(); itE++ ) {
                Edge3D* e = *itE;
                bool add = false;
                const PolygonFace3D::FaceSet& containingFaces = e->getPolygons();
                PolygonFace3D::FaceSet::const_iterator itF2 = containingFaces.begin();
                for( ; itF2 != containingFaces.end(); itF2++ ) {
                    // if the id is less than the one of the clipping face,
                    // the edge is also part of an old face
                    //
                    if( ((*itF2)->getID() < firstID) ) {
                        add = true;
                        break;
                    }
                }   // for

                if( add == true ) {
                    // simply add the edge. the edges will be sorted later
                    //
                    clippedFace->addEdge(*itE);
                }
            }   // for

            delete face1; // delete the face
            face1 = 0;
            faces_.erase(itF1++);
        } else {
            // delete old faces which do not have at least 3 edges left anymore.
            // this may happen if an AABB is clipped against a plane being paralle
            // to one of its sides, for example.
            //
            if( face1->getNumEdges() <= 2 ) {
                delete *itF1;
                faces_.erase(itF1++);
            } else {
                ++itF1;
            }
        }   // else
    }   // for

    // sort the new edges to a correctly connected polygon and insert
    // the face to the list of faces if the ammount of edges is >= 3.
    // otherwise delete the face.
    //
    if( clippedFace->getNumEdges() >= 3 ) {
        clippedFace->sortEdges();
        faces_.insert(clippedFace);

        // adopt max. face ID...
        //
        maxFace_ = clippedFace->getID();
    } else {
        // delete edges for the clipped face (less than 3), as the
        // do not form a proper face.
        //
        list<Edge3D*> edges = clippedFace->getEdges();
        list<Edge3D*>::iterator it = edges.begin();
        for( ; it != edges.end(); it++ ) {
            delete *it;
            *it = 0;
        }
        edges.clear();

        delete clippedFace;
        clippedFace = 0;

        // adopt max. face ID...
        //
        maxFace_ = firstID - 1;
    }
}

// private methods
//

void Polygon3D::clear() {
    EdgeVertex3D::VertexSet::iterator itV = vertices_.begin();
    for( ; itV != vertices_.end(); itV++ ) {
        delete *itV;
    }
    vertices_.clear();

    PolygonFace3D::FaceSet::iterator itF = faces_.begin();
    for( ; itF != faces_.end(); itF++ ) {
        delete *itF;
    }
    faces_.clear();

    Edge3D::EdgeSet::iterator itE = edges_.begin();
    for( ; itE != edges_.end(); itE++ ) {
        delete *itE;
    }
    edges_.clear();
}

void Polygon3D::clone(const voreen::Polygon3D& poly) {
    if( this == &poly )
        return;

    maxFace_ = poly.getMaxFace();

    const PolygonFace3D::FaceSet& faces = poly.getFaces();
    PolygonFace3D::FaceSet::const_iterator itF = faces.begin();
    for( ; itF != faces.end(); itF++ ) {
        const PolygonFace3D* myFace = *itF;
        if( myFace == 0 )
            continue;

        PolygonFace3D* clonedFace = new PolygonFace3D(*myFace);
        std::pair<PolygonFace3D::FaceSet::iterator, bool> prF = faces_.insert(clonedFace);
        if( prF.second == false ) {
            delete clonedFace;
            clonedFace = *(prF.first);
        }

        const std::list<Edge3D*>& edges = myFace->getEdges();
        std::list<Edge3D*>::const_iterator itE = edges.begin();
        for( ; itE != edges.end(); itE++ ) {
            Edge3D* myEdge = *itE;
            if( myEdge == 0 )
                continue;

            EdgeVertex3D* clonedFirst = new EdgeVertex3D( *(myEdge->getFirst()) );
            std::pair<EdgeVertex3D::VertexSet::iterator, bool> prV = vertices_.insert(clonedFirst);
            if( prV.second == false ) {
                delete clonedFirst;
                clonedFirst = *(prV.first);
            }

            EdgeVertex3D* clonedSecond = new EdgeVertex3D( *(myEdge->getSecond()) );
            prV = vertices_.insert(clonedSecond);
            if( prV.second == false ) {
                delete clonedSecond;
                clonedSecond = *(prV.first);
            }

            Edge3D* clonedEdge = Edge3D::createEdge(clonedFirst, clonedSecond, this);
            std::pair<std::list<Edge3D*>::iterator, bool> prE = edges_.insert(clonedEdge);
            if( prE.second == false ) {
                delete clonedEdge;
                clonedEdge = *(prE.first);
            }

            clonedFace->addEdge(clonedEdge);
        }   // for
    }   // for

}

float Polygon3D::roundFloat(const float f) const {
    const double SCALE = 1000000.0;

    // 1. convert float to double
    //
    double d = (double) f;

    // 2. shift decimal-point by 6 positions left
    //
    d *= SCALE;

    // 3. crop following digits by converting to long
    //
    long l = (long) d;

    // 4. remove all digits left of decimal-point
    //
    d -= (double) l;

    // 5. extract the first digit on righthandside of decimal-point
    //
    d *= (d > 0.0f ) ? 10.0 : -10.0;
    unsigned int digit = (unsigned int) d;

    // 6. decide wether to ceil or to floor the number and shift
    // digits 6 positions back
    //
    d = (digit > 4) ? ((double) (l + 1.0)) : ((double) l);
    d /= SCALE;
  
    // 7. convert number back to float and return
    //
    return (float) d;
}

}   // namespace

#endif
