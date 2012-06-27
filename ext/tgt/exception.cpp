/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#include "tgt/exception.h"

namespace tgt {

using std::string;

//------------------------------------------------------------------------------

const char* Exception::what() const throw() {
    return what_.c_str();
};

//------------------------------------------------------------------------------

std::string FileException::getErrorMessage(const string& exception, const string& /*what*/,
                                           const string& filename) const throw()
{
    string s = exception;

    if (!what_.empty())
        s += ": " + what_;

    if (!filename_.empty())
        s += " (" + filename + ")";

    return s;
}

const char* FileException::what() const throw() {
    return getErrorMessage("FileException", what_, filename_).c_str();
};

//------------------------------------------------------------------------------

const char* FileNotFoundException::what() const throw() {
    return getErrorMessage("FileNotFoundException", what_, filename_).c_str();
};

//------------------------------------------------------------------------------

const char* FileAccessException::what() const throw() {
    return getErrorMessage("FileAccessException", what_, filename_).c_str();
};

//------------------------------------------------------------------------------

const char* CorruptedFileException::what() const throw() {
    return getErrorMessage("CorruptedFileException", what_, filename_).c_str();
};

//------------------------------------------------------------------------------

const char* UnsupportedFormatException::what() const throw() {
    return getErrorMessage("UnsupportedFormatException", what_, filename_).c_str();
};

//------------------------------------------------------------------------------

const char* IOException::what() const throw() {
    return getErrorMessage("IOException", what_, filename_).c_str();
};

} // namespace tgt
