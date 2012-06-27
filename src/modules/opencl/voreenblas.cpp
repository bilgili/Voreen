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

#include "voreen/modules/opencl/voreenblas.h"

inline int volumeCoordsToIndex(int x, int y, int z, const tgt::ivec3& dim) {
    return z*dim.y*dim.x + y*dim.x + x;
}

namespace voreen {

using namespace cl;

const std::string VoreenBlas::loggerCat_("voreen.opencl.VoreenBlas");

VoreenBlas::VoreenBlas() :
    initialized_(false),
    opencl_(0),
    context_(0),
    device_(0),
    queue_(0),
    prog_(0)
{
}

VoreenBlas::~VoreenBlas() {
    clearCL();
}

void VoreenBlas::clearCL() {
    delete prog_;
    delete queue_;
    delete context_;
    delete opencl_;
    prog_ = 0;
    queue_ = 0;
    context_ = 0;
    device_ = 0;
    opencl_ = 0;
}

void VoreenBlas::initialize() throw(VoreenException) {
    //TODO: Central OpenCL init in voreen

    opencl_ = new OpenCL();

    const std::vector<Platform>& platforms = opencl_->getPlatforms();
    if (platforms.empty()) {
        LERROR("No OpenCL platforms found");
        clearCL();
        throw VoreenException("No OpenCL platforms found");
    }

    const std::vector<Device*>& devices = platforms[0].getDevices();
    if (devices.empty()) {
        LERROR("No devices in platform found");
        clearCL();
        throw VoreenException("No devices in platform found");
    }
    device_ = devices[0];

    context_ = new Context(Context::generateGlSharingProperties(), device_);
    queue_ = new CommandQueue(context_, devices.back());

    std::string kernelFile = "../../src/modules/opencl/voreenblas.cl";
    LINFO("Loading program: " << kernelFile);
    prog_ = new Program(context_);
    bool success = (prog_->loadSource(kernelFile) && prog_->build(devices.back()));
    if (!success) {
        clearCL();
        LERROR("Unable to load program: " << kernelFile);
        throw VoreenException("Unable to load program: " + kernelFile);
    }

    initialized_ = true;
}

bool VoreenBlas::isInitialized() const {
    return initialized_;
}

void VoreenBlas::sAXPY_CL(size_t vecSize, float* vecx, float* vecy, float alpha, float* result) {

    if (!initialized_) {
        LERROR("Not initialized. Aborting.");
        return;
    }

    Kernel* kernel = prog_->getKernel("sAXPY");
    if (!kernel) {
        LERROR("No kernel 'sAXPY' found");
        return;
    }

    int workSize = 1024 << 2;

    Buffer xBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(float)*vecSize, vecx);
    Buffer yBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(float)*vecSize, vecy);

    Buffer resultBuffer(context_, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(float)*vecSize);

    kernel->setArg(0, vecSize);
    kernel->setArg(1, xBuffer);
    kernel->setArg(2, yBuffer);
    kernel->setArg(3, alpha);
    kernel->setArg(4, resultBuffer);

    queue_->enqueue(kernel, workSize);

    queue_->enqueueRead(&resultBuffer, (void*)(result), true);
    queue_->finish();
}

void VoreenBlas::sAXPY_CPU(size_t vecSize, float* vecx, float* vecy, float alpha, float* result) {
    for (size_t i=0; i<vecSize; ++i) {
        result[i] = alpha*vecx[i] + vecy[i];
    }
}

float VoreenBlas::sDOT_CL(size_t vecSize, float* vecx, float* vecy) {

    if (!initialized_) {
        LERROR("Not initialized. Aborting.");
        return -1.f;
    }

    Kernel* kernel = prog_->getKernel("sDOT");
    if (!kernel) {
        LERROR("No kernel 'sDOT' found");
        return -1.f;
    }

    int workSize = 1024 << 2;
    int localWorkSize = std::min<int>(512, device_->getMaxWorkGroupSize());

    Buffer xBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(float)*vecSize, vecx);
    Buffer yBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(float)*vecSize, vecy);

    Buffer resultBuffer(context_, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(float));
    int mutex = 0;
    Buffer mutexBuffer(context_, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(int), &mutex);

    kernel->setArg(0, vecSize);
    kernel->setArg(1, xBuffer);
    kernel->setArg(2, yBuffer);
    kernel->setArg(3, resultBuffer);
    kernel->setArg(4, mutexBuffer);
    kernel->setArg(5, sizeof(float)*localWorkSize, 0);

    queue_->enqueue(kernel, workSize, localWorkSize);

    float result;
    queue_->enqueueRead(&resultBuffer, (void*)(&result), true);
    queue_->finish();

    return result;
}

float VoreenBlas::sDOT_CPU(size_t vecSize, float* vecx, float* vecy) {
    float result = 0.f;
    for (size_t i=0; i<vecSize; i++) {
        result += vecx[i]*vecy[i];
    }
    return result;
}


float VoreenBlas::sNRM2_CL(size_t vecSize, float* vecx) {

    if (!initialized_) {
        LERROR("Not initialized. Aborting.");
        return -1.f;
    }

    Kernel* kernel = prog_->getKernel("sNRM2");
    if (!kernel) {
        LERROR("No kernel 'sNRM2' found");
        return 0.f;
    }

    int workSize = 1024 << 2;
    int localWorkSize = std::min<int>(512, device_->getMaxWorkGroupSize());

    Buffer xBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(float)*vecSize, vecx);

    Buffer resultBuffer(context_, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(float));
    int mutex = 0;
    Buffer mutexBuffer(context_, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int), &mutex);
    Buffer semaphorBuffer(context_, CL_MEM_ALLOC_HOST_PTR, sizeof(int));

    kernel->setArg(0, vecSize);
    kernel->setArg(1, xBuffer);
    kernel->setArg(2, resultBuffer);
    kernel->setArg(3, mutexBuffer);
    kernel->setArg(4, semaphorBuffer);
    kernel->setArg(5, sizeof(float)*localWorkSize, 0);
    queue_->enqueue(kernel, workSize, localWorkSize);

    float result;
    queue_->enqueueRead(&resultBuffer, (void*)(&result), true);
    queue_->finish();

    return result;
}

float VoreenBlas::sNRM2_CPU(size_t vecSize, float* vecx) {
    float result = 0.f;
    for (size_t i=0; i<vecSize; i++) {
        result += vecx[i]*vecx[i];
    }
    return sqrt(result);
}

void VoreenBlas::sSpMVEll_CL(const EllMatrix<float>& mat, const float* vec, float* result) {

    if (!initialized_) {
        LERROR("Not initialized. Aborting.");
        return;
    }

    Kernel* kernel = prog_->getKernel("sSpMV_Ell");
    if (!kernel) {
        LERROR("No kernel 'sSpMV_Ell' found");
        return;
    }

    int workSize = 1024 << 2;

    Buffer matBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, mat.getMatrixBufferSize(), mat.getMatrix());
    Buffer indicesBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, mat.getIndicesBufferSize(), mat.getIndices());
    Buffer vecBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*mat.getNumCols(), const_cast<float*>(vec));
    Buffer resultBuffer(context_, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(float)*mat.getNumRows());

    kernel->setArg(0, mat.getNumRows());
    kernel->setArg(1, mat.getNumCols());
    kernel->setArg(2, mat.getNumColsPerRow());
    kernel->setArg(3, indicesBuffer);
    kernel->setArg(4, matBuffer);
    kernel->setArg(5, vecBuffer);
    kernel->setArg(6, resultBuffer);

    queue_->enqueue(kernel, workSize);

    queue_->enqueueRead(&resultBuffer, (void*)(result), true);
    queue_->finish();
}

void VoreenBlas::sSpMVEll_CPU(const EllMatrix<float>& mat, const float* vec, float* result) {

    for (size_t row=0; row < mat.getNumRows(); ++row) {
        result[row] = 0.f;
        for (size_t colIndex=0; colIndex < mat.getNumColsPerRow(); ++colIndex)
            result[row] += mat.getValueByIndex(row, colIndex) * vec[mat.getColumn(row, colIndex)];
    }
}

void VoreenBlas::hSpMVEll_CL(const EllMatrix<int16_t>& mat, const float* vec, float* result) {

    if (!initialized_) {
        LERROR("Not initialized. Aborting.");
        return;
    }

    Kernel* kernel = prog_->getKernel("hSpMV_Ell");
    if (!kernel) {
        LERROR("No kernel 'hSpMV_Ell' found");
        return;
    }

    int workSize = 1024 << 2;

    Buffer matBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, mat.getMatrixBufferSize(), mat.getMatrix());
    Buffer indicesBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, mat.getIndicesBufferSize(), mat.getIndices());
    Buffer vecBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*mat.getNumCols(), const_cast<float*>(vec));
    Buffer resultBuffer(context_, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(float)*mat.getNumRows());

    kernel->setArg(0, mat.getNumRows());
    kernel->setArg(1, mat.getNumCols());
    kernel->setArg(2, mat.getNumColsPerRow());
    kernel->setArg(3, indicesBuffer);
    kernel->setArg(4, matBuffer);
    kernel->setArg(5, vecBuffer);
    kernel->setArg(6, resultBuffer);

    queue_->enqueue(kernel, workSize);

    queue_->enqueueRead(&resultBuffer, (void*)(result), true);
    queue_->finish();
}

void VoreenBlas::hSpMVEll_CPU(const EllMatrix<int16_t>& mat, const float* vec, float* result) {

    float maxValue = static_cast<float>((1<<15) - 1);

    for (size_t row=0; row < mat.getNumRows(); ++row) {
        result[row] = 0.f;
        for (size_t colIndex=0; colIndex < mat.getNumColsPerRow(); ++colIndex)
            result[row] += (mat.getValueByIndex(row, colIndex) * vec[mat.getColumn(row, colIndex)]) / maxValue;
    }
}

float VoreenBlas::sSpInnerProductEll_CL(const EllMatrix<float>& mat, const float* vecx, const float* vecy) {

    if (!initialized_) {
        LERROR("Not initialized. Aborting.");
        return -1.f;
    }

    Kernel* kernel = prog_->getKernel("sInnerProduct_Ell");
    if (!kernel) {
        LERROR("No kernel 'sInnerProduct_Ell' found");
        return -1.f;
    }

    int workSize = 1024 << 2;
    int localWorkSize = std::min<int>(512, device_->getMaxWorkGroupSize());

    Buffer matBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, mat.getMatrixBufferSize(), mat.getMatrix());
    Buffer indicesBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, mat.getIndicesBufferSize(), mat.getIndices());
    Buffer vecxBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*mat.getNumCols(), const_cast<float*>(vecx));
    Buffer vecyBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*mat.getNumCols(), const_cast<float*>(vecy));
    int mutex = 0;
    Buffer mutexBuffer(context_, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(int), &mutex);
    Buffer resultBuffer(context_, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(float));

    kernel->setArg(0, mat.getNumRows());
    kernel->setArg(1, mat.getNumCols());
    kernel->setArg(2, mat.getNumColsPerRow());
    kernel->setArg(3, indicesBuffer);
    kernel->setArg(4, matBuffer);
    kernel->setArg(5, vecxBuffer);
    kernel->setArg(6, vecyBuffer);
    kernel->setArg(7, resultBuffer);
    kernel->setArg(8, mutexBuffer);
    kernel->setArg(9, sizeof(float)*localWorkSize, 0);

    queue_->enqueue(kernel, workSize);

    float result;
    queue_->enqueueRead(&resultBuffer, (void*)(&result), true);
    queue_->finish();

    return result;
}

