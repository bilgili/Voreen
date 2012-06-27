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

#include "voreen/core/io/nrrdvolumereader.h"

#include <fstream>
#include <iostream>

#include "tgt/exception.h"
#include "tgt/vector.h"

#include "voreen/core/io/textfilereader.h"
#include "voreen/core/io/rawvolumereader.h"
#include "voreen/core/volume/volumeatomic.h"

using tgt::vec3;
using tgt::ivec3;

namespace voreen {

const std::string NrrdVolumeReader::loggerCat_ = "voreen.io.VolumeReader.nrrd";

VolumeCollection* NrrdVolumeReader::read(const std::string &fileName)
    throw(tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{
    std::string objectFilename = "";
    ivec3 resolution;
    vec3 sliceThickness;
    std::string format;
    std::string model;
    bool error = false;

    LINFO("NrrdVolumeReader: " << fileName);
    TextFileReader reader(fileName);

    if (!reader) {
        LERROR("Unable to open " << fileName);
        return 0;
    }

    std::string type;
    std::istringstream args;
    int bits = 0;

    std::string magic = reader.getMagicNumber();

    if (magic != "NRRD0001") {
        LERROR("Format error");
        return 0;
    }

    while (reader.getNextLine(type, args, false)) {
        LINFO("Type : " << type);

        if (type == "datafile:") {
            args >> objectFilename;
            LINFO("Value: " << objectFilename);
        } else if (type == "sizes:") {
            args >> resolution[0];
            args >> resolution[1];
            args >> resolution[2];
            LINFO("Value: " << resolution[0] << " x " <<
                  resolution[1] << " x " << resolution[2]);
        } else if (type == "spacings:") {
            args >> sliceThickness[0] >> sliceThickness[1] >> sliceThickness[2];
            LINFO("Value: " << sliceThickness[0] << " " <<
                  sliceThickness[1] << " " << sliceThickness[2]);
        } else if (type == "type:") {
            args >> format;
            if ( format == "uchar" ||
                format == "unsigned char" ||
                format == "uint8" ||
                format == "uint8_t") {
                    format = "UCHAR";
                    model = "I";
            } else if ( format == "ushort" ||
                format == "unsigned short" ||
                format == "unsigned short int" ||
                format == "uint16" ||
                format == "uint16_t") {
                    format = "USHORT";
                    model = "I";
                    break;
            } else if ( format == "float") {
                    format = "FLOAT16";
                    model = "I";
                    break;
            } else if ( format == "uint" ||
                format == "unsigned int" ||
                format == "uint32" ||
                format == "uint32_t") {
                    format = "UCHAR";
                    model = "RGBA";
                    break;
            }
            LINFO("Value: " << format);
        } else {
            LWARNING("Unknown type: " << type);
        }
        //... "encoding:" and other value-types should be added
        if (args.fail()) {
            LERROR("Format error");
            error = true;
        }
    }

    if (!error) {
        RawVolumeReader rawReader;
        rawReader.readHints(resolution, sliceThickness, bits, model, format);

        // do we have a relative path?
        if ((objectFilename.substr(0,1) != "/")  && (objectFilename.substr(0,1) != "\\") &&
            (objectFilename.substr(1,2) != ":/") && (objectFilename.substr(1,2) != ":\\"))
        {
            size_t p = fileName.find_last_of("\\");
            if (p == std::string::npos)
                p = fileName.find_last_of("/");

            // construct path relative to dat file
            objectFilename = fileName.substr(0, p + 1) + objectFilename;
        }

        VolumeCollection* collection = rawReader.read(objectFilename);
        if (!collection->empty())
            collection->first()->setOrigin(VolumeOrigin(fileName));
        return collection;

    } else
        return 0;
}

} // namespace voreen
