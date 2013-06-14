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

#ifndef VRN_VOLUMEOCTREEPORT_H
#define VRN_VOLUMEOCTREEPORT_H

#include "voreen/core/ports/genericport.h"

#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/optionproperty.h"

#include "volumeoctreebase.h"

namespace voreen {

#ifdef DLL_TEMPLATE_INST
template class VRN_CORE_API GenericPort<VolumeOctreeBase>;
#endif

class VRN_CORE_API VolumeOctreePort : public GenericPort<VolumeOctreeBase> {
public:
    VolumeOctreePort(PortDirection direction, const std::string& id, const std::string& guiName = "",
               bool allowMultipleConnections = false,
               Processor::InvalidationLevel invalidationLevel = Processor::INVALID_RESULT);

    virtual Port* create(PortDirection direction, const std::string& id, const std::string& guiName = "") const;
    virtual std::string getClassName() const { return "VolumeOctreePort"; }
    virtual std::string getContentDescription() const;
    virtual std::string getContentDescriptionHTML() const;

    virtual tgt::col3 getColorHint() const;

};

} // namespace

#endif // VRN_VOLUMEOCTREEPORT_H
