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

#include "voreen/core/plotting/parser/plotfunctiontoken.h"


namespace voreen {
namespace glslparser {

//----------  FunctionToken ---------------------------------------------------
FunctionToken::FunctionToken(const int tokenID, const std::string& value, const int index)
    : IdentifierToken(tokenID,value)
    , index_(index)
{}

Token* FunctionToken::getCopy() const {
    return new FunctionToken(getTokenID(),getValue(),index_);
}

int FunctionToken::getIndex() {
    return index_;
}

//----------  VariablesToken ---------------------------------------------------

VariablesToken::VariablesToken(const int tokenID, const std::string& value)
    : IdentifierToken(tokenID,value)
{}

Token* VariablesToken::getCopy() const {
    return new VariablesToken(getTokenID(),getValue());
}

//----------  OperatorToken ---------------------------------------------------

OperatorToken::OperatorToken(const int tokenID, const char value, const int parameter)
    : GenericToken<char>(tokenID,value)
    , parameter_(parameter)
{}

Token* OperatorToken::getCopy() const {
    return new OperatorToken(getTokenID(),getValue(),parameter_);
}

void OperatorToken::setParameter(int parameter) {
    parameter_ = parameter;
}

int OperatorToken::getParameter() {
    return parameter_;
}


} // namespace glslparser

} // namespace voreen
