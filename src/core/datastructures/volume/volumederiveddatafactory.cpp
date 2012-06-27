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

#include "voreen/core/datastructures/volume/volumederiveddatafactory.h"
#include "voreen/core/datastructures/volume/volumehash.h"
#include "voreen/core/datastructures/volume/volumeminmax.h"
#include "voreen/core/datastructures/volume/volumepreview.h"

namespace voreen {

const std::string VolumeDerivedDataFactory::getTypeString(const std::type_info& type) const {
    if (type == typeid(VolumeHash))
        return "VolumeHash";
    else if (type == typeid(VolumeMinMax))
        return "VolumeMinMax";
    else if (type == typeid(VolumePreview))
        return "VolumePreview";
    else
        return "";
}

Serializable* VolumeDerivedDataFactory::createType(const std::string& typeString) {
    if (typeString == "VolumeHash")
        return new VolumeHash();
    else if (typeString == "VolumeMinMax")
        return new VolumeMinMax();
    else if (typeString == "VolumePreview")
        return new VolumePreview();
    else
        return 0;
}

} // namespace voreen
