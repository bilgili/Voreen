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

#ifdef VRN_WITH_DCMTK

#include "voreen/core/io/dicomvolumereader.h"
#include "voreen/core/io/dicommovescu.h"
#include "voreen/core/io/dicomfindscu.h"

#include "voreen/core/volume/volumeatomic.h"
#include "tgt/texture.h"

#ifdef WIN32
#define HAVE_SSTREAM_H 1
#define HAVE_CLASS_TEMPLATE 1
#define HAVE_FUNCTION_TEMPLATE 1
//#define USE_STRINGSTREAM 1
//#pragma include_alias( <sstream.h>, <sstream> )
#else
#define HAVE_CONFIG_H

// prevent warnings
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef HAVE_STAT

#include <dcmtk/config/osconfig.h>
#endif

#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmdata/dcitem.h>
#include <dcmtk/dcmdata/dcelem.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcdirrec.h>
#include <dcmtk/dcmdata/dcdicdir.h>
#include <dcmtk/dcmdata/dcdict.h>
#include <dcmtk/dcmdata/dcrledrg.h>

#include <dcmtk/dcmjpeg/djdecode.h>    /* for dcmjpeg decoders */
#include <dcmtk/dcmjpeg/dipijpeg.h>    /* for dcmimage JPEG plugin */
#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmimage/diregist.h>

#ifdef WIN32
#pragma comment (lib,"WS2_32.lib")
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <fcntl.h>
#include <signal.h>
#include <windows.h>
#include <direct.h>
#endif // WIN32

#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmdata/dcdebug.h>
#include <dcmtk/dcmdata/cmdlnarg.h>
#include <dcmtk/ofstd/ofconapp.h>
#include <dcmtk/dcmjpeg/djencode.h>  /* for dcmjpeg encoders */
#include <dcmtk/dcmjpeg/djrplol.h>  /* for DJ_RPLossless */
#include <dcmtk/dcmjpeg/djrploss.h>  /* for DJ_RPLossy */

// Prevent warnings because of redefined symbols
#undef PACKAGE_NAME
#undef PACKAGE_BUGREPORT
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

// Includes for directory-listing
#ifdef WIN32
#include <windows.h>
#include <cctype>
#else
#include <limits.h>
#include <dirent.h>
#endif

#include <fstream>
#include <iostream>
#include <map>
#include <algorithm>
#include <cstdlib>

using std::string;
using std::vector;

