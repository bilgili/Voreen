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

#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/condition.h"

#include "tgt/filesystem.h"

namespace voreen {

FileDialogProperty::FileDialogProperty(const std::string& id, const std::string& guiText,
                               const std::string& dialogCaption, const std::string& directory,
                               const std::string& fileFilter, FileDialogProperty::FileMode fileMode,
                               int invalidationLevel)
    : StringProperty(id, guiText, "", invalidationLevel),
      dialogCaption_(dialogCaption),
      directory_(directory),
      fileMode_(fileMode),
      fileFilter_(fileFilter)
{

    if (fileMode == DIRECTORY)
        set(directory_);
}

FileDialogProperty::FileDialogProperty() {
}

Property* FileDialogProperty::create() const {
    return new FileDialogProperty();
}

const std::string& FileDialogProperty::getDialogCaption() const {
    return dialogCaption_;
}

void FileDialogProperty::setDialogCaption(const std::string& caption) {
    dialogCaption_ = caption;
}

const std::string& FileDialogProperty::getDirectory() const {
    return directory_;
}

void FileDialogProperty::setDirectory(const std::string& dir) {
    directory_ = dir;
}

const std::string& FileDialogProperty::getFileFilter() const {
    return fileFilter_;
}

void FileDialogProperty::setFileFilter(const std::string& filter) {
    fileFilter_ = filter;
}

FileDialogProperty::FileMode FileDialogProperty::getFileMode() const {
    return fileMode_;
}

void FileDialogProperty::setFileMode(FileMode mode) {
    fileMode_ = mode;
}

void FileDialogProperty::serialize(XmlSerializer& s) const {
    Property::serialize(s);

    // convert path to an relative one with respect to the document's path
    std::string path = value_;
    if (path.empty())
        s.serialize("noPathSet", true);
    else
        s.serialize("noPathSet", false);

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

    // An empty serialized value does not necessarily mean that it wasn't set, but could also mean that it was the
    // same path as the document path passed during serialization, which makes the relative path empty.  We need an
    // extra bool to remember if this was the case.
    try {
        bool noPathSet;
        s.deserialize("noPathSet", noPathSet);
        if(noPathSet) {
            set("");
            return;
        }
    }
    catch (XmlSerializationNoSuchDataException) {
        s.removeLastError();
    }

    std::string value;
    s.deserialize("value", value);

    // convert path relative to the document's path to an absolute one
    if (!s.getDocumentPath().empty() && !tgt::FileSystem::isAbsolutePath(value))
        value = tgt::FileSystem::absolutePath(tgt::FileSystem::dirName(s.getDocumentPath()) + "/" + value);


    try {
        set(value);
    }
    catch (Condition::ValidationFailed& e) {
        s.addError(e);
    }
}

}   // namespace
