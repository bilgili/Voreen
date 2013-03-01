/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#include "modules/mod_sampler2d.frag"

uniform sampler2D colorTexLeft_;
uniform sampler2D colorTexRight_;

uniform sampler2D depthTexLeft_;
uniform sampler2D depthTexRight_;

uniform int colorCode_;
uniform bool useDepthTex_;
uniform TextureParameters texParams_;



void main() {
        vec2 fragCoord = gl_FragCoord.xy * screenDimRCP_;
        vec4 fragColor = textureLookup2Dnormalized(colorTexLeft_, texParams_, fragCoord);
        FragData0.r = fragColor.r;
        FragData0.a = fragColor.a;
        
        fragColor = textureLookup2Dnormalized(colorTexRight_, texParams_, fragCoord);

        if(colorCode_ == 2){ //RED_CYAN
            FragData0.gb = fragColor.gb;
            FragData0.a = max(FragData0.a,fragColor.a);
            //FragData0 = vec4(0.0,1.0,1.0,1.0);
        } else if(colorCode_ == 1){ //RED_BLUE
            FragData0.b = fragColor.b;
            FragData0.g = 0.0;
            FragData0.a = max(FragData0.a,fragColor.a);
            //FragData0 = vec4(0.0,0.0,1.0,1.0);
        } else { //RED_GREEN
            FragData0.g = fragColor.g;
            FragData0.b = 0.0;
            FragData0.a = max(FragData0.a,fragColor.a);
            //FragData0 = vec4(0.0,1.0,0.0,1.0);
        }
        
        if(useDepthTex_){
            float FragDepthLeft = textureLookup2Dnormalized(depthTexLeft_, texParams_, fragCoord).z;
            float FragDepthRight = textureLookup2Dnormalized(depthTexRight_, texParams_, fragCoord).z;
            gl_FragDepth = max(FragDepthLeft,FragDepthRight);
        }
}