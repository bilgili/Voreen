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

#ifndef VRN_PROPERTYWIDGET_H
#define VRN_PROPERTYWIDGET_H

#include "voreen/core/vis/properties/property.h"

class TiXmlElement;

namespace voreen {

/**
 * A PropertyWidget is a graphical Representation for a Property. It
 * allows manipulation of the Property's value.
 */
class PropertyWidget {
public:
    
    virtual ~PropertyWidget() {}
    
    /**
     * The Widget should update itself from the Property's value when this is called.
     */
    virtual void update() = 0;
    virtual void setVisible(bool state) = 0;
    
    /**
     * When the Property is destroyed it calls disconnect so the Widget knows
     * the property is no longer around
     */
    virtual void disconnect() = 0;

    /**
     * Allows property widgets to serialize their state. 
     *
     * This function is called by the Property's serialization routine. 
     * If not overwritten in a subclass, it returns 0.
     */
    virtual TiXmlElement* getWidgetMetaData() const {
        return 0;
    }

};


} // namespace voreen

#endif // VRN_PROPERTYWIDGET_H