namespace voreen {

const std::string DicomVolumeReader::loggerCat_ = "voreen.DicomVolumeReader";

DicomVolumeReader::DicomVolumeReader(IOProgress* progress)
    : VolumeReader(progress),
      scalars_(0)
{
    extensions_.push_back("dcm");
    extensions_.push_back("dicom");

    if (!dcmDataDict.isDictionaryLoaded()) {
        // The data dictionary is needed for loading Dicom files.
        //FIXME: This should raise an exception and inform the user.
        LERROR("Warning: no data dictionary loaded, check environment variable: " <<
               DCM_DICT_ENVIRONMENT_VARIABLE);
    }
}

void DicomVolumeReader::setSecurityOptions(const DicomSecurityOptions& security) {
    security_ = security;
}

int DicomVolumeReader::loadSlice(const std::string& fileName, int posScalar) {

    DcmFileFormat fileformat;
    DcmDataset *dataset;  // Pixel data might be compressed

    OFCondition status = fileformat.loadFile(fileName.c_str(), EXS_Unknown,
                                             EGL_withoutGL, DCM_MaxReadLength, ERM_autoDetect);
    if (status.bad()) {
        LERROR("Error loading file " << fileName << ": " << status.text());
        return 0;
    }

    dataset = fileformat.getDataset();

    DicomImage image(&fileformat, dataset->getOriginalXfer());
    image.hideAllOverlays(); // do not show overlays by default (would write 0xFFFF into the data)

    // For CT modality we need to apply the rescale slope and intercept, as some datasets have
    // varying rescale values between slices. We apply these by setting a default window. This
    // should be sufficient for all CT data, as the should all contain Hounsfield units.
    OFString modality;
    if (dataset->findAndGetOFString(DCM_Modality, modality).good()) {
        if (modality == "CT")
            image.setWindow(1024, 4096);
    }

    if (image.getStatus() != EIS_Normal) {
        LERROR("Error creating DicomImage from file " << fileName << ": "
               << image.getString(image.getStatus()));
        return 0;
    }

    // Render pixel data into scalar array
    if (!image.getOutputData(&scalars_[posScalar * bytesPerVoxel_], dx_ * dy_ * bytesPerVoxel_, bits_)) {
        LERROR("Failed to render pixel data "
               << image.getOutputDataSize(bits_) << " vs. " << dx_ * dy_ * bytesPerVoxel_);

        return 0;
    }

    // Return number of voxels rendered
    return dx_ * dy_;
}


// Anonymous namespace
namespace {

/*
 * Helper for extracting dctmk tags
 */
string getItemString(DcmItem* item, const DcmTagKey &tagKey) {
    OFString s;
    if (item->findAndGetOFString(tagKey, s).good()) {
        return string(s.c_str());
    } else {
        DcmTag tag(tagKey);
        LWARNINGC("voreen.DicomVolumeReader", "!!! can't retrieve tag " << tag.getTagName() << "!!!");
        return "";
    }
}

/*
 * Sorts strings according to the x value of an vec3
 */
bool slices_cmp_x(pair<string, tgt::vec3> a, pair<string, tgt::vec3> b) { //slow
   return a.second.x < b.second.x;
}

/*
 * Sorts strings according to the y value of an vec3
 */
bool slices_cmp_y(pair<string, tgt::vec3> a, pair<string, tgt::vec3> b) { //slow
   return a.second.y < b.second.y;
}

/*
 * Sorts strings according to the z value of an vec3
 */
bool slices_cmp_z(pair<string, tgt::vec3> a, pair<string, tgt::vec3> b) { //slow
   return a.second.z < b.second.z;
}

// needed because of some conflicts with std::tolower() and transform()
char mytolower(char c) {
    return std::tolower(static_cast<unsigned char>(c));
}

} // namespace


Volume* DicomVolumeReader::readDicomFiles(const vector<string> &fileNames,
                                          const string &filterSeriesInstanceUID,
                                          bool skipBroken)
    throw (tgt::FileException, std::bad_alloc)
{
    // register JPEG codec
    DJDecoderRegistration::registerCodecs(EDC_photometricInterpretation, EUC_default, EPC_default, OFFalse);

    vector<pair<string, tgt::vec3> > slices;  // (filename, position)

    float x_spacing = 1, y_spacing = 1, z_spacing = 1; // For the resulting Volume
    float rowspacing = 1, colspacing = 1; // As read from PixelSpacing attribute

    LINFO("Reading metadata from " << fileNames.size() << " files...");

    string filter(filterSeriesInstanceUID);
    if (!filter.empty())
        LINFO("Filter for SeriesInstanceUID set to: " << filter);

    // First read metadata from all files.
    // This might be suboptimal when it is already clear which files belong to a certain series
    // (i.e. from DICOMDIR). There it might be better to read metadata and slice data at the
    // same time.
    bool found_first = false;
    vector<string>::const_iterator it_files = fileNames.begin();
    if (getProgress())
        getProgress()->setNumSteps(fileNames.size());
    int i = 0;
    while (it_files != fileNames.end()) {
        if (getProgress())
            getProgress()->set(i);
        i++;

        DcmFileFormat fileformat;

//        LINFO("Reading metadata for " << (*it_files));

        OFCondition status = fileformat.loadFile((*it_files).c_str());
        if (status.bad()) {
            if (skipBroken) {
                // File might be a broken DICOM but probably it is just some other non-DICOM file
                // lying around in the directory, so just skip it.
                LINFO("Skipping file " << (*it_files) << ": " << status.text());
                it_files++;
                continue;
            } else {
                LERROR("Error loading file " << (*it_files) << ": " << status.text());
                return 0;
            }
        }

        DcmDataset *dataset = fileformat.getDataset();

        OFString tmpString;
        if (dataset->findAndGetOFString(DCM_StudyInstanceUID, tmpString).bad())
            LERROR("no StudyInstanceUID in file " << (*it_files));
        string studyInstanceUID(tmpString.c_str());
        if (dataset->findAndGetOFString(DCM_SeriesInstanceUID, tmpString).bad())
            LERROR("no SeriesInstanceUID in file " << (*it_files));
        string seriesInstanceUID(tmpString.c_str());

        // First file with matching series UID
        if (!found_first) {

            // If no filter given, the first file specifies the series UID
            if (filter.empty()) {
                filter = seriesInstanceUID;
                LINFO("    Now loading first series found: " << seriesInstanceUID);
            }

            if (seriesInstanceUID == filter) {
                found_first = true;
                LINFO("    Study Description : " << getItemString(dataset, DCM_StudyDescription));
                LINFO("    Series Description : " << getItemString(dataset, DCM_SeriesDescription));
                std::string mod = getItemString(dataset, DCM_Modality);
                LINFO("    Modality : " << mod);
                std::transform(mod.begin(), mod.end(), mod.begin(), mytolower);
                modality_ = Modality(mod);

                if (dataset->findAndGetOFStringArray(DCM_Rows, tmpString).good()) {
                    dy_ = atoi(tmpString.c_str());
                } else {
                    LERROR("Can't retrieve DCM_Rows from file " << (*it_files));
                    dy_ = 0;
                    found_first = false;
                }
                if (dataset->findAndGetOFStringArray(DCM_Columns, tmpString).good()) {
                    dx_ = atoi(tmpString.c_str());
                } else {
                    LERROR("Can't retrieve DCM_Columns from file " << (*it_files));
                    dx_ = 0;
                    found_first = false;
                }
                if (dataset->findAndGetOFStringArray(DCM_BitsStored, tmpString).good()) {
                    bits_ = atoi(tmpString.c_str());
                } else {
                    LERROR("Can't retrieve DCM_BitsStored from file " << (*it_files));
                    bits_ = 16;//TODO
                    found_first = false;
                }

                LINFO("    Size: " << dx_ << "x" << dy_ << ", " << bits_ << " bits");

                // Extract PixelSpacing
                OFString rowspacing_str, colspacing_str;
                if (dataset->findAndGetOFString(DCM_PixelSpacing, rowspacing_str, 0).good() &&
                    dataset->findAndGetOFString(DCM_PixelSpacing, colspacing_str, 1).good()) {
                    LINFO("    PixelSpacing: (" << rowspacing_str << "; " << colspacing_str << ")");
                    if (rowspacing_str != colspacing_str)
                        LWARNING("row-spacing != colspacing: " << rowspacing_str << " vs. " << colspacing_str);

                    rowspacing = static_cast<float>(atof(rowspacing_str.c_str()));
                    colspacing = static_cast<float>(atof(colspacing_str.c_str()));
                }
            }
        }

        // Matching series UID?
        if (seriesInstanceUID == filter) {
            OFString tmpStrPosX;
            OFString tmpStrPosY;
            OFString tmpStrPosZ;
            tgt::vec3 imagePositionPatient;
            OFString imageType;
            // Position is given by z-component of ImagePositionPatient
            if (dataset->findAndGetOFString(DCM_ImagePositionPatient, tmpStrPosX, 0).good() &&
                dataset->findAndGetOFString(DCM_ImagePositionPatient, tmpStrPosY, 1).good() &&
                dataset->findAndGetOFString(DCM_ImagePositionPatient, tmpStrPosZ, 2).good())
            {
                imagePositionPatient.x = static_cast<float>(atof(tmpStrPosX.c_str()));
                imagePositionPatient.y = static_cast<float>(atof(tmpStrPosY.c_str()));
                imagePositionPatient.z = static_cast<float>(atof(tmpStrPosZ.c_str()));
            }
            else {
                LERROR("Can't retrieve DCM_ImagePositionPatient from file " << (*it_files));
            }
/* //TODO: necessary for some US(?) datasets
            if (dataset->findAndGetOFString(DCM_ImageType, imageType, 3).good()) {
                LINFO("Image type: " << imageType);
            }
            else {
                LERROR("Can't retrieve DCM_ImageType from file " << (*it_files));
                imageType = "M";
            }
            if (imageType == "M") {
                LINFO("Add image to stack.");
                slices.push_back(make_pair(*it_files, imagePositionPatient));
            }
*/
            slices.push_back(make_pair(*it_files, imagePositionPatient));
        }
        else {
            LDEBUG("  File " << (*it_files) << " has different SeriesInstanceUID - skipping");
        }
        it_files++;
    }

    if (slices.size() == 0) {
        throw tgt::CorruptedFileException("Found no DICOM slices");
    }

    // Determine in which direction the slices are arranged and sort by position.
    // Furthermore the slice spacing is determined.
    //     the first slice must be included
    //     first solution: add difference between 2 slices to numerator
    //     second solution: divide by less than 1
    float slicespacing = 1;
    tgt::vec3 imagePositionPatient0;
    tgt::vec3 imagePositionPatient1;
    float imagePositionZ = -1.f;
    if (slices.size() > 1) {
        imagePositionPatient0 = slices[0].second;
        imagePositionPatient1 = slices[1].second;
        if (imagePositionPatient0.x != imagePositionPatient1.x) {
            LINFO("Slices are arranged in x direction.");
            std::sort(slices.begin(), slices.end(), slices_cmp_x);
            slicespacing = (slices[slices.size()-1].second.x - slices[0].second.x) / (slices.size()-1);
            imagePositionZ = slices[slices.size()-1].second.x;
        }
        if (imagePositionPatient0.y != imagePositionPatient1.y) {
            LINFO("Slices are arranged in y direction.");
            std::sort(slices.begin(), slices.end(), slices_cmp_y);
            slicespacing = (slices[slices.size()-1].second.y - slices[0].second.y) / (slices.size()-1);
            imagePositionZ = slices[slices.size()-1].second.y;
        }
        if (imagePositionPatient0.z != imagePositionPatient1.z) {
            LINFO("Slices are arranged in z direction.");
            std::sort(slices.begin(), slices.end(), slices_cmp_z);
            slicespacing = (slices[slices.size()-1].second.z - slices[0].second.z) / (slices.size()-1);
            imagePositionZ = slices[slices.size()-1].second.z;
        }
    }

    dz_ = slices.size();

    switch (bits_) {
    case  8: bytesPerVoxel_ = 1; break;
    case 12: bytesPerVoxel_ = 2; break;
    case 16: bytesPerVoxel_ = 2; break;
    case 32: bytesPerVoxel_ = 4; break;
    }
    scalars_ = new uint8_t[dx_ * dy_ * dz_ * bytesPerVoxel_];

    LINFO("We have " << dz_ << " slices. [" << dx_ << "x" << dy_ << "]");

    x_spacing = rowspacing;
    y_spacing = colspacing;
    z_spacing = slicespacing;

    LINFO("Spacing: (" << x_spacing << "; " << y_spacing << "; " << z_spacing << ")");

    // Now read the actual slices from the files
    LINFO("Building volume...");
    LINFO("Reading slice data from " << slices.size() << " files...");

    int posScalar = 0;
    vector<pair<string, tgt::vec3> >::iterator it_slices = slices.begin();
    if (getProgress())
        getProgress()->setNumSteps(slices.size());
    i = 0;
    while (it_slices != slices.end()) {
        if (getProgress())
            getProgress()->set(i);
        i++;

//        LINFO((*it_slices).first << " (position: " << (*it_slices).second << ")");
        int slicesize = loadSlice((*it_slices).first, posScalar);
        posScalar += slicesize;
        it_slices++;
    }

    // deregister global decompression codecs
    DJDecoderRegistration::cleanup();

    Volume* dataset = 0;

    switch (bits_) {
    case 8:
        dataset = new VolumeUInt8(scalars_,
                                  tgt::ivec3(dx_, dy_, dz_),
                                  tgt::vec3(x_spacing, y_spacing, z_spacing),
                                  bits_);
        dataset->meta().setFileName(slices[0].first);
        dataset->meta().setImagePositionZ(imagePositionZ);
        break;
    case 12:
    case 16:
        dataset = new VolumeUInt16(reinterpret_cast<uint16_t*>(scalars_),
                                   tgt::ivec3(dx_, dy_, dz_),
                                   tgt::vec3(x_spacing, y_spacing, z_spacing),
                                   bits_);
        dataset->meta().setFileName(slices[0].first);
        dataset->meta().setImagePositionZ(imagePositionZ);
        break;
    default:
        LERROR("Unsupported bit depth: " << bits_);
        delete[] scalars_;
        scalars_ = 0;
        return 0;
    }

    LINFO("Building volume complete.");
    return dataset;
}

vector<string> DicomVolumeReader::getFileNamesInDir(const string& dirName) {
    vector<string> files;

#ifdef WIN32

    // Windows-specific directory listing
    bool            finished = false;
    HANDLE          hList;
    TCHAR           szDir[MAX_PATH+1];
    WIN32_FIND_DATA FileData;

    // Setup file spec (contains wildcard)
    sprintf(szDir, "%s*", dirName.c_str());

    hList = FindFirstFile(szDir, &FileData);
    if (hList != INVALID_HANDLE_VALUE) {
        while (!finished) {
            if (!(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                files.push_back(dirName + FileData.cFileName);
            }

            if (!FindNextFile(hList, &FileData))
                finished = true;
        }
    }

    FindClose(hList);

#else

    // POSIX directory listing
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(dirName.c_str())) == NULL) {
        LERROR("Can't list directory: " << dirName);
        return files;
    }
    std::string name;
    while ((ent = readdir(dir))) {
        name = ent->d_name;
        if ((ent->d_type != DT_DIR) && (name != ".") && (name != "..")) {
            files.push_back(dirName + ent->d_name);
        }
    }
    closedir(dir);

#endif

