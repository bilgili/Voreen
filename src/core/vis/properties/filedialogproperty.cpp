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

#include "voreen/core/vis/properties/filedialogproperty.h"
#include "voreen/core/vis/properties/condition.h"
#include "voreen/core/vis/propertywidgetfactory.h"

namespace voreen {

FileDialogProp::FileDialogProp(const std::string& id, const std::string& guiText,
                               const std::string& dialogCaption, const std::string& directory,
                               const std::string& fileFilter, bool invalidate, bool invalidateShader)
    : TemplateProperty<std::string>(id, guiText, "", invalidate, invalidateShader),
      dialogCaption_(dialogCaption),
      directory_(directory),
      fileFilter_(fileFilter)
{}

TiXmlElement* FileDialogProp::serializeToXml() const {
    TiXmlElement* propElem = Property::serializeToXml();

    propElem->SetAttribute("value", get());

    if (getSerializeMetaData()) {
        propElem->SetAttribute("class", "FileDialogProperty");
        propElem->SetAttribute("caption", getDialogCaption());
        propElem->SetAttribute("directory", getDirectory());
        propElem->SetAttribute("filefilter", getFileFilter());
    }
    
    return propElem;
}

void FileDialogProp::updateFromXml(TiXmlElement* propElem) {
    Property::updateFromXml(propElem);
    if (propElem->Attribute("value"))
        try {
            set(propElem->Attribute("value"));
        } catch (Condition::ValidationFailed& e) {
            errors_.store(e);
        }
    else
        errors_.store(XmlAttributeException("Attribute 'value' missing in Property element!"));
}

PropertyWidget* FileDialogProp::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

}   // namespace
