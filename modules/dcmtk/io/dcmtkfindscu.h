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

#ifndef VRN_DCMTKFINDSCU_H
#define VRN_DCMTKFINDSCU_H

#include "modules/dcmtk/io/dcmtkseriesinfo.h"
#include <string>
#include <vector>

namespace voreen {

/**
 * Retrieve information about datasets stored on a Dicom PACS.
 */
class DcmtkFindSCU {
public:
    /**
     * Retrieves information about all series on a PACS.
     *
     * @param ourTitle calling AE title
     * @param peer host name of the PACS
     * @param port port of the PACS
     * @param peerTitle called AE title
     * @param is filled with series information
     * @return 0 on success
     */
    static int find(const std::string& ourTitle, const std::string& peer,
                    int port, const std::string& peerTitle,
                    std::vector<DcmtkSeriesInfo>* series,
                    const DcmtkSecurityOptions& security = DcmtkSecurityOptions());

    /**
     * Retrieves information about all series on a PACS.
     *
     * @param url see DicomVolumeReader::listSeries() for URL format.
     * @return 0 on success
     */
    static int find(const std::string& url,
                    std::vector<DcmtkSeriesInfo>* series,
                    const DcmtkSecurityOptions& security = DcmtkSecurityOptions());
};

} // namespace voreen

#endif // VRN_DCMTKFINDSCU_H
