/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "voreen/core/utils/stringutils.h"

#include <algorithm>
#include <cstdio>
#include <iomanip>

namespace voreen {

std::string itos(int i, int stringLength /*= -1*/, char fillChar /*= '0'*/) {
    std::stringstream s;
    if(stringLength > 0)
        s << std::setw(stringLength) << std::setfill(fillChar) << i;
    else
        s << i;
    return s.str();
}

std::string itos(uint32_t i, int stringLength /*= -1*/, char fillChar /*= '0'*/) {
    std::stringstream s;
    if(stringLength > 0)
        s << std::setw(stringLength) << std::setfill(fillChar) << i;
    else
        s << i;
    return s.str();
}

std::string itos(uint64_t i, int stringLength /*= -1*/, char fillChar /*= '0'*/) {
    std::stringstream s;
    if(stringLength > 0)
        s << std::setw(stringLength) << std::setfill(fillChar) << i;
    else
        s << i;
    return s.str();
}

std::string ftos(float f, int precision /* =-1*/) {
    char buffer[50];
    if (precision > -1)
        sprintf(buffer, "%.*f", precision, f);
    else
        sprintf(buffer, "%f", f);
    return std::string(buffer);
}

std::string dtos(double d, int precision /* = -1*/) {
    char buffer[50];
    if (precision > -1)
        sprintf(buffer, "%.*f", precision, d);
    else
        sprintf(buffer, "%f", d);
    return std::string(buffer);
}

#if !defined(_MSC_VER) || (_MSC_VER < 1600)

int stoi(const std::string& s) {
    int result = 0;
    std::istringstream isst;
    isst.str(s);
    isst >> result;
    return result;
}

float stof(const std::string& s) {
    float result = 0.f;
    std::istringstream isst;
    isst.str(s);
    isst >> result;
    return result;
}

double stod(const std::string& s) {
    double result = 0.0;
    std::istringstream isst;
    isst.str(s);
    isst >> result;
    return result;
}

#endif

char* strToChr(const std::string& s){
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

std::string strReplaceFirst(const std::string& str, const std::string& from, const std::string& to) {
    std::string strConv(str);
    size_t foundHere;
    if((foundHere = strConv.find(from)) != std::string::npos) {
        strConv.replace(foundHere, from.size(), to);
    }
    return strConv;
}

std::string strReplaceLast(const std::string& str, const std::string& from, const std::string& to) {
    std::string strConv(str);
    size_t foundHere;
    if((foundHere = strConv.rfind(from)) != std::string::npos) {
        strConv.replace(foundHere, from.size(), to);
    }
    return strConv;
}

std::string trim(std::string str, const std::string& charlist) {
    //trim from beginning:
    while(!str.empty()) {
        char curChar = str[0];

        if(charlist.find(curChar) != std::string::npos)
            str.erase(0, 1); //delete first character
        else
            break;
    }

    //trim from end:
    while(!str.empty()) {
        char curChar = str[str.size()-1];

        if(charlist.find(curChar) != std::string::npos)
            str.erase(str.size()-1, 1); //delete last character
        else
            break;
    }

    return str;
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::vector<std::string> strSplit(const std::string& str, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(str);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> strSplit(const std::string& str, const std::string& delim) {
    std::vector<std::string> result;
    if (delim.length() == 0 || str.length() == 0) {
        result.push_back(str);
        return result;
    }

    size_t pos = 0;
    while (pos != std::string::npos) {
        size_t last = pos;
        pos = str.find(delim, last);
        result.push_back(str.substr(last, pos-last));
        if (pos != std::string::npos)
            pos += delim.size();
    }
    return result;
}

bool endsWith(const std::string& input, const std::string& ending) {
    size_t li = input.length();
    size_t le = ending.length();
    if(li >= le) {
        for(size_t i=0; i<le; i++) {
            if(ending[i] != input[li - le + i])
                return false;
        }
        return true;
    }
    else
        return false;
}

bool startsWith(const std::string& input, const std::string& start) {
    size_t li = input.length();
    size_t ls = start.length();
    if(li >= ls) {
        for (size_t i=0; i<ls; i++) {
            if (start[i] != input[i])
                return false;
        }
        return true;
    }
    else
        return false;
}

} // namespace
