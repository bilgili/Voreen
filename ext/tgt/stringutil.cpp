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

#include "stringutil.h"

#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

/**
 * Replaces the file extension of a filename
 * @param filename. If no extension present, the new one is simply appended.
 * @param file extension (without dot).
 * @return filename with new extension
 */
string StringUtil::replaceExtension(const string& filename, const string& newExt) {
    return (filename.substr(0, filename.rfind('.')) + "." + newExt);
}
/**
 * Castet einen Intwert in einen String
 * @param number Die umzuwandelnde Zahl
 * @param size  the minimal length of the string returned
 */
string StringUtil::castIntToString(int number, std::streamsize size) {
	ostringstream oss;
	oss<<setfill('0')<<setw(size)<<number;
	return oss.str();
}

