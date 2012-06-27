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

#include "voreen/core/io/quadhidacvolumereader.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include "tgt/exception.h"

#include "voreen/core/volume/volumeatomic.h"

/*
// excerpt of quadhidac file
Title=Dynamic PET Image Sequence.
;% Using List-mode Expectation Maximisation

Subsets=98
Rho=1.200000
ntf=1
dxyr=0.400000
dt=[900]

nbx=150.000000
nby=150.000000
nbz=276.000000

FrameTime=900

-------------
quadhidac files with title KS PET have a mixed x- and y-dimension
*/

namespace voreen {

const std::string QuadHidacVolumeReader::loggerCat_ = "voreen.io.VolumeReader.QuadHidac";

VolumeCollection* QuadHidacVolumeReader::read(const std::string &fileName)
    throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{
    std::string title;
    int subsets;
    int ntf;
    float dxyr;
    float rho;
    int dt;
    tgt::ivec3 dims;
    tgt::ivec3 croppedDims(0);
    unsigned int frameTime;
    tgt::mat4 transformationMatrix = tgt::mat4::identity;

    std::fstream file(fileName.c_str(), std::ios::in | std::ios::binary);

    if (!file)
        throw new tgt::IOException();

    LINFO(fileName);
    char ch = 0;
    std::string typeStr;
    std::string argumentsStr;
    bool type = true;
    bool comment = false;
    bool error = false;
    int pos=-1;

    while ((ch != 0x0c) && (!file.eof())) {
        file.read(&ch, 1);

        if ((ch != 0x0c) && (ch != 0x0a)) {
            if (ch == '=') {
                type = false;
                LINFO("Type: " << typeStr);
            } else if (ch == ';') {
                comment = true;
                type = false;
            } else {
                if (type)
                    typeStr += ch;
                else
                    argumentsStr += ch;
            }
        } else {
            if (comment) {
                LINFO("Comment: " << argumentsStr);
            } else {
                std::stringstream argStream(argumentsStr);

                if (typeStr == "") {
                } else if (typeStr == "Title") {
                    title = argumentsStr;
                    pos = title.find("KS PET");
                    LINFO("Value: " << title);
                } else if (typeStr == "Subsets") {
                    argStream >> subsets;
                    LINFO("Value: " << subsets);
                } else if (typeStr == "Rho") {
                    argStream >> rho;
                    LINFO("Value: " << rho);
                } else if (typeStr == "ntf") {
                    argStream >> ntf;
                    LINFO("Value: " << ntf);
                } else if (typeStr == "dxyr") {
                    argStream >> dxyr;
                    LINFO("Value: " << dxyr);
                } else if (typeStr == "row1") {
                    argStream >> transformationMatrix.t00;
                    argStream >> transformationMatrix.t01;
                    argStream >> transformationMatrix.t02;
                    argStream >> transformationMatrix.t03;

                } else if (typeStr == "row2") {
                    argStream >> transformationMatrix.t10;
                    argStream >> transformationMatrix.t11;
                    argStream >> transformationMatrix.t12;
                    argStream >> transformationMatrix.t13;
                } else if (typeStr == "row3") {
                    argStream >> transformationMatrix.t20;
                    argStream >> transformationMatrix.t21;
                    argStream >> transformationMatrix.t22;
                    argStream >> transformationMatrix.t23;
                } else if (typeStr == "row4") {
                    argStream >> transformationMatrix.t30;
                    argStream >> transformationMatrix.t31;
                    argStream >> transformationMatrix.t32;
                    argStream >> transformationMatrix.t33;
                } else if (typeStr == "dt") {
                    //argStream >> dt;
                    dt = 0;
                    LWARNING("parsing of 'dt' type not implemented!");
                    LINFO("Value: " << dt);
                } else if (typeStr == "nbx") {
                    // if a KS PET file is read then pos = 0
                    if (pos == -1){
                        argStream >> dims.x;
                        LINFO("Value: " << dims.x);

                    }else { argStream >> dims.y;
                        LINFO("Value: " << dims.y);
                        LINFO("nbx and nby are twisted");
                    }

                } else if (typeStr == "nby") {
                    //argStream >> dims.y;
                    if (pos == -1){
                        argStream >> dims.y;
                        LINFO("Value: " << dims.y);
                    }else { argStream >> dims.x;
                        LINFO("Value: " << dims.x);
                    }
                } else if (typeStr == "nbz") {
                    argStream >> dims.z;
                    LINFO("Value: " << dims.z);

                } else if (typeStr == "croppednbx") {
                    // if a KS PET file is read then pos = 0
                    if (pos == -1) {
                        argStream >> croppedDims.x;
                    } else {
                        argStream >> croppedDims.y;
                    }

                } else if (typeStr == "croppednby") {
                    // if a KS PET file is read then pos = 0
                    if (pos == -1) {
                        argStream >> croppedDims.y;
                    } else {
                        argStream >> croppedDims.x;
                    }

                } else if (typeStr == "croppednbz") {
                    argStream >> croppedDims.z;

                } else if (typeStr == "FrameTime") {
                    argStream >> frameTime;
                    LINFO("Value: " << frameTime);
                } else {
                    LWARNING(typeStr << " is unknown (value(s): " << argumentsStr << ")");
                }

                if (argStream.fail()) {
                    LERROR("Format error");
                    error = true;
                }
            }

            typeStr = "";
            argumentsStr = "";
            type = true;
            comment = false;
        }
    }

    if (error)
        return 0;

    // if croppedDims is available, use it, nicer version is in work
    if (croppedDims.z > 0)
        dims = croppedDims;

    float* slice = new float[dims.x * dims.y];
    float min;
    float max;

    min = max = 0.0;

    uint16_t* scalars = new uint16_t[hmul(dims)];
    uint16_t* p = scalars;
    for (tgt::ivec3 i = tgt::ivec3::zero; i.z < dims.z; ++i.z) {
        file.read(reinterpret_cast<char*>(slice), dims.x*dims.y*4);

        for (i.y = 0; i.y < dims.y; ++i.y) {
            for (i.x = 0; i.x < dims.x; ++i.x) {
                float val;
                val = slice[i.y * dims.x + i.x]*10.0f;

                *p = static_cast<unsigned short>(floor(val + 0.5f));
                ++p;

                if (val < min)
                    min = val;

                if (val > max)
                    max = val;
            }
        }
    }

    // check whether we have a 12 or a 16 bit dataset
    int bits = max <= 4095.0f ? 12 : 16;
    VolumeUInt16* dataset = new VolumeUInt16(scalars, dims, tgt::vec3(dxyr), bits);
    dataset->setTransformation(transformationMatrix);

    LINFO("min/max value: " << min << "/" << max);
    if (min < 0.0f) {
        LERROR("Cannot handle negative values.");
    }

    // clean up
    file.close();
    delete[] slice;

    VolumeCollection* volumeCollection = new VolumeCollection();
    VolumeHandle* volumeHandle = new VolumeHandle(dataset, 0.0f);
    volumeCollection->add(volumeHandle);

    return volumeCollection;
}

} // namespace voreen
