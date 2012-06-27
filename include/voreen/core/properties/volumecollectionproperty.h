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

#ifndef VRN_VOLUMECOLLECTIONPROPERTY_H
#define VRN_VOLUMECOLLECTIONPROPERTY_H

#include "voreen/core/properties/templateproperty.h"
#include "voreen/core/properties/condition.h"

namespace voreen {

class VolumeCollection;

/**
 * Property encapsulating a VolumeCollection object.
 */
class VolumeCollectionProperty : public TemplateProperty<VolumeCollection*> {

public:
    /**
     * Constructor.
     *
     * @param ident identifier that is used in serialization
     * @param guiText text that is shown in the gui
     * @param value the initial volume handle to assign
     */
    VolumeCollectionProperty(const std::string& id, const std::string& guiText, VolumeCollection* const value = 0,
        Processor::InvalidationLevel invalidationLevel = Processor::INVALID_PARAMETERS);

    /// @see Property::createWidget
    virtual PropertyWidget* createWidget(PropertyWidgetFactory* f);

    /// @see Property::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Property::deserialize
    virtual void deserialize(XmlDeserializer& s);

protected:
    /// Deletes the assigned collection.
    virtual void deinitialize() throw (VoreenException);

};

} // namespace voreen

#endif // VRN_VOLUMECOLLECTIONPROPERTY_H
