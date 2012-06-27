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

#ifndef VRN_POLYGON3D_H
#include "voreen/core/vis/slicing/polygon3d.h"

using std::list;

namespace voreen {

// public methods
//

EdgeVertex3D::EdgeVertex3D(const tgt::vec3& v) : v_(v) {
}

EdgeVertex3D::EdgeVertex3D(const tgt::vec3& v, Edge3D* const edge) : v_(v) {
    if (edge != 0)
        addEdge(edge);
}

EdgeVertex3D::EdgeVertex3D(const EdgeVertex3D& vertex) {
    clone(vertex);
}

EdgeVertex3D::~EdgeVertex3D() {
//std::cout << "~EdgeVertex3D() on " << v_ << "...\n";
    clear();
}

EdgeVertex3D& EdgeVertex3D::operator=(const EdgeVertex3D& vertex) {
    clear();
    clone(vertex);
    return *this;
}

bool EdgeVertex3D::operator ==(const EdgeVertex3D& v) const {
    return (v_ == v.getVertex());
}

bool EdgeVertex3D::operator !=(const EdgeVertex3D& v) const {
    return (! (v_ == v.getVertex()) );
}

bool EdgeVertex3D::operator <(const EdgeVertex3D& v) const {
    const tgt::vec3& v2 = v.getVertex();
    if ( v_.x < v2.x )
        return true;
    if ( (v_.x == v2.x) && (v_.y < v2.y) )
        return true;
    if ( (v_.x == v2.x) && (v_.y == v2.y) && (v_.z < v2.z) )
        return true;

    return false;
}

bool EdgeVertex3D::operator >(const EdgeVertex3D& v) const {
    const tgt::vec3& v2 = v.getVertex();
    if ( v_.x > v2.x )
        return true;
    if ( (v_.x == v2.x) && (v_.y > v2.y) )
        return true;
    if ( (v_.x == v2.x) && (v_.y == v2.y) && (v_.z > v2.z) )
        return true;

    return false;
}

const Edge3D::EdgeSet& EdgeVertex3D::getEdges() const {
    return edges_;
}

tgt::vec3 EdgeVertex3D::getVertex() const {
    return v_;
}
void EdgeVertex3D::setVertex(const tgt::vec3& v) {
    v_ = v;
}

bool EdgeVertex3D::addEdge(Edge3D* e) {
    if (e != 0) {
        // due to special comparison on pointers, the edge will NOT
        // be inserted, if the DEREFERENCED edge pointers are equal!
        //
        edges_.insert(e);
        return true;
    }
    return false;
}

void EdgeVertex3D::removeEdge(Edge3D* const e) {
    edges_.erase(e);
}

bool EdgeVertex3D::isAdjacentToEdge(Edge3D* e) const {
    if (e == 0)
        return false;

    Edge3D::EdgeSet::const_iterator it = edges_.find(e);

    return (it != edges_.end());
}

// private
//

void EdgeVertex3D::clear() {
    // disconnect this vertex from all adjacent
    // edges. the becomes degenerated due to
    // disconnect and will be deleted.
    //
    Edge3D::EdgeSet::iterator it = edges_.begin();
    for (; it != edges_.end(); ++it ) {
        if (*it != 0)
            (*it)->disconnect(this);

        delete *it;
    }
    edges_.clear();
}

void EdgeVertex3D::clone(const EdgeVertex3D& vertex) {
    v_ = vertex.getVertex();
}

}   // namespace

#endif
