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

#ifndef VRN_DICOMVOLUMEREADER_H
#define VRN_DICOMVOLUMEREADER_H

#include "voreen/core/io/volumereader.h"
#include "voreen/core/io/dicomseriesinfo.h"
#include "voreen/core/volume/volume.h"
#include "voreen/core/volume/modality.h"

#include <string>
#include <vector>

class DcmFileFormat;
class DicomImage;
class voreen::Volume;

namespace voreen {

/**
 * Loader for Dicom datasets. As Dicom has no direct support for volumetric datasets, volumes
 * are specified as series of slice images, each stored in a single file.
 */
class DicomVolumeReader : public VolumeReader {
public:
    DicomVolumeReader();
    virtual ~DicomVolumeReader() {}

    void setSecurityOptions(const DicomSecurityOptions& security);

    /**
     * Loads a Dicom volume dataset.
     *
     * @param fileName Specifies the Dicom dataset. This can be:
     * <ul>
     *
     *   <li>Filename of a single Dicom image. (This is not very
     *   useful, since a single file can not contain multiple slices.
     *   It may contain multiple frames, but these are only used for
     *   animation)</li>
     *
     *   <li>Path to a "DICOMDIR" file: This loads all image
     *   referenced in the DICOMDIR. To specify the series that should
     *   be loaded, add the series UID to the file name, separated by
     *   '?'. If no series UID is specified, the first series found in
     *   der DICOMDIR is loaded.</li>
     *
     *   <li>A pathname ending with "/" or "\": All Dicom files inside
     *   the specified directory are loaded</li>
     *
     *   <li>A URL starting with "dicom://": Images are retrieved via
     *   the network from the specified PACS. Such a URL must look
     *   like:<br>
     *   "dicom://<ourtitle>:<retrievePort>@<peer>:<port>/<peertitle>?seriesInstanceUID=<uid>"</li>
     *
     * </ul>
     *
     * @return Volume or 0 if an error occured.
     */
    virtual VolumeSet* read(const std::string& fileName)
        throw(tgt::CorruptedFileException, tgt::IOException, std::bad_alloc);

    /**
     * Loads a single Dicom slice into a 2D-tgt::Texture.
     *
     */
    tgt::Texture* readSingleSliceIntoTex(const std::string& sliceFileName);

    /**
     * Extracts a single slice from a Dicom-Loop and returns it in the form of a 2D-Texture.
     *
     */
    tgt::Texture* readDicomLoopSlice(DcmFileFormat* dfile , unsigned int frame);

    /**
     * Initialises a Dicom Loop-Dataset and finds out the frame number and the used framerate.
     *
     */
    DcmFileFormat* initDicomLoop(const std::string& fileName, unsigned int& frameCount, unsigned int& fps);

    /**
     * Loads a Dicom volume dataset.
     *
     * @fileNames List of Dicom file names
     * @return Volume or 0 if an error occured.
     */
    virtual Volume* read(const std::vector<std::string>& fileNames);

    /**
     * Lists all series found in a file/dicomdir/PACS.
     *
     * @param fileName Specifies the Dicom dataset. This can be:
     * <ul>
     *
     *   <li>Filename of a single Dicom image. This simply returns the
     *   single series UID in the file</li>
     *
     *   <li>Path to a "DICOMDIR" file.</li>
     *
     *   <li>A pathname ending with "/" or "\": All Dicom files inside
     *   the specified directory are searched</li>
     *
     *   <li>A URL starting with "dicom://": The referenced PACS is
     *   asked for all its series by using C-FIND. Such a URL must look
     *   like:<br>
     *   "dicom://<ourtitle>@<peer>:<port>/<peertitle>"</li>
     *
     * </ul>
     *
     * @return List of found series UIDs.
     */
    virtual std::vector<DicomSeriesInfo> listSeries(const std::string &fileName);

    /**
     * Helper function that returns all filenames contained in a given directory.
     */
    static std::vector<std::string> getFileNamesInDir(const std::string& dirName);

private:
    /**
     * Load files referenced in a <tt>DICOMDIR</tt> file.
     *
     * @param fileName Path to a <tt>DICOMDIR</tt> file
     * @param filterSeriesInstanceUID Specifies the series that should be
     * loaded. If the string is empty, the first series found is loaded.
     */
    virtual Volume* readDicomDir(const std::string& fileName,
                                 const std::string& filterSeriesInstanceUID = "");


    /**
     * Load a single Dicom file. This does not really make sense,
     * since a single file can not contain multiple slices, only
     * multiple frames (for an animation).
     *
     * @param fileName Path to a DICOM file
     */
    virtual Volume* readDicomFile(const std::string& fileName);

    /**
     * Loads all Dicom files from a list that have a specific series UID.
     *
     * @fileNames List of Dicom file names
     * @filterSeriesInstanceUID Specifies the series that should be
     * loaded. If the string is empty, the first series found is loaded.
     */
    virtual Volume* readDicomFiles(const std::vector<std::string>& fileNames,
                                   const std::string& filterSeriesInstanceUID = "");

    /**
     * Retrieves information about the series of a Dicom file.
     *
     * @param fileName single Dicom file
     * @param series is filled with the retrieved series information
     * @return series information was retrieved successfully
     */
    virtual bool findSeriesDicomFile(const std::string& fileName,
                                     std::vector<DicomSeriesInfo>& series);

    /**
     * Retrieves information about the series of a \c DICOMDIR file.
     *
     * @param fileName \c DICOMDIR file
     * @param series is filled with the retrieved series information
     * @return series information was retrieved successfully
     */
    virtual bool findSeriesDicomDir(const std::string& fileName,
                                    std::vector<DicomSeriesInfo>& series);

    /**
     * Loads a single Dicom image (=slice) into into \p scalars_ at the
     * position specified by \p posScalar.
     *
     * @deprecated should be encapsulated
     */
    virtual int loadSlice(const std::string& fileName, int posScalar);

    /**
     * Generates a 2D-Texture from a DicomImage.
     *
     */
    tgt::Texture* generate2DTextureFromDcmImage(DicomImage* image);

    uint8_t* scalars_;
    int dx_, dy_, dz_;
    int bits_;
    int bytesPerVoxel_;
    Modality modality_;
    DicomSecurityOptions security_;

    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_DICOMVOLUMEREADER_H
