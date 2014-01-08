/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "dcmtkvolumereader.h"

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/meta/primitivemetadata.h"
#include "voreen/core/utils/stringutils.h"

#include "tgt/texture.h"
#include "voreen/core/io/progressbar.h"

#include "voreendcmtk.h"
#include "dcmtkmovescu.h"
#include "dcmtkfindscu.h"

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
#include <dcmtk/dcmimgle/didocu.h>
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
#if defined(VRN_DCMTK_VERSION_354)
#include <dcmtk/dcmdata/dcdebug.h>
#endif
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
//#undef PACKAGE_VERSION

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

using std::vector;
using std::pair;
using std::string;

const std::string DcmtkVolumeReader::loggerCat_ = "voreen.dcmtk.DcmtkVolumeReader";

DcmtkVolumeReader::DcmtkVolumeReader(ProgressBar* progress)
    : VolumeReader(progress),
      scalars_(0)
{
    extensions_.push_back("dcm");
    extensions_.push_back("ima");
    //extensions_.push_back("dicom");

    filenames_.push_back("DICOMDIR");

    protocols_.push_back("dicom");

    if (!dcmDataDict.isDictionaryLoaded()) {
        // The data dictionary is needed for loading Dicom files.
        //FIXME: This should raise an exception and inform the user.
        LERROR("Warning: no data dictionary loaded, check environment variable: " <<
               DCM_DICT_ENVIRONMENT_VARIABLE);
    }
    else {
        // register RLE and JPEG decompression codecs
        DcmRLEDecoderRegistration::registerCodecs(OFFalse /*pCreateSOPInstanceUID*/, OFTrue);
        DJDecoderRegistration::registerCodecs(EDC_photometricInterpretation, EUC_default, EPC_default, OFTrue);
    }

#ifdef VRN_DCMTK_VERSION_355
    LINFO("Dcmtk version: " << OFFIS_DCMTK_VERSION_STRING);
#endif
}

VolumeReader* DcmtkVolumeReader::create(ProgressBar* progress) const {
    return new DcmtkVolumeReader(progress);
}

DcmtkVolumeReader::~DcmtkVolumeReader() {
    DJDecoderRegistration::cleanup();
    DcmRLEDecoderRegistration::cleanup();
}

void DcmtkVolumeReader::setSecurityOptions(const DcmtkSecurityOptions& security) {
    security_ = security;
}

