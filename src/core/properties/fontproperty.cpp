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

#include "voreen/core/properties/fontproperty.h"

#include "voreen/core/properties/condition.h"
#include "voreen/core/voreenapplication.h"

#include "tgt/filesystem.h"

namespace voreen {

FontProperty::FontProperty(const std::string& id, const std::string& guiText, tgt::Font* value,
                           int invalidationLevel)
                               : TemplateProperty<tgt::Font*>(id, guiText, value, invalidationLevel)
{
    if (!value)
        value_ = new tgt::Font(VoreenApplication::app()->getFontPath("VeraMono.ttf"), 12, tgt::Font::TextureFont, 999);
}

FontProperty::FontProperty() {
    value_ = 0;
}

FontProperty::~FontProperty() {
    delete value_;
}

Property* FontProperty::create() const {
    return new FontProperty();
}

void FontProperty::serialize(XmlSerializer& s) const {
    Property::serialize(s);

    tgtAssert(value_, "no font object");
    s.serialize("fontType", tgt::Font::getFontTypeName(value_->getFontType()));
    s.serialize("fontSize", value_->getSize());
    s.serialize("fontName", tgt::FileSystem::fileName(get()->getFontName()));
    s.serialize("textAlignment", tgt::Font::getTextAlignmentName(value_->getTextAlignment()));
    s.serialize("verticalTextAlignment", tgt::Font::getVerticalTextAlignmentName(value_->getVerticalTextAlignment()));
    s.serialize("lineWidth", value_->getLineWidth());
}

void FontProperty::deserialize(XmlDeserializer& s) {
    Property::deserialize(s);

    int fontSize;
    std::string fontName;
    std::string fontTypeName;
    tgt::Font::FontType fontType;
    std::string textAlignmentName;
    tgt::Font::TextAlignment textAlignment;
    std::string verticalTextAlignmentName;
    tgt::Font::VerticalTextAlignment verticalTextAlignment;
    float lineWidth;

    s.deserialize("fontType", fontTypeName);
    fontType = tgt::Font::getFontType(fontTypeName);
    s.deserialize("fontSize", fontSize);
    s.deserialize("fontName", fontName);
    fontName = tgt::FileSystem::fileName(fontName);
    s.deserialize("textAlignment", textAlignmentName);
    textAlignment = tgt::Font::getTextAlignment(textAlignmentName);
    s.deserialize("verticalTextAlignment", verticalTextAlignmentName);
    verticalTextAlignment = tgt::Font::getVerticalTextAlignment(verticalTextAlignmentName);
    s.deserialize("lineWidth", lineWidth);

    if (!fontName.empty() && fontType != tgt::Font::NIL) {
        delete value_;
        set(new tgt::Font(VoreenApplication::app()->getFontPath(fontName), fontSize, fontType, lineWidth, textAlignment, verticalTextAlignment));
    }
}

} // namespace
