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

#include "tgt/tgt_math.h"
#include <time.h>
#include "shrot.h"

namespace voreen {

SHRot::SHRot(int numBands) {
    numBands_ = numBands;
    numCoeffs_ = numBands_*numBands_;

    facs_ = std::vector<float>(2*numBands_);
    facs_[0] = 1.f;
    facs_[1] = 1.f;
    for(size_t i = 2; i < facs_.size(); i++)
        facs_[i] = i*facs_[i-1];
    dySubDiag_ = std::vector<float>();
    dydyDiag_ = std::vector<float>();
    initDiagonals(numBands_);
}

std::vector<float> SHRot::calcNewSHCoeffs(const std::vector<float>& oldCoeffs, const tgt::mat4& diffRot) {

    std::vector<float> wholesA = std::vector<float>(numBands_, 0.f);
    for(int l = 1; l < numBands_; l++) {
        for(int m = -l; m <= l; m++) {
            int index = l*(l+1)+m;
            wholesA.at(l) += oldCoeffs.at(index)*oldCoeffs.at(index);
        }
        wholesA.at(l) = sqrtf(wholesA.at(l));
    }

    // convert to euler angles
    float alpha, beta, gamma;
    alpha = atan2(diffRot.t12, diffRot.t02);
    beta  = atan2(sqrt(diffRot.t02*diffRot.t02 + diffRot.t12*diffRot.t12), diffRot.t22);
    gamma = atan2(diffRot.t21, -diffRot.t20);
    std::vector<float> res = shRotZ(shRotYdiff15(shRotZ(oldCoeffs, alpha), beta), gamma);

    std::vector<float> wholesB = std::vector<float>(numBands_, 0.f);
    for(int l = 1; l < numBands_; l++) {
        for(int m = -l; m <= l; m++) {
            int index = l*(l+1)+m;
            wholesB.at(l) += res.at(index)*res.at(index);
        }
        wholesB.at(l) = sqrtf(wholesB.at(l));
    }

    for(int l = 1; l < numBands_; l++) {
        for(int m = -l; m <= l; m++) {
            int index = l*(l+1)+m;
            res.at(index) = (res.at(index) / wholesB.at(l)) * wholesA.at(l);
        }
    }

    return res;
}

std::vector<float> SHRot::shRotYdiff15(const std::vector<float>& coeffs, float angle) {
    float angSq = 0.5f*angle*angle;
    std::vector<float> res = std::vector<float>(coeffs.size());
    res[0] = coeffs[0];
    for(size_t i=1; i < coeffs.size() - 1; i++) {
        res[i] = coeffs[i] * (1.f + angSq*dydyDiag_[i]) +
            angle * (dySubDiag_[i]*coeffs[i-1] - dySubDiag_[i+1]*coeffs[i+1]);
    }
    size_t last = coeffs.size()-1;
    res[last] = coeffs[last] * (1.f + angSq*dydyDiag_[last]) + angle * dySubDiag_[last] * coeffs[last-1];
    return res;
}

std::vector<float> SHRot::shRotZ(const std::vector<float>& coeffs, float angle) {
    std::vector<float> res = std::vector<float>(coeffs.size(), 0.f);
    for(int l = 0; l < numBands_; l++) {
        res[l*(l+1)] = coeffs[l*(l+1)];
        for(int m = 1; m <= l; m++) {
            res[l*(l+1)-m] = coeffs[l*(l+1)-m]*cosf((float)m*angle) - coeffs[l*(l+1)+m]*sinf((float)m*angle);
            res[l*(l+1)+m] = coeffs[l*(l+1)-m]*sinf((float)m*angle) + coeffs[l*(l+1)+m]*cosf((float)m*angle);
        }
    }
    return res;
}

void SHRot::initDiagonals(int maxgrad) {
    dySubDiag_.push_back(0.f);
    dydyDiag_.push_back(0.f);
    for(int l = 1; l <= maxgrad; l++) {
        for(int help = -l; help <= l-1; help++) {
            dySubDiag_.push_back(Ry(1, l, help+1, help));
        }
        for(int help2 = -l; help2 <= l; help2++) {
            dydyDiag_.push_back(Ry(2, l, help2, help2));
        }
    }
}

float SHRot::Ry(const int k, const int l, const int m, const int n) {
    if(((l==0 && m==0 && n==0) || (l==1 && m==-1 && n==-1)) && k == 0)
        return 1.f;
    else if((l==0 && m==0 && n==0) || (l==1 && m==-1 && n==-1))
        return 0.f;
    else if(l==1 && ((m==-1 && n==0) || (m==-1 && n==1) || (m==0 && n==-1) || (m==1 && n==-1)))
        return 0.f;
    else if(l==1 && ((m==0 && n==0) || (m==1 && n==1)))
        return 1.f - (float)k; // ONLY FOR k <= 2!!!!!
    else if(l==1 && ((m==0 && n==1)))
        return -(float)(k%2); // ONLY FOR k <= 2!!!!!
    else if(l==1 && ((m==1 && n==0)))
        return (float)(k%2); // ONLY FOR k <= 2!!!!!
    else
        return u(l,m,n) * dU(k,l,m,n) +
               v(l,m,n) * dV(k,l,m,n) +
               w(l,m,n) * dW(k,l,m,n);
}

float SHRot::u(const int l, const int m, const int n) {
    float divisor;
    if(abs(n) < l)
        divisor = (float)((l+n)*(l-n));
    else
        divisor = (float)((2*l)*(2*l-1));

    return sqrtf(((float)((l+m)*(l-m))) / divisor);
}

float SHRot::v(const int l, const int m, const int n) {
    float divisor;
    if(abs(n) < l)
        divisor = (float)((l+n)*(l-n));
    else
        divisor = (float)((2*l)*(2*l-1));

    float divident = (1.f + kronDelta(m, 0))*((float)(l+abs(m))-1.f)*((float)(l+abs(m)));
    return 0.5f*sqrtf(divident / divisor)*(1.f - 2.f*kronDelta(m, 0));
}

float SHRot::w(const int l, const int m, const int n) {
    float divisor;
    if(abs(n) < l)
        divisor = (float)((l+n)*(l-n));
    else
        divisor = (float)((2*l)*(2*l-1));

    float divident = ((float)(l-abs(m))-1.f)*((float)(l-abs(m)));
    return -0.5f*sqrtf(divident / divisor)*(1.f - kronDelta(m, 0));
}

float SHRot::dU(const int k, const int l, const int m, const int n) {
    if(abs(m) == l)
        return 0.f;
    else
        return dP(k, l, m, n, 0);
}

float SHRot::dV(const int k, const int l, const int m, const int n) {
    if(m > 1)
        return dP(k, l, m-1, n, 1) - dP(k, l, -m+1, n, -1);
    else if(m == 1)
        return sqrtf(2.f)*dP(k, l, 0, n, 1);
    else if(m == 0)
        return dP(k, l, 1, n, 1) + dP(k, l, -1, n, -1);
    else if(m == -1)
        return sqrtf(2.f)*dP(k, l, 0, n, -1);
    else
        return dP(k, l, -m-1, n, -1) + dP(k, l, m+1, n, 1);
}

float SHRot::dW(const int k, const int l, const int m, const int n) {
    if(abs(m) == l || abs(m) == l-1)
        return 0.f;
    if(m > 0)
        return dP(k, l, m+1, n, 1) + dP(k, l, -m-1, n, -1);
    else
        return dP(k, l, m-1, n, 1) - dP(k, l, -m+1, n, -1);
}

float SHRot::dP(const int k, const int l, const int m, const int n, const int i) {
    if(abs(n) < l)
        return dT(k, 1, i, 0, l-1, m, n);
    else if(n == l)
        return dT(k, 1, i, 1, l-1, m, l-1) - dT(k, 1, i, -1, l-1, m, -l+1);
    else
        return dT(k, 1, i, 1, l-1, m, -l+1) + dT(k, 1, i, -1, l-1, m, l-1);
}

float SHRot::dT(const int k, const int l1, const int m1, const int n1, const int l2, const int m2, const int n2) {
    float result = 0.f;
    for(int i = 0; i <= k; i++) {
        result += binCoeff(k, i) * Ry(i, l1, m1, n1) * Ry(k-i, l2, m2, n2);
    }
    return result;
}

} // namespace

