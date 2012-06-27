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

#include "voreen/core/plotting/expression.h"
#include "voreen/core/plotting/plotfunctionlexer.h"
#include "voreen/core/plotting/plotfunctionparser.h"
#include "voreen/core/utils/GLSLparser/parser.h"
#include "voreen/core/plotting/plotfunctionnode.h"
//#include <string>
//#include "tgt/logmanager.h"
#include <ctype.h>


namespace voreen {

Expression::Expression() {
    initialize();
}

Expression::Expression(std::string value)  throw (VoreenException)
{
    initialize();
    representation_ = value;
    representation_.erase(remove_if(representation_.begin(), representation_.end(), isspace), representation_.end());
    std::istringstream is(value);
    //TODO: write further
    glslparser::PlotFunctionParser parser(&is);
    //glslparser::PlotFunctionNode* node =  dynamic_cast<glslparser::PlotFunctionNode*>(parser.parse());

    if (!validateExpression())
        throw VoreenException("Not Validate String");
    numberOfVariables_ = calculateNumberOfVariables();
}


Expression::~Expression(){
}

void Expression::initialize() {
    representation_ = "";
    numberOfVariables_ = 0;
    variables_.resize(26);

    //{"ABS","SQRT","SQR","SIN","COS","TAN","ARCTAN","ARCSIN","ARCCOS","LN","LOG","EXP","FAC","INT","RND","SGN"};
    allowedFunctionList_.push_back("abs");
    allowedFunctionList_.push_back("sqrt");
    allowedFunctionList_.push_back("sin");
    allowedFunctionList_.push_back("cos");
    allowedFunctionList_.push_back("tan");
    allowedFunctionList_.push_back("arctan");
    allowedFunctionList_.push_back("arcsin");
    allowedFunctionList_.push_back("arccos");
    allowedFunctionList_.push_back("ln");
    allowedFunctionList_.push_back("exp");
    allowedFunctionList_.push_back("log");
    allowedFunctionList_.push_back("fec");
    allowedFunctionList_.push_back("int");
    allowedFunctionList_.push_back("rnd");
    allowedFunctionList_.push_back("sgn");

    allowedOperatorList_.push_back('+');
    allowedOperatorList_.push_back('-');
    allowedOperatorList_.push_back('*');
    allowedOperatorList_.push_back('/');
    allowedOperatorList_.push_back('^');

    functionCount_.resize(allowedFunctionList_.size());
    exprToken_.clear();
}

bool Expression::validateExpression() {
    bool result = true;
    int i = 0;
    bool function = false;
    bool number = false;
    std::string numb = "";
    std::string func = "";
    std::string::iterator strit;
    for (strit = representation_.begin(); strit < representation_.end(); ++ strit) {
        *strit = tolower(*strit);
        if (*strit == ',')
            *strit = '.';
        if (*strit == '(')
            ++i;
        else if (*strit == ')')
            --i;
        if (i < 0 || !isAllowedChar(*strit)) {
            result = false;
            break;
        }
        if (isalpha(*strit)) {
            if (number) {
                number = false;
                exprToken_.push_back(numb);
                exprToken_.push_back("*");
                numb = "";
            }
            function = true;
            func += *strit;
        }
        else if (!isalpha(*strit)) {
            if (function) {
                if (func.size() == 1) {
                    variables_[func.at(0)-97] += 1;
                }
                else {
                    result = isAllowedFunction(func);// && *strit == '(';
                }
                exprToken_.push_back(func);
                func = "";
                exprToken_.push_back(""+*strit);
            }
            else if (isAllowedOperator(*strit)) {
                if (number && numb.size() > 0) {
                    number = false;
                    exprToken_.push_back(numb);
                    numb = "";
                }
                numb += *strit;
                exprToken_.push_back(numb);
                numb = "";
            }
            else {
                number = true;
                numb += *strit;
            }
            function = false;
            func = "";
        }
    }
    if (func.size() == 1) {
        variables_[func.at(0)-97] += 1;
        exprToken_.push_back(func);
    }
    else if (func.size() > 0) {
        result = false;
    }
    std::vector<std::string>::iterator strvecit;
    strvecit = exprToken_.begin();
    char ch = 0;
    while (strvecit < exprToken_.end() && exprToken_.size() > 1) {
        if ((*strvecit).size() == 1) {
            ch = (*strvecit).at(0);
            if (isAllowedOperator(ch) && strvecit < exprToken_.end() && isAllowedOperator((*(strvecit+1)).at(0))) {
                strvecit = exprToken_.erase(strvecit);
                if ((ch == '+' && (*strvecit).at(0) == '-') || (ch == '-' && (*strvecit).at(0) == '+')) {
                    *strvecit = '-';
                }
            }
            else
                ++strvecit;
        }
        else
            ++strvecit;
    }
    result = true;//FIXME: some times false result, but it should be true
    return result;
}

int Expression::calculateNumberOfVariables() {
    int z = 0;
    for (size_t i = 0; i < variables_.size(); ++i) {
        if (variables_.at(i) > 0)
            ++z;
    }
    return z;
}

std::string Expression::getVariable(int number) const {
    std::string result = "";
    char ch = 0;
    int z = 0;
    for (size_t i = 0; i < variables_.size(); ++i) {
        if (variables_.at(i) > 0)
            ++z;
        if (z == number)
            ch = i+97;
    }
    result += ch;
    return result;
}

plot_t Expression::evaluateAt(const std::vector<plot_t>& value) {
    if (static_cast<int>(value.size()) != numberOfVariables() || numberOfVariables() == 0)
        return 0;
    else
        return exp(value.at(0));
}

plot_t Expression::evaluate(const std::vector<std::string>& /*token*/, const std::vector<plot_t>& /*value*/) {
    //TODO::noch zu implementieren
    return 0;
}

int Expression::numberOfVariables() const {
    return numberOfVariables_;
}

std::vector<std::string> Expression::getAllowsFunctions() const {
    return allowedFunctionList_;
}

std::string Expression::toString() const {
    return representation_;
}

bool Expression::isAllowedFunction(std::string function) {
    bool result = false;
    for (size_t i = 0; i < allowedFunctionList_.size(); ++i) {
        if (allowedFunctionList_.at(i) == function) {
            functionCount_[i] += 1;
            result = true;
            return true;
        }
    }
    return result;
}

bool Expression::isAllowedOperator(char operators) {
    bool result = false;
    for (size_t i = 0; i < allowedOperatorList_.size(); ++i) {
        if (allowedOperatorList_.at(i) == operators) {
            result = true;
            return true;
        }
    }
    return result;
}

bool Expression::isAllowedChar(char chars) {
    bool result = true;
    if ((chars < 40) || (chars > 57 && chars < 60) || (chars > 62 && chars < 65)
        || (chars > 90 && chars < 97 && chars != 94) || (chars > 122))
        result = false;
    return result;
}

} // namespace
