/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#ifndef VRN_GDCMVOLUMEREADER_H
#define VRN_GDCMVOLUMEREADER_H

#include "voreen/core/io/volumereader.h"
#include "../gdcmmodule.h"

// include this before any GDCM header in order to make sure that C99 types are defined
#include "tgt/types.h"

#ifdef VRN_GDCM_VERSION_22
#include <gdcm-2.2/gdcmImageReader.h>
#include <gdcm-2.2/gdcmStringFilter.h>
#else
#include <gdcm-2.0/gdcmImageReader.h>
#include <gdcm-2.0/gdcmStringFilter.h>
#endif

#include "../dicominfo.h"
#include "../dicomdict.h"
#include "../customdicomdict.h"

#include "dicomnetworkconnector.h"

#include <string>
#include <vector>

namespace voreen {

/**
 * Reader for volume data in DICOM format, based on GDCM (Grassroots DICOM) v2.
 *
 * @see http://sourceforge.net/apps/mediawiki/gdcm/index.php?title=Main_Page
 */
class VRN_CORE_API GdcmVolumeReader : public VolumeReader
{
public:
    GdcmVolumeReader(voreen::ProgressBar* progress = 0);
    virtual ~GdcmVolumeReader();
    virtual VolumeReader* create(ProgressBar* progress = 0) const;

    virtual std::string getClassName() const   { return "GdcmVolumeReader"; }
    virtual std::string getFormatDescription() const { return "DICOM format, using GDCMv2 library"; }

    /**
     * Reads the DICOM file (or files in a given directory or DICOMDIR) and returns a VolumeCollection constructed of it.
     *
     * @param url Specifies the Dicom dataset. This can be:
     * <ul>
     *
     *   <li>Filename of a single Dicom image. In this case, all DICOM files in the directory
     *   are used, filtered by the SeriesInstanceUID of the given file</li>
     *
     *   <li>Path to a "DICOMDIR" file.</li>
     *
     *   <li>A pathname ending with "/" or "\": All Dicom files inside
     *   the specified directory are searched</li>
     *
     * </ul>
     * Additional information in the form of a VolumeURL-url may be included in the url and is considered
     *
     * @return a VolumeCollection* constructed of the given data
     */
    virtual VolumeCollection* read(const std::string& url)
        throw (tgt::FileException, std::bad_alloc);

    /**
     * Reads the DICOM Dataset specified by the VolumeURL and returns a Volume constructed of it.
     * If the VolumeURL contains additional SearchParameters (e.g. SeriesInstanceUID), the files are selected by this information, so
     * that only the data corresponding to these SearchParameters is loaded.
     *
     * @param origin the origin of the Volume to be loaded
     *
     * @return a VolumeBase* constructed of the given data
     */
    virtual VolumeBase* read(const VolumeURL& origin)
        throw (tgt::FileException, std::bad_alloc);


    /**
     * Loads the Standard Dictionary from an XML file.
     * The XML file of the Dictionary has to be located in <Gdcm-Module>/Dicts and named "StandardDictionary.xml"
     */
    void loadStandardDict() const throw (tgt::FileException);

    /**
     * Implementation of the VolumeReader interface.
     * Lists all possible Volumes to be loaded as VolumeURLs
     *
     * @param url Specifies the Dicom dataset. This can be:
     * <ul>
     *
     *   <li>Filename of a single Dicom image. In this case, all DICOM files in the directory
     *   are used, filtered by the SeriesInstanceUID of the given file</li>
     *
     *   <li>Path to a "DICOMDIR" file.</li>
     *
     *   <li>A pathname ending with "/" or "\": All Dicom files inside
     *   the specified directory are searched</li>
     *
     * </ul>
     * Additional information in the form of a VolumeURL-url may be included in the url and is considered
     */
    virtual std::vector<VolumeURL> listVolumes(const std::string& url) const
        throw (tgt::FileException);

    /**
     * Reimplementation of the method in VolumeReader base class that works also with network paths in "dicom-scp" protocol (which are not changed).
     */
    virtual VolumeURL convertOriginToRelativePath(const VolumeURL& origin, const std::string& basePath) const;