int DcmtkVolumeReader::loadSlice(const std::string& fileName, size_t posScalar) {

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
    if (!image.getOutputData(&scalars_[posScalar * bytesPerVoxel_], dx_ * dy_ * bytesPerVoxel_, bitsStored_)) {
        LERROR("Failed to render pixel data "
               << image.getOutputDataSize(bitsStored_) << " vs. " << dx_ * dy_ * bytesPerVoxel_);

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
 * Sorts strings according to a double value interpreted as the distance of the Image Origin to the Volume Origin
 */
bool slices_cmp_dist(std::pair<string, float> a, std::pair<string, float> b) {
   return a.second < b.second;
}

// needed because of some conflicts with std::tolower() and transform()
char mytolower(char c) {
    return std::tolower(static_cast<unsigned char>(c));
}

} // namespace


Volume* DcmtkVolumeReader::readDicomFiles(const vector<string> &fileNames,
                                          const string &filterSeriesInstanceUID,
                                          bool skipBroken)
    throw (tgt::FileException, std::bad_alloc)
{
    if (!fileNames.empty())
        LINFO("Loading DICOM data set: " << tgt::FileSystem::dirName(*fileNames.begin()));

    // register JPEG codec
    DJDecoderRegistration::registerCodecs(EDC_photometricInterpretation, EUC_default, EPC_default, OFFalse);

    //vector<pair<string, tgt::vec3> > slices;  // (filename, position)
    vector<pair<string, float> > slices; // (filename, distance)

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
    int i = 0;
    if (getProgressBar() && !fileNames.empty())
        getProgressBar()->setTitle("Loading DICOM Data Set");

    while (it_files != fileNames.end()) {
        if (getProgressBar()) {
            getProgressBar()->setProgressMessage("Reading slice '" + tgt::FileSystem::fileName(*it_files) + "' ...");
            getProgressBar()->setProgress(static_cast<float>(i) / static_cast<float>(fileNames.size()));
        }
        i++;

        DcmFileFormat fileformat;

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
                    bitsStored_ = atoi(tmpString.c_str());
                }
                else {
                    LERROR("Can't retrieve DCM_BitsStored from file " << (*it_files));
                    bitsStored_ = 16;//TODO
                    found_first = false;
                }
                if (dataset->findAndGetOFStringArray(DCM_SamplesPerPixel, tmpString).good()) {
                    samplesPerPixel_ = atoi(tmpString.c_str());
                }
                else {
                    LERROR("Can't retrieve DCM_SamplesPerPixel from file " << (*it_files));
                    samplesPerPixel_ = 1;
                    found_first = false;
                }

                LINFO("    Size: " << dx_ << "x" << dy_ << ", " << bitsStored_*samplesPerPixel_ << " bits");

                // Extract PixelSpacing
                OFString rowspacing_str, colspacing_str;
                if (dataset->findAndGetOFString(DCM_PixelSpacing, rowspacing_str, 0).good() &&
                    dataset->findAndGetOFString(DCM_PixelSpacing, colspacing_str, 1).good()) {
                    LINFO("    PixelSpacing: (" << rowspacing_str << "; " << colspacing_str << ")");
                    if (rowspacing_str != colspacing_str)
                        LWARNING("row-spacing != colspacing: " << rowspacing_str << " vs. " << colspacing_str);

                    rowspacing = static_cast<float>(stof(trim(string(rowspacing_str.c_str()))));
                    colspacing = static_cast<float>(stof(trim(string(colspacing_str.c_str()))));
                }

                // Extract Slice Orientation and calculate Slice Normal
                OFString tmpStrXOrX;
                OFString tmpStrXOrY;
                OFString tmpStrXOrZ;
                OFString tmpStrYOrX;
                OFString tmpStrYOrY;
                OFString tmpStrYOrZ;

                if (dataset->findAndGetOFString(DCM_ImageOrientationPatient, tmpStrXOrX, 0).good() &&
                    dataset->findAndGetOFString(DCM_ImageOrientationPatient, tmpStrXOrY, 1).good() &&
                    dataset->findAndGetOFString(DCM_ImageOrientationPatient, tmpStrXOrZ, 2).good() &&
                    dataset->findAndGetOFString(DCM_ImageOrientationPatient, tmpStrYOrX, 3).good() &&
                    dataset->findAndGetOFString(DCM_ImageOrientationPatient, tmpStrYOrY, 4).good() &&
                    dataset->findAndGetOFString(DCM_ImageOrientationPatient, tmpStrYOrZ, 5).good())
                {
                    xOrientation_.x = static_cast<float>(stof(trim(string(tmpStrXOrX.c_str()))));
                    xOrientation_.y = static_cast<float>(stof(trim(string(tmpStrXOrY.c_str()))));
                    xOrientation_.z = static_cast<float>(stof(trim(string(tmpStrXOrZ.c_str()))));
                    yOrientation_.x = static_cast<float>(stof(trim(string(tmpStrYOrX.c_str()))));
                    yOrientation_.y = static_cast<float>(stof(trim(string(tmpStrYOrY.c_str()))));
                    yOrientation_.z = static_cast<float>(stof(trim(string(tmpStrYOrZ.c_str()))));
                }
                else {
                    LERROR("Can't retrieve DCM_ImageOrientationPatient from file " << (*it_files));
                }

                //calculate slice normal
                normal_ = tgt::cross(xOrientation_,yOrientation_);
            }
        }

        // Matching series UID?
        if (seriesInstanceUID == filter) {
            OFString tmpStrPosX;
            OFString tmpStrPosY;
            OFString tmpStrPosZ;
            tgt::vec3 imagePositionPatient;
            OFString imageType;
            // Extract ImagePositionPatient of the file
            if (dataset->findAndGetOFString(DCM_ImagePositionPatient, tmpStrPosX, 0).good() &&
                dataset->findAndGetOFString(DCM_ImagePositionPatient, tmpStrPosY, 1).good() &&
                dataset->findAndGetOFString(DCM_ImagePositionPatient, tmpStrPosZ, 2).good())
            {
                imagePositionPatient.x = static_cast<float>(stof(trim(string(tmpStrPosX.c_str()))));
                imagePositionPatient.y = static_cast<float>(stof(trim(string(tmpStrPosY.c_str()))));
                imagePositionPatient.z = static_cast<float>(stof(trim(string(tmpStrPosZ.c_str()))));
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
            //calculate distance along slice normal
            float dist = tgt::dot(normal_,imagePositionPatient);

            slices.push_back(make_pair(*it_files, dist));
        }
        else {
            LDEBUG("  File " << (*it_files) << " has different SeriesInstanceUID - skipping");
        }
        it_files++;
    }

    if (slices.size() == 0)
        throw tgt::CorruptedFileException("Found no DICOM slices");

    // Sort slices and determine slice spacing.
    float slicespacing = 1;

    if (slices.size() > 1) {
        std::sort(slices.begin(), slices.end(), slices_cmp_dist);

        slicespacing = slices[1].second - slices[0].second;

        if (slicespacing == 0.f) {
            LWARNING("z spacing is 0.0, correcting to 1.0");
            slicespacing = 1.f;
        }
    }

    //get offset from first slice
    OFString tmpX;
    OFString tmpY;
    OFString tmpZ;
    DcmFileFormat firstFileFormat;
    firstFileFormat.loadFile(slices[0].first.c_str());
    DcmDataset *firstDataSet = firstFileFormat.getDataset();
    if (firstDataSet->findAndGetOFString(DCM_ImagePositionPatient, tmpX, 0).good() &&
        firstDataSet->findAndGetOFString(DCM_ImagePositionPatient, tmpY, 1).good() &&
        firstDataSet->findAndGetOFString(DCM_ImagePositionPatient, tmpZ, 2).good())
    {
        offset_.x = static_cast<float>(stof(trim(string(tmpX.c_str()))));
        offset_.y = static_cast<float>(stof(trim(string(tmpY.c_str()))));
        offset_.z = static_cast<float>(stof(trim(string(tmpZ.c_str()))));
    }
    else
        offset_ = tgt::vec3(0.f, 0.f, 0.f);

    dz_ = static_cast<int>(slices.size());

    switch (bitsStored_*samplesPerPixel_) {
        case  8: bytesPerVoxel_ = 1; break;
        case 12: bytesPerVoxel_ = 2; break;
        case 16: bytesPerVoxel_ = 2; break;
        case 24: bytesPerVoxel_ = 3; break;
        case 32: bytesPerVoxel_ = 4; break;
        default:
            throw tgt::CorruptedFileException("Unknown bit depth", *it_files);
    }
    // casts needed to handle files > 4 GB
    scalars_ = new uint8_t[(size_t)dx_ * (size_t)dy_ * (size_t)dz_ * (size_t)bytesPerVoxel_];

    LINFO("We have " << dz_ << " slices. [" << dx_ << "x" << dy_ << "]");

    x_spacing = rowspacing;
    y_spacing = colspacing;
    z_spacing = slicespacing;

    LINFO("Spacing: (" << x_spacing << "; " << y_spacing << "; " << z_spacing << ")");

    // Now read the actual slices from the files
    LINFO("Building volume...");
    LINFO("Reading slice data from " << slices.size() << " files...");

    size_t posScalar = 0;
    std::vector<std::pair<string, float> >::iterator it_slices = slices.begin();
    i = 0;
    while (it_slices != slices.end()) {
        if (getProgressBar()) {
            getProgressBar()->setProgressMessage("Loading slice '" + tgt::FileSystem::fileName((*it_slices).first) + "' ...");
            getProgressBar()->setProgress(static_cast<float>(i) / static_cast<float>(slices.size()));
        }
        i++;

        int slicesize = loadSlice((*it_slices).first, posScalar);
        posScalar += slicesize;
        it_slices++;
    }
    if (getProgressBar())
        getProgressBar()->hide();

    // deregister global decompression codecs
    DJDecoderRegistration::cleanup();

    VolumeRAM* dataset = 0;

    switch (bitsStored_*samplesPerPixel_) {
    case 8:
        dataset = new VolumeRAM_UInt8(scalars_,
                                  tgt::ivec3(dx_, dy_, dz_));
        break;
    case 12:
    case 16:
        dataset = new VolumeRAM_UInt16(reinterpret_cast<uint16_t*>(scalars_),
                                   tgt::ivec3(dx_, dy_, dz_));
        break;
    case 24:
        dataset = new VolumeRAM_3xUInt8(reinterpret_cast<tgt::col3*>(scalars_),
            tgt::ivec3(dx_, dy_, dz_));
        break;
    case 32:
            dataset = new VolumeRAM_UInt32(reinterpret_cast<uint32_t*>(scalars_),
                                   tgt::ivec3(dx_, dy_, dz_));
        break;
    default:
        LERROR("Unsupported bit depth: " << bitsStored_*samplesPerPixel_);
        delete[] scalars_;
        scalars_ = 0;
        return 0;
    }

    LINFO("Building volume complete.");
    Volume* vh = new Volume(dataset, tgt::vec3(x_spacing, y_spacing, z_spacing), tgt::vec3(0.0f));
    vh->setOffset(offset_);

    //construct PhysicalToWorld-Matrix for correct orientation of Volume
    tgt::Matrix4<float> ptw(tgt::vec4(xOrientation_, 0), tgt::vec4(yOrientation_, 0), tgt::dvec4(normal_, 0), tgt::dvec4(0,0,0,1));
    vh->setPhysicalToWorldMatrix(ptw);

    if (vh) {
        //add Meta Data
        OFString tmpString;
        voreen::DcmtkSeriesInfo tmp;
        if (firstDataSet->findAndGetOFString(DCM_SeriesInstanceUID, tmpString).good()) {
            tmp.uid_ = tmpString.c_str();
        }
    #if defined(VRN_DCMTK_VERSION_354)
        if (firstDataSet->findAndGetOFString(DCM_PatientsName, tmpString).good()) {
    #else
        if (firstDataSet->findAndGetOFString(DCM_PatientName, tmpString).good()) {
    #endif
           tmp.patientsName_ = tmpString.c_str();
           vh->getMetaDataContainer().addMetaData("PatientName", new StringMetaData(string(tmp.patientsName_)));
        }
        if (firstDataSet->findAndGetOFString(DCM_PatientID, tmpString).good()) {
           tmp.patientId_ = tmpString.c_str();
           vh->getMetaDataContainer().addMetaData("PatientID", new StringMetaData(string(tmp.patientId_)));
        }
        if (firstDataSet->findAndGetOFString(DCM_Modality, tmpString).good()) {
            tmp.modality_ = tmpString.c_str();
            vh->getMetaDataContainer().addMetaData("Modality", new StringMetaData(string(tmp.modality_)));
        }
        if (firstDataSet->findAndGetOFString(DCM_SeriesDescription, tmpString).good()) {
            tmp.description_ = tmpString.c_str();
            vh->getMetaDataContainer().addMetaData("SeriesDescription", new StringMetaData(string(tmp.description_)));
        }
        if (firstDataSet->findAndGetOFString(DCM_SeriesInstanceUID, tmpString).good()) {
            string seriesID(tmpString.c_str());
            vh->getMetaDataContainer().addMetaData("SeriesInstanceUID", new StringMetaData(seriesID));
        }
        if (firstDataSet->findAndGetOFString(DCM_StudyDate, tmpString).good()) {
           tmp.studyDate_ = tmpString.c_str();
           //vh->getMetaDataContainer().addMetaData("StudyDate", new StringMetaData(string(tmp.studyDate_)));
           // convert study date: format is either 'yyyymmdd' or 'yyyy.mm.dd'
           std::string dateStr = tmp.studyDate_;
           if (dateStr.length() != 8 && dateStr.length() != 10) {
               LWARNING("Study Date has unknown format: " << dateStr);
               vh->getMetaDataContainer().addMetaData("Study Date", new StringMetaData(tmp.studyDate_));
           }
           else { // date tag has valid format
               int year;
               int month;
               int day;
               if (dateStr.length() == 8) { // format: yyyymmdd
                   year = stoi(dateStr.substr(0, 4));
                   month = stoi(dateStr.substr(4, 2));
                   day = stoi(dateStr.substr(6, 2));
               }
               else if (dateStr.length() == 10) { // format: yyyy.mm.dd
                   year = stoi(dateStr.substr(0, 4));
                   month = stoi(dateStr.substr(5, 2));
                   day = stoi(dateStr.substr(8, 2));
               }
               DateTime date = DateTime::createDate(year, month, day);
               vh->getMetaDataContainer().addMetaData("Study Date", new DateTimeMetaData(date));
           }
        }
        if (firstDataSet->findAndGetOFString(DCM_StudyTime, tmpString).good()) {
            tmp.studyTime_ = tmpString.c_str();
            //vh->getMetaDataContainer().addMetaData("StudyTime", new StringMetaData(string(tmp.studyTime_)));
            // convert study time: format is either 'hhmmss.frac' or 'hh:mm:ss.frac'
            std::string timeStr = tmp.studyTime_;
            int hour, minute, second;
            hour = minute = second = -1;
            std::vector<std::string> fracSplit = strSplit(timeStr, '.'); // remove fractional part
            if (fracSplit.size() == 2) {
                std::vector<std::string> timeSplit = strSplit(fracSplit[0], ':');
                if (timeSplit.size() == 1) { // format: 'hhmmss'
                    hour = stoi(timeSplit[0].substr(0, 2));
                    minute = stoi(timeSplit[0].substr(2, 2));
                    second = stoi(timeSplit[0].substr(4, 2));
                }
                else if (timeSplit.size() == 3) { // format: 'hh:mm:ss'
                    hour = stoi(timeSplit[0]);
                    minute = stoi(timeSplit[1]);
                    second = stoi(timeSplit[2]);
                }
            }
            if (hour > -1 && minute > -1 && second > -1) {
                DateTime time = DateTime::createTime(hour, minute, second, 0);
                vh->getMetaDataContainer().addMetaData("Study Time", new DateTimeMetaData(time));
            }
            else {
                LWARNING("Study Time has unknown format: " << timeStr);
                vh->getMetaDataContainer().addMetaData("Study Time", new StringMetaData(timeStr));
            }
        }
    }

    return vh;
}

