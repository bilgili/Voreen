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

#ifndef VRN_STRINGUTILS_H
#define VRN_STRINGUTILS_H

#include <string>

namespace voreen {

/// Converts an int to a string using stringstreams
std::string itos(int i);

/// Converts a string to an int using stringstreams
int stoi(const std::string& s);

/// Converts a float to a string using stringstreams
std::string ftos(float f);

/// Converts a string to a float using stringstreams
float stof(const std::string& s);

/// Converts a double to a string using stringstreams
std::string dtos(double d);

/// Converts a string to a double using stringstreams
double stod(const std::string& s);

/**
 * Converts the string to a null-terminated char array with length s.size()+1.
 * Deleting the allocated memory is up to the caller.
 */
char* strtochr(const std::string& s);

/**
 * Returns a copy of \p str where all occurrences of \p from have been replaced by \p to.
 */
std::string strReplaceAll(const std::string& str, const std::string& from, const std::string& to);

} // namespace

#endif // VRN_STRINGUTILS_H
