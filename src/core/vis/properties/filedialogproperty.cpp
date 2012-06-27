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
#include "voreen/core/vis/properties/propertywidgetfactory.h"

namespace voreen {

FileDialogProperty::FileDialogProperty(const std::string& id, const std::string& guiText,
                               const std::string& dialogCaption, const std::string& directory,
                               const std::string& fileFilter, FileDialogProperty::FileMode fileMode,
                               Processor::InvalidationLevel invalidationLevel)
    : TemplateProperty<std::string>(id, guiText, "", invalidationLevel),
      dialogCaption_(dialogCaption),
      directory_(directory),
      fileMode_(fileMode),
      fileFilter_(fileFilter)
{

    if (fileMode == DIRECTORY)
        set(directory_);
}

void FileDialogProperty::serialize(XmlSerializer& s) const {
    Property::serialize(s);

    // convert path to an relative one with respect to the document's path
    std::string path = value_;
    if (!path.empty() && !s.getDocumentPath().empty())
        path = tgt::FileSystem::relativePath(path, tgt::FileSystem::dirName(s.getDocumentPath()));

    // cleanup path: replace backslashes
    std::string::size_type pos = path.find("\\");
    while (pos != std::string::npos) {
        path[pos] = '/';
        pos = path.find("\\");
    }

    s.serialize("value", path);
}

void FileDialogProperty::deserialize(XmlDeserializer& s) {
    Property::deserialize(s);

    std::string value;
    s.deserialize("value", value);

    // convert path relative to the document's path to an absolute one
    if (!value.empty() && !s.getDocumentPath().empty())
        value = tgt::FileSystem::absolutePath(tgt::FileSystem::dirName(s.getDocumentPath()) + "/" + value);

    try {
        set(value);
    }
    catch (Condition::ValidationFailed& e) {
        s.addError(e);
    }
}

PropertyWidget* FileDialogProperty::createWidget(PropertyWidgetFactory* f) {
    return f->createWidget(this);
}

}   // namespace
