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
#include "voreen/core/plotting/parser/plotfunctiontoken.h"
#include "voreen/core/plotting/parser/plotfunctionvisitor.h"

namespace voreen {

class VoreenException;


/**
 * \brief This class contains an expression and is able to evaluate it.
 *
 */
class PlotExpression {
public:

    /// default constructor
    PlotExpression();

    /// constructor with Expression-String and Expression-Name
    PlotExpression(const std::string& value, const std::string& expressionName = "f") throw (VoreenException);

    /// destructor
    virtual ~PlotExpression();

    /**
     * \brief  Evaluates the expression.
     *
     * \param value    the value for which the expression will be evaluated
     *
     * \return         plot_t result of the evaluation
     *                 if the result defined result ist std::numeric_limits<plot_t>::quiet_NaN()
     */
    plot_t evaluateAt(const std::vector<plot_t>& value);

    /// returns the number of variables of the expression
    int numberOfVariables() const;
    /// gives back the variable as string which position you want.
    std::string getVariable(int number) const;

    /// gives back the domain of the function
    const std::vector<std::vector<Interval<plot_t> > >& getDomain() const;

    /// set the Expressionname if it is not empty
    void setExpressionName(const std::string& expressionName);
    /// returns the Expressionname
    const std::string& getExpressionName() const;

    /// returns the Stringrepresentation of the Expression
    std::string getExpressionString() const;

    /// returns stringdescription of this expression (e.g. f(x)=x^2)
    std::string toString() const;

private:

    struct VariableValues {
    public:
        int count;
        int numberOfVariable;
    };

    struct TokenFunction {
        std::vector<glslparser::Token*> function;
        std::vector<glslparser::Token*> interval;
        std::vector<Interval<plot_t> > domain;
    };


    void initialize();
    int calculateNumberOfVariables();
    void calculateDomain();
    void evaluateDomain(std::stack<glslparser::Token*>& tokens, int index, int& number);

    plot_t evaluate(std::stack<glslparser::Token*>& tokens, const std::vector<plot_t>& value);

    /// string represantion of the expression
    std::string representation_;
    std::string expressionName_;

    int numberOfVariables_;

    std::vector<VariableValues> variables_;

    /// domain of the function
    std::vector<std::vector<Interval<plot_t> > > domain_;
    std::vector<TokenFunction> functionVector_;

    static const int charOffset_;
    static const std::string loggerCat_;
};


}// namespace voreen


#endif // VRN_EXPRESSION_H