    return files;
}


Volume* DicomVolumeReader::readDicomDir(const string &fileName,
                                        const string &filterSeriesInstanceUID)
{
    //FIXME: Need real OS-independent function here
    string dir = string(fileName).substr(0, string(fileName).length() - string("DICOMDIR").length());
    vector<string> files;

    OFCondition status;

    DcmDicomDir dicomdir(fileName.c_str());
    DcmDirectoryRecord* root = &(dicomdir.getRootRecord());
    DcmDirectoryRecord* PatientRecord = NULL;
    DcmDirectoryRecord* StudyRecord = NULL;
    DcmDirectoryRecord* SeriesRecord = NULL;
    DcmDirectoryRecord* FileRecord = NULL;
    OFString tmpString;

    // Analyze DICOMDIR
    while ((PatientRecord = root->nextSub(PatientRecord)) != NULL) {
        LINFO("Patient Name : " << getItemString(PatientRecord, DCM_PatientsName));

        while ((StudyRecord = PatientRecord->nextSub(StudyRecord)) != NULL) {
            LINFO("  Study, UID : " << getItemString(StudyRecord, DCM_StudyInstanceUID));
            LINFO("    Study Description : " << getItemString(StudyRecord, DCM_StudyDescription));

            // Read all series and filter according to SeriesInstanceUID
            while ((SeriesRecord = StudyRecord->nextSub(SeriesRecord)) != NULL) {
                string seriesUID = getItemString(SeriesRecord, DCM_SeriesInstanceUID);

                if (!filterSeriesInstanceUID.empty() && seriesUID != filterSeriesInstanceUID)
                    continue;

                LINFO("    Series, Instance UID : " << seriesUID);
                LINFO("      Series Description : " << getItemString(SeriesRecord, DCM_SeriesDescription));
                LINFO("      Modality : " << getItemString(SeriesRecord, DCM_Modality));

                while ((FileRecord = SeriesRecord->nextSub(FileRecord)) != NULL) {
                    char *referencedFileID=NULL;
                    if (FileRecord->findAndGetOFStringArray(DCM_ReferencedFileID, tmpString).good()) {
                        referencedFileID = const_cast<char*>(tmpString.c_str());
                    } else {
                        LWARNING("Can't retrieve DCM_ReferencedFileID from DICOMDIR file!");
                    }

                    string filename(referencedFileID);
#ifndef WIN32
                    size_t pos;
                    while ((pos = filename.find_first_of('\\')) != string::npos) {
                        filename.replace(pos, 1, "/");
                    }
#endif
                    files.push_back(dir + filename);
                }
            }
        }
    }

    return readDicomFiles(files, filterSeriesInstanceUID);
}