float VoreenBlas::sSpInnerProductEll_CPU(const EllMatrix<float>& mat, const float* vecx, const float* vecy) {

    float result = 0.f;
    for (size_t row=0; row < mat.getNumRows(); ++row) {
        float dot = 0.f;
        for (size_t colIndex=0; colIndex < mat.getNumColsPerRow(); ++colIndex)
            dot += mat.getValueByIndex(row, colIndex) * vecy[mat.getColumn(row, colIndex)];
        result += dot*vecx[row];
    }
    return result;
}

int VoreenBlas::sSpConjGradEll_CL(const EllMatrix<float>& mat, const float* vec, float* result,
                                   float* initial, Preconditioner precond, float threshold, int maxIterations) {

    if (!mat.isSymmetric()) {
        LERROR("Symmetric matrix expected.");
        return -1;
    }

    if (!initialized_) {
        LERROR("Not initialized. Aborting.");
        return -1;
    }

    Kernel* kernelSapxy = prog_->getKernel("sAXPY");
    if (!kernelSapxy) {
        LERROR("No kernel 'sAXPY' found");
        return -1;
    }

    Kernel* kernelSdot = prog_->getKernel("sDOT");
    if (!kernelSdot) {
        LERROR("No kernel 'sDOT' found");
        return -1;
    }

    Kernel* kernelSpmv = prog_->getKernel("sSpMV_Ell");
    if (!kernelSpmv) {
        LERROR("No kernel 'sSpMV_Ell' found");
        return -1;
    }

    Kernel* kernelSnrm2 = prog_->getKernel("sNRM2");
    if (!kernelSnrm2) {
        LERROR("No kernel 'sNRM2' found");
        return -1;
    }

    size_t vecSize = mat.getNumRows();
    int workSize = 1024 << 2;
    int localWorksize = std::min<int>(512, device_->getMaxWorkGroupSize());

    if (mat.getNumRows() < (size_t)device_->getMaxWorkGroupSize())
        workSize = localWorksize = device_->getMaxWorkGroupSize();

    bool initialAllocated = false;
    if (initial == 0) {
        initial = new float[vecSize];
        initialAllocated = true;
        for (size_t i=0; i<vecSize; ++i)
            initial[i] = 0.f;
    }

    EllMatrix<float>* preconditioner = 0;
    Buffer* precondBuf = 0;
    Buffer* precondIndicesBuf = 0;
    Buffer* zBuf = 0;

    Buffer matBuf(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, mat.getMatrixBufferSize(), mat.getMatrix());
    Buffer indicesBuf(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, mat.getIndicesBufferSize(), mat.getIndices());

    Buffer tmpBuf(context_, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float)*vecSize, const_cast<float*>(vec));
    Buffer xBuf(context_, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float)*vecSize, initial);
    Buffer rBuf(context_, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(float)*vecSize);
    Buffer pBuf(context_, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(float)*vecSize);

    if (precond == Jacobi) {
        preconditioner = new EllMatrix<float>(mat.getNumRows(), mat.getNumRows(), 1);
        for (size_t i=0; i<mat.getNumRows(); i++)
            preconditioner->setValueByIndex(i, i, 0, 1.f / std::max(mat.getValue(i,i), 1e-6f));

        precondBuf = new Buffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            preconditioner->getMatrixBufferSize(), preconditioner->getMatrix());
        precondIndicesBuf = new Buffer(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, preconditioner->getIndicesBufferSize(), preconditioner->getIndices());

        zBuf = new Buffer(context_, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(float)*vecSize);
    }

    int mutex;
    Buffer mutexBuf(context_, CL_MEM_ALLOC_HOST_PTR, sizeof(int));
    Buffer scalarBuf(context_, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(float));
    Buffer nominatorBuf(context_, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(float));
    Buffer denominatorBuf(context_, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(float));
    Buffer alphaBuf(context_, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(float));

    // r <= A*x_0
    kernelSpmv->setArg(0, vecSize);
    kernelSpmv->setArg(1, vecSize);
    kernelSpmv->setArg(2, mat.getNumColsPerRow());
    kernelSpmv->setArg(3, indicesBuf);
    kernelSpmv->setArg(4, matBuf);
    kernelSpmv->setArg(5, xBuf);
    kernelSpmv->setArg(6, rBuf);
    queue_->enqueue(kernelSpmv, workSize);

    // p <= -r + b
    kernelSapxy->setArg(0, vecSize);
    kernelSapxy->setArg(1, rBuf);
    kernelSapxy->setArg(2, tmpBuf);
    kernelSapxy->setArg(3, -1.f);
    kernelSapxy->setArg(4, pBuf);
    queue_->enqueue(kernelSapxy, workSize);

    // r <= -r + b
    kernelSapxy->setArg(0, vecSize);
    kernelSapxy->setArg(1, rBuf);
    kernelSapxy->setArg(2, tmpBuf);
    kernelSapxy->setArg(3, -1.f);
    kernelSapxy->setArg(4, rBuf);
    queue_->enqueue(kernelSapxy, workSize);

    // preconditioning
    if (precond == Jacobi) {
        kernelSpmv->setArg(0, vecSize);
        kernelSpmv->setArg(1, vecSize);
        kernelSpmv->setArg(2, preconditioner->getNumColsPerRow());
        kernelSpmv->setArg(3, precondIndicesBuf);
        kernelSpmv->setArg(4, precondBuf);
        kernelSpmv->setArg(5, rBuf);
        kernelSpmv->setArg(6, zBuf);
        queue_->enqueue(kernelSpmv, workSize);

        //memcpy(pBuf, zBuf, vecSize * sizeof(float));
        kernelSapxy->setArg(0, vecSize);
        kernelSapxy->setArg(1, zBuf);
        kernelSapxy->setArg(2, zBuf);
        kernelSapxy->setArg(3, 0.f);
        kernelSapxy->setArg(4, pBuf);
        queue_->enqueue(kernelSapxy, workSize);
    }

    int iteration  = 0;

    while (iteration < maxIterations) {

        iteration++;

        // r_k^T*r_k
        kernelSdot->setArg(0, vecSize);
        kernelSdot->setArg(1, rBuf);
        if (precond == Jacobi)
            kernelSdot->setArg(2, zBuf);
        else
            kernelSdot->setArg(2, rBuf);
        kernelSdot->setArg(3, nominatorBuf);
        kernelSdot->setArg(4, mutexBuf);
        kernelSdot->setArg(5, sizeof(float)*localWorksize, 0);
        mutex = 0;
        queue_->enqueueWrite(&mutexBuf, &mutex, true);    //< initialize mutex
        queue_->enqueue(kernelSdot, workSize, localWorksize);

        // tmp <= A * p_k
        kernelSpmv->setArg(0, vecSize);
        kernelSpmv->setArg(1, vecSize);
        kernelSpmv->setArg(2, mat.getNumColsPerRow());
        kernelSpmv->setArg(3, indicesBuf);
        kernelSpmv->setArg(4, matBuf);
        kernelSpmv->setArg(5, pBuf);
        kernelSpmv->setArg(6, tmpBuf);
        queue_->enqueue(kernelSpmv, workSize);

        // dot(p_k^T, tmp)
        kernelSdot->setArg(0, vecSize);
        kernelSdot->setArg(1, pBuf);
        kernelSdot->setArg(2, tmpBuf);
        kernelSdot->setArg(3, denominatorBuf);
        kernelSdot->setArg(4, mutexBuf);
        kernelSdot->setArg(5, sizeof(float)*localWorksize, 0);
        mutex = 0;
        queue_->enqueueWrite(&mutexBuf, &mutex, true);
        queue_->enqueue(kernelSdot, workSize, localWorksize);

        float denominator;
        float nominator;
        queue_->enqueueRead(&nominatorBuf, (void*)(&nominator), true);
        queue_->enqueueRead(&denominatorBuf, (void*)(&denominator), true);
        float alpha = nominator / denominator;

        // x <= alpha*p + x
        kernelSapxy->setArg(0, vecSize);
        kernelSapxy->setArg(1, pBuf);
        kernelSapxy->setArg(2, xBuf);
        kernelSapxy->setArg(3, alpha);
        kernelSapxy->setArg(4, xBuf);
        queue_->enqueue(kernelSapxy, workSize);

        // r <= -alpha*tmp + r
        kernelSapxy->setArg(0, vecSize);
        kernelSapxy->setArg(1, tmpBuf);
        kernelSapxy->setArg(2, rBuf);
        kernelSapxy->setArg(3, -alpha);
        kernelSapxy->setArg(4, rBuf);
        queue_->enqueue(kernelSapxy, workSize);

        // norm(r_k+1)
        float beta;
        if (precond == Jacobi) {
            kernelSpmv->setArg(0, vecSize);
            kernelSpmv->setArg(1, vecSize);
            kernelSpmv->setArg(2, preconditioner->getNumColsPerRow());
            kernelSpmv->setArg(3, precondIndicesBuf);
            kernelSpmv->setArg(4, precondBuf);
            kernelSpmv->setArg(5, rBuf);
            kernelSpmv->setArg(6, zBuf);
            queue_->enqueue(kernelSpmv, workSize);

            kernelSdot->setArg(0, vecSize);
            kernelSdot->setArg(1, rBuf);
            kernelSdot->setArg(2, zBuf);
            kernelSdot->setArg(3, scalarBuf);
            kernelSdot->setArg(4, mutexBuf);
            kernelSdot->setArg(5, sizeof(float)*localWorksize, 0);
            mutex = 0;
            queue_->enqueueWrite(&mutexBuf, &mutex, true);
            queue_->enqueue(kernelSdot, workSize, localWorksize);
            queue_->enqueueRead(&scalarBuf, (void*)(&beta), true);
        }
        else {
            kernelSdot->setArg(0, vecSize);
            kernelSdot->setArg(1, rBuf);
            kernelSdot->setArg(2, rBuf);
            kernelSdot->setArg(3, scalarBuf);
            kernelSdot->setArg(4, mutexBuf);
            kernelSdot->setArg(5, sizeof(float)*localWorksize, 0);
            mutex = 0;
            queue_->enqueueWrite(&mutexBuf, &mutex, true);
            queue_->enqueue(kernelSdot, workSize, localWorksize);
            queue_->enqueueRead(&scalarBuf, (void*)(&beta), true);
        }

        if (sqrt(beta) < threshold)
            break;

        beta /= nominator;

        // p <= beta*p + r
        kernelSapxy->setArg(0, vecSize);
        kernelSapxy->setArg(1, pBuf);
        if (precond == Jacobi)
            kernelSapxy->setArg(2, zBuf);
        else
            kernelSapxy->setArg(2, rBuf);
        kernelSapxy->setArg(3, beta);
        kernelSapxy->setArg(4, pBuf);
        queue_->enqueue(kernelSapxy, workSize);
    }

    queue_->enqueueRead(&xBuf, (void*)(result), true);
    queue_->finish();

    if (initialAllocated)
        delete[] initial;

    delete preconditioner;
    delete precondBuf;
    delete precondIndicesBuf;
    delete zBuf;

    return iteration;
}

