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

#include "voreen/core/datastructures/roi/roicube.h"
#include "voreen/core/datastructures/geometry/meshlistgeometry.h"

#include "voreen/core/io/serialization/xmlserializer.h"
#include "voreen/core/io/serialization/xmldeserializer.h"

namespace voreen {

using tgt::vec3;
using tgt::vec4;
using tgt::mat4;

const std::string ROICube::loggerCat_ = "voreen.ROICube";

ROICube::ROICube() : ROINormalizedGeometry()
{
}

ROICube::ROICube(Grid grid, tgt::vec3 center, tgt::vec3 dimensions) : ROINormalizedGeometry(grid, center, dimensions)
{
}

bool ROICube::inROINormalized(tgt::vec3 p) const {
    if(tgt::hand(tgt::greaterThanEqual(p, vec3(-1.0f))) && tgt::hand(tgt::lessThanEqual(p, vec3(1.0f))))
        return true;
    else
        return false;
}

MeshListGeometry* ROICube::generateNormalizedMesh() const {
    MeshListGeometry* geometry = new MeshListGeometry();
    geometry->addMesh(MeshGeometry::createCube(vec3(-1.0f), vec3(1.0f), vec3(0.0f), vec3(1.0f), getColor().xyz(), getColor().xyz()));
    return geometry;
}

MeshListGeometry* ROICube::generateNormalizedMesh(tgt::plane pl) const {
    FaceGeometry planeFace = createQuad(pl, getColor());

    vec4 xm(-1.0f, 0.0f, 0.0f, 1.0f);
    vec4 xp(1.0f, 0.0f, 0.0f, 1.0f);
    vec4 ym(0.0f, -1.0f, 0.0f, 1.0f);
    vec4 yp(0.0f, 1.0f, 0.0f, 1.0f);
    vec4 zm(0.0f, 0.0f, -1.0f, 1.0f);
    vec4 zp(0.0f, 0.0f, 1.0f, 1.0f);

    planeFace.clip(xm);
    planeFace.clip(xp);
    planeFace.clip(ym);
    planeFace.clip(yp);
    planeFace.clip(zm);
    planeFace.clip(zp);

    MeshGeometry mg;

    if(planeFace.getVertexCount() > 2)
        mg.addFace(planeFace);

    MeshListGeometry* mlg = new MeshListGeometry();
    mlg->addMesh(mg);
    return mlg;
}

} // namespace
