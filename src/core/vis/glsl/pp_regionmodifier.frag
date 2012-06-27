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

#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE shadeTex_;
uniform SAMPLER2D_TYPE depthTex_;
uniform SAMPLER2D_TYPE shadeTexMask_;
//uniform SAMPLER2D_TYPE depthTexMask_;

uniform vec4 segmentId_;
uniform vec4 destColor_;
//uniform int mode_;

/***
 * The main method.
 ***/
void main() {
    vec4 fragCoord = gl_FragCoord;
    vec4 sourceColor = textureLookup2D(shadeTex_, vec2(fragCoord.x, fragCoord.y) ).rgba;
    vec4 maskColor = textureLookup2D(shadeTexMask_, vec2(fragCoord.x, fragCoord.y) ).rgba;
    vec4 destColor = sourceColor;
	// this does not reliably work due to limited accuracy of floating-point numbers:
    //   if (maskColor == segmentId_) {
    if ( round(maskColor.a*255.0) == round(segmentId_.a*255.0) ) {
        #ifdef RV_MODE_REPLACE
        //if ( mode_ == 0 /* REPLACE */ ) {
            destColor = destColor_;
        //}
        #endif
        #ifdef RV_MODE_BLEND
        //else if (mode_ == 1 /* BLEND */ ) {
            destColor = destColor_*destColor_.a + destColor*(1.0 - destColor_.a);
        //}
        #endif
    }
    gl_FragColor = destColor;
    gl_FragDepth = textureLookup2D(depthTex_, fragCoord.xy ).z;
 }