int VoreenBlas::sSpConjGradEll_CPU(const EllMatrix<float>& mat, const float* vec, float* result,
                                    float* initial, Preconditioner precond, float threshold, int maxIterations) {

    if (!mat.isSymmetric()) {
        LERROR("Symmetric matrix expected.");
        return -1;
    }

    size_t vecSize = mat.getNumRows();

    bool initialAllocated = false;
    if (initial == 0) {
        initial = new float[vecSize];
        initialAllocated = true;
        for (size_t i=0; i<vecSize; ++i)
            initial[i] = 0.f;
    }

    float* xBuf = result;
    memcpy(xBuf, initial, sizeof(float)*vecSize);

    float* tmpBuf = initial;
    memcpy(tmpBuf, vec, sizeof(float)*vecSize);

    float* rBuf = new float[vecSize];
    float* pBuf = new float[vecSize];

    float* zBuf = 0;

    EllMatrix<float>* preconditioner = 0;
    if (precond == Jacobi) {
        preconditioner = new EllMatrix<float>(mat.getNumRows(), mat.getNumRows(), 1);
        for (size_t i=0; i<mat.getNumRows(); i++)
            preconditioner->setValueByIndex(i, i, 0, 1.f / std::max(mat.getValue(i,i), 1e-6f));

        zBuf = new float[vecSize];
    }

    float nominator;
    float denominator;

    int iteration = 0;

    // r <= A*x_0
    sSpMVEll_CPU(mat, xBuf, rBuf);

    // p <= -r + b
    sAXPY_CPU(vecSize, rBuf, tmpBuf, -1.f, pBuf);

    // r <= -r + b
    sAXPY_CPU(vecSize, rBuf, tmpBuf, -1.f, rBuf);

    // preconditioning
    if (precond == Jacobi) {
        sSpMVEll_CPU(*preconditioner, rBuf, zBuf);
        memcpy(pBuf, zBuf, vecSize * sizeof(float));
    }

    while (iteration < maxIterations) {

        iteration++;

        // norm(r_k)
        nominator = sDOT_CPU(vecSize, rBuf, rBuf);

        if (precond == Jacobi)
            nominator = sDOT_CPU(vecSize, rBuf, zBuf);
        else
            nominator = sDOT_CPU(vecSize, rBuf, rBuf);

        // tmp <= A * p_k
        sSpMVEll_CPU(mat, pBuf, tmpBuf);

        // dot(p_k^T, tmp)
        denominator = sDOT_CPU(vecSize, pBuf, tmpBuf);

        float alpha = nominator / denominator;

        // x <= alpha*p + x
        sAXPY_CPU(vecSize, pBuf, xBuf, alpha, xBuf);

        // r <= -alpha*tmp + r
        sAXPY_CPU(vecSize, tmpBuf, rBuf, -alpha, rBuf);

        float beta;

        // norm(r_k+1)
        if (precond == Jacobi) {
            sSpMVEll_CPU(*preconditioner, rBuf, zBuf);
            beta = sDOT_CPU(vecSize, rBuf, zBuf);
        }
        else {
            beta = sDOT_CPU(vecSize, rBuf, rBuf);
        }

        if (sqrt(beta) < threshold)
            break;

        beta /= nominator;

        // p <= beta*p + r
        if (precond == Jacobi) {
            sAXPY_CPU(vecSize, pBuf, zBuf, beta, pBuf);
        }
        else {
            sAXPY_CPU(vecSize, pBuf, rBuf, beta, pBuf);
        }
    }

    delete[] rBuf;
    delete[] pBuf;
    delete[] zBuf;
    delete preconditioner;

    if (initialAllocated)
        delete[] initial;

    return iteration;
}

int VoreenBlas::hSpConjGradEll_CL(const EllMatrix<int16_t>& mat, const float* vec, float* result,
                                   float* initial, float threshold, int maxIterations) {

    if (!mat.isSymmetric()) {
        LERROR("Symmetric matrix expected.");
        return -1;
    }

    if (!initialized_) {
        LERROR("Not initialized. Aborting.");
        return -1;
    }

    Kernel* kernelSapxy = prog_->getKernel("sAXPY");
    if (!kernelSapxy) {
        LERROR("No kernel 'sAXPY' found");
        return -1;
    }

    Kernel* kernelSdot = prog_->getKernel("sDOT");
    if (!kernelSdot) {
        LERROR("No kernel 'sDOT' found");
        return -1;
    }

    Kernel* kernelSpmv = prog_->getKernel("hSpMV_Ell");
    if (!kernelSpmv) {
        LERROR("No kernel 'hSpMV_Ell' found");
        return -1;
    }

    size_t vecSize = mat.getNumRows();
    cl_uint workSize = 1024 << 2;
    int localWorksize = std::min<int>(512, device_->getMaxWorkGroupSize());

    if (mat.getNumRows() < device_->getMaxWorkGroupSize())
        workSize = localWorksize = device_->getMaxWorkGroupSize();

    bool initialAllocated = false;
    if (initial == 0) {
        initial = new float[vecSize];
        initialAllocated = true;
        for (size_t i=0; i<vecSize; ++i)
            initial[i] = 0.f;
    }

    Buffer matBuf(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, mat.getMatrixBufferSize(), mat.getMatrix());
    Buffer indicesBuf(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, mat.getIndicesBufferSize(), mat.getIndices());

    Buffer tmpBuf(context_, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float)*vecSize, const_cast<float*>(vec));
    Buffer xBuf(context_, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(float)*vecSize, initial);
    Buffer rBuf(context_, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(float)*vecSize);
    Buffer pBuf(context_, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(float)*vecSize);

    int mutex;
    Buffer mutexBuf(context_, CL_MEM_ALLOC_HOST_PTR, sizeof(int));
    Buffer scalarBuf(context_, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(float));
    Buffer nominatorBuf(context_, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(float));
    Buffer denominatorBuf(context_, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(float));
    Buffer alphaBuf(context_, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof(float));

    // r <= A*x_0
    kernelSpmv->setArg(0, vecSize);
    kernelSpmv->setArg(1, vecSize);
    kernelSpmv->setArg(2, mat.getNumColsPerRow());
    kernelSpmv->setArg(3, indicesBuf);
    kernelSpmv->setArg(4, matBuf);
    kernelSpmv->setArg(5, xBuf);
    kernelSpmv->setArg(6, rBuf);
    queue_->enqueue(kernelSpmv, workSize);

    // p <= -r + b
    kernelSapxy->setArg(0, vecSize);
    kernelSapxy->setArg(1, rBuf);
    kernelSapxy->setArg(2, tmpBuf);
    kernelSapxy->setArg(3, -1.f);
    kernelSapxy->setArg(4, pBuf);
    queue_->enqueue(kernelSapxy, workSize);

    // r <= -r + b
    kernelSapxy->setArg(0, vecSize);
    kernelSapxy->setArg(1, rBuf);
    kernelSapxy->setArg(2, tmpBuf);
    kernelSapxy->setArg(3, -1.f);
    kernelSapxy->setArg(4, rBuf);
    queue_->enqueue(kernelSapxy, workSize);

    int iteration  = 0;

    while (iteration < maxIterations) {

        iteration++;

        // r_k^T*r_k
        kernelSdot->setArg(0, vecSize);
        kernelSdot->setArg(1, rBuf);
        kernelSdot->setArg(2, rBuf);
        kernelSdot->setArg(3, nominatorBuf);
        kernelSdot->setArg(4, mutexBuf);
        kernelSdot->setArg(5, sizeof(float)*localWorksize, 0);
        mutex = 0;
        queue_->enqueueWrite(&mutexBuf, &mutex, true);    //< initialize mutex
        queue_->enqueue(kernelSdot, workSize, localWorksize);

        // tmp <= A * p_k
        kernelSpmv->setArg(0, vecSize);
        kernelSpmv->setArg(1, vecSize);
        kernelSpmv->setArg(2, mat.getNumColsPerRow());
        kernelSpmv->setArg(3, indicesBuf);
        kernelSpmv->setArg(4, matBuf);
        kernelSpmv->setArg(5, pBuf);
        kernelSpmv->setArg(6, tmpBuf);
        queue_->enqueue(kernelSpmv, workSize);

        // dot(p_k^T, tmp)
        kernelSdot->setArg(0, vecSize);
        kernelSdot->setArg(1, pBuf);
        kernelSdot->setArg(2, tmpBuf);
        kernelSdot->setArg(3, denominatorBuf);
        kernelSdot->setArg(4, mutexBuf);
        kernelSdot->setArg(5, sizeof(float)*localWorksize, 0);
        mutex = 0;
        queue_->enqueueWrite(&mutexBuf, &mutex, true);
        queue_->enqueue(kernelSdot, workSize, localWorksize);

        float denominator;
        float nominator;
        queue_->enqueueRead(&nominatorBuf, (void*)(&nominator), true);
        queue_->enqueueRead(&denominatorBuf, (void*)(&denominator), true);
        float alpha = nominator / denominator;

        // x <= alpha*p + x
        kernelSapxy->setArg(0, vecSize);
        kernelSapxy->setArg(1, pBuf);
        kernelSapxy->setArg(2, xBuf);
        kernelSapxy->setArg(3, alpha);
        kernelSapxy->setArg(4, xBuf);
        queue_->enqueue(kernelSapxy, workSize);

        // r <= -alpha*tmp + r
        kernelSapxy->setArg(0, vecSize);
        kernelSapxy->setArg(1, tmpBuf);
        kernelSapxy->setArg(2, rBuf);
        kernelSapxy->setArg(3, -alpha);
        kernelSapxy->setArg(4, rBuf);
        queue_->enqueue(kernelSapxy, workSize);

        // norm(r_k+1)
        kernelSdot->setArg(0, vecSize);
        kernelSdot->setArg(1, rBuf);
        kernelSdot->setArg(2, rBuf);
        kernelSdot->setArg(3, scalarBuf);
        kernelSdot->setArg(4, mutexBuf);
        kernelSdot->setArg(5, sizeof(float)*localWorksize, 0);
        mutex = 0;
        queue_->enqueueWrite(&mutexBuf, &mutex, true);
        queue_->enqueue(kernelSdot, workSize, localWorksize);
        float beta;
        queue_->enqueueRead(&scalarBuf, (void*)(&beta), true);

        if (sqrt(beta) < threshold)
            break;

        beta /= nominator;

        // p <= beta*p + r
        kernelSapxy->setArg(0, vecSize);
        kernelSapxy->setArg(1, pBuf);
        kernelSapxy->setArg(2, rBuf);
        kernelSapxy->setArg(3, beta);
        kernelSapxy->setArg(4, pBuf);
        queue_->enqueue(kernelSapxy, workSize);
    }

    queue_->enqueueRead(&xBuf, (void*)(result), true);
    queue_->finish();

    if (initialAllocated)
        delete[] initial;

    return iteration;
}

int VoreenBlas::hSpConjGradEll_CPU(const EllMatrix<int16_t>& mat, const float* vec, float* result,
                                    float* initial, float threshold, int maxIterations) {

    if (!mat.isSymmetric()) {
        LERROR("Symmetric matrix expected.");
        return -1;
    }

    size_t vecSize = mat.getNumRows();

    bool initialAllocated = false;
    if (initial == 0) {
        initial = new float[vecSize];
        initialAllocated = true;
        for (size_t i=0; i<vecSize; ++i)
            initial[i] = 0.f;
    }

    float* xBuf = result;
    memcpy(xBuf, initial, sizeof(float)*vecSize);

    float* tmpBuf = initial;
    memcpy(tmpBuf, vec, sizeof(float)*vecSize);

    float* rBuf = new float[vecSize];
    float* pBuf = new float[vecSize];

    float nominator;
    float denominator;

    int iteration = 0;

    // r <= A*x_0
    hSpMVEll_CPU(mat, xBuf, rBuf);

    // p <= -r + b
    sAXPY_CPU(vecSize, rBuf, tmpBuf, -1.f, pBuf);

    // r <= -r + b
    sAXPY_CPU(vecSize, rBuf, tmpBuf, -1.f, rBuf);

    while (iteration < maxIterations) {

        iteration++;

        // norm(r_k)
        nominator = sDOT_CPU(vecSize, rBuf, rBuf);

        // tmp <= A * p_k
        hSpMVEll_CPU(mat, pBuf, tmpBuf);

        // dot(p_k^T, tmp)
        denominator = sDOT_CPU(vecSize, pBuf, tmpBuf);

        float alpha = nominator / denominator;

        // x <= alpha*p + x
        sAXPY_CPU(vecSize, pBuf, xBuf, alpha, xBuf);

        // r <= -alpha*tmp + r
        sAXPY_CPU(vecSize, tmpBuf, rBuf, -alpha, rBuf);

        // norm(r_k+1)
        float beta = sDOT_CPU(vecSize, rBuf, rBuf);

        if (sqrt(beta) < threshold)
            break;

        beta /= nominator;

        // p <= beta*p + r
        sAXPY_CPU(vecSize, pBuf, rBuf, beta, pBuf);
    }

    delete[] rBuf;
    delete[] pBuf;

    if (initialAllocated)
        delete[] initial;

    return iteration;
}