Volume* DicomVolumeReader::readDicomFile(const string &fileName) {
    vector<string> files;
    files.push_back(fileName);
    return readDicomFiles(files, "");
}


bool DicomVolumeReader::findSeriesDicomFile(const string &fileName, vector<DicomSeriesInfo> &series) {

    DcmFileFormat fileformat;
    LINFO("  Reading file " << fileName);

    OFCondition status = fileformat.loadFile(fileName.c_str());
    if (status.bad()) {
        LERROR("Error loading file " << fileName << ": " << status.text());
        return false;
    }

    DcmDataset *dataset = fileformat.getDataset();

    OFString tmpString;
    voreen::DicomSeriesInfo tmp;
    if (dataset->findAndGetOFString(DCM_SeriesInstanceUID, tmpString).good()) {
        tmp.uid_ = tmpString.c_str();
        if (dataset->findAndGetOFString(DCM_PatientsName, tmpString).good()) {
           tmp.patientsName_ = tmpString.c_str();
        }
        if (dataset->findAndGetOFString(DCM_PatientID, tmpString).good()) {
           tmp.patientId_ = tmpString.c_str();
        }
        if (dataset->findAndGetOFString(DCM_StudyDate, tmpString).good()) {
            tmp.studyDate_ = tmpString.c_str();
        }
        if (dataset->findAndGetOFString(DCM_StudyTime, tmpString).good()) {
            tmp.studyTime_ = tmpString.c_str();
        }
        if (dataset->findAndGetOFString(DCM_Modality, tmpString).good()) {
            tmp.modality_ = tmpString.c_str();
        }
        series.push_back(tmp);
        return true;
    } else {
        LERROR("no SeriesInstanceUID in file " << fileName);
        return false;
    }

}