vector<string> DcmtkVolumeReader::getFileNamesInDir(const string& dirName) {

    std::vector<std::string> fileNames = tgt::FileSystem::readDirectory(dirName);
    for (size_t i=0; i<fileNames.size(); i++)
        fileNames[i] = dirName + "/" + fileNames[i];

    return fileNames;
}


Volume* DcmtkVolumeReader::readDICOMDIR(const string &fileName,
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
#if defined(VRN_DCMTK_VERSION_354)
        LINFO("Patient Name : " << getItemString(PatientRecord, DCM_PatientsName));
#else
        LINFO("Patient Name : " << getItemString(PatientRecord, DCM_PatientName));
#endif
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


Volume* DcmtkVolumeReader::readDicomFile(const string &fileName) {
    vector<string> files;
    files.push_back(fileName);
    return readDicomFiles(files, "");
}


bool DcmtkVolumeReader::findSeriesDicomFile(const string &fileName, vector<DcmtkSeriesInfo> &series) const {

    DcmFileFormat fileformat;
    LINFO("  Reading file " << fileName);

    OFCondition status = fileformat.loadFile(fileName.c_str());
    if (status.bad()) {
        LERROR("Error loading file " << fileName << ": " << status.text());
        return false;
    }

    DcmDataset *dataset = fileformat.getDataset();

    OFString tmpString;
    voreen::DcmtkSeriesInfo tmp;
    if (dataset->findAndGetOFString(DCM_SeriesInstanceUID, tmpString).good()) {
        tmp.uid_ = tmpString.c_str();
#if defined(VRN_DCMTK_VERSION_354)
        if (dataset->findAndGetOFString(DCM_PatientsName, tmpString).good()) {
#else
        if (dataset->findAndGetOFString(DCM_PatientName, tmpString).good()) {
#endif
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

bool DcmtkVolumeReader::findSeriesDicomDir(const string &fileName, vector<DcmtkSeriesInfo> &series) const {

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
    voreen::DcmtkSeriesInfo tmp;

    // Analyze DICOMDIR:
    while ((PatientRecord = root->nextSub(PatientRecord)) != NULL) {
        // patient level
        tmp = voreen::DcmtkSeriesInfo();
#if defined(VRN_DCMTK_VERSION_354)
        extractString(PatientRecord, DCM_PatientsName, tmp.patientsName_);
#else
        extractString(PatientRecord, DCM_PatientName, tmp.patientsName_);
#endif
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
    // ^/([A-Za-z0-9.\-_]+)\?SeriesInstanceUID=[0-9\.]$
    const string number_point = "0123456789.";
    const string peer_name = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-" + number_point;
    string s(path);

    // ^/
    if (path.substr(0, 1) != "/") return false;

    // ([A-Za-z0-9.\-_]+)\?SeriesInstanceUID=
    size_t pos = s.find_first_not_of(peer_name, 1);
    if (pos <= 1) return false;
    peerTitle = s.substr(1, pos - 1);
    size_t pos2 = s.find("?SeriesInstanceUID=", pos);
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


VolumeList* DcmtkVolumeReader::read(const string &url)
    throw (tgt::FileException, std::bad_alloc)
{
    VolumeURL origin(url);
    std::string fileName = origin.getPath();

    dx_ = dy_ = dz_ = 0;
    Volume* volume;

    std::string seriesInstanceUID = "";
    if (tgt::FileSystem::dirExists(fileName)) {
        // Handle reading of entire directories
        if (fileName.find("dicom://") == 0)
            volume = readDicomFiles(getFileNamesInDir(fileName.substr(8)), "", true);
        else
            volume = readDicomFiles(getFileNamesInDir(fileName), "", true);
    }
    /*else if (fileName.find("dicom://") == 0) {

        // Handle Dicom network connection to PACS server (TODO: currently not working)

        string connection = fileName.substr(0, fileName.find("/", 8));
        string path = fileName.substr(fileName.find("/", 8));
        string peertitle, seriesInstanceUID;

        if (!DcmtkMoveSCU::init(connection) || !analyze_URL_path(path, peertitle, seriesInstanceUID)) {
            LERROR("Invalid URL for DicomMoveSCU: " << fileName);
            return 0;
        }

        vector<string> files;
        if (DcmtkMoveSCU::moveSeries(seriesInstanceUID, peertitle, &files) != 0)
            return 0;

        volume = readDicomFiles(files);
        if (volume)
            volume->setOrigin(VolumeURL("dicom", fileName));
    } */
    else if (pathIsDicomDir(fileName)) {
        // Handle DICOMDIR: "../DICOMDIR?SeriesInstanceUID=<seriesID>"
        seriesInstanceUID = origin.getSearchParameter("SeriesInstanceUID", false);
        volume = readDICOMDIR(fileName, seriesInstanceUID);
    }
    else {
        /*
        volume = readDicomFile(fileName);
        */

        // Handle single dicom file: get SeriesUID as filter and load all files in the directory
        std::string dir = tgt::FileSystem::dirName(fileName);

        DcmFileFormat fileformat;

        OFCondition status = fileformat.loadFile(fileName.c_str());
        if (status.bad()) {
                LERROR("Error loading file " << fileName << ": " << status.text());
                throw tgt::FileException("Failed to load", fileName);
        }

        DcmDataset *dataset = fileformat.getDataset();

        OFString tmpString;
        if (dataset->findAndGetOFString(DCM_SeriesInstanceUID, tmpString).bad())
            LWARNING("no SeriesInstanceUID in file " << fileName);
        string id(tmpString.c_str());

        volume = readDicomFiles(getFileNamesInDir(dir),id,true);
    }

    if (volume) {
        VolumeList* volumeList = new VolumeList();
        volume->setOrigin(VolumeURL("dicom", fileName, (!seriesInstanceUID.empty() ? "SeriesInstanceUID=" + seriesInstanceUID : "")));
        volumeList->add(volume);
        return volumeList;
    }
    else {
        LERROR("Failed to load from URL: " << origin.getURL());
        throw tgt::FileException("Failed to load", origin.getURL());
    }
}

Volume* DcmtkVolumeReader::read(const std::vector<std::string> &fileNames) {
    dx_ = dy_ = dz_ = 0;
    return readDicomFiles(fileNames);
}

VolumeBase* DcmtkVolumeReader::read(const VolumeURL& origin)
    throw (tgt::FileException, std::bad_alloc)
{
    VolumeList* collection = read(origin.getURL());
    tgtAssert(collection && !collection->empty(), "exception expected");
    VolumeBase* result = collection->first();
    delete collection;
    return result;
}

vector<DcmtkSeriesInfo> DcmtkVolumeReader::listSeries(const string &fileName) const {
    vector<DcmtkSeriesInfo> series;

    if (fileName.find("dicom://") == 0) {
        DcmtkFindSCU::find(fileName, &series, security_);
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

std::vector<VolumeURL> DcmtkVolumeReader::listVolumes(const std::string& url) const
    throw (tgt::FileException)
{
    std::vector<VolumeURL> result;
    VolumeURL urlOrigin(url);
    // if URL already contains a seriesID, return it directly
    if (urlOrigin.getSearchParameter("SeriesInstanceUID", false) != "") {
        result.push_back(urlOrigin);
        return result;
    }

    // delegate to listSeries() and convert returned DcmtkSeriesInfos to VolumeURLs
    std::vector<DcmtkSeriesInfo> seriesInfos = listSeries(urlOrigin.getPath());
    for (size_t i=0; i<seriesInfos.size(); i++) {
        DcmtkSeriesInfo& info = seriesInfos.at(i);
        VolumeURL origin(url);
        origin.addSearchParameter("SeriesInstanceUID", info.uid_);
        origin.getMetaDataContainer().addMetaData("SeriesUID", new StringMetaData(info.uid_));
        origin.getMetaDataContainer().addMetaData("Num Slices", new StringMetaData(info.numImages_));
        origin.getMetaDataContainer().addMetaData("Patient Name", new StringMetaData(info.patientsName_));
        origin.getMetaDataContainer().addMetaData("Patient ID", new StringMetaData(info.patientId_));
        origin.getMetaDataContainer().addMetaData("Modality", new StringMetaData(info.modality_));
        origin.getMetaDataContainer().addMetaData("Description", new StringMetaData(info.description_));

        // convert study date: format is either 'yyyymmdd' or 'yyyy.mm.dd'
        std::string dateStr = info.studyDate_;
        if (dateStr.length() != 8 && dateStr.length() != 10) {
            LWARNING("Study Date has unknown format: " << dateStr);
            origin.getMetaDataContainer().addMetaData("Study Date", new StringMetaData(info.studyDate_));
        }
        else { // date tag has valid format
            int year;
            int month;
            int day;
            if (dateStr.length() == 8) { // format: yyyymmdd
                year = stoi(dateStr.substr(0, 4));
                month = stoi(dateStr.substr(4, 2));
                day = stoi(dateStr.substr(6, 2));
            }
            else if (dateStr.length() == 10) { // format: yyyy.mm.dd
                year = stoi(dateStr.substr(0, 4));
                month = stoi(dateStr.substr(5, 2));
                day = stoi(dateStr.substr(8, 2));
            }
            DateTime date = DateTime::createDate(year, month, day);
            origin.getMetaDataContainer().addMetaData("Study Date", new DateTimeMetaData(date));
        }

        // convert study time: format is either 'hhmmss.frac' or 'hh:mm:ss.frac'
        std::string timeStr = info.studyTime_;
        int hour, minute, second;
        hour = minute = second = -1;
        std::vector<std::string> fracSplit = strSplit(timeStr, '.'); // remove fractional part
        if (fracSplit.size() == 2) {
            std::vector<std::string> timeSplit = strSplit(fracSplit[0], ':');
            if (timeSplit.size() == 1) { // format: 'hhmmss'
                hour = stoi(timeSplit[0].substr(0, 2));
                minute = stoi(timeSplit[0].substr(2, 2));
                second = stoi(timeSplit[0].substr(4, 2));
            }
            else if (timeSplit.size() == 3) { // format: 'hh:mm:ss'
                hour = stoi(timeSplit[0]);
                minute = stoi(timeSplit[1]);
                second = stoi(timeSplit[2]);
            }
        }
        if (hour > -1 && minute > -1 && second > -1) {
            DateTime time = DateTime::createTime(hour, minute, second, 0);
            origin.getMetaDataContainer().addMetaData("Study Time", new DateTimeMetaData(time));
        }
        else {
            LWARNING("Study Time has unknown format: " << timeStr);
            origin.getMetaDataContainer().addMetaData("Study Time", new StringMetaData(timeStr));
        }

        result.push_back(origin);
    }
    return result;
}


DicomImage* DcmtkVolumeReader::readDicomImage(const std::string& fileName, int startFrame, int numFrames) {

    DicomImage* dicomImage = 0;

    // load dicom file
    try {
        LINFO("Reading DICOM file " << fileName << " ...");
        unsigned int flags = 0;
#ifdef VRN_DCMTK_VERSION_355
        flags = CIF_UsePartialAccessToPixelData;
#endif
        dicomImage = new DicomImage(fileName.c_str(), flags, startFrame, numFrames);
    }
    catch (std::exception& e) {
        LERROR("Failed to load file " << fileName << ": " << e.what());
        dicomImage = 0;
    }

    // check for errors
    if (dicomImage && dicomImage->getStatus() == EIS_Normal) {
        LINFO("num frames: " << dicomImage->getFrameCount()
            << ", dimensions: " << dicomImage->getWidth() << "x" << dicomImage->getHeight()
            << ", bit depth: " << dicomImage->getDepth()
            << ", color model: " << DicomImage::getString(dicomImage->getPhotometricInterpretation())
            << ", frame size: " << dicomImage->getOutputDataSize() << " bytes");
    }
    else if (dicomImage) {
        LERROR("Loading dicom file " << fileName << " failed: " << DicomImage::getString(dicomImage->getStatus()));
        delete dicomImage;
        dicomImage = 0;
    }
    else {
        LERROR("Loading dicom file " << fileName << " failed");
    }

    return dicomImage;
}

tgt::Texture* DcmtkVolumeReader::readSingleSlice(DicomImage* image, int frame) {

    // check parameters
    tgtAssert(image, "No dicom image object");
    if (frame < 0 || frame >= (int)image->getFrameCount()) {
        LWARNING("Invalid frame number: " << frame);
        return 0;
    }

    // delegate loading to readSliceSequence
    std::vector<tgt::Texture*> sequence = readSliceSequence(image, frame, 1);
    tgtAssert(sequence.size() <= 1, "Sequence with more than 1 element returned.");
    if (sequence.empty())
        return 0;
    else
        return sequence.front();
}

tgt::Texture* DcmtkVolumeReader::readSingleSlice(const std::string& fileName, int frame) {

    // delegate loading to readSliceSequence
    std::vector<tgt::Texture*> sequence = readSliceSequence(fileName, frame, 1);
    tgtAssert(sequence.size() <= 1, "Sequence with more than 1 element returned.");
    if (sequence.empty())
        return 0;
    else
        return sequence.front();
}

std::vector<tgt::Texture*> DcmtkVolumeReader::readSliceSequence(DicomImage* dicomImage, int startFrame, int numFrames) {

    std::vector<tgt::Texture*> result;

    // check parameters
    tgtAssert(dicomImage, "No dicom image object");
    if (startFrame < 0 || startFrame >= (int)dicomImage->getFrameCount()) {
        LWARNING("Invalid start frame number: " << startFrame);
        return result;
    }
    if (numFrames < 0) {
        LWARNING("Invalid frame count: " << numFrames);
        return result;
    }

    // check bit depth
    GLenum dataType;
    switch (dicomImage->getDepth()) {
        case 8:
            dataType = GL_UNSIGNED_BYTE;
            break;
        default:
            LERROR("Bit depth " << dicomImage->getDepth() << " not supported.");
            return result;
    }

    // check color model
    GLint format;
    if (dicomImage->isMonochrome()) {
        format = GL_LUMINANCE;
    }
    else if (dicomImage->getPhotometricInterpretation() == EPI_RGB) {
        format = GL_RGB;
    }
    else {
        LERROR("Color model " << DicomImage::getString(dicomImage->getPhotometricInterpretation()) << " not supported.");
        return result;
    }

    // check image data buffer size (size of a single frame)
    int numPixels = dicomImage->getWidth()*dicomImage->getHeight();
    if (format == GL_LUMINANCE) {
        if (static_cast<int>(dicomImage->getOutputDataSize()) != numPixels) {
            LERROR("Unexpected frame data size: " << dicomImage->getOutputDataSize());
            return result;
        }
    }
    else if (format == GL_RGB) {
        if (static_cast<int>(dicomImage->getOutputDataSize()) != 3*numPixels) {
            LERROR("Unexpected frame data size: " << dicomImage->getOutputDataSize());
            return result;
        }
    }

    // calculate index of last frame to extract
    int endFrame;
    if (numFrames <= 0 || (numFrames > ((int)dicomImage->getFrameCount()))) {
        endFrame = dicomImage->getFrameCount() - 1;
    }
    else {
        endFrame = startFrame + (numFrames-1);
    }
    tgtAssert(endFrame >= 0 && endFrame < (int)dicomImage->getFrameCount(), "Invalid end frame id");

    // for each frame: render pixel data into texture array and create texture from it
    tgt::ivec3 frameDim = tgt::ivec3(dicomImage->getWidth(), dicomImage->getHeight(), 1);
    for (int frame = startFrame; frame <= endFrame; frame++) {

        // render pixel data
        uint8_t* texData = new uint8_t[dicomImage->getOutputDataSize()];
        if (!dicomImage->getOutputData(texData, dicomImage->getOutputDataSize(), 8, frame)) {
            LERROR("Failed to render pixel data for frame: " << frame);
            delete[] texData;
        }
        else {
            // create texture
            tgt::Texture* tex = new tgt::Texture(texData, frameDim, format, dataType);
            if (tex) {
                result.push_back(tex);
            }
            else {
                LERROR("Failed to create texture for frame: " << frame);
                delete[] texData;
            }
        }
    }

    return result;
}

std::vector<tgt::Texture*> DcmtkVolumeReader::readSliceSequence(const std::string& fileName,
    int startFrame, int numFrames) {

    std::vector<tgt::Texture*> result;

    DicomImage* dicomImage = readDicomImage(fileName, startFrame, numFrames);
    if (!dicomImage)
        return result;
    else
        result = readSliceSequence(dicomImage, 0, numFrames);
    delete dicomImage;

    return result;
}

} // namespace
