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

#include "voreen/core/io/zipvolumereader.h"

#include "voreen/core/application.h"
#include "voreen/core/io/ioprogress.h"
#include "voreen/core/io/multivolumereader.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/io/volumeserializer.h"

#include <ziparchive/ZipArchive.h>
#include <fstream>

using std::string;

namespace voreen {

const std::string ZipVolumeReader::loggerCat_("voreen.io.ZipVolumeReader");

ZipVolumeReader::ZipVolumeReader(VolumeSerializerPopulator* populator, IOProgress* progress)
  : VolumeReader(progress),
    populator_(populator)
{
    name_ = "Zip Reader";
    extensions_.push_back("zip");
}

VolumeHandle* ZipVolumeReader::readFromOrigin(const VolumeHandle::Origin& origin) {
    VolumeHandle* result = 0;

    // Remove the prefix "zip://"
    size_t extensionPos = origin.filename.find(".zip");
    std::string zipName = origin.filename.substr(0, extensionPos + 4);
    std::string fileName = origin.filename.substr(extensionPos + 5);

    try {
        CZipArchive zip;
        zip.Open(zipName.c_str());

        int indexFile = zip.FindFile(fileName.c_str());
        std::string temporaryPath = VoreenApplication::app()->getTemporaryPath();

        if (indexFile != ZIP_FILE_INDEX_NOT_FOUND) {
            for (int iter = 0; iter < zip.GetCount(); ++iter)
                zip.ExtractFile(iter, temporaryPath.c_str());

            VolumeSet* volumeSet = populator_->getVolumeSerializer()->load(temporaryPath + "/" + fileName);
            VolumeHandle::Origin origin = volumeSet->getAllVolumeHandles().at(0)->getOrigin();
            std::string originWithoutTempDir = origin.filename.substr(temporaryPath.length() + 1);
            origin.filename = "zip://" + zipName + "/" + originWithoutTempDir;
            volumeSet->getAllVolumeHandles().at(0)->setOrigin(origin);
            result = volumeSet->getAllVolumeHandles().at(0);

            for (int iter = 0; iter < zip.GetCount(); ++iter) {
                std::string path = temporaryPath + "/" + zip.GetFileInfo(iter)->GetFileName();
                remove(path.c_str());
            }

        }
        else
            throw tgt::FileNotFoundException("Specific file within zip file not found", origin.filename);
    }
    catch (CZipException e) {
        throw tgt::FileException("ZipVolumeReader::readFromOrigin(" + origin.filename + "): " + e.GetErrorDescription());
    }

    return result;
}

VolumeSet* ZipVolumeReader::read(const std::string& fileName)
    throw (tgt::FileException, std::bad_alloc)
{

    try {
        CZipArchive zip;
        zip.Open(fileName.c_str());

        int indexFile = zip.FindFile("index.mv");
        std::string temporaryPath = VoreenApplication::app()->getTemporaryPath();
        std::string indexFilePath = temporaryPath + "/index.mv";

        // No index file exists, so we have to create one ourselves
        if (indexFile == ZIP_FILE_INDEX_NOT_FOUND) {
            std::ofstream file(indexFilePath.c_str());
            
            for (int iter = 0; iter < zip.GetCount(); ++iter) {
                std::string packedFileName = zip.GetFileInfo(iter)->GetFileName();
                // Put all filenames in the index.mv which end with dat
                if (packedFileName.find(".dat") != std::string::npos)
                    file << packedFileName.c_str() << std::endl;
            }
            file.close();
        }

        // Extract all volumes from the archive and save them locally
        for (int iter = 0; iter < zip.GetCount(); ++iter)
            zip.ExtractFile(iter, temporaryPath.c_str());
        
        // Load the volumes with the help of a temporary multivolumereader
        VolumeSet* volumeSet = MultiVolumeReader(populator_, getProgress()).read(indexFilePath);
        volumeSet->setName(tgt::FileSystem::fileName(fileName));

        // Set the correct origins
        for (size_t iter = 0; iter < volumeSet->getAllVolumeHandles().size(); ++iter) {
            VolumeHandle::Origin origin = volumeSet->getAllVolumeHandles().at(iter)->getOrigin();
            std::string originWithoutTempDir = origin.filename.substr(temporaryPath.length() + 1);
            origin.filename = "zip://" + fileName + "/" + originWithoutTempDir;
            volumeSet->getAllVolumeHandles().at(iter)->setOrigin(origin);
        }

        // Delete the extracted (and the possibly created index) files
        remove(indexFilePath.c_str());
        for (int iter = 0; iter < zip.GetCount(); ++iter) {
            std::string path = temporaryPath + "/" + zip.GetFileInfo(iter)->GetFileName();
            remove(path.c_str());
        }

        return volumeSet;
    }
    catch (CZipException e) {
        throw tgt::FileException("ZipVolumeReader::read(" + fileName + "): " + e.GetErrorDescription());
    }
}

} // namespace voreen