namespace {

void extractString(DcmDirectoryRecord* record, const DcmTagKey& field, std::string& output) {
    OFString tmpString;
    if (record->findAndGetOFString(field, tmpString).good())
        output = tmpString.c_str();
}

} // namespace

bool DicomVolumeReader::findSeriesDicomDir(const string &fileName, vector<DicomSeriesInfo> &series) {

    //FIXME: Need real OS-independent function here
    string dir = string(fileName).substr(0, string(fileName).length() - string("DICOMDIR").length());

    OFCondition status;

    DcmDicomDir dicomdir(fileName.c_str());
    DcmDirectoryRecord* root = &(dicomdir.getRootRecord());
    DcmDirectoryRecord* PatientRecord = NULL;
    DcmDirectoryRecord* StudyRecord = NULL;
    DcmDirectoryRecord* SeriesRecord = NULL;
    DcmDirectoryRecord* ImageRecord = NULL;
    OFString tmpString;
    voreen::DicomSeriesInfo tmp;

    // Analyze DICOMDIR:
    while ((PatientRecord = root->nextSub(PatientRecord)) != NULL) {
        // patient level
        tmp = voreen::DicomSeriesInfo();
        extractString(PatientRecord, DCM_PatientsName, tmp.patientsName_);
        extractString(PatientRecord, DCM_PatientID, tmp.patientId_);

        while ((StudyRecord = PatientRecord->nextSub(StudyRecord)) != NULL) {
            // study level
            extractString(StudyRecord, DCM_StudyDate, tmp.studyDate_);
            extractString(StudyRecord, DCM_StudyTime, tmp.studyTime_);
            extractString(StudyRecord, DCM_Modality, tmp.modality_);

            while ((SeriesRecord = StudyRecord->nextSub(SeriesRecord)) != NULL) {
                // series level
                int numimages = 0;
                while ((ImageRecord = SeriesRecord->nextSub(ImageRecord)) != NULL) {
                    // image level: count number of images
                    numimages++;
                }
                std::ostringstream s;
                s << numimages;
                tmp.numImages_ = s.str();

                extractString(SeriesRecord, DCM_Modality, tmp.modality_);
                extractString(SeriesRecord, DCM_SeriesDescription, tmp.description_);
                extractString(SeriesRecord, DCM_SeriesInstanceUID, tmp.uid_);

                // now we have all needed information about the series
                series.push_back(tmp);
            }
        }
    }

    return true;
}