    /**
     * Reimplementation of the method in VolumeReader base class that works also with network paths in "dicom-scp" protocol (which are not changed).
     */
    virtual VolumeURL convertOriginToAbsolutePath(const VolumeURL& origin, const std::string& basePath) const;

#ifdef VRN_GDCM_VERSION_22 // network support
    /// find patients of a DICOM SCP using C-FIND
    std::vector<PatientInfo> findNetworkPatients(const std::string& remote, const std::string& call, uint16_t portno) const throw (tgt::FileException);

    /// find studies for a patient of a DICOM SCP using C-FIND
    std::vector<StudyInfo> findNetworkStudies(const std::string& remote, const std::string& call, uint16_t portno, const std::string& patientID) const throw (tgt::FileException);

    /// find series in a study of a patient of a DICOM SCP using C-FIND
    std::vector<SeriesInfo> findNetworkSeries(const std::string& remote, const std::string& call, uint16_t portno, const std::string& patientID, const std::string& StudyID) const throw (tgt::FileException);
#endif

private:

    /**
     * Helper method that returns all filenames contained in a given directory.
     */
    virtual std::vector<std::string> getFileNamesInDir(const std::string& dirName) const;

    /**
     * Helper method that determines if a file exists and is a readable Dicom file.
     *
     * @param url the path to the file that should be tested
     * @return true, if the file is a Dicom File, false else
     */
    virtual bool isDicomFile(const std::string &url) const;

    /**
     * Helper method that determines if a file is a DICOMDIR.
     *
     * @param url the path to the file that should be tested
     * @return true, if the file is a DICOMDIR, false else
     */
    virtual bool isDicomDir(const std::string &url) const;

    /**
     * Helper method that -if present- removes single space at the end of a string.
     *
     * @param s the string that should be trimmed
     */
    /*static void trimString(std::string *s);*/

    /**
     * Helper function that returns a Gdcm::Tag constructed by the information of the DicomDictEntry given.
     */
    static gdcm::Tag getTagFromDictEntry(const DicomDictEntry& entry);

    /**
     * Sets the Meta Data specified by the DicomDict to the MetaDataContainer, using the given gdcm::StringFilter
     */
    static void setMetaDataFromDict(MetaDataContainer* container, const DicomDict* dict, gdcm::StringFilter* sf);

    /**
     * Helper method that returns all CustomDicomDicts in the directory <Gdcm-Module>/Dicts/CustomDicts
     */
    std::vector<CustomDicomDict> getCustomDicts() const;

    /**
     * Helper method that filters the files by the given SeriesInstanceUID.
     * All files have to be readable DICOM files and StandardDictionary has to be already loaded!
     */
    std::vector<std::string> getFilesInSeries(std::vector<std::string> filenames, std::string seriesInstanceUID) const;

    /**
     * Helper method that loads a DICOMDIR file.
     * the method subdivideAndLoadDicomFiles is called per SeriesInstanceUID found in the DICOMDIR
     * -> calls subdivideAndLoadFiles for each Series found in the DICOMDIR
     *
     * @param origin VolumeURL for the file to be loaded (eventually containing additional Search tags)
     */
    virtual VolumeCollection* readDicomDir(const VolumeURL &origin)
                        throw (tgt::FileException, std::bad_alloc);

    /**
     * Helper function that gets the filenames and a VolumeURL.
     * If there is no SeriesInstanceUID-searchParameter in the VolumeURL, the SeriesInstanceUID of the first file is used.
     * The given files are checked and files with different SeriesInstanceUID or files that could not be read are ignored.
     * -> calls method subdivideAndLoadDicomFiles
     *
     * @param fileNames the files that should be selected and loaded
     * @param origin VolumeURL containing at least the path
     *
     * @return a collection of Volumes constructed out of the given files
     */
    virtual VolumeCollection* selectAndLoadDicomFiles(const std::vector<std::string> &fileNames, const VolumeURL &origin)
                                throw (tgt::FileException, std::bad_alloc);

