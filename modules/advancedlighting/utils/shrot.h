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

#ifndef VRN_SHROT_H
#define VRN_SHROT_H

#include "tgt/matrix.h"

#include <vector>

namespace voreen {

class SHRot {

public:
    /**
     * Constructor.
     */
    SHRot(int numBands);
    ~SHRot() {}

    std::vector<float> calcNewSHCoeffs(const std::vector<float>& oldCoeffs, const tgt::mat4& diffRot);

    const std::vector<float>& getDiag() const {
        return dydyDiag_;
    }

    const std::vector<float>& getSubDiag() const {
        return dySubDiag_;
    }

private:
    int numBands_;
    int numCoeffs_;
    std::vector<float> facs_;

    std::vector<float> dySubDiag_;
    std::vector<float> dydyDiag_;

    float kronDelta(int m, int n) const {
        return (m == n) ? 1.0f : 0.0f;
    }

    float binCoeff(int k, int i) const {
        return facs_[k] / (facs_[i]*facs_[k-i]);
    }

    void initDiagonals(int maxgrad);
    std::vector<float> shRotZ(const std::vector<float>& coeffs, float angle);
    std::vector<float> shRotYdiff15(const std::vector<float>& coeffs, float angle);

    float Ry(const int k, const int l, const int m, const int n);

    float u(const int l, const int m, const int n);
    float v(const int l, const int m, const int n);
    float w(const int l, const int m, const int n);

    float dU(const int k, const int l, const int m, const int n);
    float dV(const int k, const int l, const int m, const int n);
    float dW(const int k, const int l, const int m, const int n);

    float dP(const int k, const int l, const int m, const int n, const int i);
    float dT(const int k, const int l1, const int m1, const int n1, const int l2, const int m2, const int n2);
};

} // namespace

#endif //VRN_SHROT_H
