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

#include "voreen/core/plotting/plotfunctionparser.h"

#include "voreen/core/plotting/plotfunctionlexer.h"
#include "voreen/core/plotting/plotfunctionnode.h"
//#include "voreen/core/utils/GLSLparser/annotations/annotationnodes.h"
#include "voreen/core/plotting/plotfunctionterminals.h"

namespace voreen {

namespace glslparser {

PlotFunctionParser::PlotFunctionParser(PlotFunctionLexer *const lexer)
    : Parser(lexer,false)
    , tokens_()
{
}

PlotFunctionParser::PlotFunctionParser(std::istream* const is) throw (std::bad_alloc)
    : Parser(new PlotFunctionLexer(is), true)
    , tokens_()
{
}

PlotFunctionParser::PlotFunctionParser(const std::string& fileName) throw (std::bad_alloc)
    : Parser(new PlotFunctionLexer(fileName,  PlotFunctionTerminals().getKeywords()), true)
    , tokens_()
{
}

PlotFunctionParser::~PlotFunctionParser() {
    while (! tokens_.empty()) {
        delete tokens_.front();
        tokens_.pop_front();
    }
}

// protected
//

Token* PlotFunctionParser::nextToken() {
    if (! tokens_.empty()) {
        Token* const t = tokens_.front();
        tokens_.pop_front();
        return t;
    }
    else
        return Parser::nextToken();

    return 0;
}
//TODO:: complete parser

void PlotFunctionParser::expandParseTree(const int productionID,
                                       const std::vector<Parser::ParserSymbol*>& /*reductionBody*/)
{
    switch (productionID) {
        case 0:  // [$START$] ::= [term]
        case 1:  // [$START$] ::= [term-list]
        case 2:  // [term-list] ::= [termwithinterval]
            break;
        case 3:  // [term-list] ::= [termwithinterval] , [term-list]
            {
                PlotFunctionTags* const tag = popNode<PlotFunctionTags>();
                PlotFunctionTags* const list = popNode<PlotFunctionTags>();

                if ((list != 0) && (tag != 0)) {
                    list->merge(tag);
                    pushNode(list);
                }
                delete tag;
            }
            break;
        case 4:  // [termwithinterval] ::= [term] : [interval]
            break;
        case 5:  // [interval] ::= [leftrange] ; [rightrange]
            break;
        case 6:  // [leftrange] ::= ( [term]
        case 7:  // [leftrange] ::= [ [term]
            break;
        case 8:  // [rightrange] ::= [term] )
        case 9:  // [rightrange] ::= [term] ]
            break;
        case 10:  // [term] ::= [term] + [addend]
        case 11:  // [term] ::= [term] - [addend]
        case 12:  // [term] ::= + [addend]
        case 13:  // [term] ::= - [addend]
            break;
        case 14:  // [addend] ::= [factor]
            break;
        case 15:  // [addend] ::= [addend] * [factor]
        case 16:  // [addend] ::= [addend] / [factor]
            break;
        case 17:  // [factor] ::= [variable]
            break;
        case 18:  // [factor] ::= [variable] ^ [factor]
            break;
        case 19:  // [variable] ::= [atom]
            break;
        case 20:  // [variable] ::= [function] [bracket-term]
            break;
        case 21:  // [bracket-term] ::= ( [term] )
            break;
        case 22:  // [function] ::= FUNCTION
            break;
        case 23:  // [atom] ::= [bracket-term]
        case 24:  // [atom] ::= [single-value]
            break;
        case 25:  // [single-value] ::= INTCONST
        case 26:  // [single-value] ::= FLOATCONST
        case 27:  // [single-value] ::= VARIABLE
            break;
    } // switch (productionID)

    //switch (productionID) {
    //    case 0:  // [$START$] ::= [annotation]
    //    case 1:  // [annotation] ::= [tag-list]
    //    case 2:  // [tag-list] ::= [tag]
    //        break;

    //    case 3:  // [tag-list] ::= [tag-list] [tag]
    //    case 4:  // [tag-list] ::= [tag-list] , [tag]
    //        {
    //            AnnotationTags* const tag = popNode<AnnotationTags>();
    //            AnnotationTags* const list = popNode<AnnotationTags>();

    //            if ((list != 0) && (tag != 0)) {
    //                list->merge(tag);
    //                pushNode(list);
    //            }
    //            delete tag;
    //        }
    //        break;

    //    case 5:  // [tag] ::= [tag-assignment] [ [value-list] ]
    //    case 6:  // [tag] ::= [tag-assignment] [single-value]
    //        {
    //            AnnotationTagValues* const values = popNode<AnnotationTagValues>();
    //            AnnotationTagName* const  names = popNode<AnnotationTagName>();

    //            if ((values != 0) && (names != 0))
    //                pushNode(new AnnotationTags(names, values));
    //        }
    //        break;

    //    case 7:  // [tag-assignment] ::= [tag-name] =
    //        break;

    //    case 8:  // [tag-assignment] ::= [tag-assignment] [tag-name] =
    //        if (reductionBody.size() == 3) {
    //            AnnotationTagName* const name = popNode<AnnotationTagName>();
    //            AnnotationTagName* const list = popNode<AnnotationTagName>();

    //            if ((list != 0) && (name != 0)) {
    //                list->addTagName(name);
    //                pushNode(list);
    //            }
    //        }
    //        break;

    //    case 9:  // [tag-name] ::= @ IDENTIFIER
    //        if (reductionBody.size() == 2) {
    //            IdentifierToken* const name =
    //                dynamic_cast<IdentifierToken* const>(reductionBody[0]->getToken());

    //            if (name != 0)
    //                pushNode(new AnnotationTagName(name));
    //        }
    //        break;

    //    case 10:  // [value-list] ::= [single-value]
    //        break;

    //    case 11:  // [value-list] ::= [value-list] , [single-value]
    //        if (reductionBody.size() == 3) {
    //            AnnotationTagValues* const val = popNode<AnnotationTagValues>();
    //            AnnotationTagValues* const list = popNode<AnnotationTagValues>();

    //            if ((list != 0) && (val != 0)) {
    //                list->merge(val);
    //                pushNode(list);
    //            }
    //            delete val;
    //        }
    //        break;

    //    case 12:  // [single-value] ::= true
    //    case 13:  // [single-value] ::= false
    //    case 14:  // [single-value] ::= INTCONST
    //    case 15:  // [single-value] ::= FLOATCONST
    //        if (reductionBody.size() == 1) {
    //            Token* const token = reductionBody[0]->getToken();

    //            if (token != 0)
    //                pushNode(new AnnotationTagValues(token));
    //        }
    //        break;

    //    case 16:  // [single-value] ::= " STRING "
    //    case 17:  // [single-value] ::= ' CHARACTER '
    //        if (reductionBody.size() == 3) {
    //            Token* const token = reductionBody[1]->getToken();

    //            if (token != 0)
    //                pushNode(new AnnotationTagValues(token));
    //        }
    //        break;

    //} // switch (productionID
}

ParserAction* PlotFunctionParser::action(const int stateID, const int symbolID) const {
    switch (stateID) {
        case 0:
            switch (symbolID) {
                case 10:  // +
                    return new ParserActionTransition(1);

                case 9:  // -
                    return new ParserActionTransition(2);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 1:
            switch (symbolID) {
                case 1:  // (
                    return new ParserActionTransition(4);

                case 16:  // FLOATCONST
                    return new ParserActionTransition(5);

                case 14:  // FUNCTION
                    return new ParserActionTransition(6);

                case 15:  // INTCONST
                    return new ParserActionTransition(7);

                case 17:  // VARIABLE
                    return new ParserActionTransition(8);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 2:
            switch (symbolID) {
                case 1:  // (
                    return new ParserActionTransition(4);

                case 16:  // FLOATCONST
                    return new ParserActionTransition(5);

                case 14:  // FUNCTION
                    return new ParserActionTransition(6);

                case 15:  // INTCONST
                    return new ParserActionTransition(7);

                case 17:  // VARIABLE
                    return new ParserActionTransition(8);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 3:
            switch (symbolID) {
                case -1:  // $END$
                    return new ParserActionAccept();

                case 10:  // +
                    return new ParserActionTransition(17);

                case 9:  // -
                    return new ParserActionTransition(18);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 4:
            switch (symbolID) {
                case 10:  // +
                    return new ParserActionTransition(1);

                case 9:  // -
                    return new ParserActionTransition(2);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 5:
            switch (symbolID) {
                case -1:  // $END$
                case 13:  // ^
                case 10:  // +
                case 9:  // -
                case 11:  // *
                case 12:  // /
                case 2:  // )
                    return new ParserActionReduce(26);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 6:
            switch (symbolID) {
                case 1:  // (
                    return new ParserActionReduce(22);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 7:
            switch (symbolID) {
                case -1:  // $END$
                case 13:  // ^
                case 10:  // +
                case 9:  // -
                case 11:  // *
                case 12:  // /
                case 2:  // )
                    return new ParserActionReduce(25);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 8:
            switch (symbolID) {
                case -1:  // $END$
                case 13:  // ^
                case 10:  // +
                case 9:  // -
                case 11:  // *
                case 12:  // /
                case 2:  // )
                    return new ParserActionReduce(27);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 9:
            switch (symbolID) {
                case -1:  // $END$
                case 10:  // +
                case 9:  // -
                case 2:  // )
                    return new ParserActionReduce(12);

                case 11:  // *
                    return new ParserActionTransition(20);

                case 12:  // /
                    return new ParserActionTransition(21);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 10:
            switch (symbolID) {
                case -1:  // $END$
                case 13:  // ^
                case 10:  // +
                case 9:  // -
                case 11:  // *
                case 12:  // /
                case 2:  // )
                    return new ParserActionReduce(19);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 11:
            switch (symbolID) {
                case -1:  // $END$
                case 13:  // ^
                case 10:  // +
                case 9:  // -
                case 11:  // *
                case 12:  // /
                case 2:  // )
                    return new ParserActionReduce(23);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 12:
            switch (symbolID) {
                case -1:  // $END$
                case 10:  // +
                case 9:  // -
                case 11:  // *
                case 12:  // /
                case 2:  // )
                    return new ParserActionReduce(14);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 13:
            switch (symbolID) {
                case 1:  // (
                    return new ParserActionTransition(4);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 14:
            switch (symbolID) {
                case -1:  // $END$
                case 13:  // ^
                case 10:  // +
                case 9:  // -
                case 11:  // *
                case 12:  // /
                case 2:  // )
                    return new ParserActionReduce(24);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 15:
            switch (symbolID) {
                case -1:  // $END$
                case 10:  // +
                case 9:  // -
                case 11:  // *
                case 12:  // /
                case 2:  // )
                    return new ParserActionReduce(17);

                case 13:  // ^
                    return new ParserActionTransition(23);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 16:
            switch (symbolID) {
                case -1:  // $END$
                case 10:  // +
                case 9:  // -
                case 2:  // )
                    return new ParserActionReduce(13);

                case 11:  // *
                    return new ParserActionTransition(20);

                case 12:  // /
                    return new ParserActionTransition(21);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 17:
            switch (symbolID) {
                case 1:  // (
                    return new ParserActionTransition(4);

                case 16:  // FLOATCONST
                    return new ParserActionTransition(5);

                case 14:  // FUNCTION
                    return new ParserActionTransition(6);

                case 15:  // INTCONST
                    return new ParserActionTransition(7);

                case 17:  // VARIABLE
                    return new ParserActionTransition(8);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 18:
            switch (symbolID) {
                case 1:  // (
                    return new ParserActionTransition(4);

                case 16:  // FLOATCONST
                    return new ParserActionTransition(5);

                case 14:  // FUNCTION
                    return new ParserActionTransition(6);

                case 15:  // INTCONST
                    return new ParserActionTransition(7);

                case 17:  // VARIABLE
                    return new ParserActionTransition(8);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 19:
            switch (symbolID) {
                case 10:  // +
                    return new ParserActionTransition(17);

                case 9:  // -
                    return new ParserActionTransition(18);

                case 2:  // )
                    return new ParserActionTransition(26);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 20:
            switch (symbolID) {
                case 1:  // (
                    return new ParserActionTransition(4);

                case 16:  // FLOATCONST
                    return new ParserActionTransition(5);

                case 14:  // FUNCTION
                    return new ParserActionTransition(6);

                case 15:  // INTCONST
                    return new ParserActionTransition(7);

                case 17:  // VARIABLE
                    return new ParserActionTransition(8);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 21:
            switch (symbolID) {
                case 1:  // (
                    return new ParserActionTransition(4);

                case 16:  // FLOATCONST
                    return new ParserActionTransition(5);

                case 14:  // FUNCTION
                    return new ParserActionTransition(6);

                case 15:  // INTCONST
                    return new ParserActionTransition(7);

                case 17:  // VARIABLE
                    return new ParserActionTransition(8);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 22:
            switch (symbolID) {
                case -1:  // $END$
                case 13:  // ^
                case 10:  // +
                case 9:  // -
                case 11:  // *
                case 12:  // /
                case 2:  // )
                    return new ParserActionReduce(20);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 23:
            switch (symbolID) {
                case 1:  // (
                    return new ParserActionTransition(4);

                case 16:  // FLOATCONST
                    return new ParserActionTransition(5);

                case 14:  // FUNCTION
                    return new ParserActionTransition(6);

                case 15:  // INTCONST
                    return new ParserActionTransition(7);

                case 17:  // VARIABLE
                    return new ParserActionTransition(8);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 24:
            switch (symbolID) {
                case -1:  // $END$
                case 10:  // +
                case 9:  // -
                case 2:  // )
                    return new ParserActionReduce(10);

                case 11:  // *
                    return new ParserActionTransition(20);

                case 12:  // /
                    return new ParserActionTransition(21);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 25:
            switch (symbolID) {
                case -1:  // $END$
                case 10:  // +
                case 9:  // -
                case 2:  // )
                    return new ParserActionReduce(11);

                case 11:  // *
                    return new ParserActionTransition(20);

                case 12:  // /
                    return new ParserActionTransition(21);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 26:
            switch (symbolID) {
                case -1:  // $END$
                case 13:  // ^
                case 10:  // +
                case 9:  // -
                case 11:  // *
                case 12:  // /
                case 2:  // )
                    return new ParserActionReduce(21);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 27:
            switch (symbolID) {
                case -1:  // $END$
                case 10:  // +
                case 9:  // -
                case 11:  // *
                case 12:  // /
                case 2:  // )
                    return new ParserActionReduce(15);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 28:
            switch (symbolID) {
                case -1:  // $END$
                case 10:  // +
                case 9:  // -
                case 11:  // *
                case 12:  // /
                case 2:  // )
                    return new ParserActionReduce(16);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        case 29:
            switch (symbolID) {
                case -1:  // $END$
                case 10:  // +
                case 9:  // -
                case 11:  // *
                case 12:  // /
                case 2:  // )
                    return new ParserActionReduce(18);

                default:
                    return new ParserActionError();
            }   // switch (symbolID)
            break;

        default:
            return new ParserActionError();

    } // switch (stateID)

    return 0;
}

int PlotFunctionParser::gotoState(const int stateID, const int symbolID) const {
    switch (stateID) {
        case 0:
            switch (symbolID) {
                case 18:  // [term]
                    return 3;

                default:
                    return -1;
            }   // switch (symbolID)
            break;

        case 1:
            switch (symbolID) {
                case 27:  // [atom]
                    return 10;

                case 29:  // [bracket-term]
                    return 11;

                case 25:  // [factor]
                    return 12;

                case 28:  // [function]
                    return 13;

                case 30:  // [single-value]
                    return 14;

                case 26:  // [variable]
                    return 15;

                case 24:  // [addend]
                    return 9;

                default:
                    return -1;
            }   // switch (symbolID)
            break;

        case 2:
            switch (symbolID) {
                case 27:  // [atom]
                    return 10;

                case 29:  // [bracket-term]
                    return 11;

                case 25:  // [factor]
                    return 12;

                case 28:  // [function]
                    return 13;

                case 30:  // [single-value]
                    return 14;

                case 26:  // [variable]
                    return 15;

                case 24:  // [addend]
                    return 16;

                default:
                    return -1;
            }   // switch (symbolID)
            break;

        case 4:
            switch (symbolID) {
                case 18:  // [term]
                    return 19;

                default:
                    return -1;
            }   // switch (symbolID)
            break;

        case 13:
            switch (symbolID) {
                case 29:  // [bracket-term]
                    return 22;

                default:
                    return -1;
            }   // switch (symbolID)
            break;

        case 17:
            switch (symbolID) {
                case 27:  // [atom]
                    return 10;

                case 29:  // [bracket-term]
                    return 11;

                case 25:  // [factor]
                    return 12;

                case 28:  // [function]
                    return 13;

                case 30:  // [single-value]
                    return 14;

                case 26:  // [variable]
                    return 15;

                case 24:  // [addend]
                    return 24;

                default:
                    return -1;
            }   // switch (symbolID)
            break;

        case 18:
            switch (symbolID) {
                case 27:  // [atom]
                    return 10;

                case 29:  // [bracket-term]
                    return 11;

                case 25:  // [factor]
                    return 12;

                case 28:  // [function]
                    return 13;

                case 30:  // [single-value]
                    return 14;

                case 26:  // [variable]
                    return 15;

                case 24:  // [addend]
                    return 25;

                default:
                    return -1;
            }   // switch (symbolID)
            break;

        case 20:
            switch (symbolID) {
                case 27:  // [atom]
                    return 10;

                case 29:  // [bracket-term]
                    return 11;

                case 28:  // [function]
                    return 13;

                case 30:  // [single-value]
                    return 14;

                case 26:  // [variable]
                    return 15;

                case 25:  // [factor]
                    return 27;

                default:
                    return -1;
            }   // switch (symbolID)
            break;

        case 21:
            switch (symbolID) {
                case 27:  // [atom]
                    return 10;

                case 29:  // [bracket-term]
                    return 11;

                case 28:  // [function]
                    return 13;

                case 30:  // [single-value]
                    return 14;

                case 26:  // [variable]
                    return 15;

                case 25:  // [factor]
                    return 28;

                default:
                    return -1;
            }   // switch (symbolID)
            break;

        case 23:
            switch (symbolID) {
                case 27:  // [atom]
                    return 10;

                case 29:  // [bracket-term]
                    return 11;

                case 28:  // [function]
                    return 13;

                case 30:  // [single-value]
                    return 14;

                case 26:  // [variable]
                    return 15;

                case 25:  // [factor]
                    return 29;

                default:
                    return -1;
            }   // switch (symbolID)
            break;

    } // switch (stateID)

    return -1;
}

ProductionStub* PlotFunctionParser::findProduction(const int productionID) const {
    switch (productionID) {
        case 0:  // [$START$] ::= [term]
            {
                int bodyIDs[] = {18};
                return new ProductionStub(0, bodyIDs, 1);
            }
        case 1:  // [$START$] ::= [term-list]
            {
                int bodyIDs[] = {19};
                return new ProductionStub(0, bodyIDs, 1);
            }
        case 2:  // [term-list] ::= [termwithinterval]
            {
                int bodyIDs[] = {20};
                return new ProductionStub(19, bodyIDs, 1);
            }
        case 3:  // [term-list] ::= [termwithinterval] , [term-list]
            {
                int bodyIDs[] = {20, 6, 19};
                return new ProductionStub(19, bodyIDs, 3);
            }
        case 4:  // [termwithinterval] ::= [term] : [interval]
            {
                int bodyIDs[] = {18, 7, 21};
                return new ProductionStub(20, bodyIDs, 3);
            }
        case 5:  // [interval] ::= [leftrange] ; [rightrange]
            {
                int bodyIDs[] = {22, 8, 23};
                return new ProductionStub(21, bodyIDs, 3);
            }
        case 6:  // [leftrange] ::= ( [term]
            {
                int bodyIDs[] = {1, 18};
                return new ProductionStub(22, bodyIDs, 2);
            }
        case 7:  // [leftrange] ::= [ [term]
            {
                int bodyIDs[] = {3, 18};
                return new ProductionStub(22, bodyIDs, 2);
            }
        case 8:  // [rightrange] ::= [term] )
            {
                int bodyIDs[] = {18, 2};
                return new ProductionStub(23, bodyIDs, 2);
            }
        case 9:  // [rightrange] ::= [term] ]
            {
                int bodyIDs[] = {18, 4};
                return new ProductionStub(23, bodyIDs, 2);
            }
        case 10:  // [term] ::= [term] + [addend]
            {
                int bodyIDs[] = {18, 10, 24};
                return new ProductionStub(18, bodyIDs, 3);
            }
        case 11:  // [term] ::= [term] - [addend]
            {
                int bodyIDs[] = {18, 9, 24};
                return new ProductionStub(18, bodyIDs, 3);
            }
        case 12:  // [term] ::= + [addend]
            {
                int bodyIDs[] = {10, 24};
                return new ProductionStub(18, bodyIDs, 2);
            }
        case 13:  // [term] ::= - [addend]
            {
                int bodyIDs[] = {9, 24};
                return new ProductionStub(18, bodyIDs, 2);
            }
        case 14:  // [addend] ::= [factor]
            {
                int bodyIDs[] = {25};
                return new ProductionStub(24, bodyIDs, 1);
            }
        case 15:  // [addend] ::= [addend] * [factor]
            {
                int bodyIDs[] = {24, 11, 25};
                return new ProductionStub(24, bodyIDs, 3);
            }
        case 16:  // [addend] ::= [addend] / [factor]
            {
                int bodyIDs[] = {24, 12, 25};
                return new ProductionStub(24, bodyIDs, 3);
            }
        case 17:  // [factor] ::= [variable]
            {
                int bodyIDs[] = {26};
                return new ProductionStub(25, bodyIDs, 1);
            }
        case 18:  // [factor] ::= [variable] ^ [factor]
            {
                int bodyIDs[] = {26, 13, 25};
                return new ProductionStub(25, bodyIDs, 3);
            }
        case 19:  // [variable] ::= [atom]
            {
                int bodyIDs[] = {27};
                return new ProductionStub(26, bodyIDs, 1);
            }
        case 20:  // [variable] ::= [function] [bracket-term]
            {
                int bodyIDs[] = {28, 29};
                return new ProductionStub(26, bodyIDs, 2);
            }
        case 21:  // [bracket-term] ::= ( [term] )
            {
                int bodyIDs[] = {1, 18, 2};
                return new ProductionStub(29, bodyIDs, 3);
            }
        case 22:  // [function] ::= FUNCTION
            {
                int bodyIDs[] = {14};
                return new ProductionStub(28, bodyIDs, 1);
            }
        case 23:  // [atom] ::= [bracket-term]
            {
                int bodyIDs[] = {29};
                return new ProductionStub(27, bodyIDs, 1);
            }
        case 24:  // [atom] ::= [single-value]
            {
                int bodyIDs[] = {30};
                return new ProductionStub(27, bodyIDs, 1);
            }
        case 25:  // [single-value] ::= INTCONST
            {
                int bodyIDs[] = {15};
                return new ProductionStub(30, bodyIDs, 1);
            }
        case 26:  // [single-value] ::= FLOATCONST
            {
                int bodyIDs[] = {16};
                return new ProductionStub(30, bodyIDs, 1);
            }
        case 27:  // [single-value] ::= VARIABLE
            {
                int bodyIDs[] = {17};
                return new ProductionStub(30, bodyIDs, 1);
            }
    } // switch (productionID)

    return 0;
}

std::string PlotFunctionParser::symbolID2String(const int symbolID) const {
    switch (symbolID) {
        case 0: return "[$START$]";
        case 1: return "(";
        case 2: return ")";
        case 3: return "[";
        case 4: return "]";
        case 5: return ".";
        case 6: return ",";
        case 7: return ":";
        case 8: return ";";
        case 9: return "-";
        case 10: return "+";
        case 11: return "*";
        case 12: return "/";
        case 13: return "^";
        case 14: return "FUNCTION";
        case 15: return "INTCONST";
        case 16: return "FLOATCONST";
        case 17: return "VARIABLE";
        case 18: return "[term]";
        case 19: return "[term-list]";
        case 20: return "[termwithinterval]";
        case 21: return "[interval]";
        case 22: return "[leftrange]";
        case 23: return "[rightrange]";
        case 24: return "[addend]";
        case 25: return "[factor]";
        case 26: return "[variable]";
        case 27: return "[atom]";
        case 28: return "[function]";
        case 29: return "[bracket-term]";
        case 30: return "[single-value]";
        case 31: return "$END$";
    }  // switch (symbolID)
    return "";
}

}   // namespace glslparser

}   // namespace voreen
