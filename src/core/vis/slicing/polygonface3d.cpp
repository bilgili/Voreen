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

#include "voreen/core/vis/slicing/polygon3d.h"

#include <queue>

using std::queue;
using std::list;
using std::set;

namespace voreen {

// public methods
//

PolygonFace3D::PolygonFace3D(const unsigned long id, const tgt::vec3& normal) 
: id_(id), normal_(normal), rebuildVertexList_(true)
{}

PolygonFace3D::PolygonFace3D(const PolygonFace3D& face) {
    clone(face);
}

PolygonFace3D::~PolygonFace3D() {
//std::cout << "~PolygonFace3D on polygon #" << id_ << "...\n";
    clear();
}

PolygonFace3D& PolygonFace3D::operator=(const PolygonFace3D& face) {
    clear();
    clone(face);
    return *this;
}

bool PolygonFace3D::operator==(const PolygonFace3D& f) const {
    return (id_ == f.getID());
}

bool PolygonFace3D::operator!=(const PolygonFace3D& f) const {
    return (id_ != f.getID());
}

bool PolygonFace3D::operator<(const PolygonFace3D& f) const {
    return (id_ < f.getID());
}

bool PolygonFace3D::operator<=(const PolygonFace3D& f) const {
    return (id_ <= f.getID());
}

bool PolygonFace3D::operator>(const PolygonFace3D& f) const {
    return (id_ > f.getID());
}

bool PolygonFace3D::operator>=(const PolygonFace3D& f) const {
    return (id_ >= f.getID());
}

unsigned long PolygonFace3D::getID() const {
    return id_;
}

const tgt::vec3& PolygonFace3D::getNormal() const {
    return normal_;
}

const list<Edge3D*> PolygonFace3D::getEdges() const {
    return edges_;
}

unsigned long PolygonFace3D::getNumEdges() const {
    return edges_.size();
}

bool PolygonFace3D::insertEdge(Edge3D* const e) {
    if (edges_.empty()) {
        edges_.push_back(e);
        e->addFace(this);
        rebuildVertexList_ = true;
        return true;
    }

    bool inserted = false;
    bool equal = false;
    EdgeVertex3D* v1 = e->getFirst();
    EdgeVertex3D* v2 = e->getSecond();
    list<Edge3D*>::iterator itCur = edges_.begin();
    list<Edge3D*>::iterator itNext = edges_.begin();
    itNext++;

    for (; itCur != edges_.end(); ++itCur, ++itNext) {
        Edge3D* cur = *itCur;
        Edge3D* next = (itNext != edges_.end()) ? *itNext : 0;

        // ensure that the next edge is also not equal to the one which has to be inserted
        //
        if ( (next != 0) && (next->isPartOf(v1) == true) && (next->isPartOf(v2) == true) ) {
            equal = true;
            break;
        }

        // check if the first vertex is part of the current regarded edge
        // 
        if (cur->isPartOf(v1)) {
            // if second is also part, the edges are equal and no insertion has to be made
            //
            if (cur->isPartOf(v2)) {
                equal = true;
                break;
            }

            // check if the second vertex is part of the next edge
            //
            if ( (next != 0) && next->isPartOf(v2) ) {
                // if v1 is part of current edge and v2 is part of the
                // next one, insert edge e between current and next
                //
                e->addFace(this);
                edges_.insert(itNext, e);
                inserted = true;
                break;
            }
        } // if (cur->isPartOf(v1))

        if ( cur->isPartOf(v2) && (inserted == false) ) {
            // if first is also part, the edges are equal and no insertion has to be made
            //
            if (cur->isPartOf(v1)) {
                equal = true;
                break;
            }

            // check if the first vertex is part of the next edge
            //
            if ( (next != 0) && next->isPartOf(v1) ) {
                // if v2 is part of current edge and v1 is part of the
                // next one, insert edge e between current and next
                //
                e->addFace(this);
                edges_.insert(itNext, e);
                inserted = true;
                break;
            }
        } // if ( cur->isPartOf(v2) && (inserted == false) )

        if (next == 0)
            break;
    }   // for (; itCur != edges_.end(); ++itCur, ++itNext)

    // if the edge has not been inserted, is not equal to any other edge, 
    // insert it at the back of the list
    //
    if ( (equal == false) && (inserted == false) ) {
        e->addFace(this);
        edges_.push_back(e);
        inserted = true;
    }

    rebuildVertexList_ = inserted;
    return inserted;
}

bool PolygonFace3D::addEdge(Edge3D* const e) {
    if ( ((e != 0) && (findEdge(e) == 0)) ) {
        e->addFace(this);
        edges_.push_back(e);
        rebuildVertexList_ = true;
        return true;
    }
    return false;
}

void PolygonFace3D::sortEdges() {
    if (edges_.size() <= 2)
        return;

    Edge3D* e = edges_.front();
    edges_.pop_front();
    EdgeVertex3D* v = e->getSecond();

    list<Edge3D*> newList;
    newList.push_back(e);

    list<Edge3D*>::iterator it = edges_.begin();
    while ( (edges_.empty() == false) && (it != edges_.end()) ) {
        e = *it;
        if (e->isPartOf(v)) {
            if ( (*(e->getFirst()) == *v) )
                v = e->getSecond();
            else
                v = e->getFirst();
            
            newList.push_back(e);
            edges_.erase(it);
            it = edges_.begin();
        }
        else
            ++it;
    }

    edges_ = newList;
    rebuildVertexList_ = true;
}

Edge3D* PolygonFace3D::findEdge(Edge3D* const e) {
    list<Edge3D*>::iterator it = edges_.begin();
    for (; it != edges_.end(); ++it ) {
        if ( (e == *it) || (*(*it) == *e) )
            return *it;
    }
    return 0;
}

bool PolygonFace3D::removeEdge(Edge3D* const e) {
    bool blret = false;
    for ( list<Edge3D*>::iterator it = edges_.begin(); it != edges_.end(); ) {
        if ( (*it == e) ) {
            it = edges_.erase(it);
            blret = true;
        }
        else
            ++it;
    }
    rebuildVertexList_ = blret;
    return blret;
}

const std::list<tgt::vec3>& PolygonFace3D::getVertices() {
    // if the vertex list is not up to date, rebuild it
    //
    if (rebuildVertexList_)
        buildVertexList();

    return vertices_;
}

//private methods
//

void PolygonFace3D::clone(const PolygonFace3D& face) {
    if (this == &face)
        return;

    id_ = face.getID();
    normal_ = face.getNormal();
    rebuildVertexList_ = true;
}

void PolygonFace3D::clear() {
    // remove the reference to this polygon from the edges by deleting
    // the corresponding list entries within the edge. Do not delete the
    // edge itself as it might be part of another face and still used!
    //
    for ( list<Edge3D*>::iterator it = edges_.begin(); it != edges_.end(); ++it ) {
        if (*it != 0)
            (*it)->removeFace(this);
    }
    edges_.clear();
}

void PolygonFace3D::buildVertexList() {
    vertices_.clear();
    rebuildVertexList_ = false;

    if (edges_.size() == 0)
        return;

    // push the first vertices of the first edge to the output.
    //
    list<Edge3D*>::const_iterator it = edges_.begin();
    Edge3D* e = *it;
    vertices_.push_back( e->getFirst()->getVertex() );
    vertices_.push_back( e->getSecond()->getVertex() );
    tgt::vec3 last = vertices_.back();
    ++it;

    // check if the last point in the output list is one of the next
    // edges one, in order to avoid broken polygons. the last point
    // in the output list must occur in the next edge.
    // if it does not, swap the both first inserted points.
    //
    if ( (it != edges_.end()) 
        && ( ! (last == (*it)->getFirst()->getVertex()) ) 
        && ( ! (last == (*it)->getSecond()->getVertex())) )
    {
        std::swap(vertices_.front(), vertices_.back());
        last = vertices_.back();
    }

    for (; it != edges_.end(); ++it ) {
        e = *it;
        tgt::vec3 v1 = e->getFirst()->getVertex();
        tgt::vec3 v2 = e->getSecond()->getVertex();

        // if the last point in the output list is identical to the first one
        // of this edge, insert the second one as far the later one is not
        // already contained at the front of the output list.
        //
        if ( (last == e->getFirst()->getVertex()) && ( !(vertices_.front() == v2) ) )
            vertices_.push_back( v2 );

        // same check for the second vertex of the current edge
        //
        if ( (last == e->getSecond()->getVertex()) && ( !(vertices_.front() == v1) ) )
            vertices_.push_back( v1 );
        
        last = vertices_.back(); // update the last inserted point
    }

    // check the windinng of this face: if it is not counter clockwise, reverse the
    // vertex list.
    //
    if (vertices_.size() >= 3) {
        std::list<tgt::vec3>::iterator it = vertices_.begin();
        tgt::vec3& v0 = *(it);
        tgt::vec3& v1 = *(++it);
        tgt::vec3& v2 = *(++it);
        tgt::vec3 dir1(v1 - v0);
        tgt::vec3 dir2(v2 - v0);
        tgt::vec3 orientation = tgt::cross(dir1, dir2);
        float f = tgt::dot(orientation, normal_);
        if (f <= 0.0f)
            vertices_.reverse();
    }
}

}   // namespace
