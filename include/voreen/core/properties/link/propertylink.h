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

#ifndef VRN_PROPERTYLINK_H
#define VRN_PROPERTYLINK_H

#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/properties/link/changedata.h"
#include "voreen/core/properties/link/linkevaluatorbase.h"
#include <vector>
#include <map>

namespace voreen {

class Property;

/**
 * This class establishes a link between two properties.
 *
 * When the value of the source property has changed the onChange method is called.
 * OnChange publishes the new value to the destination property by executing the
 * link evaluator assigned to the link.
 *
 * @see LinkEvaluatorBase
 */
class PropertyLink : public Serializable {
public:
    /**
     * Registers itself as an listener of the source property.
     * If no linkEvaluator is passed, an id-link is created.
     *
     * Source and destination properties must not be null.
     */
    PropertyLink(Property* src, Property* dest, LinkEvaluatorBase* linkEvaluator = 0);

    /**
     * Removes this link from the source property.
     */
    virtual ~PropertyLink();

    /**
     * Callback-function when source property value has changed. It may modify
     * the new value and publish it to the destination property. Moreover, it
     * detects circular linking and stops publishing on detection.
     *
     * @throws VoreenException, if link execution failed. This usually
     *      because of incompatible data types.
     */
    void onChange(ChangeData& data)
        throw (VoreenException);

    /**
     * Forces the propagation with the current value thus testing functionality.
     *
     * @warn This function is deliberately not const, since it actually executes
     *      the link and might therefore affect the destination property.
     *
     * @return true, if the propagation succeeded. If false is returned,
     *      linking the respective properties is not possible.
     */
    bool testPropertyLink();

    /**
     * Assigns a new evaluator to enable transparent transition to a new
     * evaluator without recreating the link.
     */
    void setLinkEvaluator(LinkEvaluatorBase* evaluator);

    /**
     * Returns the link's evaluator.
     */
    LinkEvaluatorBase* getLinkEvaluator() const;

    /**
     * Returns the link's source property.
     */
    Property* getSourceProperty() const;

    /**
     * Returns the link's destination property.
     */
    Property* getDestinationProperty() const;

    /**
     * @see Serializable::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Serializable::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

    /**
     * Checks if the properties p1 and p2 are linkable. If a property is not linkable,
     * the most likely explanation will be that there is no conversion implemented in
     * \sa BoxObject.
     * \param p1 a property
     * \param p2 a property
     * \return true, if p1 and p2 are compatible, false otherwise
     */
    static bool arePropertiesLinkable(const Property* p1, const Property* p2);

private:
    /// Default constructor needed for serialization.
    PropertyLink();
    friend class XmlDeserializer;

    Property* src_;                 ///< Source property of the link.
    Property* dest_;                ///< Destination property of the link.
    LinkEvaluatorBase* evaluator_;  ///< Evaluator responsible for actually executing the link.

    /// Used for cycle prevention during link execution.
    static std::vector<Property*> visitedProperties_;
    /// Category used in logging
    static const std::string loggerCat_;
};

} // namespace

#endif // VRN_PROPERTYLINK_H
