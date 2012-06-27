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

#ifndef VRN_VOLUMEVIEW_H
#define VRN_VOLUMEVIEW_H

#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumecontainer.h"

#include <QPixmap>

namespace voreen {

class VolumeViewHelper{
public:
    /**
     * Returns a Preview with the height of the given int
     * @param the height of the preview
     */
    static QPixmap generatePreview(Volume*, int);

    /**
     * Returns a quadratic Preview with the height of the given int and white border
     * @param the height of the preview
     * @param the size of the border
     */
    static QPixmap generateBorderedPreview(Volume*, int /*height*/, int /* border*/);

    /**
     * Returns a formatted std::string wich contains Volume Information
     */
    static std::string volumeInfoString(VolumeHandle*);

    /**
     * Returns the Volumetype
     */
    static std::string getVolumeType(Volume*);

    /**
     * Returns the Volumename (incl. path)
     */
    static std::string getVolumeName(VolumeHandle*);

    /**
     * Returns the Volumename (excl. path)
     */
    static std::string getStrippedVolumeName(VolumeHandle*);

    /**
     * Returns the Volumepath
     */
    static std::string getVolumePath(VolumeHandle*);

    /**
     * Returns the dimension
     */
    static std::string getVolumeDimension(Volume*);

    /**
     * Returns the spacing
     */
    static std::string getVolumeSpacing(Volume*);

    /**
     * Returns a string desribing the amount of memory
     * that is actually occupied by the volume.
     */
    static std::string getVolumeMemorySize(Volume*);

    static long getVolumeMemorySizeByte(Volume*);

};

} // namespace

#endif
