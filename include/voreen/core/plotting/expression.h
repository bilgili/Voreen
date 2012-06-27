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

#ifndef VRN_EXPRESSION_H
#define VRN_EXPRESSION_H

#include "voreen/core/plotting/interval.h"
#include "voreen/core/plotting/plotbase.h"

namespace voreen {

class VoreenException;

/**
 * \brief This class contains an expression and is able to evaluate it.
 *
 */
class Expression {
public:

    /// default constructor
    Expression();

    Expression(std::string value) throw (VoreenException);

    /// destructor
    virtual ~Expression();

    /**
     * \brief  Evaluates the expression.
     *
     * \param value    the value for which the expression will be evaluated
     *
     * \return         plot_t result of the evaluation
     */
    plot_t evaluateAt(const std::vector<plot_t>& value);

    /// returns the number of variables of the expression
    int numberOfVariables() const;

    bool validateExpression();

    std::string getVariable(int number) const;

    std::vector<std::string> getAllowsFunctions() const;

    std::string toString() const;


private:

    void initialize();
    int calculateNumberOfVariables();
    bool isAllowedFunction(std::string function);
    bool isAllowedOperator(char operators);
    bool isAllowedChar(char chars);

    plot_t evaluate(const std::vector<std::string>& token, const std::vector<plot_t>& value);

    /// string represantion of the expression
    std::string representation_;

    int numberOfVariables_;

    std::vector<int> variables_;
    std::vector<int> functionCount_;
    std::vector<std::string> exprToken_;
    //std::vector<glslparser::Token*> tokenvector;

    /// domain of the function
    Interval<plot_t>* domain_;

    std::vector<std::string> allowedFunctionList_;//{"ABS","SQRT","SQR","SIN","COS","TAN","ARCTAN","ARCSIN","ARCCOS","LN","LOG","EXP","FAC","INT","RND","SGN"};
    std::vector<char> allowedOperatorList_;
};


}// namespace voreen


#endif // VRN_EXPRESSION_H
