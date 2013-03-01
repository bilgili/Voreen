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

#ifndef VRN_DCMTKVOLUMEWRITER_H
#define VRN_DCMTKVOLUMEWRITER_H

#include <string>
#include <vector>

#include "voreen/core/io/volumewriter.h"
#include "voreen/core/datastructures/volume/volumeram.h"

class DcmFileFormat;

namespace voreen {

/**
 * Information about a certain volume dataset as saved in a DICOM
 * file, e.g. patient id, date, etc.
 */
class DcmtkVolumeDescription {
public:
    /**
     * Creates an empty description.
     *
     * @param studyInstanceUID UID for the study, will be generated if empty
     * @param seriesInstanceUID UID for the series (=volume),
     *                          generated if empty
     */
    DcmtkVolumeDescription(const std::string& studyInstanceUID = "",
                           const std::string& seriesInstanceUID = "");

    const std::string& getStudyInstanceUID() const { return studyInstanceUID; }
    const std::string& getSeriesInstanceUID() const { return seriesInstanceUID; }

    /// Modality of the dataset
    enum Modality {
        MODALITY_CT,   ///< computed tomography
        MODALITY_PET   ///< positron emission tomography
    };

    void setModality(Modality m) { modality = m; }
    Modality getModality() const { return modality; }

    bool addField(const std::string& tag, const std::string& value);

    const std::vector<std::pair<std::string, std::string> >& getFields() const { return fields; }
protected:
    static const std::string loggerCat_;
private:
    std::string studyInstanceUID;
    std::string seriesInstanceUID;
    Modality modality;

    std::vector<std::pair<std::string, std::string> > fields;
};

/**
 * Writes the volume into a series of Dicom slices, using the DCMTK library.
 */
class DcmtkVolumeWriter : public VolumeWriter {
public:
    DcmtkVolumeWriter();
    virtual VolumeWriter* create(ProgressBar* progress = 0) const;

    virtual std::string getClassName() const   { return "DcmtkVolumeWriter"; }
    virtual std::string getFormatDescription() const { return "DICOM format, using the DCMTK library"; }

    /**
     * Saves a Volume to the given file.
     *
     * @param fileName The file name where the data should go.
     * @param Volume The volume which should be saved.
     */
    virtual void write(const std::string& fileName, const VolumeBase* volumeHandle)
        throw (tgt::IOException);

    /**
     * Writes a volume into a series of dicom files.
     *
     * @param desc Metadata for the new Dicom file
     * @param volume Raw volume dataset
     * @param fileNamePrefix prefix for the Dicom file names
     * @param fileNameSuffic suffix for the Dicom file names
     */
    static bool writeDicomFiles(const DcmtkVolumeDescription& desc, const VolumeRAM* volume,
                                const std::string& fileNamePrefix,
                                const std::string& fileNameSuffix = ".dcm");


private:
    /**
     * Writes a single volume slice into a dicom file.
     *
     * @param desc Metadata for the new Dicom file
     * @param volume Raw volume dataset
     * @param layer Layer in the volume to write
     * @param fileformat Internal file format object
     * @param fileName name for the new file
     */
    static bool writeDicomFile(const DcmtkVolumeDescription& desc, const VolumeRAM* volume,
                               int layer, DcmFileFormat& fileformat,
                               const std::string& fileName);

    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_DCMTKVOLUMEWRITER_H
