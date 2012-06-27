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

#include "voreen/core/plotting/functionlibrary.h"
#include "voreen/core/plotting/expression.h"
#include "tgt/logmanager.h"

// please do not delete read last lines!
#include "voreen/core/plotting/plotfunctiongrammar.h"
#include "voreen/core/utils/GLSLparser/generator/parsertable.h"

#include <fstream>

namespace voreen {

const std::string FunctionLibrary::loggerCat_("voreen.plotting.FunctionLibrary");

FunctionLibrary::FunctionLibrary()
{
}

FunctionLibrary::~FunctionLibrary(){
}

std::pair<plot_t,plot_t> FunctionLibrary::linearRegression(const std::vector<std::pair<plot_t,plot_t> >& points) throw (VoreenException) {
    std::vector<plot_t> result;
    if (points.size() <= 0)
        throw VoreenException ("Nothing to Fit");

    plot_t sumx = 0;
    for (size_t i = 0; i < points.size(); ++i) {
        if (points[i].first == points[i].first)
            sumx += points[i].first;
    }
    plot_t xMid = sumx/points.size();
    plot_t sumy = 0;

    for (size_t i = 0; i < points.size(); ++i) {
        if (points[i].second == points[i].second)
            sumy += points[i].second;
    }
    plot_t yMid = sumy/points.size();
    plot_t b = 0;
    plot_t numerator = 0;
    plot_t denominator = 0;
    plot_t diff1;
    plot_t diff2;
    for (size_t i = 0; i< points.size(); ++i) {
        diff1 = points[i].first - xMid;
        diff2 = points[i].second - yMid ;
        numerator += (diff1 * diff2);
        denominator += (diff1 * diff1);
    }
    b = numerator / denominator;
    plot_t a = yMid - ( b * xMid);

    return std::pair<plot_t,plot_t>(a,b);
}

std::pair<Expression,plot_t> FunctionLibrary::fittingLinear(const std::vector<std::pair<plot_t,plot_t> >& points) {
    std::pair<plot_t,plot_t> result =  FunctionLibrary::linearRegression(points);
    std::stringstream Str("");
    Str.imbue(std::locale::classic());
    Str << result.first << " + " << result.second << "*x";
    plot_t MSE = 0;
    for (size_t i = 0; i < points.size(); ++i) {
        MSE += pow(result.first + result.second*points.at(i).first-points.at(i).second,2);
    }
    MSE = MSE/points.size();
    return std::pair<Expression,plot_t>(Expression(Str.str()),MSE);
}

std::pair<Expression,plot_t> FunctionLibrary::fittingSquare(const std::vector<std::pair<plot_t,plot_t> >& points) {
    std::vector<std::pair<plot_t,plot_t> > logPoints;
    for (size_t i = 0; i < points.size(); ++i) {
        logPoints.push_back(std::pair<plot_t,plot_t>(pow(points.at(i).first,2),points.at(i).second));
    }
    std::pair<plot_t,plot_t> result = FunctionLibrary::linearRegression(logPoints);
    std::stringstream Str("");
    Str.imbue(std::locale::classic());
    Str << result.first << " + " << result.second << "*x^2";
    plot_t MSE = 0;
    for (size_t i = 0; i < points.size(); ++i) {
        MSE += pow(result.first + result.second*pow(points.at(i).first,2)-points.at(i).second,2);
    }
    MSE = MSE/points.size();
    return std::pair<Expression,plot_t>(Expression(Str.str()),MSE);
}

std::pair<Expression,plot_t> FunctionLibrary::fittingCubic(const std::vector<std::pair<plot_t,plot_t> >& points) {
    std::vector<std::pair<plot_t,plot_t> > logPoints;
    for (size_t i = 0; i < points.size(); ++i) {
        logPoints.push_back(std::pair<plot_t,plot_t>(pow(points.at(i).first,3),points.at(i).second));
    }
    std::pair<plot_t,plot_t> result = FunctionLibrary::linearRegression(logPoints);
    std::stringstream Str("");
    Str.imbue(std::locale::classic());
    Str << result.first << " + " << result.second << "*x^3";
    plot_t MSE = 0;
    for (size_t i = 0; i < points.size(); ++i) {
        MSE += pow(result.first + result.second*pow(points.at(i).first,3)-points.at(i).second,2);
    }
    MSE = MSE/points.size();
    return std::pair<Expression,plot_t>(Expression(Str.str()),MSE);
}
std::pair<Expression,plot_t> FunctionLibrary::fittingMulti(int dimension,const std::vector<std::pair<plot_t,plot_t> >& points) {
    std::vector<std::pair<plot_t,plot_t> > logPoints;
    for (size_t i = 0; i < points.size(); ++i) {
        logPoints.push_back(std::pair<plot_t,plot_t>(pow(points.at(i).first,dimension),points.at(i).second));
    }
    std::pair<plot_t,plot_t> result = FunctionLibrary::linearRegression(logPoints);
    std::stringstream Str("");
    Str.imbue(std::locale::classic());
    Str << result.first << " + " << result.second << "*x^" << dimension;
    plot_t MSE = 0;
    for (size_t i = 0; i < points.size(); ++i) {
        MSE += pow(result.first + result.second*pow(points.at(i).first,dimension)-points.at(i).second,2);
    }
    MSE = MSE/points.size();
    return std::pair<Expression,plot_t>(Expression(Str.str()),MSE);
}

Expression FunctionLibrary::simpleSpline(const std::vector<std::pair<plot_t,plot_t> >& points) {

    std::stringstream Str("");
    Str.imbue(std::locale::classic());
    //Str << result.first << " + " << result.second << "*x^" ;
    plot_t m;
    plot_t b;
    for (size_t i=0; i<points.size()-1; ++i) {
        m = (points.at(i+1).second - points.at(i).second) / (points.at(i+1).first - points.at(i).first);
        b = points.at(i).second - m*points.at(i).first;
        Str << m << " * x + " << b << ":[" << points.at(i).first << ";" << points.at(i+1).first << "];";
    }
    std::string expr = Str.str();
    return Expression(expr);
}
Expression FunctionLibrary::bSpline(const std::vector<std::pair<plot_t,plot_t> >& points) {

    std::stringstream Str("");
    Str.imbue(std::locale::classic());
    //Str << result.first << " + " << result.second << "*x^" ;
    plot_t m = 0.0;
    plot_t b;
    plot_t k;
    for (size_t i=0; i<points.size()-1; ++i) {
    std::vector<plot_t> z;
      z.resize(points.size()+1);
      z[i+1] = -z[i] + 2 * (m);
        m = (points.at(i+1).second - points.at(i).second) / (points.at(i+1).first - points.at(i).first);
        b = points.at(i).second - m*points.at(i).first;
        k = (z[i+1] - z[i]) / (2*(points.at(i+1).first - points.at(i).first));
        Str << points.at(i).second << " + " << z[i] << " * (x - " << points.at(i).first << ") + " << k << " * ( x-" << pow(points.at(i).first,2);
    }
    std::string expr = Str.str();
    return Expression(expr);
}

Expression FunctionLibrary::cubicSpline(const std::vector<std::pair<plot_t,plot_t> >& points) {

    std::stringstream Str("");
    Str.imbue(std::locale::classic());
    //Str << result.first << " + " << result.second << "*x^" ;
    plot_t m;
    plot_t b;
    for (size_t i=0; i<points.size()-1; ++i) {
        m = (points.at(i+1).second - points.at(i).second) / (points.at(i+1).first - points.at(i).first);
        b = points.at(i).second - m*points.at(i).first;
        Str << m << " * x + " << b << ":[" << points.at(i).first << ";" << points.at(i+1).first << "];";
    }
    std::string expr = Str.str();
    return Expression(expr);
}

std::pair<Expression,plot_t> FunctionLibrary::fittingLogarithmus(const std::vector<std::pair<plot_t,plot_t> >& points) {
    std::vector<std::pair<plot_t,plot_t> > logPoints;
    for (size_t i = 0; i < points.size(); ++i) {
        logPoints.push_back(std::pair<plot_t,plot_t>(log(points.at(i).first)/ log(10.0f),points.at(i).second));
    }
    std::pair<plot_t,plot_t> result = FunctionLibrary::linearRegression(logPoints);
    std::stringstream Str("");
    Str.imbue(std::locale::classic());
    Str << result.first << " + " << result.second << "*log(x)";
    plot_t MSE = 0;
    for (size_t i = 0; i < points.size(); ++i) {
        MSE += pow(result.first + result.second*log(points.at(i).first)/ log(10.0f)-points.at(i).second,2);
    }
    MSE = MSE/points.size();
    return std::pair<Expression,plot_t>(Expression(Str.str()),MSE);
}

std::pair<Expression,plot_t> FunctionLibrary::fittingPower(const std::vector<std::pair<plot_t,plot_t> >& points) {
    std::vector<std::pair<plot_t,plot_t> > logPoints;
    for (size_t i = 0; i < points.size(); ++i) {
        logPoints.push_back(std::pair<plot_t,plot_t>(log(points.at(i).first)/ log(10.0f),log(points.at(i).second)/ log(10.0f)));
    }
    std::pair<plot_t,plot_t> result = FunctionLibrary::linearRegression(logPoints);
    std::stringstream Str("");
    Str.imbue(std::locale::classic());
    Str << exp(result.first) << " * x^" << result.second;
    plot_t MSE = 0;
    for (size_t i = 0; i < points.size(); ++i) {
        MSE += pow(exp(result.first) * exp(result.second*(log(points.at(i).first)/ log(10.0f)))-points.at(i).second,2);
    }
    MSE = MSE/points.size();
    return std::pair<Expression,plot_t>(Expression(Str.str()),MSE);
}

std::pair<Expression,plot_t> FunctionLibrary::fittingExponential(const std::vector<std::pair<plot_t,plot_t> >& points) {
    std::vector<std::pair<plot_t,plot_t> > logPoints;
    for (size_t i = 0; i < points.size(); ++i) {
        logPoints.push_back(std::pair<plot_t,plot_t>(points.at(i).first,log(points.at(i).second)/log(10.0f)));
    }
    std::pair<plot_t,plot_t> result = FunctionLibrary::linearRegression(logPoints);
    std::stringstream Str("");
    Str.imbue(std::locale::classic());
    Str << exp(result.first) << " * " << result.second << "^x";
    plot_t MSE = 0;
    for (size_t i = 0; i < points.size(); ++i) {
        MSE += pow(result.first + result.second*points.at(i).first-points.at(i).second,2);
    }
    MSE = MSE/points.size();
    return std::pair<Expression,plot_t>(Expression(Str.str()),MSE);
}

std::pair<Expression,plot_t> FunctionLibrary::fittingSin(const std::vector<std::pair<plot_t,plot_t> >& points) {
    std::vector<std::pair<plot_t,plot_t> > logPoints;
    for (size_t i = 0; i < points.size(); ++i) {
        logPoints.push_back(std::pair<plot_t,plot_t>(points.at(i).first,points.at(i).second));
    }
    std::pair<plot_t,plot_t> result = FunctionLibrary::linearRegression(logPoints);
    std::stringstream Str("");
    Str.imbue(std::locale::classic());
    Str << result.first << " + " << asin(result.second) << "*x";
    plot_t MSE = 0;
    for (size_t i = 0; i < points.size(); ++i) {
        MSE += pow(result.first + asin(result.second)*points.at(i).first-points.at(i).second,2);
    }
    MSE = MSE/points.size();
    return std::pair<Expression,plot_t>(Expression(Str.str()),MSE);
}

std::pair<Expression,plot_t> FunctionLibrary::fittingCos(const std::vector<std::pair<plot_t,plot_t> >& points) {
    std::vector<std::pair<plot_t,plot_t> > logPoints;
    for (size_t i = 0; i < points.size(); ++i) {
        logPoints.push_back(std::pair<plot_t,plot_t>(points.at(i).first,points.at(i).second));
    }
    std::pair<plot_t,plot_t> result = FunctionLibrary::linearRegression(logPoints);
    std::stringstream Str("");
    Str.imbue(std::locale::classic());
    Str << result.first << " + " << acos(result.second) << "*x";
    plot_t MSE = 0;
    for (size_t i = 0; i < points.size(); ++i) {
        MSE += pow(result.first + acos(result.second)*points.at(i).first-points.at(i).second,2);
    }
    MSE = MSE/points.size();
    return std::pair<Expression,plot_t>(Expression(Str.str()),MSE);
}

plot_t FunctionLibrary::fittingInterpol(const std::vector<std::pair<plot_t,plot_t> >& interpolPoints, plot_t x) {
    plot_t mult;
    plot_t result = 0;
    for ( size_t i=0; i<interpolPoints.size(); ++i) {
        mult = 1;
        for( size_t k=0; k<interpolPoints.size(); ++k) {
            if (k != i) {
            mult *= (x-interpolPoints.at(k).first) / (interpolPoints.at(i).first - interpolPoints.at(k).first);
        }
        result += mult * interpolPoints.at(i).second;
    }
        }
    return result;

}






//PlotFittingFunctionChomsky:: PlotFittingFunctionChomsky const {
// int i, j, k, n, m;
//
//
//double A[n][m+1];
//double L[n][m+1];
//
// // Einträge der Matrix A eingeben:
//    for (i=0;i<n;i++)
//      for (j=0;j<m+1;j++)
//        {
//         cout <<"Bitte Eintrag A[" <<i+1 <<"," <<j+1 <<"] eingeben:";
//         cin >>A[i][j];
//        }
//
//       for (i=0;i<n;i++)
//        for (j=0;j<m+1;j++)
//          L[i][j]=A[i][j];
//
//    // Jetzt kommt der Algorithmus für die Cholesky-Zerlegung:
//     int p;
//   for ( k = 0; k < n; k++ )
//    {
//            for ( j = 0; j < m+1; j++ )
//            L[k][m+1] = A[k][m+1] - L[k][j-k+m+1]*L[k][j-k+m+1];
//        if ( L[k][m+1] <= 0 ) break;
//        L[k][m+1] = sqrt( L[k][m+1] );
//
//        p=min(k+m, n);
//        for ( i = k+1; i < p; i++ )
//        {
//            for ( j = 0; j < k; j++ )
//                L[i][k-i+m+1] =A[i][k-i+m+1]- L[i][j]*L[k-i+m+1][j];
//            L[i][k-i+m+1] =L[i][m+1]/ L[k][m+1];
//           L[k][i] = L[i][k];
//        }
//    }
//
//    cout <<"\n\n Die Cholesky-Zerlegung der Matrix ist: \n";
//
//     for (i=0; i<n; i++)
//     {
//        {
//         for (j=0; j<m+1; j++)
//          cout << L[i][j] <<"\t";
//        }
//        cout <<"\n" ;
//     }
//
//   system("pause");
//    return 0;
//
//}

//float cubic_interpolate( float y0, float y1, float y2, float y3, float mu ) {

//double a, b, diff;
//        unsigned int j = 1;
//
//        while ((m_x[j] < input) && (j < count))
//            j++;
//
//        diff = m_x[j] - m_x[j-1];
//
//        a = (m_x[j] - input) / diff;    //div should not be 0
//        b = (input - m_x[j-1]) / diff;
//
//        return (a*m_y[j-1] + b*m_y[j] + ((a*a*a - a)*m_der[j - 1] +
//               (b*b*b - b)*m_der[j])*(diff*diff) / 6);

//std::vector<std::vector<plot_t>> matrix;
//for( size_t x=0; x<matrix.size(); ++x){
//    for(size_t y=0; y<matrix.at(0).size(); ++y) {
//        for( size_t k=0; k<matrix.size(); ++k){
//
//        }
//
//    }
//}









/////////////////////////////////////FUNCTION FOR THE PARSERGENERATOR///////////////////
//
//       DO NOT DELETE THE LINE AFTER THIS UNTIL THE PARSER IS READY
//
////////////////////////////////////////////////////////////////////////////////////////

void FunctionLibrary::createPlotFunctionParser() {
    glslparser::PlotFunctionGrammar g;

    // Print the grammar
    //
    std::ofstream ofs("\\temp\\grammar.txt");
    ofs << "Terminals:\n";
    std::set<glslparser::GrammarSymbol*> terms = g.getTerminals();
    for (std::set<glslparser::GrammarSymbol*>::const_iterator it = terms.begin(); it != terms.end(); ++it)
        ofs << "ID " << (*it)->getSymbolID() << ":\t" << (*it)->toString() << "\n";

    ofs << "\nProductions:\n";
    std::vector<glslparser::Production> productions = g.getProductions();
    for (size_t i = 0; i < productions.size(); ++i)
        ofs << "ID  " << productions[i].getProductionID() << ":\t" << productions[i].toString() << "\n";
    ofs.close();

    glslparser::ParserTable* table = g.createParserTable(true);
    if (table != 0) {
        table->htmlOutput("\\temp\\table.html");

        std::ofstream ofs("\\temp\\code.cpp");
        table->generateActionCode(ofs, "stateID", "symbolID");
        ofs << "\n";
        table->generateGotoCode(ofs, "stateID", "symbolID");
        ofs << "\n";
        table->generateProductionsCode(ofs, "productionID");
        ofs << "\n";
        table->generateSymbolsCode(ofs, "symbolID");
        ofs.close();
    }
    delete table;
}




} // namespace voreen
