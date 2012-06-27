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

/**
 * Single-precision vector sum: z = alpha*X + Y;
 */
__kernel void sAXPY(
  unsigned int n,
  __global const float* X,
  __global const float* Y,
  float alpha,
  __global float* z
)
{
    // get work item id
    unsigned int gid = get_global_id(0);
    unsigned int globalSize = get_global_size(0);

    int block = 0;
    while ((block*globalSize+gid) < n) {
        z[block*globalSize+gid] = alpha*X[block*globalSize+gid] + Y[block*globalSize+gid];
        block++;
    }
}

/**
 * Single-precision dot product: z = X*Y
 */
__kernel void sDOT(
  unsigned int n,
  __global const float* X,
  __global const float* Y,
  __global float* z,
  __global int* mutex,
  __local float* sdata
)
{
    // get local (within workgroup) and global indices of workitem
    unsigned int tid = get_local_id(0);
    unsigned int gid = get_global_id(0);
    unsigned int globalSize = get_global_size(0);

    // Each work-item accumulates as many products as necessary
    // and writes the result to shared workgroup mem
    float partialProduct = 0.0;
    int block = 0;
    while ((block*globalSize+gid) < n) {
        partialProduct += X[block*globalSize+gid]*Y[block*globalSize+gid];
        block++;
    }
    sdata[tid] = partialProduct;

    // synchronize all work items before reduction
    barrier(CLK_LOCAL_MEM_FENCE);

    // sum up component products within workgroup by reduction in shared mem
    for (unsigned int s=1; s < get_local_size(0); s *= 2) {
        if (2*s*tid < get_local_size(0)) {
            sdata[2*s*tid] += sdata[2*s*tid + s];
        }

        // synchronize workgroup after each reduction step
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    // first global item initializes result buffer and releases mutex
    if (gid==0) {
        z[0] = 0.0;
        atom_cmpxchg(mutex,0,1);
    }

    // no sdata[0] of each workgroup contains the sum of the workgroup's component products
    // => first item of each workgroup adds this to the final result
    if (tid == 0) {
        while (atom_cmpxchg(mutex,1,0)==0);  // acquire mutex
        z[0] += sdata[0];
        atom_cmpxchg(mutex,0,1);             // release mutex
    }
}

/**
 * Single-precision vector 2-norm (eclidean distance): y = ||X||_2
 */
__kernel void sNRM2(
  unsigned int n,
  __global const float* X,
  __global float* y,
  __global int* mutex,
  __global int* semaphor,
  __local float* sdata
)
{
    // get local (within workgroup) and global indices of workitem
    unsigned int tid = get_local_id(0);
    unsigned int gid = get_global_id(0);
    unsigned int globalSize = get_global_size(0);
    unsigned int numGroups = get_num_groups(0);

    // Each work-item accumulates as many products as necessary
    // and writes the result to shared workgroup mem
    float partialProduct = 0.0;
    int block = 0;
    while ((block*globalSize+gid) < n) {
        partialProduct += X[block*globalSize+gid]*X[block*globalSize+gid];
        block++;
    }
    sdata[tid] = partialProduct;

    // synchronize all work items before reduction
    barrier(CLK_LOCAL_MEM_FENCE);

    // sum up component products within workgroup by reduction in shared mem
    for (unsigned int s=1; s < get_local_size(0); s *= 2) {
        if (2*s*tid < get_local_size(0)) {
            sdata[2*s*tid] += sdata[2*s*tid + s];
        }

        // synchronize workgroup after each reduction step
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    // first global item initializes result buffer/semaphor and releases mutex
    if (gid==0) {
        y[0] = 0.0;
        semaphor[0] = 0;
        atom_cmpxchg(mutex,0,1);
    }

    // no sdata[0] of each workgroup contains the sum of the workgroup's component products
    // => first item of each workgroup adds this to the final result
    if (tid == 0) {
        while (atom_cmpxchg(mutex,1,0)==0);  // acquire mutex
        y[0] += sdata[0];                    // add workgroup result to final result
        semaphor[0]++;                       // count number of groups
        if (semaphor[0] == numGroups)        // all groups have passed => compute final result
            y[0] = sqrt(y[0]);
        atom_cmpxchg(mutex,0,1);             // release mutex
    }
}

/**
 * Single-precision sparse matrix-vector multiplication
 * for a matrix stored in ELLPACK format: y = M*x;
 */
__kernel void sSpMV_Ell(
  unsigned int num_rows,
  unsigned int num_cols,
  unsigned int num_cols_per_row,
  __global const int* indices,
  __global const float* M,
  __global const float* x,
  __global float* y
)
{

    // get work item id
    unsigned int gid = get_global_id(0);
    unsigned int globalSize = get_global_size(0);

    // each work item iterates over a range of rows and computes the dot products
    int block = 0;
    while ((block*globalSize+gid) < num_rows) {
        unsigned int row = block*globalSize+gid;
        float dot = 0.0;
        // iterate over cols of current matrix row
        for (unsigned int n = 0; n < num_cols_per_row ; n++){
            //const unsigned int index = num_cols_per_row * row + n;  //< row-order
            unsigned int index = n*num_rows + row;
            dot += M[index] * x[indices[index]];
        }
        y[row] = dot;
        block++;
    }
}

/**
 * Sparse matrix-vector multiplication for a
 * half-precision matrix (16 bit integer)
 * stored in ELLPACK format: y = M*x;
 */
__kernel void hSpMV_Ell(
  unsigned int num_rows,
  unsigned int num_cols,
  unsigned int num_cols_per_row,
  __global const int* indices,
  __global const short* M,
  __global const float* x,
  __global float* y
)
{

    // get work item id
    unsigned int gid = get_global_id(0);
    unsigned int globalSize = get_global_size(0);

    float valueScale = 1.0 / (float)((1<<15) - 1);

    // each work item iterates over a range of rows and computes the dot products
    int block = 0;
    while ((block*globalSize+gid) < num_rows) {
        unsigned int row = block*globalSize+gid;
        float dot = 0.0;
        // iterate over cols of current matrix row
        for (unsigned int n = 0; n < num_cols_per_row ; n++){
            //const unsigned int index = num_cols_per_row * row + n;  //< row-order
            unsigned int index = n*num_rows + row;
            dot += (M[index]*valueScale) * x[indices[index]];
        }
        y[row] = dot;
        block++;
    }
}

/**
 * Single-precision general inner product for a sparse matrix
 * stored in ELLPACK format: z = x^t*A*y;
 */
__kernel void sInnerProduct_Ell(
  unsigned int num_rows,
  unsigned int num_cols,
  unsigned int num_cols_per_row,
  __global const int* indices,
  __global const float* A,
  __global const float* x,
  __global const float* y,
  __global float* z,
  __global int* mutex,
  __local float* sdata
)
{

    // get work item id
    unsigned int tid = get_local_id(0);
    unsigned int gid = get_global_id(0);
    unsigned int globalSize = get_global_size(0);

    // each work item iterates over a range of rows and computes the dot products
    int block = 0;
    float partialProduct = 0.0;
    while ((block*globalSize+gid) < num_rows) {
        int row = block*globalSize+gid;
        float dot = 0.0;
        // iterate over cols of current matrix row
        for (unsigned int n = 0; n < num_cols_per_row ; n++){
            //unsigned int index = num_cols_per_row * row + n;  //< row-order
            unsigned int index = n*num_rows + row;
            int col = indices[index];
            dot += A[index] * y[col];
        }
        partialProduct += dot * x[row];
        block++;
    }
    sdata[tid] = partialProduct;

    // synchronize all work items before reduction
    barrier(CLK_LOCAL_MEM_FENCE);

    // sum up component products within workgroup by reduction in shared mem
    for (unsigned int s=1; s < get_local_size(0); s *= 2) {
        if (2*s*tid < get_local_size(0)) {
            sdata[2*s*tid] += sdata[2*s*tid + s];
        }

        // synchronize workgroup after each reduction step
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    // first global item initializes result buffer and releases mutex
    if (gid==0) {
        z[0] = 0.0;
        atom_cmpxchg(mutex,0,1);
    }

    // no sdata[0] of each workgroup contains the sum of the workgroup's component products
    // => first item of each workgroup adds this to the final result
    if (tid == 0) {
        while (atom_cmpxchg(mutex,1,0)==0);  // acquire mutex
        z[0] += sdata[0];
        atom_cmpxchg(mutex,0,1);             // release mutex
    }
}
