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

#ifndef VRN_PROPERTYLINK_H
#define VRN_PROPERTYLINK_H

#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/vis/properties/link/changedata.h"
#include "voreen/core/vis/properties/link/linkevaluatorbase.h"
#include <vector>
#include <map>

namespace voreen {

class Property;

/**
 * This class establishes a link between two properties. When the value of the
 * source property has changed the OnChange-method is called. OnChange publishes
 * the new value either directly or modified by an IEvaluator to the destination
 * property.
 */
class PropertyLink : public Serializable {
public:
    /**
     * Registers itself as an listener of the source property.
     * If no linkEvaluator is passed, an id-link is created.
     */
    PropertyLink(Property* src, Property* dest, LinkEvaluatorBase* linkEvaluator = 0);

    /**
     * Removes this link from the source property.
     */
    virtual ~PropertyLink();

    virtual bool isActive() const { return getLinkEvaluator()->isActive(); }

    /**
     * Callback-function when source property value has changed. It may modifies
     * the new value and publishes it to the destination property. Moreover, it
     * detects circular linking and stops publishing on detection.
     */
    void onChange(ChangeData& data);

    /**
     * Forces the propagation with the current value thus testing functionality.
     *
     * @return true, if the propagation succeeded. If false is returned,
     *      linking the respective properties is not possible.
     */
    bool testPropertyLink();

    /**
     * Set a new evaluator to replace the currently used one.
     */
    void setLinkEvaluator(LinkEvaluatorBase* evaluator);

    /// @see setLinkEvaluator
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

private:
    Property* src_;
    Property* dest_;
    static std::vector<Property*> visitedProperties_;
    LinkEvaluatorBase* evaluator_;


    friend class XmlDeserializer;
    /// Default constructor needed for serialization.
    PropertyLink();
};

} // namespace

#endif // VRN_PROPERTYLINK_H
