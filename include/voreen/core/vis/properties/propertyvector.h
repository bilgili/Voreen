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

#ifndef VRN_PROPERTYVECTOR_H
#define VRN_PROPERTYVECTOR_H

#include "tgt/vector.h"
#include "tgt/logmanager.h"
#include "voreen/core/vis/properties/templateproperty.h"
#include "voreen/core/vis/properties/condition.h"

#include <vector>
#include <string>

namespace voreen {

class PropertyVector : public Property {
public:

    PropertyVector(const std::string& id, const std::string& guiText, std::vector<Property*> properties); 

    virtual ~PropertyVector();

    void addProperty(Property* prop);

    const std::vector<Property*>& getProperties() const;

    template<typename T>
    T* getProperty(int id) {
        T* prop = 0;       
        if (id >= 0 && id < getNumProperties()) {
            prop = dynamic_cast<T*>(properties_[id]);
            if (!prop) {
                LERROR("Property is of invalid type");
            }
        }
        else {
            LERROR("Invalid property id: " << id);
        }
        return prop;
    } 

    int getNumProperties() const;

    void updateFromXml(TiXmlElement* propElem);

    TiXmlElement* serializeToXml() const;

    PropertyWidget* createWidget(PropertyWidgetFactory* f);

    void setOwner(Processor* processor);

    std::string toString() const;

protected:
    std::vector<Property*> properties_;

    static const std::string loggerCat_;

};

}   // namespace

#endif
