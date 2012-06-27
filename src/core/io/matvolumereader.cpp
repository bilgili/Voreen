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

#include "voreen/core/io/matvolumereader.h"
#include "voreen/core/volume/volumeatomic.h"

#include <fstream>
#include <iostream>

#include "tgt/exception.h"
#include "tgt/vector.h"

using std::string;
using tgt::vec3;
using tgt::ivec3;

namespace voreen {

const std::string MatVolumeReader::loggerCat_("voreen.io.MatVolumeReader");

MatVolumeReader::MatVolumeReader() {
     name_ = "Mat Reader";
     extensions_.push_back("mat");
}

MatVolumeReader::~MatVolumeReader() {
}

VolumeSet* MatVolumeReader::read(const std::string& fileName)
    throw(tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{
    LINFO("MatVolumeReader: Trying to parse " << fileName << "...");
    MATFile* pmat;
    pmat = matOpen(fileName.c_str(), "r");
    if (pmat == 0) {
        LERROR("Can't open file " << fileName);
        throw tgt::IOException("Can't open file.");
    }

    // Get directory
    char** dir;
    int numDir;
    dir = matGetDir(pmat, &numDir);
    if (dir == 0) {
        LERROR("Can't read file directory.");
        throw tgt::IOException("Can't read file directory.");
    }
    for (int i=0; i<numDir; ++i)
        LINFO("Found array " << dir[i]);
    // In order to use matGenNextXXX correctly, we need to reopen the file.
    matClose(pmat);
    pmat = matOpen(fileName.c_str(), "r");

    VolumeSet* volSet = new VolumeSet(tgt::FileSystem::fileName(fileName));

    // Get headers of all variables.
    for (int i=0; i<numDir; ++i) {
        const char *name;
        mxArray *pa = matGetNextVariable(pmat, &name);
        if (pa == 0) {
            LERROR("Can't read info for variable " << dir[i]);
            throw tgt::CorruptedFileException();
        }

        readMatFile(pa, volSet, dir[i]);
    }
    mxFree(dir);
    return volSet;
}

void MatVolumeReader::readMatFile(mxArray* pa, VolumeSet* volSet, char* name) {
    LINFO("Details for " << name);
    int dims = mxGetNumberOfDimensions(pa);
    LINFO("  has " << dims << " dimensions.");
    const MatSizeType *matDim = mxGetDimensions(pa);
    int numElems = 1;
    for (int j=0; j<dims; ++j)
        numElems *= matDim[j];

    switch (mxGetClassID(pa)) {
        case mxUNKNOWN_CLASS:
            LINFO("  The class cannot be determined.");
            LERROR(" This is currently not supported.");
            break;
        case mxCELL_CLASS: {
            LINFO("  cell mxArray. Trying to parse content.");
            for (MatSizeType j=0; j<matDim[0]*matDim[1]; ++j) {
                mxArray *cell = mxGetCell(pa, j);
                if (!cell) {
                    LERROR("Can't read cell " << j);
                    throw tgt::CorruptedFileException();
                }
                else {
                    readMatFile(cell, volSet, name);
                }
            }
            break;
        }
        case mxSTRUCT_CLASS:
            LINFO("  structure mxArray.");
            for (int j = 0; j < mxGetNumberOfFields(pa); ++j) {
                readMatFile(mxGetFieldByNumber(pa, 0, j), volSet, const_cast<char*>(mxGetFieldNameByNumber(pa, j)));
            }
            break;
        case mxCHAR_CLASS:
            LINFO("string mxArray.");
            LERROR("This format is currently not supported.");
            break;
        case mxLOGICAL_CLASS:
            LINFO("logical mxArray.");
            LERROR("This format is currently not supported.");
            break;
        case mxDOUBLE_CLASS:
            LINFO("double-precision, floating-point numbers.");
            readMatrix(pa, volSet, name);
            break;
        case mxSINGLE_CLASS:
            LINFO("single-precision, floating-point numbers.");
            LWARNING("The reader for this format is not tested.");
            readMatrix(pa, volSet, name);
            break;
        case mxINT8_CLASS:
            LINFO("signed 8-bit integers. ");
            readMatrix(pa, volSet, name);
            break;
        case mxUINT8_CLASS:
            LINFO("unsigned 8-bit integers. ");
            readMatrix(pa, volSet, name);
            break;
        case mxINT16_CLASS:
            LINFO("signed 16-bit integers. ");
            readMatrix(pa, volSet, name);
            break;
        case mxUINT16_CLASS:
            LINFO("unsigned 16-bit integers.");
            readMatrix(pa, volSet, name);
            break;
        case mxINT32_CLASS:
            LINFO("signed 32-bit integers.");
            readMatrix(pa, volSet, name);
            break;
        case mxUINT32_CLASS:
            LINFO("unsigned 32-bit integers.");
            readMatrix(pa, volSet, name);
            break;
        case mxINT64_CLASS:
            LINFO("signed 64-bit integers.");
            LERROR("This format is currently not supported.");
            break;
        case mxUINT64_CLASS:
            LINFO("unsigned 64-bit integers.");
            LERROR("This format is currently not supported.");
            break;
        case mxFUNCTION_CLASS:
            LINFO("function handle mxArray");
            LERROR("This format is currently not supported.");
            break;
        default:
            LERROR("  Unknown class type");
            LERROR("  This format is currently not supported.");
            break;
    }
}

void MatVolumeReader::readMatrix(mxArray* pa, VolumeSet* volSet, char* name) {
    int dims = mxGetNumberOfDimensions(pa);
    const MatSizeType* matDim = mxGetDimensions(pa);
    tgt::ivec3 dim(matDim[0], matDim[1], matDim[2]);
    int wMax;
    switch (dims) {
        case 3:
            wMax = 1;
            LINFO("  Matrix with 3 dimensions found.");
            break;
        case 4:
            wMax = matDim[3];
            LINFO("  Matrix with 4 dimensions found.");
            break;
        default:
            LERROR("  Matrix has " << dims << " dimensions. Currently only matrices with 3 or 4 dimensions are supported.");
            return;
    }
    VolumeSeries* series = new VolumeSeries(name);
    volSet->addSeries(series);
    for (int w=0; w<wMax; ++w) {
        LINFO("  Read and add volume " << w);
        Volume* dataset;
        switch (mxGetClassID(pa)) {
            case mxSINGLE_CLASS:
                dataset = readMatrix<float>(pa, dim, matDim, w);
                break;
            case mxDOUBLE_CLASS:
                dataset = readMatrix<double>(pa, dim, matDim, w);
                break;
            case mxINT8_CLASS:
                dataset = readMatrix<int8_t>(pa, dim, matDim, w);
                break;
            case mxUINT8_CLASS:
                dataset = readMatrix<uint8_t>(pa, dim, matDim, w);
                break;
            case mxINT16_CLASS:
                dataset = readMatrix<int16_t>(pa, dim, matDim, w);
                break;
            case mxUINT16_CLASS:
                dataset = readMatrix<uint16_t>(pa, dim, matDim, w);
                break;
            case mxINT32_CLASS:
                dataset = readMatrix<int32_t>(pa, dim, matDim, w);
                break;
            case mxUINT32_CLASS:
                dataset = readMatrix<uint32_t>(pa, dim, matDim, w);
                break;
            default:
                dataset = 0;
                break;
        }
        if(dataset) {
            VolumeHandle* handle = new VolumeHandle(dataset, static_cast<const float>(w));
            handle->setOrigin(
                    series->getParentSet()->getName(),
                    name,
                    static_cast<const float>(w));
            series->addVolumeHandle(handle);
        }
    }
}

} // namespace voreen
