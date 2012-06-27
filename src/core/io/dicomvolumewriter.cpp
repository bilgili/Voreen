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

#ifdef VRN_WITH_DCMTK

#include "voreen/core/io/dicomvolumewriter.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"

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

#include <dcmtk/dcmjpeg/djdecode.h>    /* for dcmjpeg decoders */
#include <dcmtk/dcmjpeg/dipijpeg.h>    /* for dcmimage JPEG plugin */
#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmdata/dcuid.h>

#include <dcmtk/dcmdata/dcdict.h>
#include <dcmtk/dcmdata/dcvrda.h>  // DcmDate
#include <dcmtk/dcmdata/dcvrtm.h>  // DcmTime
#include <dcmtk/ofstd/ofdatime.h>
#include <dcmtk/dcmjpeg/djencode.h>  /* for dcmjpeg encoders */
#include <dcmtk/dcmjpeg/djrplol.h>   /* for DJ_RPLossless */
#include <dcmtk/dcmdata/dctag.h>

// Prevent warnings because of redefined symbols
#undef PACKAGE_NAME
#undef PACKAGE_BUGREPORT
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

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

#include <fstream>
#include <iostream>
#include <map>
#include <algorithm>

using std::string;
using std::vector;
using std::pair;

namespace voreen {

const std::string DicomVolumeDescription::loggerCat_ = "voreen.io.DicomVolumeDescription";

DicomVolumeDescription::DicomVolumeDescription(const std::string &studyInstanceUID,
                                               const std::string &seriesInstanceUID)
    : modality(MODALITY_CT)
{
    char uid[100];
    if (studyInstanceUID.empty()) {
        dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT);
        this->studyInstanceUID = uid;
    } else {
        this->studyInstanceUID = studyInstanceUID;
    }


    if (seriesInstanceUID.empty()) {
        dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT);
        this->seriesInstanceUID = uid;
    } else {
        this->seriesInstanceUID = seriesInstanceUID;
    }
}

bool DicomVolumeDescription::addField(const string &tag, const string &value) {
    // Check if tag is valid
    DcmTag tmp;
    if (DcmTag::findTagFromName(tag.c_str(), tmp).bad()) {
        LWARNING("addField failed for tag: " << tag);
        return false;
    }

    fields.push_back(pair<string, string>(tag, value));
    return true;
}

const std::string DicomVolumeWriter::loggerCat_("voreen.io.DicomVolumeWriter");

DicomVolumeWriter::DicomVolumeWriter() {
}