namespace {

// Helper for analyzing a Dicom URL for C-MOVE
bool analyze_URL_path(const string &path, string &peerTitle, string &seriesInstanceUID) {
    // ^/([A-Za-z0-9.\-_]+)\?seriesInstanceUID=[0-9\.]$
    const string number_point = "0123456789.";
    const string peer_name = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-" + number_point;
    string s(path);

    // ^/
    if (path.substr(0, 1) != "/") return false;

    // ([A-Za-z0-9.\-_]+)\?seriesInstanceUID=
    size_t pos = s.find_first_not_of(peer_name, 1);
    if (pos <= 1) return false;
    peerTitle = s.substr(1, pos - 1);
    size_t pos2 = s.find("?seriesInstanceUID=", pos);
    if (pos2 != pos) return false;

    //[0-9\.]$
    if (s.find_first_not_of(number_point, pos2 + 19) != string::npos) return false;
    seriesInstanceUID = s.substr(pos2 + 19);

    return true;
}

// Is path pointing to a DICOMDIR file?
bool pathIsDicomDir(const string &path) {
    size_t pos = path.find("?");
    if (pos == string::npos) pos = path.size();

    //FIXME: A file "fooDICOMDIR" would be incorrectly identified
    return (pos >= 8 && (path.substr(pos - 8, 8) == "DICOMDIR" || path.substr(pos - 8, 8) == "dicomdir"));
}

// Path ends with '/' or '\'
bool pathIsDir(const string &path) {
    return (path[path.length()-1] == '/' || path[path.length()-1] == '\\');
}

} // namespace


VolumeSet* DicomVolumeReader::read(const string &fileName)
    throw (tgt::FileException, std::bad_alloc)
{
    dx_ = dy_ = dz_ = 0;
    Volume* volume;

    if (fileName.find("dicom://") == 0) {
        // Handle Dicom network connection to PACS
        string connection = fileName.substr(0, fileName.find("/", 8));
        string path = fileName.substr(fileName.find("/", 8));
        string peertitle, seriesInstanceUID;

        if (!DicomMoveSCU::init(connection) || !analyze_URL_path(path, peertitle, seriesInstanceUID)) {
            LERROR("Invalid URL for DicomMoveSCU: " << fileName);
            return 0;
        }

        vector<string> files;
        if (DicomMoveSCU::moveSeries(seriesInstanceUID, peertitle, &files) != 0)
            return 0;

        volume = readDicomFiles(files);
    }
    else if (pathIsDicomDir(fileName)) {
        // Handle DICOMDIR: "../DICOMDIR?<filter>"
        size_t pos = fileName.find("?");
        string file = fileName.substr(0, pos);
        string filter = (pos != string::npos ? fileName.substr(pos + 1) : "");
        LINFO(file << " : " << filter);
        volume = readDicomDir(file, filter);
    }
    else if (pathIsDir(fileName)) {
        // Handle reading of entire directories
        volume = readDicomFiles(getFileNamesInDir(fileName), "", true);
    } else {
        // Handle single dicom file
        volume = readDicomFile(fileName);
    }

    if (volume) {
        VolumeSet* volumeSet = new VolumeSet(tgt::File::fileName(fileName));
        VolumeSeries* volumeSeries = new VolumeSeries("unknown", modality_);
        volumeSet->addSeries(volumeSeries);
        VolumeHandle* volumeHandle = new VolumeHandle(volume, 0.0f);
        volumeHandle->setOrigin(fileName, "unknown", 0.0f);
        volumeSeries->addVolumeHandle(volumeHandle);
        return volumeSet;
    } else {
        throw tgt::FileException("Got NULL volume");
    }
}

