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

#include "voreen/core/plotting/plotfunction.h"
#include "voreen/core/plotting/functionlibrary.h"
#include "voreen/core/plotting/plotdata.h"



namespace voreen {

PlotFunction::PlotFunction()
    : PlotBase(0,0)
    , expr_(Expression())
{
}

PlotFunction::PlotFunction(const std::string& expressionString)
    : PlotBase(0,0)
    , expr_(Expression(expressionString))
{
    PlotBase::reset(expr_.numberOfVariables(),1);
    for (int i = 0; i < expr_.numberOfVariables(); ++i) {
        setColumnLabel(i,expr_.getVariable(i));
    }
    setColumnLabel(expr_.numberOfVariables(),expr_.toString());
}


PlotFunction::PlotFunction(const Expression& expr)
    :PlotBase(expr.numberOfVariables(),1)
    , expr_(expr)
{
    for (int i = 0; i < expr.numberOfVariables(); ++i) {
        setColumnLabel(i,expr.getVariable(i));
    }
    setColumnLabel(expr.numberOfVariables(),expr.toString());
}

PlotFunction::PlotFunction(const PlotFunction& rhs)
    : PlotBase(rhs)
    , expr_(rhs.expr_)
{
}

PlotFunction::~PlotFunction() {
}

PlotFunction& PlotFunction::operator=(PlotFunction rhs) {
    PlotBase::operator=(rhs);
    std::swap(expr_, rhs.expr_);
    return *this;
}


plot_t PlotFunction::evaluateAt(const std::vector<plot_t>& value) {
    return expr_.evaluateAt(value);
}

std::vector<std::vector<plot_t> > PlotFunction::evaluateAt(const std::vector<Interval<plot_t> >& interval,const std::vector<plot_t>& step) {
    std::vector<std::vector<plot_t> > result;
    result.clear();
    if (interval.size() != step.size() || (expr_.numberOfVariables() != static_cast<int>(step.size())))
        return result;
    std::vector<plot_t> x;
    for (size_t j = 0; j < interval.size(); ++j) {
        if (interval.at(j).getLeftOpen()) {
            x.push_back(interval.at(j).getLeft() + step.at(j));
        }
        else {
            x.push_back(interval.at(j).getLeft());
        }
    }
    plot_t y;
    std::vector<plot_t> partVector;
    for (size_t j = 0; j < interval.size(); ++j) {
        while (interval.at(j).contains(x.at(j))) {
            partVector.clear();
            y = evaluateAt(x);
            for (size_t i = 0; i < step.size(); ++i) {
                partVector.push_back(x[i]);
            }
            partVector.push_back(y);
            result.push_back(partVector);
            x[j] += step.at(j);
        }
    }
    return result;
}


bool PlotFunction::select(const std::vector<Interval<plot_t> >& interval, const std::vector<plot_t>& step, PlotData& target) {
    if (interval.size() != step.size() || (expr_.numberOfVariables() != static_cast<int>(step.size())))
        return false;
    std::vector<std::vector<plot_t> > result = evaluateAt(interval,step);
    target.reset(expr_.numberOfVariables(),1);
    for (size_t i = 0; i < result.size(); ++i) {
        target.insert(result.at(i));
    }
    for (int i = 0; i < target.getColumnCount(); ++i) {
        if (i < expr_.numberOfVariables()) {
            target.setColumnLabel(i,getColumnLabel(i));
        }
        else {
            target.setColumnLabel(i,expr_.toString());
        }
    }
    return true;
}

bool PlotFunction::select(const Interval<plot_t>& interval, const plot_t& step, PlotData& target) {
    std::vector<Interval<plot_t> > intervalVector;
    intervalVector.push_back(interval);
    std::vector<plot_t> stepVector;
    stepVector.push_back(step);
    return select(intervalVector, stepVector,target);
}

bool PlotFunction::select(const std::vector<std::pair<plot_t,plot_t> >& interval, const std::vector<plot_t>& step, PlotData& target) {
    std::vector<Interval<plot_t> > intervals;
    Interval<plot_t> intV;
    for (size_t i = 0; i < interval.size(); ++i) {
        if (interval.at(i).first <= interval.at(i).second) {
            intV = Interval<plot_t>(interval.at(i).first,interval.at(i).second);
        }
        else {
            intV = Interval<plot_t>(interval.at(i).second,interval.at(i).first);
        }
        intervals.push_back(intV);
    }
    return select(intervals,step,target);
}

bool PlotFunction::select(Interval<plot_t>* interval, plot_t* step, int count, PlotData& target) {
    std::vector<Interval<plot_t> > intervals;
    std::vector<plot_t> steps;
    for (int i = 0; i < count; ++i) {
        intervals.push_back(interval[i]);
        steps.push_back(step[i]);
    }
    return select(intervals,steps,target);
}

/*
PlotData* PlotFunction::select(Interval<plot_t>* interval, plot_t* step, int count) {

    //check that count equals the number of variables in expr_
    if (count != expr_.numberOfVariables())
        throw "Wrong number of Variables.";

    //number of key columns equals count, one data column
    PlotData* data = new PlotData(count, 1);

    //the function is of type R->R
    if (count == 1) {
        for (plot_t x = (interval[0].getLeftOpen() ? interval[0].getLeft() + step[0] : interval[0].getLeft());
            interval[0].contains(x) ; x+= step[0]) {
            plot_t value[] = {x};
            plot_t row[] = {x, expr_.evaluateAt((value))};
            data->insert(row, 2);
        }
    }
    //the function is of typre RxR->R
    else if (count == 2) {
        for (plot_t x = (interval[0].getLeftOpen() ? interval[0].getLeft() + step[0] : interval[0].getLeft());
            (x < interval[0].getRight()) || (x == interval[0].getRight() && interval[0].getRightOpen()) ; x+= step[0]) {
            for (plot_t y = (interval[1].getLeftOpen() ? interval[1].getLeft() + step[1] : interval[1].getLeft());
                (y < interval[1].getRight()) || (y == interval[1].getRight() && interval[1].getRightOpen()) ; y+= step[1]) {
            plot_t value[] = {x, y};
            plot_t row[] = {x, y, expr_.evaluateAt((value))};
            data->insert(row, 3);
            }
        }
    }
    return data;

}
*/
}

