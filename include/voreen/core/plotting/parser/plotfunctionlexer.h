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

#ifndef VRN_PLOTFUNCTIONLEXER_H
#define VRN_PLOTFUNCTIONLEXER_H

#include "voreen/core/utils/GLSLparser/lexer.h"
#include "voreen/core/plotting/parser/plotfunctionterminals.h"

namespace voreen {

namespace glslparser {

class PlotFunctionLexer : public Lexer {
public:

    PlotFunctionLexer(std::istream* const is, const std::map<std::string, Word>& keywords = PlotFunctionTerminals().getKeywords())
        throw (std::bad_alloc);

    PlotFunctionLexer(const std::string& fileName, const std::map<std::string, Word>& keywords = PlotFunctionTerminals().getKeywords())
        throw (std::bad_alloc);

    virtual ~PlotFunctionLexer();
    /// return the next token in the expression
    virtual Token* scan();

private:
    Token* nextToken();

    IdentifierToken* scanIdentifier() throw (std::runtime_error);
    ConstantToken* scanNumber() throw (std::runtime_error);

private:
    static std::map<std::string, Word> keywords_;
    bool readingString_;
};

}   // namespace glslparser

}   // namespace voreen

#endif
