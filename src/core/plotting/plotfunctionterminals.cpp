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

#include "voreen/core/plotting/plotfunctionterminals.h"

namespace voreen {

namespace glslparser {

PlotFunctionTerminals::PlotFunctionTerminals() {
    insertSymbol("(", ID_LPAREN);
    insertSymbol(")", ID_RPAREN);
    insertSymbol("[", ID_LBRACKET);
    insertSymbol("]", ID_RBRACKET);
    insertSymbol(".", ID_DOT);
    insertSymbol(",", ID_COMMA);
    insertSymbol(":", ID_COLON);
    insertSymbol(";", ID_SEMICOLON);
    insertSymbol("-", ID_DASH);
    insertSymbol("+", ID_PLUS);
    insertSymbol("*", ID_STAR);
    insertSymbol("/", ID_SLASH);
    insertSymbol("^", ID_CARET);

//    insertSymbol("IDENTIFIER", ID_IDENTIFIER);
    insertSymbol("FUNCTION", ID_FUNCTION);
    insertSymbol("INTCONST", ID_INTCONST);
    insertSymbol("FLOATCONST", ID_FLOATCONST);
    insertSymbol("VARIABLE", ID_VARIABLE);
}

}   // namespace glslparser

}   // namespace voreen