Volume* DicomVolumeReader::read(const std::vector<std::string> &fileNames) {
    dx_ = dy_ = dz_ = 0;
    return readDicomFiles(fileNames);
}

vector<DicomSeriesInfo> DicomVolumeReader::listSeries(const string &fileName) {
    vector<DicomSeriesInfo> series;

    if (fileName.find("dicom://") == 0) {
        DicomFindSCU::find(fileName, &series, security_);
    }
    else if (pathIsDicomDir(fileName)) {
        // Handle DICOMDIR
        findSeriesDicomDir(fileName, series);
    }
    else if (pathIsDir(fileName)) {
        // Handle reading of entire directories
        LERROR("!!! listSeries not implemented for entire directories !!!");
    } else {
        // Handle single dicom file
        findSeriesDicomFile(fileName, series);
    }
    return series;
}

tgt::Texture* DicomVolumeReader::readSingleSliceIntoTex(const string& sliceFileName) {
    DiRegister* dr = new DiRegister();

    const char *opt_ifname = sliceFileName.c_str();

    OFBool opt_verbose = OFTrue;
    E_FileReadMode opt_readMode = ERM_autoDetect;
    E_TransferSyntax opt_oxfer = EXS_LittleEndianExplicit;
    E_TransferSyntax opt_ixfer = EXS_Unknown;

    // JPEG parameters
    E_DecompressionColorSpaceConversion opt_decompCSconversion = EDC_photometricInterpretation;
    E_UIDCreation opt_uidcreation = EUC_default;

    // register global decompression codecs
    DJDecoderRegistration::registerCodecs(
      opt_decompCSconversion,
      opt_uidcreation,
      EPC_default,
      opt_verbose);

    /* make sure data dictionary is loaded */
    if (!dcmDataDict.isDictionaryLoaded()) {
        LERROR( "Warning: no data dictionary loaded, "
             << "check environment variable: "
             << DCM_DICT_ENVIRONMENT_VARIABLE);
    }

    // open inputfile
    if ((opt_ifname == NULL) || (strlen(opt_ifname) == 0)) {
        LERROR( "invalid filename: <empty string>");
        return 0;
    }

    OFCondition error = EC_Normal;
    DcmFileFormat fileformat;
    error = fileformat.loadFile(opt_ifname, opt_ixfer, EGL_noChange, DCM_MaxReadLength, opt_readMode);
    if (error.bad())
    {
        LERROR( "Error: "
             << error.text()
             << ": reading file: " <<  opt_ifname);
        return 0;
    }
    DcmDataset *dataset = fileformat.getDataset();

    DcmXfer opt_oxferSyn(opt_oxfer);

    error = dataset->chooseRepresentation(opt_oxfer, NULL);
    if (error.bad()) {
        LERROR("Error: "
             << error.text()
             << ": decompressing file: " <<  opt_ifname);
        return 0;
    }

    if (dataset->canWriteXfer(opt_oxfer)) {
        LINFO("Output transfer syntax " << opt_oxferSyn.getXferName() << " can be written");
    } else {
        LERROR("No conversion to transfer syntax " << opt_oxferSyn.getXferName()
             << " possible!");
        return 0;
    }

    fileformat.loadAllDataIntoMemory();

    //DicomImage image(&fileformat, dataset->getOriginalXfer());
    DicomImage image(&fileformat, opt_oxfer);
    image.hideAllOverlays(); // do not show overlays by default (would write 0xFFFF into the data)

    if (image.getStatus() != EIS_Normal) {
        LERROR("Error creating DicomImage from file " << sliceFileName << ": " <<
                        image.getString(image.getStatus()));
        return 0;
    }

    tgt::Texture* tex = generate2DTextureFromDcmImage(&image);
    delete dr;

    //// deregister global codecs
    DJDecoderRegistration::cleanup();
    return tex;
}

tgt::Texture* DicomVolumeReader::generate2DTextureFromDcmImage(DicomImage* image) {

    uint8_t* imgData = new uint8_t[image->getOutputDataSize()];
    // Render pixel data into scalar array
    if (!image->getOutputData(imgData, image->getOutputDataSize(), 0)) {
        LERROR("Failed to render pixel data ");
        return 0;
    }

    GLenum depth;
    switch(image->getDepth()) {
        case 8:
            depth = GL_UNSIGNED_BYTE;
            break;
        default:
            LERROR("bit-depth " << image->getDepth() << " currently not supported.");
            return 0;
    }

    tgt::Texture* tex = new tgt::Texture(tgt::ivec3(image->getWidth(), image->getHeight(), 1), GL_RGB, depth);

    // in my test-dataset the resulting textures were mirrored along the x-axis (i.e., y-coordinates were swapped), so we deal with that here...
    // I don't know if that is always the case? (FL)
    for(int y = 0; y < tex->getDimensions().y; ++y) {
        for(int x = 0; x < tex->getDimensions().x; ++x) {
            tex->texel<tgt::col3>(x, tex->getDimensions().y - y - 1)[0] = imgData[(y*image->getWidth() + x)*3    ];
            tex->texel<tgt::col3>(x, tex->getDimensions().y - y - 1)[1] = imgData[(y*image->getWidth() + x)*3 + 1];
            tex->texel<tgt::col3>(x, tex->getDimensions().y - y - 1)[2] = imgData[(y*image->getWidth() + x)*3 + 2];
        }
    }

    delete[] imgData;
    return tex;
}

