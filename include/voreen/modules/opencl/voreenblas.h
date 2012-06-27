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

#ifndef VRN_VOREENBLAS_H
#define VRN_VOREENBLAS_H

#include "voreen/modules/base/processors/volume/volumeprocessor.h"
#include "voreen/core/utils/clwrapper.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/boolproperty.h"

#include <string>
#include <time.h>

namespace voreen {

// forward declaration
template<class T>
class EllMatrix;

class VoreenBlas {

public:

    /**
     * Preconditioner to be used by
     * the conjugate gradient matrix solver.
     */
    enum Preconditioner {
        NoPreconditioner,
        Jacobi
    };

    VoreenBlas();
    virtual ~VoreenBlas();

    virtual void initialize() throw(VoreenException);
    bool isInitialized() const;

    void sAXPY_CL(size_t vecSize, float* vecx, float* vecy, float alpha, float* result);
    void sAXPY_CPU(size_t vecSize, float* vecx, float* vecy, float alpha, float* result);

    float sDOT_CL(size_t vecSize, float* vecx, float* vecy);
    float sDOT_CPU(size_t vecSize, float* vecx, float* vecy);

    float sNRM2_CL(size_t vecSize, float* vecx);
    float sNRM2_CPU(size_t vecSize, float* vecx);

    void sSpMVEll_CL(const EllMatrix<float>& mat, const float* vec, float* result);
    void sSpMVEll_CPU(const EllMatrix<float>& mat, const float* vec, float* result);

    void hSpMVEll_CL(const EllMatrix<int16_t>& mat, const float* vec, float* result);
    void hSpMVEll_CPU(const EllMatrix<int16_t>& mat, const float* vec, float* result);

    float sSpInnerProductEll_CL(const EllMatrix<float>& mat, const float* vecx, const float* vecy);
    float sSpInnerProductEll_CPU(const EllMatrix<float>& mat, const float* vecx, const float* vecy);

    int sSpConjGradEll_CL(const EllMatrix<float>& mat, const float* vec, float* result,
        float* initial = 0, Preconditioner precond = NoPreconditioner, float threshold = 1e-4f, int maxIterations = 1000);
    int sSpConjGradEll_CPU(const EllMatrix<float>& mat, const float* vec, float* result,
        float* initial = 0, Preconditioner precond = NoPreconditioner, float threshold = 1e-4f, int maxIterations = 1000);

    int hSpConjGradEll_CL(const EllMatrix<int16_t>& mat, const float* vec, float* result,
        float* initial = 0, float threshold = 1e-4f, int maxIterations = 1000);
    int hSpConjGradEll_CPU(const EllMatrix<int16_t>& mat, const float* vec, float* result,
        float* initial = 0, float threshold = 1e-4f, int maxIterations = 1000);

    bool runTests();

private:

    bool testSAXPY();
    bool testSDOT();
    bool testSNRM2();
    bool testSSpMVEll();
    bool testHSpMVEll();
    bool testSSpInnerProductEll();
    bool testSSpConjGradEll();
    bool testHSpConjGradEll();

    void randomizeEll(EllMatrix<float>& mat);
    void randomizeEll(EllMatrix<int16_t>& mat);

    void randomizeEllPositiveDefinite(EllMatrix<float>& mat);
    void randomizeEllPositiveDefinite(EllMatrix<int16_t>& mat);

    bool cmpVectors(size_t vecSize, float* vecx, float* vecy, float relThresh = 1e6);

    void clearCL();

    bool initialized_;

    cl::OpenCL* opencl_;
    cl::Context* context_;
    cl::Device* device_;
    cl::CommandQueue* queue_;
    cl::Program* prog_;

    static const std::string loggerCat_; ///< category used in logging
};

template<class T>
class EllMatrix {
public:
    EllMatrix(size_t numRows, size_t numCols, size_t numColsPerRow);
    ~EllMatrix();

    void setValue(size_t row, size_t col, T value);
    T getValue(size_t row, size_t col) const;

    void setValueByIndex(size_t row, size_t col, size_t colIndex, T value);
    T getValueByIndex(size_t row, size_t colIndex) const;

    size_t getColumn(size_t row, size_t colIndex) const;
    int getColumnIndex(size_t row, size_t col) const;
    size_t getNumRowEntries(size_t row) const;

    T* getMatrix() const;
    size_t* getIndices() const;

    size_t getNumRows() const;
    size_t getNumCols() const;
    size_t getNumColsPerRow() const;

    size_t getMatrixBufferSize() const;
    size_t getIndicesBufferSize() const;

    bool isQuadratic() const;
    bool isSymmetric() const;

    std::string toString() const;

private:
    inline size_t internalIndex(size_t row, size_t col) const;
    size_t getNextFreeColIndex(size_t row) const;

    size_t numRows_;
    size_t numCols_;
    size_t numColsPerRow_;
    bool columnOrder_;

    T* M_;
    size_t* indices_;
};

} //namespace


// ------------------------------------------------------------------------------
// template definitions
template<class T>
voreen::EllMatrix<T>::EllMatrix(size_t numRows, size_t numCols, size_t numColsPerRow) :
    numRows_(numRows),
    numCols_(numCols),
    numColsPerRow_(numColsPerRow),
    columnOrder_(true)
{
    tgtAssert(numRows > 0 && numCols > 0, "Invalid matrix dimensions");
    tgtAssert(numColsPerRow > 0 && numColsPerRow <= numCols, "Invalid number of cols per row");

    M_ = new T[numRows*numColsPerRow];
    indices_ = new size_t[numRows*numColsPerRow];

    for (size_t i=0; i<numRows*numColsPerRow; ++i) {
        M_[i] = static_cast<T>(0);
        indices_[i] = 0;
    }
}

