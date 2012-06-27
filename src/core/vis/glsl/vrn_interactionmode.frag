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

#include "modules/mod_sampler2d.frag"

uniform SAMPLER2D_TYPE shadeTex_;
uniform SAMPLER2D_TYPE depthTex_;
uniform SAMPLER2D_TYPE overlayTex_;

uniform int interactionCoarseness_;
uniform bool useOverlay_;
uniform float overlayOpacity_;

/***
 * The main method.
 ***/
void main() {

    vec2 fragCoord = gl_FragCoord.xy;

    fragCoord.x /= float(interactionCoarseness_);
    fragCoord.y /= float(interactionCoarseness_);

    // correction offsets for coarseness mode: prevents fragments from
    // being projected to a position outside the significant image area
    if (interactionCoarseness_ > 1) {
        fragCoord.x -= 0.5;
        fragCoord.y -= 0.5;
    }

    gl_FragColor = textureLookup2D(shadeTex_, fragCoord.xy);
    gl_FragDepth = textureLookup2D(depthTex_, fragCoord.xy).z;

}
