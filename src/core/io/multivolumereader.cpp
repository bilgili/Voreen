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

#include "voreen/core/io/multivolumereader.h"

#include <fstream>
#include <iostream>

#include "voreen/core/io/volumeserializer.h"
#include "voreen/core/io/volumeserializerpopulator.h"

using std::string;

namespace voreen {

const string MultiVolumeReader::loggerCat_ = "voreen.io.VolumeReader.mv";

MultiVolumeReader::MultiVolumeReader(VolumeSerializerPopulator* populator, IOProgress* progress)
    : VolumeReader(progress),
      populator_(populator)
{
    name_ = "MultiVolume Reader";
    extensions_.push_back("mv");
}

VolumeSet* MultiVolumeReader::read(const std::string& fileName)
    throw (tgt::FileException, std::bad_alloc)
{
    std::fstream fin(fileName.c_str(), std::ios::in | std::ios::binary);
    if (!fin.good() || fin.eof() || !fin.is_open())
        throw tgt::FileNotFoundException("Unable to open mv file for reading", fileName);

    VolumeSet* volumeSet = new VolumeSet(tgt::File::fileName(fileName));

    // extract file path from file name
    string path;
    if (fileName.rfind("/") != string::npos)
        path = fileName.substr(0, fileName.rfind("/") + 1);
    else if (fileName.rfind("\\") != string::npos)
        path = fileName.substr(0, fileName.rfind("\\") + 1);

    do {
        string line;
        getline(fin, line);

        if (line.empty())
            continue;

        // If the line was delimited by a '\r\n' the '\r' will still be the last character
        // so remove it
        if (line[line.length()-1] == char(13))
            line = line.substr(0, line.length()-1);

        LINFO("Loading:  " << line);
        
        VolumeSet* curVolumeSet = populator_->getVolumeSerializer()->load(path + line);

        // Move series from this file into the output VolumeSet
        if (curVolumeSet != 0) {
            const VolumeSeries::SeriesSet& seriesSet = curVolumeSet->getSeries();
            while (seriesSet.begin() != seriesSet.end()) {
                VolumeSeries* series = *(seriesSet.begin());
                curVolumeSet->removeSeries(series);
                volumeSet->addSeries(series);
            }
            delete curVolumeSet;
        }
    } while (!fin.eof());

    fin.close();

    return volumeSet;
}

} // namespace voreen
