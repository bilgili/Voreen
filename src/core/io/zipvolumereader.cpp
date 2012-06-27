/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/core/io/textfilereader.h"
#include "voreen/core/io/zipvolumereader.h"
#include "voreen/core/io/rawvolumereader.h"
#include "voreen/core/io/pvmvolumereader.h"
#include "voreen/core/io/quadhidacvolumereader.h"

#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/io/ioprogress.h"

#include "tgt/exception.h"
#include "tgt/vector.h"
#include "tgt/texturemanager.h"

#include <fstream>
#include <iostream>

// TODO: this is the stdafx.h from ZipArchive
// is this header here really necessary?
#include <ziparchive/stdafx.h>
#include <ziparchive/ZipArchive.h>

using std::string;

namespace voreen {

const std::string ZipVolumeReader::loggerCat_("voreen.io.ZipVolumeReader");

ZipVolumeReader::ZipVolumeReader(IOProgress* progress /*= 0*/) : VolumeReader(progress)
{
    name_ = "Zip Reader";
    extensions_.push_back("zip");
}

bool ZipVolumeReader::parseDescFile(const std::string& filename) {
    int nbrTags;
    string objectType;
    string gridType;
    string version;
    VolInfo* volInfo = 0;
    tgt::vec3 resolution;
    tgt::vec3 sliceThickness;
    std::string format;
    std::string objectModel;
    bool error = false;

    TextFileReader reader(filename);

    format = "UCHAR";
    objectModel = "I";

    if (!reader ) {
        LERROR("Unable to open " << filename);
        return false;
    }
    std::string type;
    std::istringstream args;
    bool ready = false;
    while (reader.getNextLine(type, args) && ready == false) {
        LINFO("Type : " << type);
        if (type == "version:") {
            args >> version;
            LINFO(version);
            if (version != "1.0") {
                LWARNING("Version not supported: " << version);
                error = true;
                ready = true;
            }
        }
        else if (type == "resolution:") {
            args >> resolution[0];
            args >> resolution[1];
            args >> resolution[2];
            LINFO("Value: " << resolution[0] << " x " <<
                  resolution[1] << " x " << resolution[2]);
        }
        else if (type == "slicethickness:") {
            args >> sliceThickness[0] >> sliceThickness[1] >> sliceThickness[2];
            LINFO("Value: " << sliceThickness[0] << " " <<
                  sliceThickness[1] << " " << sliceThickness[2]);
        }
        else if (type == "format:") {
            args >> format;
            LINFO("Value: " << format);
        }
        else if (type == "nbrtags:") {
            args >> nbrTags;
            LINFO("Value: " << nbrTags);
        }
        else if (type == "objecttype:") {
            args >> objectType;
            LINFO("Value: " << objectType);
        }
        else if (type == "objectmodel:") {
            args >> objectModel;
            LINFO("Value: " << objectModel);
        }
        else if (type == "modality:") {
            if (!volInfo ) {
                LERROR("Filename must precede"
                       "this token.");
                error = true;
            }
            std::string modality;
            args >> modality;
            volInfo->modality_ = modality;
            LINFO("Value: " << volInfo->modality_);
        }
        else if (type == "gridtype:") {
            args >> gridType;
            LINFO("Value: " << gridType);
        }
        else if (type == "filename:") {
            if (volInfo) {
                volInfos_.push_back(volInfo);
            }
            volInfo = new VolInfo(resolution, sliceThickness, format, objectModel);
            args >> volInfo->filename_;
            LINFO("Value: " << volInfo->filename_);
        }
        else if (type == "extinfo:") {
            if (!volInfo ) {
                LERROR("Filename must precede"
                       "this token.");
                error = true;
            }
            string extInfo;
            getline(args, extInfo);
            volInfo->extInfo_.push_back(extInfo);
            LINFO("Value: " << extInfo);
        }
        else if (type == "timestamp:") {
            if (!volInfo ) {
                LERROR("Filename must precede"
                       "this token.");
                error = true;
            }
            args >> volInfo->timeStamp_;
            LINFO("Value: " << volInfo->timeStamp_);
        }
        else if (type == "metastring:") {
            if (!volInfo ) {
                LERROR("Filename must precede"
                       "this token.");
                error = true;
            }
            args >> volInfo->metaString_;
            LINFO("Value: " << volInfo->metaString_);
        }
        else {
            LERROR("Unknown type");
            error = true;
        }
        if (args.fail() ) {
            LERROR("Format error");
            error = true;
        }
    }
    if (volInfo) {
        volInfos_.push_back(volInfo);
        volInfo = 0;
    }
    return !error;
}

VolumeSet* ZipVolumeReader::read(const std::string &fileName, bool generateVolumeGL)
    throw (tgt::CorruptedFileException, tgt::IOException, std::bad_alloc)
{
    LINFO("ZipVolumeReader: " << fileName);

    CZipArchive zip;
    volInfos_.clear();

    try {
        zip.Open(fileName.c_str());
        int descIndex = zip.FindFile("description.txt");
        if (descIndex == -1) {
            LERROR("Description file not found.");
            return 0;
        }
        VolumeSet* volumeSet = new VolumeSet(fileName);
        CZipMemFile descFile;
        zip.ExtractFile(descIndex, ".");
        parseDescFile("description.txt");
        remove("description.txt");
        if (progress_)
            progress_->setNumSteps(volInfos_.size());
        for(unsigned int i=0; i<volInfos_.size(); ++i) {
            VolInfo *volInfo = volInfos_.at(i);
            int indexInZip = zip.FindFile(volInfo->filename_.c_str());
            if (indexInZip == -1) {
                LERROR("File not found: " << volInfo->filename_);
            }
            else {
                LINFO("Load file " << volInfo->filename_.c_str());

                std::string endingtest = volInfo->filename_;
                std::transform(endingtest.begin(), endingtest.end(), endingtest.begin(), tolower);

                VolumeSet* curVolumeSet = 0;
                if (volInfo->filename_.find(".raw") != std::string::npos) {
                    zip.ExtractFile(indexInZip, ".");
                    RawVolumeReader reader;
                    reader.readHints( volInfo->resolution_, volInfo->sliceThickness_, 8,
                                     volInfo->objectModel_, volInfo->format_, 0,
                                     tgt::mat4::identity, Modality::MODALITY_UNKNOWN, -1.f, volInfo->metaString_ );
                    curVolumeSet = reader.read(volInfo->filename_, generateVolumeGL);
                    remove(volInfo->filename_.c_str());
                }
                else if (volInfo->filename_.find(".pvm") != std::string::npos) {
#ifdef VRN_WITH_PVM
                    zip.ExtractFile(indexInZip, ".");
                    PVMVolumeReader reader(NULL);
                    curVolumeSet = reader.read(volInfo->filename_, generateVolumeGL);
                    remove(volInfo->filename_.c_str());
#else // VRN_WITH_PVM
                    LERROR("PVM found in zip-file, but program was compiled without PVM-support.");
#endif // VRN_WITH_PVM
                }
                else if (volInfo->filename_.find(".i4d") != std::string::npos) {
                    zip.ExtractFile(indexInZip, ".");
                    QuadHidacVolumeReader reader;
                    curVolumeSet = reader.read(volInfo->filename_, generateVolumeGL);
                    remove(volInfo->filename_.c_str());
                }
                else {
                    zip.ExtractFile(indexInZip, ".");
                    Volume* volume = (Volume*)TexMgr.load(volInfo->filename_, tgt::Texture::LINEAR, false);
                    VolumeSeries* volumeSeries = new VolumeSeries(volumeSet, "unknown", Modality::MODALITY_UNKNOWN);
                    volumeSet->addSeries(volumeSeries);
                    VolumeHandle* volumeHandle = new VolumeHandle(volumeSeries, volume, 0.0f);
                    volumeSeries->addVolumeHandle(volumeHandle);
                    if( generateVolumeGL == true )
                        volumeHandle->generateHardwareVolumes(VolumeHandle::HARDWARE_VOLUME_GL);

                    remove(volInfo->filename_.c_str());
                }

                if( curVolumeSet != 0 ) {
                    const VolumeSeries::SeriesSet& seriesSet = curVolumeSet->getSeries();
                    VolumeSeries::SeriesSet::const_iterator itSeries = seriesSet.begin();
                    for( ; itSeries != seriesSet.end(); ++itSeries ) {
                        if( *itSeries == 0 )
                            continue;

                        // create a new VolumeSeries based on the ones from the temporary VolumeSet,
                        // set its modality to the one from volInfo and add it to the final VolumeSet in
                        // volumeSet
                        //
                        VolumeSeries* newSeries = new VolumeSeries(volumeSet, (*itSeries)->getName(), volInfo->modality_);
                        volumeSet->addSeries(newSeries, false);

                        // iterate over all handles in the temporary VolumeSet in curVolumeSet
                        // to remove them from it and append them to the final VolumeSet volumeSet.
                        // This prevents from the need of copying the volumes and preserves
                        // them from being deleted on deleting curVolumeSet!
                        //
                        const VolumeHandle::HandleSet& handleSet = (*itSeries)->getVolumeHandles();
                        VolumeHandle::HandleSet::const_iterator itHandle = handleSet.begin();
                        for( ; itHandle != handleSet.end(); ) {
                            if( *itHandle == 0 )
                                continue;
                            VolumeHandle* handle = (*itSeries)->removeVolumeHandle(*(itHandle++));
                            handle->setTimestep(volInfo->timeStamp_);
                            newSeries->addVolumeHandle(handle, true);
                        }
                        
                    }
                    delete curVolumeSet;
                    curVolumeSet = 0;
                }
                if (progress_)
                    progress_->set(i);
            }   // else
        }   // for
        zip.Close();
        return volumeSet;
    }   // try
    catch (CZipException e) {
        LERROR("ZipArchive: " << e.GetErrorDescription());
    }

    return 0;
}

} // namespace voreen
