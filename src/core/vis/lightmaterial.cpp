/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#include "voreen/core/vis/lightmaterial.h"

namespace voreen {

const Identifier LightMaterial::setLightPosition_("set.lightPosition");
const Identifier LightMaterial::setLightAmbient_("set.lightAmbient");
const Identifier LightMaterial::setLightDiffuse_("set.lightDiffuse");
const Identifier LightMaterial::setLightSpecular_("set.lightSpecular");
const Identifier LightMaterial::setLightAttenuation_("set.lightAttenuation");
const Identifier LightMaterial::setMaterialAmbient_("set.materialAmbient");
const Identifier LightMaterial::setMaterialDiffuse_("set.material.Diffuse");
const Identifier LightMaterial::setMaterialSpecular_("set.materialSpecular");
const Identifier LightMaterial::setMaterialEmission_("set.materialEmission");
const Identifier LightMaterial::setMaterialShininess_("set.materialShininess");
const Identifier LightMaterial::switchUseOpenGLMaterial_("switch.useOpenGLMaterial");
const Identifier LightMaterial::switchPhongAddAmbient_("switch.phongAddAmbient");
const Identifier LightMaterial::switchPhongAddDiffuse_("switch.phongAddDiffuse");
const Identifier LightMaterial::switchPhongAddSpecular_("switch.phongAddSpecular");
const Identifier LightMaterial::switchPhongApplyAttenuation_("switch.phongApplyAttenuation");

} // namespace voreen
