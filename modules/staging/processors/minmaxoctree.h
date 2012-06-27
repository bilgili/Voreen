/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#ifndef VRN_MINMAXOCTREE_H
#define VRN_MINMAXOCTREE_H

#include "tgt/vector.h"

namespace voreen {

struct OctreeNode {
    tgt::vec3 coordllf;
    tgt::vec3 coordurb;
    float minvox;
    float maxvox;
    float mingrad;
    float maxgrad;
    bool opaque;
    OctreeNode* parent;
    OctreeNode* llf;
    OctreeNode* lrf;
    OctreeNode* ulf;
    OctreeNode* urf;
    OctreeNode* llb;
    OctreeNode* lrb;
    OctreeNode* ulb;
    OctreeNode* urb;

    OctreeNode() : parent(0), llf(0), lrf(0), ulf(0), urf(0), llb(0), lrb(0), ulb(0), urb(0) {}
    ~OctreeNode() {delete llf; delete lrf; delete ulf; delete urf; delete llb; delete lrb; delete ulb; delete urb;}
};

class OctreeLevel {
private:
    tgt::ivec3 size_;
    OctreeNode** nodes_;
public:
    OctreeLevel(tgt::ivec3 size) : size_(size) {
        nodes_ = new OctreeNode*[size.x*size.y*size.z];
        for (int i=0; i<size.x*size.y*size.z; i++) {
            nodes_[i] = new OctreeNode();
        }
    }

    ~OctreeLevel() {
        delete[] nodes_;
    }

    OctreeNode* getNode(tgt::ivec3 pos) {
        return nodes_[pos.x*(size_.y*size_.z)+pos.y*size_.z+pos.z];
    }
};

} // namespace

#endif // VRN_MINMAXOCTREE_H