    /**
     * Helper function before the actual loading (using readDicomFiles): Checks, if any available CustomDicomDict fits the given files (that already should all have the same SeriesInstanceUID).
     * Subdivides and selects the files according to that CustomDicomDict and loads every group by calling readDicomFiles.
     *
     * @param fileNames the files that should be loaded
     * @param origin VolumeURL containing at least the path and also the SeriesInstanceUID of the files
     *
     * @return a collection of Volumes constructed out of the given files
     */
    virtual VolumeCollection* subdivideAndLoadDicomFiles(const std::vector<std::string> &fileNames, const VolumeURL &origin)
                              throw (tgt::FileException, std::bad_alloc);

    /**
     * Helper function that does the actual loading of all files given.
     * Awaits files of one SeriesInstanceUID that belong to one Volume (should only be called by subdivideAndLoadDicomFiles).
     *
     * @param fileNames vector with the fileNames of the files to load
     * @param origin the VolumeURL of the Volume to be loaded
     *
     * @return if reading was succesful, a Volume* constructed of the DICOM slices will be returned
     */
    virtual Volume* readDicomFiles(const std::vector<std::string> &fileNames, const VolumeURL &origin)
                              throw (tgt::FileException, std::bad_alloc);

    /**
     * Helper function that reads a single slice.
     *
     * @param fileName name of the file to be loaded
     * @param posScalar the position in the scalars_ array where this particular slice's pixel data should begin
     * @param numberOfFrames the number of frames for multiframe images (for non-multiframe images 1)
     *
     * @return returns the number of voxels rendered
     */
    virtual int loadSlice(const std::string& fileName, size_t posScalar, int numberOfFrames = 1);

    /**
     * Helper function that constructs the correct type of MetaData Object by knowing its type from a DictEntry and its value
     */
    static MetaDataBase* constructMetaData(const DicomDictEntry &entry, const std::string &value);

    /**
     * Helper function that tries to construct a DateTimeMetaData.
     * The function tries to use (in this order) AcquisitionDateTime, AcquisitionDate and AcquisitionTime, SeriesDate and SeriesTime, and StudyDate and StudyTime.
     * If the construction fails (e.g. because none of the tag combination is found), the function will return 0.
     */
    virtual MetaDataBase* constructVolumeDateTime(const DicomDict* dict, gdcm::StringFilter* sf) const;

    /**
     * Helper function that lists the VolumeURL found in a Single DICOM image file (including the necessary search strings)
     */
    virtual std::vector<VolumeURL> listVolumesSingleDicomImage(const VolumeURL& origin) const
        throw (tgt::FileException);

    /**
     * Helper function that lists all DICOM VolumeURLs found in a Directory (including the necessary search strings)
     */
    virtual std::vector<VolumeURL> listVolumesDirectory(const VolumeURL& origin) const
        throw (tgt::FileException);

    /**
     * Helper function that lists all DICOM VolumeURLs found in a DICOMDIR (including the necessary search strings)
     */
    virtual std::vector<VolumeURL> listVolumesDicomDir(const VolumeURL& origin) const
        throw (tgt::FileException);

    /**
     * Helper method that subdivides DICOM Image files by the tags defined in the CustomDicomDict (subdivisionTags)
     *
     * @return for each group that was found, a std::vector<std::string> is be returned, all together in one std::vector
     */
    virtual std::vector<std::vector<std::string> > subdivideSeriesFilesByCustomDict(std::vector<std::string> filenames, CustomDicomDict customDict) const;

    /**
     * Helper method that uses search parameters of a VolumeURL with "dicom-scp" protocol to construct a local temporary path
     */
    std::string constructLocalPathFromNetworkOrigin(const VolumeURL& o) const;

    DicomInfo info_; ///< Object containing all relevant meta information about the volume

    char* scalars_; ///< contains the actual pixel data of all slices

    gdcm::PixelFormat::ScalarType scalarType_; ///< type of the scalar data

    static const std::string loggerCat_;

    mutable DicomDict* dict_; ///< pointer to the Standard Dictionary (when loaded)
};

}

#endif // VRN_GDCMVOLUMEREADER_H
