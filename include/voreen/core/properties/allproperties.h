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

// NOTE: In order to reduce compile times, this file should *not*
// be included by processors. Please include only those properties
// that you actually use!

#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/buttonproperty.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/colormapproperty.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/fontproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/plotentitiesproperty.h"
#include "voreen/core/properties/matrixproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/plotpredicateproperty.h"
#include "voreen/core/properties/plotdataproperty.h"
#include "voreen/core/properties/plotselectionproperty.h"
#include "voreen/core/properties/propertyvector.h"
#include "voreen/core/properties/shaderproperty.h"
#include "voreen/core/properties/stringproperty.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/volumecollectionproperty.h"
#include "voreen/core/properties/volumehandleproperty.h"

// deprecated
#include "voreen/core/properties/colorproperty.h"
#include "voreen/core/properties/lightproperty.h"