tgt::Texture* DicomVolumeReader::readDicomLoopSlice(DcmFileFormat* dfile, unsigned int num) {

    unsigned long       opt_compatibilityMode = CIF_MayDetachPixelData | CIF_TakeOverExternalDataset;

    OFCmdUnsignedInt    opt_frame = num;
    OFCmdUnsignedInt    opt_frameCount = 1;

    E_DecompressionColorSpaceConversion opt_decompCSconversion = EDC_photometricInterpretation;

    // register RLE decompression codec
    DcmRLEDecoderRegistration::registerCodecs(OFFalse /*pCreateSOPInstanceUID*/, OFTrue);

    // register JPEG decompression codecs
    DJDecoderRegistration::registerCodecs(opt_decompCSconversion, EUC_default, EPC_default, OFTrue);

    LINFO("preparing pixel data.");

    E_TransferSyntax xfer = dfile->getDataset()->getOriginalXfer();

    DicomImage *di = new DicomImage(dfile, xfer, opt_compatibilityMode, opt_frame - 1, opt_frameCount);
    if (di == NULL)
        LERROR("Out of memory");

    if (di->getStatus() != EIS_Normal)
        LERROR(DicomImage::getString(di->getStatus()));

    tgt::Texture* tex = generate2DTextureFromDcmImage(di);

    DcmRLEDecoderRegistration::cleanup();
    DJDecoderRegistration::cleanup();

    //FIXME For some reason, deleting the image leads to crashes... I'll have to look into that. (FL)
    //delete di;
    return tex;
}


DcmFileFormat* DicomVolumeReader::initDicomLoop(const std::string& filename, unsigned int& frameCount, unsigned int& fps) {

    /* make sure data dictionary is loaded */
    if (!dcmDataDict.isDictionaryLoaded())
        LERROR("Warning: no data dictionary loaded, check environment variable" << DCM_DICT_ENVIRONMENT_VARIABLE);

    E_FileReadMode readMode = ERM_autoDetect;
    E_TransferSyntax xfer = EXS_Unknown;
    static OFCmdUnsignedInt maxReadLength = 4096; // default is 4 KB

    const DcmTagKey* printTagKeys[2];

    /* it is a name */
    const DcmDataDictionary& globalDataDict = dcmDataDict.rdlock();
    const DcmDictEntry* dicentFrameNum = globalDataDict.findEntry("NumberOfFrames");
    const DcmDictEntry* dicentFps      = globalDataDict.findEntry("RecommendedDisplayFrameRate");

    /* note for later */
    printTagKeys[0] = new DcmTagKey(dicentFrameNum->getKey());
    printTagKeys[1] = new DcmTagKey(dicentFps->getKey());
    dcmDataDict.unlock();

    DcmFileFormat* dfile = new DcmFileFormat();
    DcmObject *dset = dfile;

    OFCondition cond = dfile->loadFile(filename.c_str(), xfer, EGL_noChange, maxReadLength, readMode);
    if (! cond.good()) {
        LERROR("Error loading file " << filename);
        return 0;
    }

    dfile->loadAllDataIntoMemory();

    DcmStack stack;
    if (dset->search(*printTagKeys[0], stack, ESM_fromHere, OFTrue) != EC_Normal)
        LERROR("Error searching TagKey");

    DcmIntegerString* frameNumObj = dynamic_cast<DcmIntegerString*>(stack.top());

    if (dset->search(*printTagKeys[1], stack, ESM_fromHere, OFTrue) != EC_Normal)
        LERROR("Error searching TagKey");

    DcmIntegerString* fpsObj = dynamic_cast<DcmIntegerString*>(stack.top());

    if(!frameNumObj || !fpsObj)
        LERROR("Error during dynamic-cast");

    Sint32 fn;
    Sint32 fp;

    frameNumObj->getSint32(fn);
    fpsObj->getSint32(fp);

    frameCount = fn;
    fps = fp;

    return dfile;
}

} // namespace voreen

#endif //VRN_WITH_DCMTK