//--------------------------------------------------------------------------------------
// test functions

bool VoreenBlas::runTests() {

    if (!initialized_) {
        LERROR("Not initialized. Aborting.");
        return false;
    }

    int numTests = 0;
    int success = 0;

    success += testSDOT() ? 1 : 0;
    numTests++;

    success += testSNRM2() ? 1 : 0;
    numTests++;

    success += testSAXPY() ? 1 : 0;
    numTests++;

    success += testSSpMVEll() ? 1 : 0;
    numTests++;

    success += testHSpMVEll() ? 1 : 0;
    numTests++;

    success += testSSpInnerProductEll() ? 1 : 0;
    numTests++;

    success += testSSpConjGradEll() ? 1 : 0;
    numTests++;

    success += testHSpConjGradEll() ? 1 : 0;
    numTests++;

    if (success == numTests) {
        LINFO("SUCCESS: " << success << "/" << numTests << " sub tests passed.");
        return true;
    }
    else {
        LWARNING("FAILED: " << success << "/" << numTests << " sub tests passed. See test results above for details.");
        return false;
    }
}

bool VoreenBlas::testSAXPY() {

    int maxDim = (1<<24) + 17;
    float errorThresh = 1e-6;

    LINFO("sAXPY Randomized Tests:");

    // initialize test vectors
    float* x = new float[maxDim];
    float* y = new float[maxDim];
    float* resultCL = new float[maxDim];
    float* resultCPU = new float[maxDim];
    float* diffBuff = new float[maxDim];
    for (int i=0; i<maxDim; i++) {
        x[i] = ((float)rand() / RAND_MAX) - 0.5f;
        y[i] = ((float)rand() / RAND_MAX) * 2.f - 0.6f;
    }

    int numTests = 0;
    int numSucc = 0;

    // run tests for varying vector sizes
    for (int i=2; i<=24; ++i) {

        size_t n = (1<<i);
        float alpha = 1.f;

        // compare results computed on GPU and CPU
        sAXPY_CL(n, x, y, alpha, resultCL);
        sAXPY_CPU(n, x, y, alpha, resultCPU);
        sAXPY_CPU(n, resultCL, resultCPU, -1.f, diffBuff);
        float norm = sNRM2_CPU(n, diffBuff);
        if (norm < errorThresh*n) {
            LINFO("n=" << n << " alpha=" << alpha <<  " : " << "passed (" << norm << ")");
            numSucc++;
        }
        else
            LWARNING("n=" << n << " alpha=" << alpha <<  " : " << "FAILED  (" << norm << ")");
        numTests++;

        // repeat test for non-power-of-two vector sizes and varying alphas
        n = (1<<i) - 1;
        alpha = 0.5f;
        sAXPY_CL(n, x, y, alpha, resultCL);
        sAXPY_CPU(n, x, y, alpha, resultCPU);
        sAXPY_CPU(n, resultCL, resultCPU, -1.f, diffBuff);
        norm = sNRM2_CPU(n, diffBuff);
        if (norm < errorThresh*n) {
            LINFO("n=" << n << " alpha=" << alpha <<  " : " << "passed (" << norm << ")");
            numSucc++;
        }
        else
            LWARNING("n=" << n << " alpha=" << alpha <<  " : " << "FAILED  (" << norm << ")");
        numTests++;

        n = (1<<i) + 17;
        alpha = -2.5f;
        sAXPY_CL(n, x, y, alpha, resultCL);
        sAXPY_CPU(n, x, y, alpha, resultCPU);
        sAXPY_CPU(n, resultCL, resultCPU, -1.f, diffBuff);
        norm = sNRM2_CPU(n, diffBuff);
        if (norm < errorThresh*n) {
            LINFO("n=" << n << " alpha=" << alpha <<  " : " << "passed (" << norm << ")");
            numSucc++;
        }
        else
            LWARNING("n=" << n << " alpha=" << alpha <<  " : " << "FAILED  (" << norm << ")");
        numTests++;

    }

    delete[] x;
    delete[] y;
    delete[] resultCL;
    delete[] resultCPU;
    delete[] diffBuff;

    if (numSucc == numTests) {
        LINFO("sAXPY Randomized Tests (passed/overall): " << numSucc << "/" << numTests);
        return true;
    }
    else {
        LWARNING("sAXPY Randomized Tests (passed/overall): " << numSucc << "/" << numTests);
        return false;
    }
}

bool VoreenBlas::testSDOT() {

    int maxDim = (1<<25) + 17;
    float errorThresh = 1e-6;

    LINFO("sDOT Randomized Tests:");

    // initialize test vectors
    float* x = new float[maxDim];
    float* y = new float[maxDim];
    for (int i=0; i<maxDim; i++) {
        x[i] = ((float)rand() / RAND_MAX) - 0.5f;
        y[i] = ((float)rand() / RAND_MAX) * 2.f - 0.6f;
    }

    int numTests = 0;
    int numSucc = 0;

    // run tests for varying vector sizes
    for (int i=2; i<=25; ++i) {

        size_t n = (1<<i);

        // compare results computed on GPU and CPU
        float resultCL = sDOT_CL(n, x, y);
        float resultCPU = sDOT_CPU(n, x, y);
        float relError = std::abs((resultCL / resultCPU) - 1.f);
        if (relError < std::min(errorThresh*n, 0.01f)) {
            LINFO("n=" << n << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError);
            numSucc++;
        }
        else
            LWARNING("n=" << n << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError << " (FAILED)");
        numTests++;

        // repeat test for non-power-of-two vector sizes
        n = (1<<i) - 1;
        resultCL = sDOT_CL(n, x, y);
        resultCPU = sDOT_CPU(n, x, y);
        relError = std::abs((resultCL / resultCPU) - 1.f);
        if (relError < errorThresh*n) {
            LINFO("n=" << n << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError);
            numSucc++;
        }
        else
            LWARNING("n=" << n << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError << " (FAILED)");
        numTests++;

        n = (1<<i) + 17;
        resultCL = sDOT_CL(n, x, y);
        resultCPU = sDOT_CPU(n, x, y);
        relError = std::abs((resultCL / resultCPU) - 1.f);
        if (relError < std::min(errorThresh*n, 0.01f)) {
            LINFO("n=" << n << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError);
            numSucc++;
        }
        else
            LWARNING("n=" << n << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError << " (FAILED)");
        numTests++;

    }

    delete[] x;
    delete[] y;

    if (numSucc == numTests) {
        LINFO("sDOT Randomized Tests (passed/overall): " << numSucc << "/" << numTests);
        return true;
    }
    else {
        LWARNING("sDOT Randomized Tests (passed/overall): " << numSucc << "/" << numTests);
        return false;
    }
}

bool VoreenBlas::testSNRM2() {

    int maxDim = (1<<24) + 17;
    float errorThresh = 1e-6;

    LINFO("sNRM2 Randomized Tests:");

    // initialize test vectors
    float* x = new float[maxDim];
    for (int i=0; i<maxDim; i++) {
        x[i] = ((float)rand() / RAND_MAX) - 0.5f;
    }

    int numTests = 0;
    int numSucc = 0;

    // run tests for varying vector sizes
    for (int i=2; i<=24; ++i) {

        size_t n = (1<<i);

        // compare results computed on GPU and CPU
        float resultCL = sNRM2_CL(n, x);
        float resultCPU = sNRM2_CPU(n, x);
        float relError = std::abs((resultCL / resultCPU) - 1.f);
        if (relError < std::min(errorThresh*n, 0.1f)) {
            LINFO("n=" << n << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError);
            numSucc++;
        }
        else
            LWARNING("n=" << n << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError << " (FAILED)");
        numTests++;

        // repeat test for non-power-of-two vector sizes
        n = (1<<i) - 1;
        resultCL = sNRM2_CL(n, x);
        resultCPU = sNRM2_CPU(n, x);
        relError = std::abs((resultCL / resultCPU) - 1.f);
        if (relError < std::min(errorThresh*n, 0.1f)) {
            LINFO("n=" << n << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError);
            numSucc++;
        }
        else
            LWARNING("n=" << n << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError << " (FAILED)");
        numTests++;

        n = (1<<i) + 17;
        resultCL = sNRM2_CL(n, x);
        resultCPU = sNRM2_CPU(n, x);
        relError = std::abs((resultCL / resultCPU) - 1.f);
        if (relError < std::min(errorThresh*n, 0.1f)) {
            LINFO("n=" << n << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError);
            numSucc++;
        }
        else
            LWARNING("n=" << n << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError << " (FAILED)");
        numTests++;

    }

    delete[] x;

    if (numSucc == numTests) {
        LINFO("sNRM2 Randomized Tests (passed/overall): " << numSucc << "/" << numTests);
        return true;
    }
    else {
        LWARNING("sNRM2 Randomized Tests (passed/overall): " << numSucc << "/" << numTests);
        return false;
    }
}