bool DicomVolumeWriter::writeDicomFile(const DicomVolumeDescription &desc, Volume* volume,
                                       int layer, DcmFileFormat &fileformat, const string &fileName) {

    LINFO("Writing file " << fileName << "...");

    char* voxels = 0;

    if (VolumeUInt8* v = dynamic_cast<VolumeUInt8*>(volume))
        voxels = (char*)v->voxel();
    else if (VolumeUInt16* v = dynamic_cast<VolumeUInt16*>(volume))
        voxels = (char*)v->voxel();

    if (voxels == 0) {
        LWARNING("Invalid volume format");
        return false;
    }

    char uid[100];
    DcmDataset *dataset = fileformat.getDataset();

    OFCondition cond;

    string classUID;
    string modalityName;
    switch (desc.getModality()) {
    case DicomVolumeDescription::MODALITY_CT:
        classUID = UID_CTImageStorage;
        modalityName = "CT";
        break;
    case DicomVolumeDescription::MODALITY_PET:
        classUID = UID_PETImageStorage;
        modalityName = "PT";
        break;
    default:
        LERROR("Unknown modality!");
        return false;
    }

    cond = dataset->putAndInsertString(DCM_SOPClassUID, classUID.c_str());
    dataset->putAndInsertString(DCM_Modality, modalityName.c_str());

    dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT);
    dataset->putAndInsertString(DCM_SOPInstanceUID, uid);

    // register global compression codecs
    DJEncoderRegistration::registerCodecs();

    dataset->putAndInsertUint16(DCM_BitsAllocated, volume->getBitsAllocated());
    dataset->putAndInsertUint16(DCM_BitsStored, volume->getBitsStored());
    dataset->putAndInsertUint16(DCM_HighBit, volume->getBitsStored() - 1); // Specifies most significant bit of
                                                                           // sample data (= BitsStored - 1)
    dataset->putAndInsertUint16(DCM_SamplesPerPixel, 1);
    dataset->putAndInsertString(DCM_PixelRepresentation, "0");

    dataset->putAndInsertUint16(DCM_Rows, volume->getDimensions().x);
    dataset->putAndInsertUint16(DCM_Columns, volume->getDimensions().y);
    dataset->putAndInsertString(DCM_PhotometricInterpretation, "MONOCHROME2");

    // set instance creation date and time
    OFString s;
    DcmDate::getCurrentDate(s);
    dataset->putAndInsertOFStringArray(DCM_InstanceCreationDate, s);
    DcmTime::getCurrentTime(s);
    dataset->putAndInsertOFStringArray(DCM_InstanceCreationTime, s);

    // insert empty type 2 attributes
    dataset->insertEmptyElement(DCM_StudyDate);
    dataset->insertEmptyElement(DCM_StudyTime);
    dataset->insertEmptyElement(DCM_AccessionNumber);
    dataset->insertEmptyElement(DCM_Manufacturer);
    dataset->insertEmptyElement(DCM_ReferringPhysiciansName);
    dataset->insertEmptyElement(DCM_StudyID);
    dataset->insertEmptyElement(DCM_ContentDate);
    dataset->insertEmptyElement(DCM_ContentTime);

    // there is no way we could determine a meaningful series number, so we just use a constant.
    dataset->putAndInsertString(DCM_SeriesNumber, "1");

    // Now add user-defined data fields
    std::vector<std::pair<std::string, std::string> > fields = desc.getFields();
    for (size_t i=0; i < fields.size(); i++) {
        DcmTag tag;
        DcmTag::findTagFromName(fields[i].first.c_str(), tag); // Validity was checked before
        dataset->putAndInsertString(tag.getXTag(), fields[i].second.c_str());
    }

    int pixelLength = volume->getDimensions().x * volume->getDimensions().y * volume->getBytesPerVoxel();
    char *pixelData = (char*)(voxels + pixelLength * layer);

    dataset->putAndInsertUint8Array(DCM_PixelData, (const Uint8*)pixelData, pixelLength);

    DJ_RPLossless rp_lossless(6, 0);
    dataset->chooseRepresentation(EXS_JPEGProcess14TransferSyntax, &rp_lossless);

    OFCondition status = fileformat.saveFile(fileName.c_str(),
                                             EXS_JPEGProcess14TransferSyntax); //EXS_LittleEndianExplicit

    if (status.bad())
        LERROR("Cannot write DICOM file (" << status.text() << ")");

    DJEncoderRegistration::cleanup();

    return true;
}

bool DicomVolumeWriter::writeDicomFiles(const DicomVolumeDescription &desc,  Volume*volume,
                                        const string &fileNamePrefix, const string &fileNameSuffix) {

    /* make sure data dictionary is loaded */
    if (!dcmDataDict.isDictionaryLoaded())
    {
        LWARNING("No data dictionary loaded");
    }

    if (volume == 0) return false;

    for (int i=0; i < volume->getDimensions().z; i++) {

        DcmFileFormat fileformat;
        DcmDataset *dataset = fileformat.getDataset();

        dataset->putAndInsertString(DCM_StudyInstanceUID, desc.getStudyInstanceUID().c_str());
        dataset->putAndInsertString(DCM_SeriesInstanceUID, desc.getSeriesInstanceUID().c_str());

        // Insert (fake) AcquisitionDate and -Time
        OFString date;
        DcmDate::getCurrentDate(date);
        dataset->putAndInsertOFStringArray(DCM_AcquisitionDate, date);

        OFString time;
        DcmTime::getCurrentTime(time);
        char s[64];
        sprintf((char*)&s, "%s.%05d", time.c_str(), i);
        dataset->putAndInsertOFStringArray(DCM_AcquisitionTime, s);


        // InstanceNumber is incremented for each slice
        sprintf((char*)&s, "%d", i + 1);
        dataset->putAndInsertString(DCM_InstanceNumber, s, true);

        // Simulate DCM_ImagePositionPatient
        sprintf((char*)&s, "0\\0\\%d", i);
        dataset->putAndInsertString(DCM_ImagePositionPatient, s, true);

        char filename[1000];
        sprintf((char*)&filename, "%s%03i%s", fileNamePrefix.c_str(), i + 1, fileNameSuffix.c_str());
        writeDicomFile(desc, volume, i, fileformat, filename);
    }

    return true;
}

} // namespace voreen

#endif // VRN_WITH_DCMTK
