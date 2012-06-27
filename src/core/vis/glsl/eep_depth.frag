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

float setValue;
uniform float bias_;
uniform SAMPLER2D_TYPE previous_;            // previous texture
uniform SAMPLER2D_TYPE previous1_;

uniform float id_;
uniform bool firstLayer_;
float check;
bool checkBitValue(in float id,in float previous){
    check=previous*256;
    for(float r=7.0;r>=0.0;r--){
        if ((check-exp2(r))>=0){
            if(id==r){
             return true;
            }
            else{
            check=check-exp2(r);
            }

        }
    }
    return false;
}


void main() {

    vec4 previous = textureLookup2D(previous_, gl_FragCoord.xy);
    vec4 previous1 = textureLookup2D(previous1_, gl_FragCoord.xy);

    float previousDepth= previous.z;
    float previousAlpha= previous1.a;
        float newdepth=(gl_FragCoord.z)-(0.00000000005*id_*bias_);
        gl_FragDepth=newdepth;

    if((newdepth)<=previousDepth){
        discard;
    }
    else{
        gl_FragColor = gl_TexCoord[0];
        bool b;
        if(firstLayer_){
                gl_FragColor.a=exp2(id_)/256.0;
        }
        else{
            b=checkBitValue(id_,(previousAlpha));
            if(b){
                gl_FragColor.a=previousAlpha-(exp2(id_)/256.0);

            }
            else{
                gl_FragColor.a=previousAlpha+(exp2(id_)/256.0);
            }
        }

    }
  }
