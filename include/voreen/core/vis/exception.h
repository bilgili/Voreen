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

#ifndef VRN_EXCEPTION_H
#define VRN_EXCEPTION_H

#include "tgt/exception.h"

namespace voreen {

class VoreenException : public tgt::Exception {
public:
    VoreenException(const std::string& what = "") : tgt::Exception(what) {}
    virtual ~VoreenException() throw() {}
};


//-------------------------------------------------------------------------------------------------

//FIXME: remove these or move into glyph module

/**
 * Unspecified message exception used as base class for glyph exceptions.
 */
class SimpleMessageException {
public:
    virtual ~SimpleMessageException() {}
    SimpleMessageException(const std::string message): message_(message) {}
    virtual std::string getMessage() { return message_; }
private:
    std::string message_;
};

/**
 * Exception thrown when glyph loading fails.
 */
class GlyphsLoadException: public SimpleMessageException {
public:
    virtual ~GlyphsLoadException() {}
    GlyphsLoadException(const std::string message):
        SimpleMessageException(message) { }
};

/**
 * Exception thrown when a value is mapped to a property to which its type does not fit.
 */
class EnumConvertException: public SimpleMessageException {
public:
    EnumConvertException(const std::string message):
        SimpleMessageException(message) {}
};

}  // namespace voreen

#endif
