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

#ifndef VRN_GEOMETRY_H
#define VRN_GEOMETRY_H

#include "voreen/core/voreencoreapi.h"
#include "voreen/core/io/serialization/abstractserializable.h"

#include "tgt/vector.h"
#include "tgt/matrix.h"
#include "tgt/bounds.h"

#include <string>
#include <vector>

namespace voreen {

/**
 * Abstract base class for Geometry objects that
 * can be passed through GeometryPorts.
 *
 * @note In order to enable serialization (necessary for caching),
 *  a factory has to be provided for each concrete subtype.
 *
 * @see GeometryFactory
 * @see VoreenModule::registerSerializerFactory
 */
class VRN_CORE_API Geometry : public AbstractSerializable {
public:
    Geometry();
    virtual ~Geometry() {}

    /**
     * Virtual constructor: supposed to return an instance of the concrete subtype.
     */
    virtual Geometry* create() const = 0;

    /**
     * Returns the name of this class as a string.
     * Necessary due to the lack of code reflection in C++.
     *
     * This method is expected to be re-implemented by each concrete subclass.
     */
    virtual std::string getClassName() const = 0;

    /**
     * Returns a copy of the geometry object.
     *
     * The default implementation uses XML serialization/deserialization
     * for cloning. For performance and accuracy reasons, however,
     * the cloning should be implemented directly.
     */
    virtual Geometry* clone() const;

    /**
     * Returns whether the geometry is equal to the passed one.
     *
     * This function is supposed to be overridden by subclasses.
     * The default implementation compares the geometries' hashes.
     *
     * @param geometry Geometry object to compare to
     * @param epsilon maximum distance at which two vertices are to be considered equal
     */
    virtual bool equals(const Geometry* geometry, double epsilon = 1e-5) const;

    /**
     * Transforms the geometry by applying the given transformation matrix.
     *
     * This function is supposed to be overridden by subclasses.
     * The default implementation is a no-op.
     */
    virtual void transform(const tgt::mat4& transformation);

    /**
     * Clips the geometry against the passed clipping plane.
     *
     * The clipping plane is represented by the equation
     * NX - c = 0 where N is the normal vector of the clipping plane
     * and corresponds to the x-, y-, and z-coordinate of the given vector,
     * and c is a scalar corresponding to the w-coordinate of the given
     * vector and X is any point on the plane.
     *
     * This function is supposed to be overridden by subclasses.
     * The default implementation is a no-op.
     *
     * @note It is not necessary that the clipping plane normal vector
     *       is normalized, but it may improve computational accuracy.
     *
     * @param clipPlane an arbitrary clipping plane
     * @param epsilon maximum distance at which two vertices are to be considered equal
     */
    virtual void clip(const tgt::vec4& clipPlane, double epsilon = 1e-5);

    /**
     * Renders the geometry.
     *
     * This function is supposed to be overridden by subclasses.
     * The default implementation is a no-op.
     */
    virtual void render() const;

    /**
     * Returns the axis-aligned bounding box of the geometry.
     *
     * This function is supposed to be overridden by subclasses.
     * The default implementation returns undefined bounds.
     */
    virtual tgt::Bounds getBoundingBox() const;

    /**
     * Returns a hash of the geometry object,
     * which is used for the caching mechanism.
     *
     * The default implementation computes an MD5 hash
     * of the serialized XML string. Subclasses may
     * override this method, but are not required to do so.
     */
    virtual std::string getHash() const;

    bool hasChanged() const;
    void setHasChanged(bool changed);

    /**
     * Dummy implementation of the AbstractSerializable interface.
     * Supposed to be overridden by each concrete subclass.
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * Dummy implementation of the AbstractSerializable interface.
     * Supposed to be overridden by each concrete subclass.
     */
    virtual void deserialize(XmlDeserializer& s);

protected:
    /// indicates whether the geometry has changed.
    bool changed_;

    static const std::string loggerCat_;
};

} // namespace

#endif  //VRN_GEOMETRY_H
