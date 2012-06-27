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

#include "voreen/core/io/siemensreader.h"

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>

#include "tgt/exception.h"
#include "tgt/logmanager.h"
#include "tgt/vector.h"

#include "voreen/core/volume/volumeatomic.h"

using std::string;
using std::vector;
using std::ifstream;
using std::ios;
using std::ostream;

namespace voreen {

const std::string SiemensReader::loggerCat_ = "voreen.io.VolumeReader.siemens";

SiemensReader::SiemensReader() {
    name_ = "Siemens 3DUS Reader";
    extensions_.push_back("hdr");
}

VolumeSet* SiemensReader::read(const std::string &fname)
    throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{
    tgt::ivec3 dimensions;
    std::string fileName;
    size_t pos;

   if ((pos = fname.rfind("hdr")) != string::npos) {
       readInfoFile(fname);

       dimensions = tgt::ivec3(datasetXRes_, datasetYRes_, datasetZRes_);

       fileName = fname;
       if (pos != string::npos)
           fileName.replace(pos, 3, "img");
   }
   else
       LERROR("unknown filename " << fname);

   if (dimensions == tgt::ivec3::zero) {
       LERROR("Wrong dimensions acquired from 3DUS info file: " << fname);
       return 0;//new VolumeContainer();
   }

   LINFO("File data dimensions: "
         << dimensions.x << "x" << dimensions.y << "x" << dimensions.z);

   std::fstream fin(fileName.c_str(), std::ios::in | std::ios::binary);
   if (!fin.good()) {
       LWARNING("Can't open file: " << fileName);
       return 0;//new VolumeContainer();
   }

   LINFO("Loading a 8bit Siemens US dataset");

   tgt::vec3 spacing(1.0f, 1.0f, static_cast<float>(dimensions.x)/dimensions.z);

   VolumeUInt8* dataset = new VolumeUInt8(dimensions, spacing);

   fin.read(reinterpret_cast<char*>(dataset->voxel()), dataset->getNumBytes());
   LDEBUG("read " << fin.gcount() << " bytes.");

   if (fin.eof())
       LWARNING("File doesn't contain enough data!");

   fin.close();

   VolumeSet* volumeSet = new VolumeSet(tgt::FileSystem::fileName(fileName));
   VolumeSeries* volumeSeries = new VolumeSeries("unknown", Modality::MODALITY_UNKNOWN);
   volumeSet->addSeries(volumeSeries);
   VolumeHandle* volumeHandle = new VolumeHandle(dataset, 0.0f);
   volumeHandle->setOrigin(fileName, "unknown", 0.0f);
   volumeSeries->addVolumeHandle(volumeHandle);
   return volumeSet;
}

vector<string> SiemensReader::splitLine(string s, const char *ch) {
    vector<string> res;
    size_t offset = 0, idx = 0;

    while ((idx = s.find(ch, offset)) != string::npos) {
        string s_tmp = s.substr(offset, idx - offset);
        // remove quotation marks
        string s2_tmp = deleteAll(s_tmp, "\"");
        res.push_back(s2_tmp);
        offset = idx + 1;
    }
    string last = s.substr(offset);
    res.push_back(deleteAll(last, "\""));

    return res;
}

// delete all occurences of a string c in the string s.
std::string& SiemensReader::deleteAll(string& s, const string& c) {
    size_t idx = 0;

    while ((idx = s.find(c, idx)) != string::npos) {
        s.erase(idx, c.size());
    }
    return s;
}

void SiemensReader::readInfoFile(const std::string& fname) {
    LINFO("reading Info File: " << fname);

    ifstream in(fname.c_str());
    if (!in) {
        LERROR("Cannot open input file " << fname);
        return;
    }

    string s;
    if (getline(in, s)) {
        vector<string> v = splitLine(s, " ");

        if (v.size()) {
            datasetXRes_ = atoi(v[0].c_str());
            datasetYRes_ = atoi(v[1].c_str());
            datasetZRes_ = atoi(v[2].c_str());
        }
    }
}

} // namespace voreen
