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

#include "voreen/core/datastructures/roi/coreroifactory.h"

#include "voreen/core/datastructures/roi/roicube.h"
#include "voreen/core/datastructures/roi/roicylinder.h"
#include "voreen/core/datastructures/roi/roisphere.h"
#include "voreen/core/datastructures/roi/roiraster.h"
#include "voreen/core/datastructures/roi/roiunion.h"
#include "voreen/core/datastructures/roi/roisubstract.h"
#include "voreen/core/datastructures/roi/roigraph.h"

namespace voreen {

CoreROIFactory::CoreROIFactory() {
    registerType(new ROICube());
    registerType(new ROICylinder());
    registerType(new ROISphere());
    registerType(new ROIRaster());
    registerType(new ROIUnion());
    registerType(new ROISubstract());
    registerType(new ROIGraph());
}

} // namespace