bool VoreenBlas::testSSpMVEll() {

    float thresh = 1e-6;
    int numTests = 0;
    int numSucc = 0;

    LINFO("sSpMV-ELL Ground Truth Test: rows=8 cols=6 colsPerRow=3");

    EllMatrix<float> mat(8, 6, 3);
    mat.setValue(0, 1, 2.f);
    mat.setValue(0, 3, -3.f);
    mat.setValue(0, 4, 5.f);
    mat.setValue(1, 0, -1.5);
    mat.setValue(1, 1, 9.f);
    mat.setValue(2, 1, 2.f);
    mat.setValue(2, 3, -3.5f);
    mat.setValue(2, 4, 7.f);
    mat.setValue(4, 0, 5.5f);
    mat.setValue(4, 5, -13.f);
    mat.setValue(5, 1, 8.f);
    mat.setValue(5, 2, 9.f);
    mat.setValue(5, 3, -5.f);
    mat.setValue(6, 0, 2.f);
    mat.setValue(6, 4, -3.f);
    mat.setValue(6, 5, -0.5f);
    mat.setValue(7, 0, 1.f);
    mat.setValue(7, 2, 2.f);
    mat.setValue(7, 5, 3.f);

    float* x = new float[6];
    x[0] = 1.f;
    x[1] = -3.f;
    x[2] = 0.f;
    x[3] = 5.f;
    x[4] = 0.5f;
    x[5] = 10.f;

    float* expRes = new float[8];
    expRes[0] = -18.5f;
    expRes[1] = -28.5;
    expRes[2] = -20.f;
    expRes[3] = 0.f;
    expRes[4] = -124.5f;
    expRes[5] = -49;
    expRes[6] = -4.5;
    expRes[7] = 31.f;

    float* yCL = new float[8];
    float* yCPU = new float[8];
    sSpMVEll_CL(mat, x, yCL);
    sSpMVEll_CPU(mat, x, yCPU);

    LINFO("Ground Truth: "  << expRes[0] << " " << expRes[1] << " " << expRes[2] << " " << expRes[3] << " " << expRes[4] << " " << expRes[5] << " " << expRes[6] << " " << expRes[7]);
    if (cmpVectors(8, expRes, yCPU, thresh)) {
        LINFO("Result CPU : " << yCPU[0]   << " " << yCPU[1]   << " " << yCPU[2]   << " " << yCPU[3]   << " " << yCPU[4]   << " " << yCPU[5]   << " " << yCPU[6]   << " " << yCPU[7] << " (passed)");
        numSucc++;
    }
    else
        LWARNING("Result CPU : " << yCPU[0]   << " " << yCPU[1]   << " " << yCPU[2]   << " " << yCPU[3]   << " " << yCPU[4]   << " " << yCPU[5]   << " " << yCPU[6]   << " " << yCPU[7] << " (FAILED)");
    if (cmpVectors(8, expRes, yCL, thresh)) {
        LINFO("Result OCL : " << yCL[0]    << " " << yCL[1]    << " " << yCL[2]    << " " << yCL[3]    << " " << yCL[4]    << " " << yCL[5]    << " " << yCL[6]    << " " << yCL[7] << " (passed)");
        numSucc++;
    }
    else
        LWARNING("Result OCL : " << yCL[0]    << " " << yCL[1]    << " " << yCL[2]    << " " << yCL[3]    << " " << yCL[4]    << " " << yCL[5]    << " " << yCL[6]    << " " << yCL[7] << " (FAILED)");
    numTests += 2;

    delete[] x;
    delete[] yCL;
    delete[] yCPU;
    delete[] expRes;


    LINFO("sSpMV-ELL Randomized Tests:");
    float* diffBuff = 0;
    for (int i=2; i<=21; ++i) {

        {
            int numRows = (1<<i);
            int numCols = numRows;
            int numColsPerRow = 1;

            EllMatrix<float> randMat(numRows, numCols, numColsPerRow);
            randomizeEll(randMat);

            x = new float[randMat.getNumCols()];
            yCL = new float[randMat.getNumRows()];
            yCPU = new float[randMat.getNumRows()];
            diffBuff = new float[randMat.getNumRows()];
            for (size_t j=0; j<randMat.getNumCols(); ++j)
                x[j] = (static_cast<float>(rand()) / RAND_MAX) * 10.f - 5.f;

            sSpMVEll_CL(randMat, x, yCL);
            sSpMVEll_CPU(randMat, x, yCPU);

            sAXPY_CPU(numRows, yCL, yCPU, -1.f, diffBuff);
            float norm = sNRM2_CPU(numRows, diffBuff);
            if (norm < thresh*numRows) {
                LINFO("rows=" << randMat.getNumRows() << " cols=" << randMat.getNumCols() << " colsPerRow= " << randMat.getNumColsPerRow() << ": passed (" << norm << ")");
                numSucc++;
            }
            else {
                LWARNING("rows=" << randMat.getNumRows() << " cols=" << randMat.getNumCols() << " colsPerRow= " << randMat.getNumColsPerRow() << ": FAILED (" << norm << ")");
            }
            numTests++;

            delete[] x;
            delete[] yCL;
            delete[] yCPU;
            delete[] diffBuff;
        }

        {
            int numRows = (1<<i) - 1;
            int numCols = tgt::iround(numRows / 1.5) + 7;
            float maxCols;
            if (numRows < 1<<12)
                maxCols = 128.f;
            else if (numRows < 1<<16)
                maxCols = 32.f;
            else if (numRows < 1<<20)
                maxCols = 8.f;
            else
                maxCols = 4.f;
            int numColsPerRow = tgt::clamp(tgt::iround((static_cast<float>(rand()) / RAND_MAX)*maxCols), 2, numCols);

            EllMatrix<float> randMat(numRows, numCols, numColsPerRow);
            randomizeEll(randMat);

            x = new float[randMat.getNumCols()];
            yCL = new float[randMat.getNumRows()];
            yCPU = new float[randMat.getNumRows()];
            diffBuff = new float[randMat.getNumRows()];
            for (size_t j=0; j<randMat.getNumCols(); ++j)
                x[j] = (static_cast<float>(rand()) / RAND_MAX) * 10.f - 5.f;

            sSpMVEll_CL(randMat, x, yCL);
            sSpMVEll_CPU(randMat, x, yCPU);
            sAXPY_CPU(numRows, yCL, yCPU, -1.f, diffBuff);
            float norm = sNRM2_CPU(numRows, diffBuff);
            if (norm < thresh*numRows) {
                LINFO("rows=" << randMat.getNumRows() << " cols=" << randMat.getNumCols() << " colsPerRow= " << randMat.getNumColsPerRow() << ": passed (" << norm << ")");
                numSucc++;
            }
            else {
                LWARNING("rows=" << randMat.getNumRows() << " cols=" << randMat.getNumCols() << " colsPerRow= " << randMat.getNumColsPerRow() << ": FAILED (" << norm << ")");
            }
            numTests++;

            delete[] x;
            delete[] yCL;
            delete[] yCPU;
            delete[] diffBuff;
        }

        {
            int numRows = (1<<i) + 37;
            int numCols = tgt::iround(numRows*1.1f) + 3;
            float maxCols;
            if (numRows < 1<<12)
                maxCols = 128.f;
            else if (numRows < 1<<16)
                maxCols = 32.f;
            else if (numRows < 1<<20)
                maxCols = 8.f;
            else
                maxCols = 4.f;
            int numColsPerRow = tgt::clamp(tgt::iround((static_cast<float>(rand()) / RAND_MAX)*maxCols), 2, numCols);

            EllMatrix<float> randMat(numRows, numCols, numColsPerRow);
            randomizeEll(randMat);

            x = new float[randMat.getNumCols()];
            yCL = new float[randMat.getNumRows()];
            yCPU = new float[randMat.getNumRows()];
            diffBuff = new float[randMat.getNumRows()];
            for (size_t j=0; j<randMat.getNumCols(); ++j)
                x[j] = (static_cast<float>(rand()) / RAND_MAX) * 10.f - 5.f;

            sSpMVEll_CL(randMat, x, yCL);
            sSpMVEll_CPU(randMat, x, yCPU);
            sAXPY_CPU(numRows, yCL, yCPU, -1.f, diffBuff);
            float norm = sNRM2_CPU(numRows, diffBuff);
            if (norm < thresh*numRows) {
                LINFO("rows=" << randMat.getNumRows() << " cols=" << randMat.getNumCols() << " colsPerRow=" << randMat.getNumColsPerRow() << ": passed (" << norm << ")");
                numSucc++;
            }
            else {
                LWARNING("rows=" << randMat.getNumRows() << " cols=" << randMat.getNumCols() << " colsPerRow=" << randMat.getNumColsPerRow() << ": FAILED (" << norm << ")");
            }
            numTests++;

            delete[] x;
            delete[] yCL;
            delete[] yCPU;
            delete[] diffBuff;
        }
    }
    if (numSucc == numTests) {
        LINFO("sSpMV-ELL Randomized Tests (passed/overall): " << numSucc << "/" << numTests);
        return true;
    }
    else {
        LWARNING("sSpMV-ELL Randomized Tests (passed/overall): " << numSucc << "/" << numTests);
        return false;
    }
}

