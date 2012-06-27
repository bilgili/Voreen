/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2010 Visualization and Computer Graphics Group, *
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

#ifndef VRN_LINKEVALUATOR_H
#define VRN_LINKEVALUATOR_H

/*
 * ATTENTION: Do not include voreen/core/properties/property.h due to circular
 *            header-file inclusion. Keep the Property forward declaration instead.
 */
#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

class Property;

/**
 * An object of this type is assigned to each PropertyLink, which uses
 * it for the actual link execution.
 */
class LinkEvaluatorBase : public AbstractSerializable {
public:
    ///Called by PropertyLink for executing the link.
    virtual void eval(Property* src, Property* dst) throw (VoreenException) = 0;

    virtual void propertiesChanged(Property* src, Property* dst);

    ///Returns the evaluator's GUI name.
    virtual std::string name() const = 0;

    /**
     * Returns the name of this class as a string.
     * Necessary due to the lack of code reflection in C++.
     *
     * This method is expected to be re-implemented by each concrete subclass.
     */
    virtual std::string getClassName() const = 0;

    //Returns true if the LinkEvaluator can link the two properties.
    virtual bool arePropertiesLinkable(const Property* p1, const Property* p2) const = 0;

    ///Virtual constructor: supposed to return an instance of the concrete LinkEvaluator class.
    virtual LinkEvaluatorBase* create() const = 0;

    /// @see Serializable::serialize
    virtual void serialize(XmlSerializer&) const {}

    /// @see Serializable::deserialize
    virtual void deserialize(XmlDeserializer&) {}
};

} // namespace

#endif // VRN_LINKEVALUATOR_H
