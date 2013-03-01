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

#ifndef VRN_DCMTKSERIESINFO_H
#define VRN_DCMTKSERIESINFO_H

#include <string>
#include <vector>

namespace voreen {

/**
 * Security options for a Dicom network connection.
 */
//FIXME: Should perhaps be placed somewhere else
struct DcmtkSecurityOptions {
    bool secureConnection_;
    bool authenticate_;
    std::string privateKeyFile_;
    std::string publicKeyFile_;
    std::vector<std::string> certificateFiles_;

    /**
     * Default security mode with encryption and authentication disabled.
     */
    DcmtkSecurityOptions()
        : secureConnection_(false), authenticate_(false) {}

    /**
     * Security mode with encryption an authentication. Only connections to known servers will
     * be allowed, so additional server certificates must be added with addCertificateFile().
     *
     * @param privateKeyFile Name of file containing the client's private key (*.pem)
     * @param publicKeyFile Name of file containing the client's public key (*.crt)
     */
    DcmtkSecurityOptions(const std::string& privateKeyFile, const std::string& publicKeyFile)
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
struct DcmtkSeriesInfo {
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

#endif  // VRN_DCMTKSERIESINFO_H
