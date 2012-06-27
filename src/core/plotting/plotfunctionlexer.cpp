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

#include "voreen/core/plotting/plotfunctionlexer.h"

#include "voreen/core/plotting/plotfunctionterminals.h"
#include "voreen/core/utils/GLSLparser/glsl/glsltoken.h"

#include <list>

namespace voreen {

namespace glslparser {

// There are only two keywords: "true" and "false", so this static member is really small.
//
std::map<std::string, Word> PlotFunctionLexer::keywords_ = PlotFunctionTerminals().getKeywords();

PlotFunctionLexer::PlotFunctionLexer(std::istream* const is, const std::map<std::string, Word>& keywords)
    throw (std::bad_alloc)
    : Lexer(is, keywords),
    readingString_(false)
{
    PlotFunctionLexer::keywords_ = keywords;
}

PlotFunctionLexer::PlotFunctionLexer(const std::string& fileName, const std::map<std::string, Word>& keywords)
    throw (std::bad_alloc)
    : Lexer(fileName, keywords),
    readingString_(false)
{
}


PlotFunctionLexer::~PlotFunctionLexer() {
}

Token* PlotFunctionLexer::scan() {
    //std::list<Token*> parts;

    //for (Token* token = nextToken(); token != 0; token = nextToken())
    //    parts.push_back(token);
    Token* token = nextToken();
    if (token)
        return token;
    else
        return new Token(-1);
}

// private methods
//
//TODO:: complete lexer
Token* PlotFunctionLexer::nextToken() {
    Token* token = 0;
    int tokenLine = getCurrentLineNumber();
    int tokenStart = getCurrentCharNumberInLine();

    try {
        do {
    //        if (readingString_) {
    //            readChar();
    //            if (peek_ == '"') {
    //                readingString_ = false;
    //                return new Token(PlotFunctionTerminals::ID_QUOTE);
    //            } else {
    //                std::string str;
    //                do {
    //                    str += peek_;
    //                    if (nextChar() != '"')
    //                        readChar();
    //                    else
    //                        break;
    //                } while (true);
    //                return new StringToken(PlotFunctionTerminals::ID_STRING, str);
    //            }
    //        }

            // Skip whitespace including newlines
            //
            do {
                readChar();
            } while (isWhitespace(peek_));

            switch (peek_) {
                case '.':
                    if (isDigit(nextChar()))
                        token = scanNumber();
                    else
                        token = new Token(PlotFunctionTerminals::ID_DOT);
                    break;  // case '.'

                case '+':
                    token = new Token(PlotFunctionTerminals::ID_PLUS);
                    break;  // case '+'

                case '-':
                    token = new Token(PlotFunctionTerminals::ID_DASH);
                    break;  // case '-'

                case '/':
                    token = new Token(PlotFunctionTerminals::ID_SLASH);
                    break;  // case '/'

                case '*':
                    token = new Token(PlotFunctionTerminals::ID_STAR);
                    break;  // case '*'

                case '[':
                    token = new Token(PlotFunctionTerminals::ID_LBRACKET);
                    break;  // case '['

                case ']':
                    token = new Token(PlotFunctionTerminals::ID_RBRACKET);
                    break;  // case ']'

                case '(':
                    token = new Token(PlotFunctionTerminals::ID_LPAREN);
                    break;  // case '('

                case ')':
                    token = new Token(PlotFunctionTerminals::ID_RPAREN);
                    break;  // case ')'

                case '^':
                    token = new Token(PlotFunctionTerminals::ID_CARET);
                    break;  // case '^'

                case ':':
                    token = new Token(PlotFunctionTerminals::ID_COLON);
                    break;  // case ':'

                case ';':
                    token = new Token(PlotFunctionTerminals::ID_SEMICOLON);
                    break;  // case ';'

                case ',':
                    token = new Token(PlotFunctionTerminals::ID_COMMA);
                    break;  // case ','

            }   // switch (peek_)

    //        switch (peek_) {
    //            case '@':
    //                token = new Token(PlotFunctionTerminals::ID_AT);
    //                break;

    //            case '=':
    //                token = new Token(PlotFunctionTerminals::ID_EQUAL);
    //                break;

    //            case '[':
    //                token = new Token(PlotFunctionTerminals::ID_LBRACKET);
    //                break;

    //            case ']':
    //                token = new Token(PlotFunctionTerminals::ID_RBRACKET);
    //                break;

    //            case ',':
    //                token = new Token(PlotFunctionTerminals::ID_COMMA);
    //                break;

    //            case '\"':
    //                token = new Token(PlotFunctionTerminals::ID_QUOTE);
    //                readingString_ = true;
    //                break;

    //            case '\'':
    //                token = new Token(PlotFunctionTerminals::ID_APOSTROPHE);
    //                break;
    //        }   // switch (peek_)

            if (token == 0) {
                if (isAlpha(peek_))
                    token = scanIdentifier(PlotFunctionTerminals::ID_VARIABLE);
                else if (isDigit(peek_) || (peek_ == '.'))
                    token = scanNumber();
            }
        } while (token == 0);
    } catch (std::runtime_error&) {
        if (token != 0) {
            token->setLineNumber(tokenLine);
            token->setCharNumber(tokenStart);
        }

        return token;
    }

    if (token != 0) {
        token->setLineNumber(tokenLine);
        token->setCharNumber(tokenStart);
    }
    return token;
}

ConstantToken* PlotFunctionLexer::scanNumber() throw (std::runtime_error) {
    ConstantToken* t = Lexer::scanNumber(PlotFunctionTerminals::ID_UNKNOWN);

    if (t != 0) {
        switch (t->getType()) {
            case ConstantToken::TYPE_FLOAT:
                t->replaceTokenID(PlotFunctionTerminals::ID_FLOATCONST);
                break;

            case ConstantToken::TYPE_UINT:
            case ConstantToken::TYPE_INT:
                t->replaceTokenID(PlotFunctionTerminals::ID_INTCONST);
                break;
        }
    }

    return t;
}

}   // namespace glslparser

}   // namespace voreen
