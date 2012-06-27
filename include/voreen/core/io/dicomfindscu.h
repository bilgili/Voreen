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

#ifdef VRN_WITH_DCMTK

#ifndef VRN_DICOMFINDSCU_H
#define VRN_DICOMFINDSCU_H

#include "voreen/core/io/dicomseriesinfo.h"
#include <string>
#include <vector>

namespace voreen {

/**
 * Retrieve information about datasets stored on a Dicom PACS.
 */
class DicomFindSCU {
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
                    std::vector<DicomSeriesInfo>* series,
                    const DicomSecurityOptions& security = DicomSecurityOptions());

    /**
     * Retrieves information about all series on a PACS.
     *
     * @param url see DicomVolumeReader::listSeries() for URL format.
     * @return 0 on success
     */
    static int find(const std::string& url,
                    std::vector<DicomSeriesInfo>* series,
                    const DicomSecurityOptions& security = DicomSecurityOptions());
};

} // namespace voreen

#endif // VRN_DICOMFINDSCU_H

#endif // VRN_WITH_DCMTK
