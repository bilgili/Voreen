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

// Enable extension GL_ARB_draw_buffers only if it is not yet directly supported
// by this GLSL version and the extension was detected by GLEW.
#if __VERSION__ < 120 && defined(VRN_GLEW_ARB_draw_buffers)
#extension GL_ARB_draw_buffers : enable
#endif

// include required shader modules
#include "modules/mod_sampler2d.frag"

#ifdef BRICKED_VOLUME
#include "modules/bricking/mod_bricking.frag"
#else
#include "modules/mod_sampler3d.frag"
#endif

#include "modules/mod_curvature.frag"
#include "modules/mod_raysetup.frag"
#include "modules/mod_transfunc.frag"
#include "modules/mod_gradients.frag"
#include "modules/mod_shading.frag"
#include "modules/mod_compositing.frag"
#include "modules/mod_depth.frag"