bool VoreenBlas::testHSpMVEll() {

    float thresh = 1e-6;
    int numTests = 0;
    int numSucc = 0;

    LINFO("hSpMV-ELL Ground Truth Test: rows=8 cols=6 colsPerRow=3");

    EllMatrix<int16_t> mat(8, 6, 3);
    mat.setValue(0, 1, 4);
    mat.setValue(0, 3, -6);
    mat.setValue(0, 4, 10);
    mat.setValue(1, 0, -3);
    mat.setValue(1, 1, 18);
    mat.setValue(2, 1, 4);
    mat.setValue(2, 3, -7);
    mat.setValue(2, 4, 14);
    mat.setValue(4, 0, 11);
    mat.setValue(4, 5, -26);
    mat.setValue(5, 1, 16);
    mat.setValue(5, 2, 18);
    mat.setValue(5, 3, -10);
    mat.setValue(6, 0, 4);
    mat.setValue(6, 4, -6);
    mat.setValue(6, 5, -1);
    mat.setValue(7, 0, 2);
    mat.setValue(7, 2, 4);
    mat.setValue(7, 5, 6);

    float* x = new float[6];
    x[0] = 1.f;
    x[1] = -3.f;
    x[2] = 0.f;
    x[3] = 5.f;
    x[4] = 0.5f;
    x[5] = 10.f;

    float scale = 2.f / ((1<<15) - 1);
    float* expRes = new float[8];
    expRes[0] = -18.5f * scale;
    expRes[1] = -28.5f * scale;
    expRes[2] = -20.f * scale;
    expRes[3] = 0.f * scale;
    expRes[4] = -124.5f * scale;
    expRes[5] = -49.f * scale;
    expRes[6] = -4.5f * scale;
    expRes[7] = 31.f * scale;

    float* yCL = new float[8];
    float* yCPU = new float[8];
    hSpMVEll_CL(mat, x, yCL);
    hSpMVEll_CPU(mat, x, yCPU);

    LINFO("Ground Truth: "  << expRes[0] << " " << expRes[1] << " " << expRes[2] << " " << expRes[3] << " " << expRes[4] << " " << expRes[5] << " " << expRes[6] << " " << expRes[7]);
    if (cmpVectors(8, expRes, yCPU, thresh)) {
        LINFO("Result CPU : " << yCPU[0]   << " " << yCPU[1]   << " " << yCPU[2]   << " " << yCPU[3]   << " " << yCPU[4]   << " " << yCPU[5]   << " " << yCPU[6]   << " " << yCPU[7] << " (passed)");
        numSucc++;
    }
    else
        LWARNING("Result CPU : " << yCPU[0]   << " " << yCPU[1]   << " " << yCPU[2]   << " " << yCPU[3]   << " " << yCPU[4]   << " " << yCPU[5]   << " " << yCPU[6]   << " " << yCPU[7] << " (FAILED)");
    if (cmpVectors(8, expRes, yCL, thresh)) {
        LINFO("Result OCL : " << yCL[0]    << " " << yCL[1]    << " " << yCL[2]    << " " << yCL[3]    << " " << yCL[4]    << " " << yCL[5]    << " " << yCL[6]    << " " << yCL[7] << " (passed)");
        numSucc++;
    }
    else
        LWARNING("Result OCL : " << yCL[0]    << " " << yCL[1]    << " " << yCL[2]    << " " << yCL[3]    << " " << yCL[4]    << " " << yCL[5]    << " " << yCL[6]    << " " << yCL[7] << " (FAILED)");

    numTests += 2;

    delete[] x;
    delete[] yCL;
    delete[] yCPU;
    delete[] expRes;

    LINFO("hSpMV-ELL Randomized Tests:");
    float* diffBuff = 0;
    for (int i=2; i<=21; ++i) {

        {
            int numRows = (1<<i);
            int numCols = numRows;
            int numColsPerRow = 1;

            EllMatrix<int16_t> randMat(numRows, numCols, numColsPerRow);
            randomizeEll(randMat);

            x = new float[randMat.getNumCols()];
            yCL = new float[randMat.getNumRows()];
            yCPU = new float[randMat.getNumRows()];
            diffBuff = new float[randMat.getNumRows()];
            for (size_t j=0; j<randMat.getNumCols(); ++j)
                x[j] = (static_cast<float>(rand()) / RAND_MAX) * 10.f - 5.f;

            hSpMVEll_CL(randMat, x, yCL);
            hSpMVEll_CPU(randMat, x, yCPU);

            sAXPY_CPU(numRows, yCL, yCPU, -1.f, diffBuff);
            float norm = sNRM2_CPU(numRows, diffBuff);
            if (norm < thresh*numRows) {
                LINFO("rows=" << randMat.getNumRows() << " cols=" << randMat.getNumCols() << " colsPerRow= " << randMat.getNumColsPerRow() << ": passed (" << norm << ")");
                numSucc++;
            }
            else {
                LWARNING("rows=" << randMat.getNumRows() << " cols=" << randMat.getNumCols() << " colsPerRow= " << randMat.getNumColsPerRow() << ": FAILED (" << norm << ")");
            }
            numTests++;

            delete[] x;
            delete[] yCL;
            delete[] yCPU;
            delete[] diffBuff;
        }

        {
            int numRows = (1<<i) - 1;
            int numCols = tgt::iround(numRows / 1.5) + 7;
            float maxCols;
            if (numRows < 1<<12)
                maxCols = 128.f;
            else if (numRows < 1<<16)
                maxCols = 32.f;
            else if (numRows < 1<<20)
                maxCols = 8.f;
            else
                maxCols = 4.f;
            int numColsPerRow = tgt::clamp(tgt::iround((static_cast<float>(rand()) / RAND_MAX)*maxCols), 2, numCols);

            EllMatrix<int16_t> randMat(numRows, numCols, numColsPerRow);
            randomizeEll(randMat);

            x = new float[randMat.getNumCols()];
            yCL = new float[randMat.getNumRows()];
            yCPU = new float[randMat.getNumRows()];
            diffBuff = new float[randMat.getNumRows()];
            for (size_t j=0; j<randMat.getNumCols(); ++j)
                x[j] = (static_cast<float>(rand()) / RAND_MAX) * 10.f - 5.f;

            hSpMVEll_CL(randMat, x, yCL);
            hSpMVEll_CPU(randMat, x, yCPU);
            sAXPY_CPU(numRows, yCL, yCPU, -1.f, diffBuff);
            float norm = sNRM2_CPU(numRows, diffBuff);
            if (norm < thresh*numRows) {
                LINFO("rows=" << randMat.getNumRows() << " cols=" << randMat.getNumCols() << " colsPerRow= " << randMat.getNumColsPerRow() << ": passed (" << norm << ")");
                numSucc++;
            }
            else {
                LWARNING("rows=" << randMat.getNumRows() << " cols=" << randMat.getNumCols() << " colsPerRow= " << randMat.getNumColsPerRow() << ": FAILED (" << norm << ")");
            }
            numTests++;

            delete[] x;
            delete[] yCL;
            delete[] yCPU;
            delete[] diffBuff;
        }

        {
            int numRows = (1<<i) + 37;
            int numCols = tgt::iround(numRows*1.1f) + 3;
            float maxCols;
            if (numRows < 1<<12)
                maxCols = 128.f;
            else if (numRows < 1<<16)
                maxCols = 32.f;
            else if (numRows < 1<<20)
                maxCols = 8.f;
            else
                maxCols = 4.f;
            int numColsPerRow = tgt::clamp(tgt::iround((static_cast<float>(rand()) / RAND_MAX)*maxCols), 2, numCols);

            EllMatrix<int16_t> randMat(numRows, numCols, numColsPerRow);
            randomizeEll(randMat);

            x = new float[randMat.getNumCols()];
            yCL = new float[randMat.getNumRows()];
            yCPU = new float[randMat.getNumRows()];
            diffBuff = new float[randMat.getNumRows()];
            for (size_t j=0; j<randMat.getNumCols(); ++j)
                x[j] = (static_cast<float>(rand()) / RAND_MAX) * 10.f - 5.f;

            hSpMVEll_CL(randMat, x, yCL);
            hSpMVEll_CPU(randMat, x, yCPU);
            sAXPY_CPU(numRows, yCL, yCPU, -1.f, diffBuff);
            float norm = sNRM2_CPU(numRows, diffBuff);
            if (norm < thresh*numRows) {
                LINFO("rows=" << randMat.getNumRows() << " cols=" << randMat.getNumCols() << " colsPerRow=" << randMat.getNumColsPerRow() << ": passed (" << norm << ")");
                numSucc++;
            }
            else {
                LWARNING("rows=" << randMat.getNumRows() << " cols=" << randMat.getNumCols() << " colsPerRow=" << randMat.getNumColsPerRow() << ": FAILED (" << norm << ")");
            }
            numTests++;

            delete[] x;
            delete[] yCL;
            delete[] yCPU;
            delete[] diffBuff;
        }
    }
    if (numSucc == numTests) {
        LINFO("hSpMV-ELL Randomized Tests (passed/overall): " << numSucc << "/" << numTests);
        return true;
    }
    else {
        LWARNING("hSpMV-ELL Randomized Tests (passed/overall): " << numSucc << "/" << numTests);
        return false;
    }
}

