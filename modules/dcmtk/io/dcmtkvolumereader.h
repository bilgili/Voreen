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

#ifndef VRN_DCMTKVOLUMEREADER_H
#define VRN_DCMTKVOLUMEREADER_H

#include "modules/dcmtk/io/dcmtkseriesinfo.h"

#include "voreen/core/io/volumereader.h"
#include "voreen/core/datastructures/volume/volumeram.h"
#include "voreen/core/datastructures/volume/modality.h"

#include "tgt/texturemanager.h"

#include <string>
#include <vector>

class DcmFileFormat;
class DicomImage;
class DiDocument;

namespace voreen {

/**
 * Loader for Dicom datasets using the DCMTK library. As Dicom has no direct support for volumetric datasets, volumes
 * are specified as series of slice images, each stored in a single file.
 */
class VRN_CORE_API DcmtkVolumeReader : public VolumeReader {
public:
    DcmtkVolumeReader(ProgressBar* progress = 0);
    virtual ~DcmtkVolumeReader();
    virtual VolumeReader* create(ProgressBar* progress = 0) const;

    virtual std::string getClassName() const   { return "DcmtkVolumeReader"; }
    virtual std::string getFormatDescription() const { return "DICOM format, using DCMTK library"; }

    void setSecurityOptions(const DcmtkSecurityOptions& security);

    /**
     * Loads a Dicom volume dataset.
     *
     * @param url Specifies the Dicom dataset. This can be:
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
    virtual VolumeList* read(const std::string& url)
        throw (tgt::FileException, std::bad_alloc);

    virtual VolumeBase* read(const VolumeURL& origin)
        throw (tgt::FileException, std::bad_alloc);

    /**
     * Loads a Dicom volume dataset stored in a file series.
     *
     * @fileNames List of Dicom file names
     * @return Volume or 0 if an error occured.
     */
    virtual Volume* read(const std::vector<std::string>& fileNames);

    /**
     * Loads a single slice from a Dicom file into a 2D tgt::Texture.
     *
     * @see readSliceSequence
     * @see readDicomImage
     *
     * @param fileName the Dicom file to read from.
     * @param frame the frame to extract.
     *
     * @return the texture created from the frame, or 0 if the file could not
     *  be read or the frame number if invalid.
     */
    tgt::Texture* readSingleSlice(const std::string& fileName, int frame = 0);

    /**
     * Loads a slice sequence from a Dicom file into a vector of 2D tgt::Textures.
     *
     * @see readDicomImage
     *
     * @param fileName the Dicom file to read from.
     * @param startFrame the start frame of the sequence to extract.
     * @param numFrames the number of frames to extracts. Must be >= 0.
     *  The default value 0 corresponds to the last slice contained by the file.
     *
     * @return the texture sequence. May be empty if the passed frame parameters are invalid,
     *  or if the pixel data could not be extracted.
     */
    std::vector<tgt::Texture*> readSliceSequence(const std::string& fileName, int startFrame = 0, int numFrames = 0);

    /**
     * Reads a Dicom file from the passed location and creates a DicomImage from it,
     * which can be used for extracting image data.
     *
     * @param startFrame start frame of the sequence to be loaded.
     * @param numFrames number of frames to be loaded.
     *
     * @return The created DicomImage, may be null. Deleting it is up to the caller.
     */
    DicomImage* readDicomImage(const std::string& fileName, int startFrame = 0, int numFrames = 0);

    /**
     * Loads a single slice from a DicomImage into a 2D tgt::Texture.
     *
     * @see readDicomImage
     *
     * @param dicomImage the Dicom image object to read from. Must not be null.
     * @param frame the frame to extract.
     *
     * @return the texture created from the frame, or 0 if the passed frame number is invalid,
     *  i.e. negative or greater/equal the number of slices contained by the file,
     *  or if the pixel data could not be extracted.
     */
    tgt::Texture* readSingleSlice(DicomImage* dicomImage, int frame = 0);

    /**
     * Loads a slice sequence from a DicomImage into a vector of 2D tgt::Textures.
     *
     * @see readDicomImage
     *
     * @param dicomImage the Dicom image object to read from. Must not be null.
     * @param startFrame the start frame of the sequence to extract.
     * @param numFrames the number of frames to extracts. Must be >= 0.
     *  The default value 0 corresponds to the last slice contained by the file.
     *
     * @return the texture sequence. May be empty if the passed frame parameters are invalid,
     *  or if the pixel data could not be extracted.
     */
    std::vector<tgt::Texture*> readSliceSequence(DicomImage* dicomImage, int startFrame = 0, int numFrames = 0);

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
    virtual std::vector<DcmtkSeriesInfo> listSeries(const std::string &fileName) const;

    /**
     * Implementation of the VolumeReader interface. Passed the URL to listSeries
     * and converts the returned DcmtkSeriesInfos to VolumeURLs.
     */
    virtual std::vector<VolumeURL> listVolumes(const std::string& url) const
        throw (tgt::FileException);

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
    virtual Volume* readDICOMDIR(const std::string& fileName,
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
     * @skipBroken Do not abort when a file could not be read. This is useful when some
     * non-DICOM files are lying around in a directory.
     */
    virtual Volume* readDicomFiles(const std::vector<std::string>& fileNames,
                                   const std::string& filterSeriesInstanceUID = "",
                                   bool skipBroken = false)
        throw (tgt::FileException, std::bad_alloc);

    /**
     * Retrieves information about the series of a Dicom file.
     *
     * @param fileName single Dicom file
     * @param series is filled with the retrieved series information
     * @return series information was retrieved successfully
     */
    virtual bool findSeriesDicomFile(const std::string& fileName,
                                     std::vector<DcmtkSeriesInfo>& series) const;

    /**
     * Retrieves information about the series of a \c DICOMDIR file.
     *
     * @param fileName \c DICOMDIR file
     * @param series is filled with the retrieved series information
     * @return series information was retrieved successfully
     */
    virtual bool findSeriesDicomDir(const std::string& fileName,
                                    std::vector<DcmtkSeriesInfo>& series) const;

    /**
     * Loads a single Dicom image (=slice) into into \p scalars_ at the
     * position specified by \p posScalar.
     *
     * @deprecated should be encapsulated
     */
    virtual int loadSlice(const std::string& fileName, size_t posScalar);

    uint8_t* scalars_;
    tgt::vec3 xOrientation_;
    tgt::vec3 yOrientation_;
    tgt::vec3 normal_;
    tgt::vec3 offset_;
    int dx_, dy_, dz_;
    int bitsStored_;
    int samplesPerPixel_;
    int bytesPerVoxel_;
    Modality modality_;
    DcmtkSecurityOptions security_;

    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_DCMTKVOLUMEREADER_H
