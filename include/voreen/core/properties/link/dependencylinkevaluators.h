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

#ifndef VRN_DEPENDENCYLINKEVALUATORS_H
#define VRN_DEPENDENCYLINKEVALUATORS_H

#include "voreen/core/properties/link/dependencylinkevaluator.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/stringproperty.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/volumeurlproperty.h"
#include "voreen/core/datastructures/volume/volume.h"

namespace voreen {

///Special DependencyLinkEvaluator for Volume that deletes entries in the history if the corresponding Volume is deleted.
// TODO revise or remove
/*class VRN_CORE_API DependencyLinkEvaluatorVolumeHandle : public DependencyLinkEvaluator, public VolumeHandleObserver {
public:
    std::string getClassName() const { return "DependencyLinkEvaluatorVolumeHandle"; }
    LinkEvaluatorBase* create() const { return new DependencyLinkEvaluatorVolumeHandle(); }

    void eval(Property* src, Property* dest) throw (VoreenException);
    void propertiesChanged(Property* src, Property* dst);

    void volumeChange(const VolumeBase*);
    void volumeHandleDelete(const VolumeBase* source);
}; */

} // namespace

#endif // VRN_DEPENDENCYLINKEVALUATORS_H
