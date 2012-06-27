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

#ifndef DICOMSERIESINFO_H
#define DICOMSERIESINFO_H

#include <string>
#include <vector>

namespace voreen {

/**
 * Security options for a Dicom network connection.
 */
//FIXME: Should perhaps be placed somewhere else
struct DicomSecurityOptions {
    bool secureConnection_;
    bool authenticate_;
    std::string privateKeyFile_;
    std::string publicKeyFile_;
    std::vector<std::string> certificateFiles_;

    /**
     * Default security mode with encryption and authentication disabled.
     */
    DicomSecurityOptions()
        : secureConnection_(false), authenticate_(false) {}

    /**
     * Security mode with encryption an authentication. Only connections to known servers will
     * be allowed, so additional server certificates must be added with addCertificateFile().
     *
     * @param privateKeyFile Name of file containing the client's private key (*.pem)
     * @param publicKeyFile Name of file containing the client's public key (*.crt)
     */
    DicomSecurityOptions(const std::string& privateKeyFile, const std::string& publicKeyFile)
        : secureConnection_(true), authenticate_(true),
          privateKeyFile_(privateKeyFile), publicKeyFile_(publicKeyFile) {}

    /**
     * Adds a server certificate.
     *
     * @param certificateFile Name of file containing a server certificate (*.crt)
     */
    void addCertificateFile(const std::string& certificateFile) {
        certificateFiles_.push_back(certificateFile);
    }
};

/**
 * Attributes of a Dicom series. The attributes correspond directly to the Dicom data fields.
 * As Dicom has no real support for volumetric datasets, series are used for modelling volumes.
 * Each image specifies a slice of the volume.
 */
struct DicomSeriesInfo {
    std::string uid_;            ///< globally unique series identifier
    std::string patientsName_;   ///< patient name
    std::string patientId_;      ///< patient identifier
    std::string studyDate_;      ///< date the study was taken
    std::string studyTime_;      ///< time the study was tacken
    std::string modality_;       ///< e.g. CT, MR, US
    std::string numImages_;      ///< number of images in the series
    std::string description_;    ///< SeriesDescription
};

} // namespace voreen

#endif  // DICOMSERIESINFO_H
