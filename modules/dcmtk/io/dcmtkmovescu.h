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

#ifndef VRN_DCMTKMOVESCU_H
#define VRN_DCMTKMOVESCU_H

#include "voreen/core/voreencoreapi.h"
#include "modules/dcmtk/io/dcmtkseriesinfo.h"

#include <string>
#include <vector>

namespace voreen {

/**
 * Downloading of datasets from a PACS.
 */
class VRN_CORE_API DcmtkMoveSCU {
public:
    static int filesDone_;
    /// Dicom information model
    typedef enum {
        QMPatientRoot = 0,       ///< use patient root information model
        QMStudyRoot = 1,         ///< use study root information model
        QMPatientStudyOnly = 2   ///< use patient/study only information model
    } QueryModel;

    /**
     * Set up connection parameters for the following C-MOVE operations.
     *
     * @param ourtitle Calling AE title (data will be sent to this AE)
     * @param retrievePort Port number for incoming associations
     * @param peer Hostname of DICOM peer
     * @param port TCP/IP port number of DICOM peer
     * @param security security options, e.g. encryption
     * @param configFile File containing association negotiation profile for
     *                   network transfer syntax selection. Profile "Default" is used.
     * @return Parameters were valid
     */
    static bool init(const std::string& ourtitle, int retrievePort,
                     const std::string& peer, int port,
                     const DcmtkSecurityOptions& security = DcmtkSecurityOptions(),
                     const std::string& configFile = "");

    /**
     * Set up connection parameters for the following C-MOVE operations.
     *
     * @param URL of the form "dicom://<ourtitle>:<retrievePort>@<peer>:<port>"
     * @param security security options, e.g. encryption
     * @param configFile File containing association negotiation profile for
     *                   network transfer syntax selection. Profile "Default" is used.
     * @return URL is valid
     */
    static bool init(const std::string& url,
                     const DcmtkSecurityOptions& security = DcmtkSecurityOptions(),
                     const std::string& configFile = "");

    /**
     * Starts a C-MOVE request.
     * Connection parameters must have been set up by init() before
     * calling this function.
     *
     * @param keys Matching keys, format <tt>0008,0052=SERIES</tt>
     * @param queryModel Dicom query model to use, usually \a QMPatientRoot
     * @param peerTitle Called AE title of peer
     * @param files Contains names of received DICOM files after transmission
     * @return Non-zero if an error occured.
     * @note The temporary files are not deleted after loading is complete.
     */
    static int move(std::vector<std::string>& keys, QueryModel queryModel,
                    const std::string& peerTitle, std::vector<std::string>* files);

    /**
     * Starts a C-MOVE request for all images of a certain series.
     * Connection parameters must have been set up by init() before
     * calling this function.
     *
     * @param seriesInstanceUID UID of the series to be fetched
     * @param peerTitle Called AE title of peer
     * @param files Contains names of received DICOM files after transmission
     * @param targetPath path where the temporary files are saved
     * @return Non-zero if an error occured.
     * @note The temporary files are not deleted after loading is complete.
     */
    static int moveSeries(const std::string& seriesInstanceUID, const std::string& peerTitle,
                          std::vector<std::string>* files, std::string targetPath = "");

    /// Cancel a started move.
    static void cancelMove();

    /// Return the number of downloaded files
    static int getNumDownloaded();
private:
    static std::string ourtitle_;
    static int retrievePort_;
    static std::string peer_;
    static int port_;
    static DcmtkSecurityOptions security_;
};

} // namespace voreen

#endif // VRN_DCMTKMOVESCU_H
