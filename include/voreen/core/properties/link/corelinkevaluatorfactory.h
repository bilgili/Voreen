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

#ifndef VRN_CORELINKEVALUATORFACTORY_H
#define VRN_CORELINKEVALUATORFACTORY_H

#include "voreen/core/properties/link/linkevaluatorfactory.h"

namespace voreen {

/**
 * Serializable factory for all LinkEvaluator classes that are part of Voreen core.
 * Is populated by the CoreModule.
 */
class VRN_CORE_API CoreLinkEvaluatorFactory : public LinkEvaluatorFactory {
public:
    CoreLinkEvaluatorFactory();

    virtual LinkEvaluatorBase* createEvaluator(const std::string& typeString);
};

} // namespace

#endif // VRN_CORELINKEVALUATORFACTORY_H
