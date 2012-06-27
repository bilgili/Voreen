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

#ifndef VRN_FUNCTIONLIBRARY_H
#define VRN_FUNCTIONLIBRARY_H

#include "voreen/core/plotting/plotbase.h"

#include "tgt/logmanager.h"

namespace voreen {

class VoreenException;
class Expression;

class FunctionLibrary {
public:

    enum ProcessorFunctionalityType {
        NONE = 0,
        SELECT = 1,
        COLUMNORDER = 2,
        FILTER = 3,
        GROUPBY = 4,
    };

    enum RegressionType {
        LINEAR = 10,
        POLYNOMIAL = 20,
        SQUARE = 22,
        CUBIC = 23,
        SIMPLESPLINE = 30,
        BSPLINE = 32,
        CUBICSPLINE = 34,
        LOGARITHMIC = 40,
        POWER = 50,
        EXPONENTIAL = 60,
        SIN = 70,
        COS = 80,
    };

    FunctionLibrary();

    ~FunctionLibrary();

    static std::pair<plot_t,plot_t> linearRegression(const std::vector<std::pair<plot_t,plot_t> >& points)throw (VoreenException);

    static std::pair<Expression,plot_t> fittingLinear(const std::vector<std::pair<plot_t,plot_t> >& points);

    static std::pair<Expression,plot_t> fittingMulti(int dimension,const std::vector<std::pair<plot_t,plot_t> >& points);

    static std::pair<Expression,plot_t> fittingSquare(const std::vector<std::pair<plot_t,plot_t> >& points);

    static std::pair<Expression,plot_t> fittingCubic(const std::vector<std::pair<plot_t,plot_t> >& points);

    static Expression simpleSpline(const std::vector<std::pair<plot_t,plot_t> >& points);

    static Expression bSpline(const std::vector<std::pair<plot_t,plot_t> >& points);

    static Expression cubicSpline(const std::vector<std::pair<plot_t,plot_t> >& points);

    static std::pair<Expression,plot_t> fittingLogarithmus(const std::vector<std::pair<plot_t,plot_t> >& points);

    static std::pair<Expression,plot_t> fittingPower(const std::vector<std::pair<plot_t,plot_t> >& points);

    static std::pair<Expression,plot_t> fittingExponential(const std::vector<std::pair<plot_t,plot_t> >& points);

    static std::pair<Expression,plot_t> fittingSin(const std::vector<std::pair<plot_t,plot_t> >& points);

    static std::pair<Expression,plot_t> fittingCos(const std::vector<std::pair<plot_t,plot_t> >& points);

    static plot_t fittingInterpol(const std::vector<std::pair<plot_t,plot_t> >& points, plot_t x);


    /// for generator the parsertables not for normal use
    static void createPlotFunctionParser();


private:

    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_FUNCTIONLIBRARY_H
