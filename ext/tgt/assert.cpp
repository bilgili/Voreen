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

#include "tgt/assert.h"

#include <sstream>
#include <iostream>

#ifdef TGT_DEBUG

bool myCustomAssert(long line, const char* filename, const char* functionName, const char* description, bool& always) {
    std::cout << "Assertion: " << filename << " " << line << ":" << std::endl << "     " << functionName << std::endl;
    std::cout << "   " << description << std::endl;
    do {
        std::cout << "   (b)reakpoint, (i)gnore, ignore (a)lways, (q)uit" << std::endl;
        std::string inStr;
        std::cin >> inStr;
        switch (inStr[0]) {
            case 'b':
                return true;
            case 'i':
                return false;
            case 'a':
                always = true;
                return false;
            case 'q':
                exit(EXIT_FAILURE);
            default:
                std::cout << "An assertion in a assertion... :(" << std::endl;
        }
    } while (true);
}

#endif // TGT_DEBUG