bool VoreenBlas::testSSpInnerProductEll() {

    LINFO("sInnerProduct-ELL Randomized Tests:");
    float errorThresh = 1e-6;
    int numTests = 0;
    int numSucc = 0;
    for (int i=3; i<=20; ++i) {

        {
            numTests++;

            int numRows = (1<<i);
            int numColsPerRow = std::min(7, numRows);

            EllMatrix<float> randMat(numRows, numRows, numColsPerRow);
            randomizeEllPositiveDefinite(randMat);
            if (!randMat.isSymmetric()) {
                LWARNING("rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": FAILED (matrix not symmetric)");
                continue;
            }

            float* x = new float[numRows];
            float* y = new float[numRows];
            for (int j=0; j<numRows; ++j) {
                x[j] = (static_cast<float>(rand()) / RAND_MAX) * 2.f - 0.5f;
                y[j] = (static_cast<float>(rand()) / RAND_MAX) * 2.f - 0.5f;
            }

            float resultCL = sSpInnerProductEll_CL(randMat, x, y);
            float resultCPU = sSpInnerProductEll_CPU(randMat, x, y);

            float relError = std::abs((resultCL / resultCPU) - 1.f);
            if (relError < std::min(errorThresh*numRows, 0.1f)) {
                LINFO("(pos. definite) n=" << numRows << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError);
                numSucc++;
            }
            else
                LWARNING("(pos. definite) n=" << numRows << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError << " (FAILED)");

            delete[] x;
            delete[] y;
        }

        {
            numTests++;

            int numRows = (1<<i) - 1;
            int numColsPerRow = std::min(7, numRows);

            EllMatrix<float> randMat(numRows, numRows, numColsPerRow);
            randomizeEll(randMat);

            float* x = new float[numRows];
            float* y = new float[numRows];
            for (int j=0; j<numRows; ++j) {
                x[j] = (static_cast<float>(rand()) / RAND_MAX) * 2.f - 0.5f;
                y[j] = (static_cast<float>(rand()) / RAND_MAX) * 2.f - 0.5f;
            }

            float resultCL = sSpInnerProductEll_CL(randMat, x, y);
            float resultCPU = sSpInnerProductEll_CPU(randMat, x, y);

            float relError = std::abs((resultCL / resultCPU) - 1.f);
            if (relError < std::min(errorThresh*numRows, 0.1f)) {
                LINFO("n=" << numRows << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError);
                numSucc++;
            }
            else
                LWARNING("n=" << numRows << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError << " (FAILED)");

            delete[] x;
            delete[] y;
        }

        {
            numTests++;

            int numRows = (1<<i) + 17;
            int numColsPerRow = std::min(7, numRows);

            EllMatrix<float> randMat(numRows, numRows, numColsPerRow);
            randomizeEll(randMat);

            float* x = new float[numRows];
            float* y = new float[numRows];
            for (int j=0; j<numRows; ++j) {
                x[j] = (static_cast<float>(rand()) / RAND_MAX) * 2.f - 0.5f;
                y[j] = (static_cast<float>(rand()) / RAND_MAX) * 2.f - 0.5f;
            }

            float resultCL = sSpInnerProductEll_CL(randMat, x, y);
            float resultCPU = sSpInnerProductEll_CPU(randMat, x, y);

            float relError = std::abs((resultCL / resultCPU) - 1.f);
            if (relError < std::min(errorThresh*numRows, 0.1f)) {
                LINFO("n=" << numRows << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError);
                numSucc++;
            }
            else
                LWARNING("n=" << numRows << ":  Result GPU: " << resultCL << ", Result CPU: " << resultCPU << ", Relative Error: " << relError << " (FAILED)");

            delete[] x;
            delete[] y;
        }

    }

    if (numSucc == numTests) {
        LINFO("sInnerProduct-ELL Randomized Tests (passed/overall): " << numSucc << "/" << numTests);
        return true;
    }
    else {
        LWARNING("sInnerProduct-ELL Randomized Tests Tests (passed/overall): " << numSucc << "/" << numTests);
        return false;
    }
}

bool VoreenBlas::testSSpConjGradEll() {

    LINFO("sSpConjGrad-ELL Randomized Tests:");
    int numTests = 0;
    int numSucc = 0;
    for (int i=3; i<=21; ++i) {

        {
            int numRows = (1<<i);
            int numColsPerRow = std::min(7, numRows);

            // initialize test matrix
            EllMatrix<float> randMat(numRows, numRows, numColsPerRow);
            randomizeEllPositiveDefinite(randMat);
            if (!randMat.isSymmetric()) {
                LWARNING("rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": FAILED (matrix not symmetric)");
                continue;
            }

            // initialize vectors
            float* x = new float[numRows];
            float* y = new float[numRows];
            float* tempBuf = new float[numRows];
            for (int j=0; j<numRows; ++j)
                x[j] = (static_cast<float>(rand()) / RAND_MAX) * 1.f;

            // compute GPU solution
            clock_t start, end;
            start = clock();
            int iterations = sSpConjGradEll_CL(randMat, x, y, 0, NoPreconditioner, numRows*1e-8f);
            end = clock();
            float duration = static_cast<float>(end-start)/CLOCKS_PER_SEC;

            // check result
            sSpMVEll_CPU(randMat, y, tempBuf);
            sAXPY_CPU(numRows, x, tempBuf, -1.f, tempBuf);
            float normGPU = sNRM2_CPU(numRows, tempBuf);
            if (normGPU < numRows*1e-6f) {
                LINFO("GPU: rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": passed (error: " << normGPU << ", iterations: " << iterations << ", duration: " << duration << " sec)");
                numSucc++;
            }
            else {
                LWARNING("GPU: rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": FAILED (" << normGPU << ")");
            }
            numTests++;

            // CPU solution
            if (i <= 18) {
                start = clock();
                iterations = sSpConjGradEll_CPU(randMat, x, y, 0, NoPreconditioner, numRows*1e-8f);
                end = clock();
                duration = static_cast<float>(end-start)/CLOCKS_PER_SEC;

                sSpMVEll_CPU(randMat, y, tempBuf);
                sAXPY_CPU(numRows, x, tempBuf, -1.f, tempBuf);
                float normCPU = sNRM2_CPU(numRows, tempBuf);
                if (normCPU < numRows*1e-6f) {
                    LINFO("CPU: rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": passed (error: " << normCPU << ", iterations: " << iterations << ", duration: " << duration << " sec)");
                    numSucc++;
                }
                else {
                    LWARNING("CPU: rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": FAILED (" << normCPU << ")");
                }
                numTests++;
            }

            delete[] x;
            delete[] y;
            delete[] tempBuf;
        }

        {
            int numRows = tgt::iround((1<<i) * 0.7f);
            int numColsPerRow = std::min(6, numRows);

            // initialize test matrix
            EllMatrix<float> randMat(numRows, numRows, numColsPerRow);
            randomizeEllPositiveDefinite(randMat);
            if (!randMat.isSymmetric()) {
                LWARNING("rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": FAILED (matrix not symmetric)");
                continue;
            }

            // initialize vectors
            float* x = new float[numRows];
            float* y = new float[numRows];
            float* tempBuf = new float[numRows];
            for (int j=0; j<numRows; ++j)
                x[j] = (static_cast<float>(rand()) / RAND_MAX) * 2.f - 0.5f;

            // compute GPU solution
            clock_t start, end;
            start = clock();
            int iterations = sSpConjGradEll_CL(randMat, x, y, 0, NoPreconditioner, numRows*1e-8f);
            end = clock();
            float duration = static_cast<float>(end-start)/CLOCKS_PER_SEC;

            // check result
            sSpMVEll_CPU(randMat, y, tempBuf);
            sAXPY_CPU(numRows, x, tempBuf, -1.f, tempBuf);
            float normGPU = sNRM2_CPU(numRows, tempBuf);
            if (normGPU < numRows*1e-6f) {
                LINFO("GPU: rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": passed (error: " << normGPU << ", iterations: " << iterations << ", duration: " << duration << " sec)");
                numSucc++;
            }
            else {
                LWARNING("GPU: rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": FAILED (" << normGPU << ")");
            }
            numTests++;

            // CPU solution
            if (i <= 18) {
                start = clock();
                iterations = sSpConjGradEll_CPU(randMat, x, y, 0, NoPreconditioner, numRows*1e-8f);
                end = clock();
                duration = static_cast<float>(end-start)/CLOCKS_PER_SEC;

                sSpMVEll_CPU(randMat, y, tempBuf);
                sAXPY_CPU(numRows, x, tempBuf, -1.f, tempBuf);
                float normCPU = sNRM2_CPU(numRows, tempBuf);
                if (normCPU < numRows*1e-6f) {
                    LINFO("CPU: rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": passed (error: " << normCPU << ", iterations: " << iterations << ", duration: " << duration << " sec)");
                    numSucc++;
                }
                else {
                    LWARNING("CPU: rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": FAILED (" << normCPU << ")");
                }
                numTests++;
            }

            delete[] x;
            delete[] y;
            delete[] tempBuf;
        }

    }

    if (numSucc == numTests) {
        LINFO("sSpConjGrad-ELL Randomized Tests (passed/overall): " << numSucc << "/" << numTests);
        return true;
    }
    else {
        LWARNING("sSpConjGrad-ELL Randomized Tests (passed/overall): " << numSucc << "/" << numTests);
        return false;
    }
}


bool VoreenBlas::testHSpConjGradEll() {

    LINFO("hSpConjGrad-ELL Randomized Tests:");
    int numTests = 0;
    int numSucc = 0;
    for (int i=3; i<=21; ++i) {

        {
            int numRows = (1<<i);
            int numColsPerRow = std::min(7, numRows);

            // initialize test matrix
            EllMatrix<int16_t> randMat(numRows, numRows, numColsPerRow);
            randomizeEllPositiveDefinite(randMat);
            if (!randMat.isSymmetric()) {
                LWARNING("rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": FAILED (matrix not symmetric)");
                continue;
            }

            // initialize vectors
            float* x = new float[numRows];
            float* y = new float[numRows];
            float* tempBuf = new float[numRows];
            for (int j=0; j<numRows; ++j)
                x[j] = (static_cast<float>(rand()) / RAND_MAX) * 1.f;

            // compute GPU solution
            clock_t start, end;
            start = clock();
            int iterations = hSpConjGradEll_CL(randMat, x, y, 0, numRows*1e-8f);
            end = clock();
            float duration = static_cast<float>(end-start)/CLOCKS_PER_SEC;

            // check result
            hSpMVEll_CPU(randMat, y, tempBuf);
            sAXPY_CPU(numRows, x, tempBuf, -1.f, tempBuf);
            float normGPU = sNRM2_CPU(numRows, tempBuf);
            if (normGPU < numRows*1e-6f) {
                LINFO("GPU: rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": passed (error: " << normGPU << ", iterations: " << iterations << ", duration: " << duration << " sec)");
                numSucc++;
            }
            else {
                LWARNING("GPU: rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": FAILED (" << normGPU << ")");
            }
            numTests++;

            // CPU solution
            if (i <= 18) {
                start = clock();
                iterations = hSpConjGradEll_CPU(randMat, x, y, 0, numRows*1e-8f);
                end = clock();
                duration = static_cast<float>(end-start)/CLOCKS_PER_SEC;

                hSpMVEll_CPU(randMat, y, tempBuf);
                sAXPY_CPU(numRows, x, tempBuf, -1.f, tempBuf);
                float normCPU = sNRM2_CPU(numRows, tempBuf);
                if (normCPU < numRows*1e-6f) {
                    LINFO("CPU: rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": passed (error: " << normCPU << ", iterations: " << iterations << ", duration: " << duration << " sec)");
                    numSucc++;
                }
                else {
                    LWARNING("CPU: rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": FAILED (" << normCPU << ")");
                }
                numTests++;
            }

            delete[] x;
            delete[] y;
            delete[] tempBuf;
        }

        {
            int numRows = tgt::iround((1<<i) * 0.7f);
            int numColsPerRow = std::min(6, numRows);

            // initialize test matrix
            EllMatrix<int16_t> randMat(numRows, numRows, numColsPerRow);
            randomizeEllPositiveDefinite(randMat);
            if (!randMat.isSymmetric()) {
                LWARNING("rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": FAILED (matrix not symmetric)");
                continue;
            }

            // initialize vectors
            float* x = new float[numRows];
            float* y = new float[numRows];
            float* tempBuf = new float[numRows];
            for (int j=0; j<numRows; ++j)
                x[j] = (static_cast<float>(rand()) / RAND_MAX) * 2.f - 0.5f;

            // compute GPU solution
            clock_t start, end;
            start = clock();
            int iterations = hSpConjGradEll_CL(randMat, x, y, 0, numRows*1e-8f);
            end = clock();
            float duration = static_cast<float>(end-start)/CLOCKS_PER_SEC;

            // check result
            hSpMVEll_CPU(randMat, y, tempBuf);
            sAXPY_CPU(numRows, x, tempBuf, -1.f, tempBuf);
            float normGPU = sNRM2_CPU(numRows, tempBuf);
            if (normGPU < numRows*1e-6f) {
                LINFO("GPU: rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": passed (error: " << normGPU << ", iterations: " << iterations << ", duration: " << duration << " sec)");
                numSucc++;
            }
            else {
                LWARNING("GPU: rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": FAILED (" << normGPU << ")");
            }
            numTests++;

            // CPU solution
            if (i <= 18) {
                start = clock();
                iterations = hSpConjGradEll_CPU(randMat, x, y, 0, numRows*1e-8f);
                end = clock();
                duration = static_cast<float>(end-start)/CLOCKS_PER_SEC;

                hSpMVEll_CPU(randMat, y, tempBuf);
                sAXPY_CPU(numRows, x, tempBuf, -1.f, tempBuf);
                float normCPU = sNRM2_CPU(numRows, tempBuf);
                if (normCPU < numRows*1e-6f) {
                    LINFO("CPU: rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": passed (error: " << normCPU << ", iterations: " << iterations << ", duration: " << duration << " sec)");
                    numSucc++;
                }
                else {
                    LWARNING("CPU: rows=" << randMat.getNumRows() << " colsPerRow= " << randMat.getNumColsPerRow() << ": FAILED (" << normCPU << ")");
                }
                numTests++;
            }

            delete[] x;
            delete[] y;
            delete[] tempBuf;
        }

    }

    if (numSucc == numTests) {
        LINFO("hSpConjGrad-ELL Randomized Tests (passed/overall): " << numSucc << "/" << numTests);
        return true;
    }
    else {
        LWARNING("hSpConjGrad-ELL Randomized Tests (passed/overall): " << numSucc << "/" << numTests);
        return false;
    }

}
void VoreenBlas::randomizeEll(EllMatrix<float>& mat) {

    tgt::vec2 range(-5.f, 10.f);

    for (size_t row=0; row < mat.getNumRows(); ++row) {
        // leave 1/100 rows empty
        if ((rand() % 100) == 0)
            continue;

        // fill 1/10 of the cols only partially
        size_t significantCols = tgt::iround(10.f * (static_cast<float>(rand()) / RAND_MAX) * mat.getNumColsPerRow() );
        significantCols = std::min(significantCols, mat.getNumColsPerRow());
        int lastCol = -1;
        for (size_t colIndex = 0; colIndex < significantCols; ++colIndex) {
            size_t col = (lastCol + 1) + tgt::iround( (static_cast<float>(rand()) / RAND_MAX) * (mat.getNumColsPerRow()-1 - lastCol));
            if (col < mat.getNumColsPerRow()) {
                float val = range.x + (static_cast<float>(rand()) / RAND_MAX) * (range.y - range.x);
                mat.setValueByIndex(row, col, colIndex, val);
                lastCol = col;
            }
        }
    }
}

void VoreenBlas::randomizeEll(EllMatrix<int16_t>& mat) {

    tgt::Vector2<int16_t> range(-(1<<15) + 1, (1<<15) - 1);

    for (size_t row=0; row < mat.getNumRows(); ++row) {
        // leave 1/100 rows empty
        if ((rand() % 100) == 0)
            continue;

        // fill 1/10 of the cols only partially
        size_t significantCols = tgt::iround(10.f * (static_cast<float>(rand()) / RAND_MAX) * mat.getNumColsPerRow() );
        significantCols = std::min(significantCols, mat.getNumColsPerRow());
        int lastCol = -1;
        for (size_t colIndex = 0; colIndex < significantCols; ++colIndex) {
            size_t col = (lastCol + 1) + tgt::iround( (static_cast<float>(rand()) / RAND_MAX) * (mat.getNumColsPerRow()-1 - lastCol));
            if (col < mat.getNumColsPerRow()) {
                int16_t val = static_cast<int16_t>(range.x + (static_cast<float>(rand()) / RAND_MAX) * (range.y - range.x));
                mat.setValueByIndex(row, col, colIndex, val);
                lastCol = col;
            }
        }
    }
}

void VoreenBlas::randomizeEllPositiveDefinite(EllMatrix<float>& mat) {

    tgtAssert(mat.isQuadratic(), "quadratic matrix expected");

    // generate random volume
    int volDim = tgt::iceil(powf((float)mat.getNumRows(), 1.f/3.f));
    tgt::ivec3 dim(volDim, volDim, tgt::iceil(volDim*1.1f));
    const int numVoxels =  dim.x*dim.y*dim.z;
    const int numSeeds = numVoxels - mat.getNumRows();
    tgtAssert(numSeeds > 0, "Invalid seed count");

    uint8_t* volume = new uint8_t[numVoxels];
    bool* seeds = new bool[numVoxels];
    for (int i=0; i<numVoxels; i++) {
        volume[i] = tgt::ifloor((static_cast<float>(rand()) / RAND_MAX) * 255.f);
        seeds[i] = false;
    }

    int seedsGenerated = 0;
    while (seedsGenerated < numSeeds) {
        for (int i=0; i<numVoxels; i++) {
            if (seedsGenerated == numSeeds)
                break;
            if (!seeds[i] && ((rand() % 10) == 0)) {
                seeds[i] = true;
                seedsGenerated++;
            }
        }
    }

    int* volIndexToRow = new int[numVoxels];
    int curRow = 0;
    for (int i=0; i<numVoxels; i++) {
        if (!seeds[i]) {
            volIndexToRow[i] = curRow;
            curRow++;
        }
        else {
            volIndexToRow[i] = -1;
        }
    }


    float maxWeightSum = 0.f;
    tgt::ivec3 coords;
    for (int z=0; z<dim.z; z++) {
        for (int y=0; y<dim.y; y++) {
            for (int x=0; x<dim.x; x++) {

                int index = volumeCoordsToIndex(x, y, z, dim);
                if (seeds[index])
                    continue;

                size_t row = volIndexToRow[index];
                tgtAssert(row < mat.getNumCols(), "Invalid row");

                float weightSum = 0.f;
                int curIntensity = volume[index];

                // x-neighbors
                if (mat.getNumColsPerRow() >= 3) {
                    if (x > 0) {
                        int x0 = volumeCoordsToIndex(x-1, y, z, dim);
                        float weight = (std::abs(curIntensity - volume[x0]) + 1) / 255.f;
                        weightSum += weight;
                        if (!seeds[x0]) {
                            size_t rowX0 = volIndexToRow[x0];
                            tgtAssert(rowX0 < mat.getNumCols(), "Invalid row");
                            mat.setValue(row, rowX0, -weight);
                            mat.setValue(rowX0, row, -weight);
                        }
                    }
                    if (x < dim.x-1) {
                        size_t x1 = volumeCoordsToIndex(x+1, y, z, dim);
                        float weight = (std::abs(curIntensity - volume[x1]) + 1) / 255.f;
                        weightSum += weight;
                        if (!seeds[x1]) {
                            size_t rowX1 = volIndexToRow[x1];
                            tgtAssert(rowX1 < mat.getNumCols(), "Invalid row");
                            mat.setValue(row, rowX1, weight);
                            mat.setValue(rowX1, row, weight);
                        }
                    }
                }

                // y-neighbors
                if (mat.getNumColsPerRow() >= 5) {
                    if (y > 0) {
                        size_t y0 = volumeCoordsToIndex(x, y-1, z, dim);
                        float weight = std::abs(curIntensity - volume[y0]) / 255.f;
                        weightSum += weight;
                        if (!seeds[y0]) {
                            size_t rowY0 = volIndexToRow[y0];
                            tgtAssert(rowY0 < mat.getNumCols(), "Invalid row");
                            mat.setValue(row, rowY0, -weight);
                            mat.setValue(rowY0, row, -weight);
                        }
                    }
                    if (y < dim.y-1) {
                        int y1 = volumeCoordsToIndex(x, y+1, z, dim);
                        float weight = std::abs(curIntensity - volume[y1]) / 255.f;
                        weightSum += weight;
                        if (!seeds[y1]) {
                            size_t rowY1 = volIndexToRow[y1];
                            tgtAssert(rowY1 < mat.getNumCols(), "Invalid row");
                            mat.setValue(row, rowY1, weight);
                            mat.setValue(rowY1, row, weight);
                        }
                    }
                }

                // z-neighbors
                if (mat.getNumColsPerRow() >= 7) {
                    if (z > 0) {
                        size_t z0 = volumeCoordsToIndex(x, y, z-1, dim);
                        float weight = std::abs(curIntensity - volume[z0]) / 255.f;
                        weightSum += weight;
                        if (!seeds[z0]) {
                            size_t rowZ0 = volIndexToRow[z0];
                            tgtAssert(rowZ0 < mat.getNumCols(), "Invalid row");
                            mat.setValue(row, rowZ0, -weight);
                            mat.setValue(rowZ0, row, -weight);
                        }
                    }
                    if (z < dim.z-1) {
                        size_t z1 = volumeCoordsToIndex(x, y, z+1, dim);
                        float weight = std::abs(curIntensity - volume[z1]) / 255.f;
                        weightSum += weight;
                        if (!seeds[z1]) {
                            size_t rowZ1 = volIndexToRow[z1];
                            tgtAssert(rowZ1 < mat.getNumCols(), "Invalid row");
                            mat.setValue(row, rowZ1, weight);
                            mat.setValue(rowZ1, row, weight);
                        }
                    }
                }

                mat.setValue(row, row, weightSum);
                maxWeightSum = std::max(maxWeightSum, weightSum);
            }
        }
    }

    delete[] seeds;
    delete[] volume;
    delete[] volIndexToRow;
}

void VoreenBlas::randomizeEllPositiveDefinite(EllMatrix<int16_t>& mat) {
    tgtAssert(mat.isQuadratic(), "quadratic matrix expected");

    // generate random volume
    int volDim = tgt::iceil(powf((float)mat.getNumRows(), 1.f/3.f));
    tgt::ivec3 dim(volDim, volDim, tgt::iceil(volDim*1.1f));
    const int numVoxels =  dim.x*dim.y*dim.z;
    const int numSeeds = numVoxels - mat.getNumRows();
    tgtAssert(numSeeds > 0, "Invalid seed count");

    uint8_t* volume = new uint8_t[numVoxels];
    bool* seeds = new bool[numVoxels];
    for (int i=0; i<numVoxels; i++) {
        volume[i] = tgt::ifloor((static_cast<float>(rand()) / RAND_MAX) * 255.f);
        seeds[i] = false;
    }

    int seedsGenerated = 0;
    while (seedsGenerated < numSeeds) {
        for (int i=0; i<numVoxels; i++) {
            if (seedsGenerated == numSeeds)
                break;
            if (!seeds[i] && ((rand() % 10) == 0)) {
                seeds[i] = true;
                seedsGenerated++;
            }
        }
    }

    int* volIndexToRow = new int[numVoxels];
    int curRow = 0;
    for (int i=0; i<numVoxels; i++) {
        if (!seeds[i]) {
            volIndexToRow[i] = curRow;
            curRow++;
        }
        else {
            volIndexToRow[i] = -1;
        }
    }


    int* diagonal = new int[mat.getNumCols()];
    float maxValue = static_cast<float>((1<<15) - 1);

    int maxWeightSum = 0;
    tgt::ivec3 coords;
    for (int z=0; z<dim.z; z++) {
        for (int y=0; y<dim.y; y++) {
            for (int x=0; x<dim.x; x++) {

                int index = volumeCoordsToIndex(x, y, z, dim);
                if (seeds[index])
                    continue;

                size_t row = volIndexToRow[index];
                tgtAssert(row < mat.getNumCols(), "Invalid row");

                int weightSum = 0;
                int16_t curIntensity = volume[index];

                // x-neighbors
                if (mat.getNumColsPerRow() >= 3) {
                    if (x > 0) {
                        size_t x0 = volumeCoordsToIndex(x-1, y, z, dim);
                        int16_t weight = std::abs(curIntensity - volume[x0]) + 1;
                        weightSum += (int)weight;
                        if (!seeds[x0]) {
                            size_t rowX0 = volIndexToRow[x0];
                            tgtAssert(rowX0 < mat.getNumCols(), "Invalid row");
                            mat.setValue(row, rowX0, -weight);
                            mat.setValue(rowX0, row, -weight);
                        }
                    }
                    if (x < dim.x-1) {
                        size_t x1 = volumeCoordsToIndex(x+1, y, z, dim);
                        int16_t weight = std::abs(curIntensity - volume[x1]) + 1;
                        weightSum += (int)weight;
                        if (!seeds[x1]) {
                            size_t rowX1 = volIndexToRow[x1];
                            tgtAssert(rowX1 < mat.getNumCols(), "Invalid row");
                            mat.setValue(row, rowX1, weight);
                            mat.setValue(rowX1, row, weight);
                        }
                    }
                }

                // y-neighbors
                if (mat.getNumColsPerRow() >= 5) {
                    if (y > 0) {
                        size_t y0 = volumeCoordsToIndex(x, y-1, z, dim);
                        int16_t weight = std::abs(curIntensity - volume[y0]);
                        weightSum += (int)weight;
                        if (!seeds[y0]) {
                            size_t rowY0 = volIndexToRow[y0];
                            tgtAssert(rowY0 < mat.getNumCols(), "Invalid row");
                            mat.setValue(row, rowY0, -weight);
                            mat.setValue(rowY0, row, -weight);
                        }
                    }
                    if (y < dim.y-1) {
                        size_t y1 = volumeCoordsToIndex(x, y+1, z, dim);
                        int16_t weight = std::abs(curIntensity - volume[y1]);
                        weightSum += (int)weight;
                        if (!seeds[y1]) {
                            size_t rowY1 = volIndexToRow[y1];
                            tgtAssert(rowY1 < mat.getNumCols(), "Invalid row");
                            mat.setValue(row, rowY1, weight);
                            mat.setValue(rowY1, row, weight);
                        }
                    }
                }

                // z-neighbors
                if (mat.getNumColsPerRow() >= 7) {
                    if (z > 0) {
                        size_t z0 = volumeCoordsToIndex(x, y, z-1, dim);
                        int16_t weight = std::abs(curIntensity - volume[z0]);
                        weightSum += (int)weight;
                        if (!seeds[z0]) {
                            size_t rowZ0 = volIndexToRow[z0];
                            tgtAssert(rowZ0 < mat.getNumCols(), "Invalid row");
                            mat.setValue(row, rowZ0, -weight);
                            mat.setValue(rowZ0, row, -weight);
                        }
                    }
                    if (z < dim.z-1) {
                        size_t z1 = volumeCoordsToIndex(x, y, z+1, dim);
                        int16_t weight = std::abs(curIntensity - volume[z1]);
                        weightSum += (int)weight;
                        if (!seeds[z1]) {
                            size_t rowZ1 = volIndexToRow[z1];
                            tgtAssert(rowZ1 < mat.getNumCols(), "Invalid row");
                            mat.setValue(row, rowZ1, weight);
                            mat.setValue(rowZ1, row, weight);
                        }
                    }
                }

                //mat.setValue(row, row, weightSum);
                diagonal[row] = weightSum;
                maxWeightSum = std::max(weightSum, maxWeightSum);
            }
        }
    }

    float scale = maxValue / maxWeightSum;
    for (size_t i=0; i<mat.getNumRows()*mat.getNumColsPerRow(); i++)
        mat.getMatrix()[i] = static_cast<int16_t>(mat.getMatrix()[i] * scale);

    for (size_t row=0; row<mat.getNumRows(); row++) {
        mat.setValue(row, row, static_cast<int16_t>(diagonal[row] * scale));
    }


    delete[] diagonal;
    delete[] seeds;
    delete[] volume;
    delete[] volIndexToRow;
}

bool VoreenBlas::cmpVectors(size_t vecSize, float* vecx, float* vecy, float relThresh) {
    for (size_t i=0; i<vecSize; ++i) {
        if ( ((std::abs(vecx[i]) > 1e2f*relThresh) || (std::abs(vecy[i]) > 1e2f*relThresh)) &&
             (std::abs((vecx[i] / vecy[i]) - 1.f) > relThresh) ) {
            return false;
        }
    }
    return true;
}

}   // namespace
