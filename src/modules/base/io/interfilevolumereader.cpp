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

#include "voreen/modules/base/io/interfilevolumereader.h"

#include <fstream>

#include "tgt/exception.h"
#include "tgt/logmanager.h"

#include "voreen/core/io/rawvolumereader.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumeoperator.h"

namespace voreen {

const std::string InterfileVolumeReader::loggerCat_("voreen.io.VolumeReader.hv");

InterfileVolumeReader::InterfileVolumeReader()  {
    extensions_.push_back("hv");
}

VolumeCollection* InterfileVolumeReader::read(const std::string &url)
    throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{
    VolumeOrigin origin(url);
    std::string fileName = origin.getPath();

    std::string imageDataFilename;
    tgt::ivec3 resolution;
    tgt::vec3 sliceThickness;
    std::string format;
    std::string bytesPerPixel;
    std::string axis_label;
    std::string dataFileName;
    float axial_pos;
    bool error = false;

    LINFO("InterfileVolumeReader: " << fileName);
    std::istream* file_ = new std::ifstream(fileName.c_str());
    if (!(*file_))
        throw tgt::IOException();

    char line[256];
    while (file_->good()) {
        //skip leading whitespaces
        *file_ >> std::ws;
        //fetch next line
        file_->getline(line, sizeof(line));
        //skip comment line
        if (line[0] == ';')
            continue;

        std::string string_line = (std::string)line;
        //separate line in 2 parts
        size_t pos = string_line.find(":=", 1);
        if (pos != std::string::npos) {
            //get key and value strings
            std::string key = removeEnclosingWhitespaces(string_line.substr(0, pos));
            std::istringstream value(removeEnclosingWhitespaces(string_line.substr(pos + 2)));
            if (key[0] == '!')
                key = key.substr(1, key.length());
            if (key == "name of data file") {
                LINFO("Type : " << key);
                value >> dataFileName;
                LINFO("Value: " << dataFileName);
            }
            else if (key == "number format") {
                LINFO("Type : " << key);
                value >> format;
                LINFO("Value: " << format);
            }
            else if (key == "number of bytes per pixel") {
                LINFO("Type : " << key);
                value >> bytesPerPixel;
                LINFO("Value: " << bytesPerPixel);
            }
            else if ((key == "matrix axis label [1]") ||
                     (key == "matrix axis label [2]") ||
                     (key == "matrix axis label [3]")) {
                value >> axis_label;
            }
            else if ((key == "matrix size [1]") ||
                     (key == "matrix size [2]") ||
                     (key == "matrix size [3]")) {
                if (axis_label == "x")
                    value >> resolution[0];
                else if (axis_label == "y")
                    value >> resolution[1];
                else if (axis_label == "z") {
                    LINFO("Type: " << "Resolution");
                    value >> resolution[2];
                    LINFO("Value: " << resolution);
                }
            }
            else if ((key == "scaling factor (mm/pixel) [1]") ||
                     (key == "scaling factor (mm/pixel) [2]") ||
                     (key == "scaling factor (mm/pixel) [3]"))
            {
                if (axis_label == "x")
                    value >> sliceThickness[0];
                else if (axis_label == "y")
                    value >> sliceThickness[1];
                else if (axis_label == "z") {
                    LINFO("Type: " << "SliceThickness");
                    value >> sliceThickness[2];
                    LINFO("Value: " << sliceThickness);
                }
            }
            else if (key == "axial position") {
                LINFO("Type: " << "Axial Position");
                value >> axial_pos;
                LINFO("Value: " << axial_pos);
            }
            else if (key == "!END OF INTERFILE") {
                break;
            } else if (key == "") {
                error = true;
                LERROR("Format error");
            }
        }
    }
    if (!error) {
        std::transform(format.begin(), format.end(), format.begin(), toupper);
        int bitsStored = static_cast<int>(pow(2.f, atoi(bytesPerPixel.c_str())));
        if (format == "FLOAT") {
            std::ostringstream oss;

            oss << bitsStored;
            format = format + oss.str();
        }
        RawVolumeReader rawReader;
        rawReader.setReadHints(resolution, sliceThickness, bitsStored, "I", format);

        if ((dataFileName.substr(0,1) != "/")  && (dataFileName.substr(0,1) != "\\") &&
            (dataFileName.substr(1,2) != ":/") && (dataFileName.substr(1,2) != ":\\"))
        {
            size_t p = fileName.find_last_of("\\");
            if (p == std::string::npos)
                p = fileName.find_last_of("/");

            // construct path relative to hv file
            dataFileName = fileName.substr(0, p + 1) + dataFileName;
        }

        VolumeCollection* volumeCollection = 0;
        try {
            volumeCollection = rawReader.read(dataFileName);
        }
        catch (...) {
            throw; // throw it to the caller
        }

        for (size_t i = 0; i < volumeCollection->size(); ++i) {
            Volume* volume = volumeCollection->at(i)->getVolume();
            if (volume == 0)
                continue;

            // This should mirror the z-axis, create a new Volume
            // and replace the one in the VolumeHandle with the
            // mirrored one. New hardware volumes should also be
            // created. (dirk)
            //
            volume->meta().setImagePositionZ(axial_pos);
            // replace the existing Volume* by the mirrored
            // one by deleting the old, non-mirrored Volume*.
            //
            volume = volume->clone();
            VolumeOperatorMirrorZ mirrorZ;
            mirrorZ.apply<void>(volume);
            volumeCollection->at(i)->setVolume(volume);
        }
        return volumeCollection;
    }
    else
        throw tgt::CorruptedFileException();
}


std::string InterfileVolumeReader::removeEnclosingWhitespaces(const std::string &s) {
    size_t pos_first = s.find_first_not_of(" \f\n\r\t\v");
    if (pos_first != std::string::npos)
        return s.substr(pos_first, s.find_last_not_of(" \f\n\r\t\v") - pos_first + 1);
    else
        // all characters are white spaces
        return "";
}

VolumeReader* InterfileVolumeReader::create(ProgressBar* /*progress*/) const {
    return new InterfileVolumeReader(/*progress*/);
}

} //namespace voreen
