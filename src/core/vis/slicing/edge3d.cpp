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

namespace voreen {

using std::set;

// public methods
//
Edge3D* Edge3D::createEdge(EdgeVertex3D* const first, EdgeVertex3D* const second, Polygon3D* const parent) {
    Edge3D* e = new Edge3D(first, second, parent);

    // add this edge to both vertices edge lists for registration. 
    // the insertion fails, if an equal edge is already contained!
    //
    EdgeVertex3D* v1 = e->getFirst();
    if (v1 != 0)
        v1->addEdge(e);

    EdgeVertex3D* v2 = e->getSecond();
    if (v2 != 0)
        v2->addEdge(e);

    return e;
}

Edge3D::Edge3D(const Edge3D& edge) {
    clone(edge);
}

Edge3D::Edge3D(EdgeVertex3D* const first, EdgeVertex3D* const second, Polygon3D* const parent) 
: first_(first), second_(second), polyParent_(parent)
{}

Edge3D::~Edge3D() {
//std::cout << "~Edge3D() on #" << this << "...\n";
    clear();
}

Edge3D& Edge3D::operator=(const Edge3D& edge) {
    clear();
    clone(edge);
    return *this;
}

EdgeVertex3D* Edge3D::getFirst() const {
    return first_;
}

EdgeVertex3D* Edge3D::getSecond() const {
    return second_;
}

const PolygonFace3D::FaceSet& Edge3D::getPolygons() const {
    return polygons_; 
}

tgt::vec3 Edge3D::getEdgeDirection() const {
    tgt::vec3 dir(second_->getVertex() - first_->getVertex());
    dir = normalize(dir);
    return dir;
}

float Edge3D::getEdgeLength() const {
    if ( (second_ == 0) || (first_ == 0) )
        return 0.0f;

    tgt::vec3 dir(second_->getVertex() - first_->getVertex());
    return length(dir);
}

void Edge3D::setFirst(EdgeVertex3D* const v) {
    first_ = v;
}

void Edge3D::setSecond(EdgeVertex3D* const v) {
    second_ = v;
}

void Edge3D::disconnect(const EdgeVertex3D* const v) {
    if (v == 0)
        return;
    
    // as this edge becomes now invalid, remove it from all faces to which
    // it belongs
    //
    PolygonFace3D::FaceSet::iterator itF = polygons_.begin();
    for (; itF != polygons_.end(); ++itF ) {
        if (*itF != 0)
            (*itF)->removeEdge(this);
    }
    polygons_.clear();

    // also remove it from parent polygon's edge list
    //
    if (polyParent_ != 0)
        polyParent_->removeEdge(this);

    // find the member whose pointer equals the vertex which
    // is about to be disconnected.
    // if the vertex is disconnected, the edge would become invalid,
    // so remove it from the other vertex' list, too, and set both members
    // to 0.
    //
    if (first_ == v)
        second_->removeEdge(this);

    if (second_ == v)
        first_->removeEdge(this);

    first_ = 0;
    second_ = 0;
}

bool Edge3D::operator==(const Edge3D& e) const {
    const EdgeVertex3D* v1 = e.getFirst();
    const EdgeVertex3D* v2 = e.getSecond();

    if ( (first_ == 0) || (second_ == 0) || (v1 == 0) || (v2 == 0) )
        return false;

    if ( ((*first_ == *v1) && (*second_ == *v2)) || ((*first_ == *v2) && (*second_ == *v1)) )
        return true;

    return false;
}

bool Edge3D::operator!=(const Edge3D& e) const {
    return (! (*this == e) );
}

bool Edge3D::addFace(PolygonFace3D* const p) {
    std::pair<PolygonFace3D::FaceSet::iterator, bool> pr = polygons_.insert(p);
    return pr.second;
}

void Edge3D::removeFace(PolygonFace3D* const p) {
    if (p == 0)
        return;
    polygons_.erase(p);
}

bool Edge3D::containsVertex(const tgt::vec3& v) const {
    EdgeVertex3D ev(v, 0);
    return containsVertex(&ev);
}

bool Edge3D::containsVertex(EdgeVertex3D* const v) const {
    if ( (first_ == 0) || (second_ == 0 ) )
        return false;

    const tgt::vec3& dirThis = getEdgeDirection();
    tgt::vec3 dirOther( (v->getVertex() - first_->getVertex()) );
    float lenOther = tgt::length(dirOther);
    dirOther = tgt::normalize(dirOther);

    // check wether this direction (from first_ to second_ vertex)
    // equals the direction from first_ to other one.
    // if both directions are parallel, the scalar product is approx. 1.0f
    //
    float f = dot(dirOther, dirThis);
    if ( (f < 0.999999f) || (f > 1.000001) )
        return false;

    // if the edges are parallel, the considered vertex can only be part
    // of this edge, if its distance to the first vertex on it is short or
    // equal to the distance from first to second one!
    //
    return (lenOther <= getEdgeLength());
}

bool Edge3D::isPartOf(const EdgeVertex3D* v) const {
    if ( (v == 0) || (first_ == 0) || (second_ == 0) )
        return false;

    if ( ((*first_ == *v) || (*second_ == *v)) )
        return true;
   
    return false;
}

Edge3D* Edge3D::split(EdgeVertex3D& v) {
    if ( (first_ == 0) || (second_ == 0) )
        return 0;

    // if the split vertex is one of the vertices contained in this edge
    // do not split the edge and return a NULL-pointer!
    //
    if ( ((*first_ == v) || (*second_ == v)) )
        return 0;

    // remove this edge from my second vertex and
    // store it in tmp
    //
    second_->removeEdge(this);
    EdgeVertex3D* tmp = second_;

    // set a copy of the split vertex as my new second vertex
    // and add myself to its list of edges and to the same polygon!
    //
    second_ = new EdgeVertex3D(v.getVertex());
    second_->addEdge(this);

    // create a new Edge with the split vertex
    // as the first one and with the previous second vertex
    // as the second one
    //
    Edge3D* e = Edge3D::createEdge(second_, tmp, polyParent_);
    
    // add e to all polygons this edge is part of.
    //
    PolygonFace3D::FaceSet::iterator it = polygons_.begin();
    for (; it != polygons_.end(); ++it)
        (*it)->insertEdge(e);
    return e;
}

Edge3D* Edge3D::split(const tgt::vec3& v) {
    // simply pack the vector into a temporary EdgeVertex3D
    // object and call the appropriated method
    //
    EdgeVertex3D ev(v);
    return split(ev);
}

// private mthods
//

void Edge3D::clear() {
    // do not delete first_ and second_ as they could sill be connected
    // with other edges and therefore form polygon faces.
    // also do not disconnect this from first_ and second_ as this edge
    // might be created temporary for comparisons and deleting it 
    // would destroy wanted connections!

    // remove the reference to this edge from all polygons containing it
    //
    PolygonFace3D::FaceSet::iterator it = polygons_.begin();
    for (; it != polygons_.end(); ++it ) {
        if (*it != 0)
            (*it)->removeEdge(this);
    }
    polygons_.clear();
}

void Edge3D::clone(const voreen::Edge3D& /*edge*/) {
    first_ = 0;
    second_ = 0;
    polyParent_ = 0;
}

}   // namespace
