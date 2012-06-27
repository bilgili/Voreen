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

#ifndef VRN_PROPERTIES_DECL_H
#define VRN_PROPERTIES_DECL_H

/**
 * @file
 * This file contains forward declarations for all Property classes only!
 * Rather include this file in your header files for Processor classes than
 * to include a whole bunch of header files for the implementations of the
 * corresponding Property class.
 */

namespace voreen {

class Property;
class BoolProperty;
class ButtonProperty;
class CameraProperty;
class ColorMapProperty;
class FileDialogProperty;
class FloatProperty;
class FloatVec2Property;
class FloatVec3Property;
class FloatVec4Property;
class FontProperty;
class IntProperty;
class IntVec2Property;
class IntVec3Property;
class IntVec4Property;
class FloatMat2Property;
class FloatMat3Property;
class FloatMat4Property;
class PlotEntitiesProperty;
class OptionPropertyBase;
class PlotPredicateProperty;
class PlotDataProperty;
class PlotSelectionProperty;
class PropertyVector;
class ShaderProperty;
class StringProperty;
class TransFuncProperty;
class VolumeCollectionProperty;
class VolumeHandleProperty;
class VolumeStreamProperty;

// deprecated
class ColorProperty;
class LightProperty;

} // namespace

#endif  // VRN_PROPERTIES_DECL_H
