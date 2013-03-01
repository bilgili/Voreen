/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#include "voreen/core/datastructures/geometry/geometry.h"

#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/utils/hashing.h"
#include "tgt/logmanager.h"

#include <sstream>

namespace voreen {

const std::string Geometry::loggerCat_("voreen.Geometry");

Geometry::Geometry()
    : transformationMatrix_(tgt::mat4::identity)
{}

Geometry* Geometry::clone() const {
    try {
        std::stringstream stream;

        // first serialize
        XmlSerializer s;
        s.serialize("this", this);
        s.write(stream);

        // then deserialize again
        XmlDeserializer d;
        d.read(stream);
        Geometry* geometry = 0;
        d.deserialize("this", geometry);

        return geometry;
    }
    catch (std::exception& e) {
        LERROR("Failed to clone geometry '" << getClassName() << "': " << e.what());
        return 0;
    }
}

bool Geometry::equals(const Geometry* geometry, double /*epsilon = 1e-6*/) const {
    tgtAssert(geometry, "null pointer passed");
    return (getHash() == geometry->getHash());
}

void Geometry::transform(const tgt::mat4& m) {
    transformationMatrix_ =  m * transformationMatrix_;
}

tgt::mat4 Geometry::getTransformationMatrix() const {
    return transformationMatrix_;
}

tgt::mat4 Geometry::getInvertedTransformationMatrix() const {
    tgt::mat4 mInv = tgt::mat4::identity;
    transformationMatrix_.invert(mInv);
    return mInv;
}

void Geometry::setTransformationMatrix(const tgt::mat4& m) {
    transformationMatrix_ = m;
}

void Geometry::clip(const tgt::plane& /*clipPlane*/, double /*epsilon = 1e-6*/) {
    LWARNING("clip() not implemented");
}

void Geometry::render() const {
    LWARNING("render() not implemented");
}

tgt::Bounds Geometry::getBoundingBox(bool /*transformed*/) const {
    LWARNING("getBoundingBox() not implemented");
    return tgt::Bounds();
}

std::string Geometry::getHash() const {
    XmlSerializer s;
    s.setUseAttributes(true);
    serialize(s);

    std::stringstream stream;
    s.write(stream);
    return VoreenHash::getHash(stream.str());
}

void Geometry::serialize(XmlSerializer& s) const {
    s.serialize("transformationMatrix", transformationMatrix_);
}

void Geometry::deserialize(XmlDeserializer& s) {
    s.optionalDeserialize("transformationMatrix", transformationMatrix_, tgt::mat4::identity);
}

} // namespace
