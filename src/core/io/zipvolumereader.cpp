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

#include "voreen/core/io/zipvolumereader.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/io/datvolumereader.h" // used to determine related .raw file name
#include "voreen/core/io/ioprogress.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/io/volumeserializer.h"

#ifdef VRN_MODULE_BASE
#include "voreen/modules/base/io/multivolumereader.h"
#endif

#include "tgt/ziparchive.h"
#include <fstream>

using std::string;

namespace voreen {

const std::string ZipVolumeReader::loggerCat_("voreen.io.ZipVolumeReader");

ZipVolumeReader::ZipVolumeReader(VolumeSerializerPopulator* populator, IOProgress* progress)
  : VolumeReader(progress),
    populator_(populator)
{
    extensions_.push_back("zip");
    protocols_.push_back("zip");
}

VolumeHandle* ZipVolumeReader::read(const VolumeOrigin& origin)
    throw (tgt::FileException, std::bad_alloc)
{
    VolumeHandle* result = 0;

    // Extract path zip and internal filename
    size_t extensionPos = origin.getPath().find(".zip");
    std::string zipName = origin.getPath().substr(0, extensionPos + 4);
    std::string fileName = origin.getPath().substr(extensionPos + 5);
    std::string temporaryPath = VoreenApplication::app()->getTemporaryPath();

    tgt::ZipArchive zip(zipName);

    tgt::File* xFile = zip.extractFile(fileName, tgt::ZipArchive::TARGET_DISK, temporaryPath);
    if (xFile == 0)
        throw tgt::FileNotFoundException("Specific file within zip file not found", origin.getPath());
    delete xFile;   // Free resources held by tgt::File
    xFile = 0;

    // Check whether this file is a .dat file and has related .raw file. If so,
    // extract that file, too, in order to enable the DatVolumeReader to find it.
    //
    std::string additionalFileName = "";
    if (tgt::FileSystem::fileExtension(fileName, true) == "dat") {
        additionalFileName = DatVolumeReader::getRelatedRawFileName(temporaryPath + "/" + fileName);

        xFile = zip.extractFile(additionalFileName, tgt::ZipArchive::TARGET_DISK, temporaryPath);
        if (xFile == 0)
            throw tgt::FileNotFoundException("Specific file within zip file not found", origin.getPath());
        delete xFile;
        xFile = 0;
    }

    VolumeCollection* volumeCollection =
        populator_->getVolumeSerializer()->load(temporaryPath + "/" + fileName);
    if (volumeCollection && !volumeCollection->empty()) {
        VolumeOrigin origin = volumeCollection->first()->getOrigin();
        std::string originWithoutTempDir = origin.getPath().substr(temporaryPath.length() + 1);
        volumeCollection->first()->setOrigin(
            VolumeOrigin("zip://" + zipName + "/" + originWithoutTempDir));
        result = volumeCollection->first();
    }

    // Delete extracted file
    //
    tgt::FileSystem::deleteFile(temporaryPath + "/" + fileName);
    if (additionalFileName.empty() == false)
        tgt::FileSystem::deleteFile(temporaryPath + "/" + additionalFileName);

    return result;
}

VolumeCollection* ZipVolumeReader::read(const std::string& url)
    throw (tgt::FileException, std::bad_alloc)
{

#ifdef VRN_MODULE_BASE
    VolumeOrigin origin(url);
    std::string fileName = origin.getPath();

    tgt::ZipArchive zip(fileName);

    std::string temporaryPath = VoreenApplication::app()->getTemporaryPath();
    std::string indexFilePath = temporaryPath + "/index.mv";
    std::vector<std::string> files = zip.getContainedFileNames();

    // No index file exists, so we have to create one ourselves
    if (zip.containsFile("index.mv") == false) {
        std::ofstream file(indexFilePath.c_str());
        for (size_t i = 0; ((file.good() == true) && (i < files.size())); ++i) {
            // Put all filenames in the index.mv which end with dat
            if (tgt::FileSystem::fileExtension(files[i], true) == "dat")
                file << files[i] << std::endl;
        }
        file.close();
    }

    // Extract all volumes from the archive and save them locally
    zip.extractFilesToDirectory(temporaryPath);

    // Load the volumes with the help of a temporary multivolumereader
    VolumeCollection* volumeCollection = MultiVolumeReader(populator_, getProgress()).read(indexFilePath);

    // Set the correct origins
    for (size_t iter = 0; volumeCollection && iter < volumeCollection->size(); ++iter) {
        VolumeOrigin origin = volumeCollection->at(iter)->getOrigin();
        std::string originWithoutTempDir = origin.getPath().substr(temporaryPath.length() + 1);
        volumeCollection->at(iter)->setOrigin(VolumeOrigin("zip://" + fileName + "/" + originWithoutTempDir));
    }

    // Delete the extracted (and the possibly created index) files
    tgt::FileSystem::deleteFile(indexFilePath);
    for (size_t i = 0; i < files.size(); ++i)
        tgt::FileSystem::deleteFile(temporaryPath + "/" + files[i]);

    return volumeCollection;
#else
    LERROR("Unable to load " << url << " (core module required)");
    return 0;
#endif
}

VolumeOrigin ZipVolumeReader::convertOriginToRelativePath(const VolumeOrigin& origin, std::string& basePath) const {

    std::string path = origin.getPath();

    // replace backslashes in path
    string::size_type pos = path.find("\\");
    while (pos != string::npos) {
        path[pos] = '/';
        pos = path.find("\\");
    }

    // Extract part after the .zip file
    // TODO: not robust, doesn't handle uppercase file names
    string::size_type zippos = path.find(".zip/");
    std::string inzip;
    if (zippos != string::npos) {
        inzip = origin.getPath().substr(zippos + 4);
        path = origin.getPath().substr(0, zippos + 4);
    }

    // create origin with relative path
    return VolumeOrigin(origin.getProtocol(), tgt::FileSystem::relativePath(path, basePath) + inzip);
}

VolumeOrigin ZipVolumeReader::convertOriginToAbsolutePath(const VolumeOrigin& origin, std::string& basePath) const {

    // build new path only if this is not an absolute path
    if (origin.getPath().find("/") != 0 && origin.getPath().find("\\") != 0 && origin.getPath().find(":") != 1) {
        return VolumeOrigin(origin.getProtocol(), tgt::FileSystem::absolutePath(basePath + "/" + origin.getPath()));
    }
    else
        return origin;
}

VolumeReader* ZipVolumeReader::create(IOProgress* progress) const {
    return new ZipVolumeReader(0, progress);
}

} // namespace voreen
