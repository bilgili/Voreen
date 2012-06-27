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

#include "voreen/core/plotting/plotfunctiongrammar.h"

#include "voreen/core/plotting/plotfunctionterminals.h"

namespace voreen {

namespace glslparser {

PlotFunctionGrammar::PlotFunctionGrammar()
    : setup_(false)
{
    setup(PlotFunctionTerminals());
}

void PlotFunctionGrammar::setup(const TerminalsMap& terminals) {
    if (setup_)
        return;

    setup_ = true;
    Grammar::setup(terminals);
    addTerminal("$START$", PlotFunctionTerminals::ID_START, GrammarSymbol::SYMBOL_START);

    addProduction("$START$", "term");
    addProduction("$START$", "term-list");

    addProduction("term-list", "termwithinterval");
    addProduction("term-list", "termwithinterval , term-list");

    addProduction("termwithinterval", "term : interval");

    addProduction("interval", "leftrange ; rightrange");

    addProduction("leftrange","( term");
    addProduction("leftrange","[ term");

    addProduction("rightrange","term )");
    addProduction("rightrange","term ]");


    addProduction("term", "term + addend");
    addProduction("term", "term - addend");
    addProduction("term", "+ addend");
    addProduction("term", "- addend");

    addProduction("addend", "factor");
    addProduction("addend", "addend * factor");
    addProduction("addend", "addend / factor");

    addProduction("factor", "variable");
    addProduction("factor", "variable ^ factor");

    addProduction("variable", "atom");
    addProduction("variable", "function bracket-term");

    addProduction("bracket-term", "( term )");

    addProduction("function", "FUNCTION");


    addProduction("atom","bracket-term");
    addProduction("atom", "single-value");
    addProduction("single-value", "INTCONST");
    addProduction("single-value", "FLOATCONST");
    addProduction("single-value", "VARIABLE");
}

}   // namespace glslparser

}   // namespace voreen