template<class T>
voreen::EllMatrix<T>::~EllMatrix() {
    delete[] M_;
    delete[] indices_;
}

template<class T>
void voreen::EllMatrix<T>::setValue(size_t row, size_t col, T value) {

    tgtAssert(row < numRows_ && col < numCols_, "Invalid indices");

    // value already set for these coordinates?
    int colIndex = getColumnIndex(row, col);

    // if no value for indices assigned, get next free col index
    if (colIndex == -1)
        colIndex = getNextFreeColIndex(row);

    if (colIndex >= 0) {
        // next free col index available
        size_t index = internalIndex(row, colIndex);
        M_[index] = value;
        indices_[index] = col;
    }
    else {
        // row is full
        LWARNINGC("EllMatrix", "Too many entries for row " << row);
    }
}

template<class T>
T voreen::EllMatrix<T>::getValue(size_t row, size_t col) const {

    tgtAssert(row < numRows_ && col < numCols_, "Invalid indices");

    int colIndex = getColumnIndex(row, col);
    if (colIndex >= 0) {
        size_t index = internalIndex(row, colIndex);
        return M_[index];
    }
    else {
        // No entry for matrix element
        return static_cast<T>(0);
    }
}

template<class T>
size_t voreen::EllMatrix<T>::getColumn(size_t row, size_t colIndex) const {
    tgtAssert(row < numRows_, "Invalid row");
    tgtAssert(colIndex < numColsPerRow_, "Invalid col index");
    return indices_[internalIndex(row, colIndex)];
}

template<class T>
int voreen::EllMatrix<T>::getColumnIndex(size_t row, size_t col) const {
    tgtAssert(row < numRows_ && col < numCols_, "Invalid indices");
    for (size_t i=0; i<numColsPerRow_; ++i) {
        if (indices_[internalIndex(row, i)] == col)
            return i;
    }
    return -1;
}

template<class T>
size_t voreen::EllMatrix<T>::getNumRowEntries(size_t row) const {
    tgtAssert(row >= 0 && row < numRows_, "Invalid row");
    size_t entries = 0;
    for (size_t i=0; i<numColsPerRow_; i++) {
        if (getValueByIndex(row, i) != static_cast<T>(0))
            entries++;
    }
    return entries;
}

template<class T>
T voreen::EllMatrix<T>::getValueByIndex(size_t row, size_t colIndex) const {
    tgtAssert(row < numRows_, "Invalid row");
    tgtAssert(colIndex < numColsPerRow_, "Invalid col index");
    return M_[internalIndex(row, colIndex)];
}

template<class T>
void voreen::EllMatrix<T>::setValueByIndex(size_t row, size_t col, size_t colIndex, T value) {
    tgtAssert(row < numRows_ && col < numCols_, "Invalid indices");
    tgtAssert(colIndex < numColsPerRow_, "Invalid col index");
    M_[row*numColsPerRow_ + colIndex] = value;
    indices_[internalIndex(row, colIndex)] = col;
}

template<class T>
T* voreen::EllMatrix<T>::getMatrix() const {
    return M_;
}

template<class T>
size_t* voreen::EllMatrix<T>::getIndices() const {
    return indices_;
}

template<class T>
size_t voreen::EllMatrix<T>::getNumRows() const {
    return numRows_;
}

template<class T>
size_t voreen::EllMatrix<T>::getNumCols() const {
    return numCols_;
}

template<class T>
size_t voreen::EllMatrix<T>::getNumColsPerRow() const {
    return numColsPerRow_;
}

template<class T>
size_t voreen::EllMatrix<T>::getMatrixBufferSize() const {
    return (sizeof(T) * numRows_ * numColsPerRow_);
}

template<class T>
size_t voreen::EllMatrix<T>::getIndicesBufferSize() const {
    return (sizeof(size_t) * numRows_ * numColsPerRow_);
}

template<class T>
bool voreen::EllMatrix<T>::isQuadratic() const {
    return (numRows_ == numCols_);
}

template<class T>
bool voreen::EllMatrix<T>::isSymmetric() const {
    if (!isQuadratic())
        return false;

    for (size_t row=0; row < numRows_; row++) {
        for (size_t colIndex=0; colIndex<getNumColsPerRow(); colIndex++) {
            size_t col = getColumn(row, colIndex);
            if (getValue(row, col) != getValue(col, row))
                return false;
        }
    }

    return true;
}

template<class T>
std::string voreen::EllMatrix<T>::toString() const {
    std::ostringstream stream;
    for (size_t row=0; row < numRows_; row++) {
        stream << "Row " << row << ": ";
        for (size_t colIndex=0; colIndex < numColsPerRow_; colIndex++) {
            float value = getValueByIndex(row, colIndex);
            if (value != static_cast<T>(0))
                stream << getColumn(row, colIndex) << "=" << value << " ";
        }
        stream << "\n";
    }
    return stream.str();
}

template<class T>
size_t voreen::EllMatrix<T>::internalIndex(size_t row, size_t col) const {
    tgtAssert(row < numRows_ && col < numColsPerRow_, "Invalid indices");
    return (columnOrder_ ? col*numRows_ + row : row*numColsPerRow_ + col);
}

template<class T>
size_t voreen::EllMatrix<T>::getNextFreeColIndex(size_t row) const {
    tgtAssert(row < numRows_ , "Invalid row");
    for (size_t i=0; i<numColsPerRow_; ++i) {
        if (static_cast<T>(M_[internalIndex(row, i)]) == static_cast<T>(0)) {
            return i;
        }
    }
    return -1;
}

#endif
