/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#ifndef VRN_MATVOLUMEREADER_H
#define VRN_MATVOLUMEREADER_H

#include <limits>

#include "voreen/core/io/volumereader.h"
#include "voreen/core/volume/volume.h"
#include "voreen/core/volume/volumeatomic.h"

// Matlab header
#include "mat.h"

namespace voreen {

#if MX_API_VER <= 0x07020000
    typedef int MatSizeType;
#else
    typedef size_t MatSizeType;
#endif


/**
 * Reads a volume dataset from a MatLab dataset.
 */
class MatVolumeReader : public VolumeReader {
public:
    MatVolumeReader();
    ~MatVolumeReader();

    virtual VolumeSet* read(const std::string& fileName)
        throw(tgt::CorruptedFileException, tgt::IOException, std::bad_alloc);

    template<class T>
    static Volume* readMatrix(mxArray* pa, tgt::ivec3 dim, const MatSizeType* matDim, int w) {
        VolumeAtomic<T>* dataset;
        try {
            dataset = new VolumeAtomic<T>(dim);
        }
        catch (std::bad_alloc) {
            throw;
        }
        T* scalars = dataset->voxel();
        T* data = (T *)mxGetData(pa);
        for (int z=0; z<dim.z; ++z) {
            for (int y=0; y<dim.y; ++y) {
                for (int x=0; x<dim.x; ++x) {
                    int pos = z*dim.x*dim.y + y*dim.x + x;
                    int matPos = w*matDim[0]*matDim[1]*matDim[2] + z*matDim[0]*matDim[1] + y*matDim[0] + x;
                    scalars[pos] = data[matPos];
                }
            }
        }
        return dataset;
    }

protected:

    void readMatFile(mxArray* pa, VolumeSet* volSet, char* name);

    /**
     * Reads a MatLab array from memory.
     */
    void readMatrix(mxArray* pa, VolumeSet* volSet, char* name);

    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_MATVOLUMEREADER_H
