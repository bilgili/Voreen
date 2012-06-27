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

#include "voreen/core/utils/stringconversion.h"

#include <sstream>

namespace voreen {

std::string itos(int i) {
    std::stringstream s;
    s << i;
    return s.str();
}

int stoi(const std::string& s) {
    int result = 0;
    std::istringstream isst;
    isst.str(s);
    isst >> result;
    return result;
}

std::string ftos(float f) {
    std::stringstream s;
    s << f;
    return s.str();
}

float stof(const std::string& s) {
    float result = 0.f;
    std::istringstream isst;
    isst.str(s);
    isst >> result;
    return result;
}

std::string dtos(double d) {
    std::stringstream s;
    s << d;
    return s.str();
}

double stod(const std::string& s) {
    double result = 0.0;
    std::istringstream isst;
    isst.str(s);
    isst >> result;
    return result;
}

char* strtochr(const std::string& s){
    char* result = new char[s.size()+1];
#ifdef WIN32
#pragma warning(disable:4996)
#endif
    s.copy(result, s.size());
#ifdef WIN32
#pragma warning(default:4996)
#endif
    result[s.size()] = '\0';
    return result;
}

std::string strReplaceAll(const std::string& str, const std::string& from, const std::string& to) {
    std::string strConv(str);
    size_t lookHere = 0;
    size_t foundHere;
    while((foundHere = strConv.find(from, lookHere)) != std::string::npos) {
        strConv.replace(foundHere, from.size(), to);
        lookHere = foundHere + to.size();
    }
    return strConv;
}

} // namespace
