/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "tgt/assert.h"

#include <sstream>
#include <iostream>
#include <cstdlib>

#ifdef TGT_DEBUG

#ifndef TGT_NON_INTERACTIVE_ASSERT
bool myCustomAssert(long line, const char* filename, const char* functionName, const char* description, bool& always) {
#else
bool myCustomAssert(long line, const char* filename, const char* functionName, const char* description, bool& /*always*/) {
#endif
    std::cout << "Assertion: " << filename << " " << line << ":" << std::endl << "     " << functionName << std::endl;
    std::cout << "   " << description << std::endl;
#ifndef TGT_NON_INTERACTIVE_ASSERT
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
#else
    exit(EXIT_FAILURE);
#endif
}

#endif // TGT_DEBUG
