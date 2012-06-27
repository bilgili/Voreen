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

#ifndef VRN_LIGTHMATERIAL_H
#define VRN_LIGTHMATERIAL_H

#include "voreen/core/vis/identifier.h"

namespace voreen {

struct LightMaterial {
    static const Identifier setLightPosition_;
    static const Identifier setLightAmbient_;
    static const Identifier setLightDiffuse_;
    static const Identifier setLightSpecular_;
    static const Identifier setLightAttenuation_;
    static const Identifier setMaterialAmbient_;
    static const Identifier setMaterialDiffuse_;
    static const Identifier setMaterialSpecular_;
    static const Identifier setMaterialEmission_;
    static const Identifier setMaterialShininess_;
    static const Identifier switchUseOpenGLMaterial_;
    static const Identifier switchPhongAddAmbient_;
    static const Identifier switchPhongAddDiffuse_;
    static const Identifier switchPhongAddSpecular_;
    static const Identifier switchPhongApplyAttenuation_;
};

} // namespace voreen

#endif // VRN_LIGTHMATERIAL_H
