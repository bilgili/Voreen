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

#ifndef VRN_ANNOTATION_TERMINALS_H
#define VRN_ANNOTATION_TERMINALS_H

#include "voreen/core/utils/GLSLparser/terminalsmap.h"

namespace voreen {

namespace glslparser {

class PlotFunctionTerminals : public TerminalsMap {
public:
    enum SymbolIDs {
        ID_START, ID_LPAREN, ID_RPAREN, ID_LBRACKET, ID_RBRACKET, ID_DOT, ID_COMMA, ID_COLON, ID_SEMICOLON,
        ID_DASH, ID_PLUS, ID_STAR, ID_SLASH, ID_CARET,

        //ID_IDENTIFIER,
        ID_FUNCTION,
        ID_INTCONST, ID_FLOATCONST, ID_VARIABLE,
        ID_UNKNOWN,
        ID_END, ID_EMPTY
    };

public:
    PlotFunctionTerminals();

};

}   // namespace glslparser

}   // namespace voreen

#endif
