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

#line 1

// variables for storing compositing results
vec4 result = vec4(0.0);
vec4 result1 = vec4(0.0);
vec4 result2 = vec4(0.0);



// declare entry and exit parameters
uniform SAMPLER2D_TYPE entryPoints_;            // ray entry points
uniform SAMPLER2D_TYPE entryPointsDepth_;       // ray entry points depth
uniform SAMPLER2D_TYPE exitPoints_;                // ray exit points
uniform SAMPLER2D_TYPE exitPointsDepth_;        // ray exit points depth

// Transfer Functions
uniform TF_SAMPLER_TYPE0 transferFunc_;
uniform TF_SAMPLER_TYPE1 transferFunc2_;
uniform TF_SAMPLER_TYPE2  transferFunc3_;


bool checkValue(in float id,in float previous){
    float check=previous;
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
void raycasting1(in vec3 first, in vec3 last){
    // calculate the required ray parameters
    float t     = 0.0;
    float tIncr = 0.0;
    float tEnd  = 1.0;
    float tempA=0.0;
    vec3 rayDirection;
    vec4 color;
    raySetup(first, last, rayDirection, tIncr, tEnd);


    RC_BEGIN_LOOP {
        vec3 samplePos = first + t * rayDirection;
        vec4 voxel = getVoxel(volume0_, volumeParameters0_, samplePos);
        vec4 voxel1= getVoxel(volume1_, volumeParameters1_, samplePos);
        vec4 voxel2= getVoxel(volume2_, volumeParameters2_, samplePos);

        // apply masking
        if (RC_NOT_MASKED(samplePos, voxel.a)) {
            // calculate gradients
            voxel.xyz = RC_CALC_GRADIENTS(voxel.xyz, samplePos, volume0_, volumeParameters0_, t, rayDirection, entryPoints_);
            voxel1.xyz = RC_CALC_GRADIENTS(voxel1.xyz, samplePos, volume1_, volumeParameters1_, t, rayDirection, entryPoints_);
            voxel2.xyz = RC_CALC_GRADIENTS(voxel2.xyz, samplePos, volume2_, volumeParameters2_, t, rayDirection, entryPoints_);

            // apply classification
            color = RC_APPLY_CLASSIFICATION(transferFunc_, voxel);
            vec4 color1 = RC_APPLY_CLASSIFICATION(transferFunc2_, voxel1);
            vec4 color2 = RC_APPLY_CLASSIFICATION(transferFunc3_, voxel2);

            // apply shading
            color.rgb = RC_APPLY_SHADING(voxel.xyz, samplePos, volumeParameters0_, color.rgb, color.rgb,vec3(1.0,1.0,1.0));
            color1.rgb = RC_APPLY_SHADING(voxel1.xyz, samplePos, volumeParameters1_, color1.rgb, color1.rgb, vec3(1.0,1.0,1.0));
            color2.rgb = RC_APPLY_SHADING(voxel2.xyz, samplePos, volumeParameters2_, color2.rgb, color2.rgb, vec3(1.0,1.0,1.0));



            // if opacity greater zero, apply compositing
            if ((color.a > 0.0) || (color1.a> 0.0) || (color2.a> 0.0)) {
                tempA=color.a+color1.a+color2.a;

                color.rgb= (color.a/(tempA))*(color.rgb);
                color1.rgb=(color1.a/(tempA))*(color1.rgb);
                color2.rgb=(color2.a/(tempA))*(color2.rgb);

                color.rgb=(color.rgb+color1.rgb+color2.rgb);

                color.a=tempA+(color.a*color1.a*color2.a)-(color.a*color1.a)-(color2.a*color1.a)-(color.a*color2.a);
                result = RC_APPLY_COMPOSITING_Multi012(result, color, samplePos, voxel.xyz, t,tDepth);

            }


        }
    } RC_END_LOOP(result);

}
void raycasting2(in vec3 first, in vec3 last){
    // calculate the required ray parameters
    float t     = 0.0;
    float tIncr = 0.0;
    float tEnd  = 1.0;
    float tempA=0.0;
    vec3 rayDirection;
    vec4 color;
    raySetup(first, last, rayDirection, tIncr, tEnd);


    RC_BEGIN_LOOP {
        vec3 samplePos = first + t * rayDirection;
        vec4 voxel = getVoxel(volume0_, volumeParameters0_, samplePos);
        vec4 voxel1= getVoxel(volume2_, volumeParameters2_, samplePos);

        // apply masking
        if (RC_NOT_MASKED(samplePos, voxel.a)) {
            // calculate gradients
            voxel.xyz = RC_CALC_GRADIENTS(voxel.xyz, samplePos, volume0_, volumeParameters0_, t, rayDirection, entryPoints_);
            voxel1.xyz = RC_CALC_GRADIENTS(voxel1.xyz, samplePos, volume2_, volumeParameters2_, t, rayDirection, entryPoints_);

            // apply classification
            color = RC_APPLY_CLASSIFICATION(transferFunc_, voxel);
            vec4 color1 = RC_APPLY_CLASSIFICATION(transferFunc3_, voxel1);

            // apply shading
            color.rgb = RC_APPLY_SHADING(voxel.xyz, samplePos, volumeParameters0_, color.rgb, color.rgb,vec3(1.0,1.0,1.0));
            color1.rgb = RC_APPLY_SHADING(voxel1.xyz, samplePos, volumeParameters2_, color1.rgb, color1.rgb, vec3(1.0,1.0,1.0));


            // if opacity greater zero, apply compositing
            if ((color.a > 0.0) || (color1.a> 0.0)) {
                tempA=color.a+color1.a;
                color.rgb= (color.a/(tempA))*(color.rgb);
                color1.rgb=(color1.a/(tempA))*(color1.rgb);
                color.rgb=(color.rgb+color1.rgb);
                color.a=tempA-(color.a*color1.a);
                result = RC_APPLY_COMPOSITING_Multi02(result, color, samplePos, voxel.xyz, t,tDepth);

            }


        }
    } RC_END_LOOP(result);

}

void raycasting3(in vec3 first, in vec3 last){
    // calculate the required ray parameters
    float t     = 0.0;
    float tIncr = 0.0;
    float tEnd  = 1.0;
    float tempA=0.0;
    vec3 rayDirection;
    vec4 color;
    raySetup(first, last, rayDirection, tIncr, tEnd);


    RC_BEGIN_LOOP {
        vec3 samplePos = first + t * rayDirection;
        vec4 voxel = getVoxel(volume2_, volumeParameters2_, samplePos);
        vec4 voxel1= getVoxel(volume1_, volumeParameters1_, samplePos);

        // apply masking
        if (RC_NOT_MASKED(samplePos, voxel.a)) {
            // calculate gradients
            voxel.xyz = RC_CALC_GRADIENTS(voxel.xyz, samplePos, volume2_, volumeParameters2_, t, rayDirection, entryPoints_);
            voxel1.xyz = RC_CALC_GRADIENTS(voxel1.xyz, samplePos, volume1_, volumeParameters1_, t, rayDirection, entryPoints_);

            // apply classification
            color = RC_APPLY_CLASSIFICATION(transferFunc3_, voxel);
            vec4 color1 = RC_APPLY_CLASSIFICATION(transferFunc2_, voxel1);

            // apply shading
            color.rgb = RC_APPLY_SHADING(voxel.xyz, samplePos, volumeParameters2_, color.rgb, color.rgb,vec3(1.0,1.0,1.0));
            color1.rgb = RC_APPLY_SHADING(voxel1.xyz, samplePos, volumeParameters1_, color1.rgb, color1.rgb, vec3(1.0,1.0,1.0));


            // if opacity greater zero, apply compositing
            if ((color.a > 0.0) || (color1.a> 0.0)) {
                tempA=color.a+color1.a;
                color.rgb= (color.a/(tempA))*(color.rgb);
                color1.rgb=(color1.a/(tempA))*(color1.rgb);
                color.rgb=(color.rgb+color1.rgb);
                color.a=tempA-(color.a*color1.a);
                result = RC_APPLY_COMPOSITING_Multi12(result, color, samplePos, voxel.xyz, t,tDepth);
            }


        }
    } RC_END_LOOP(result);

}

void raycasting4(in vec3 first, in vec3 last){
    // calculate the required ray parameters
    float t     = 0.0;
    float tIncr = 0.0;
    float tEnd  = 1.0;
    vec3 rayDirection;
    vec4 color;
    float tempA=0.0;
    raySetup(first, last, rayDirection, tIncr, tEnd);
    vec4 color1;

    RC_BEGIN_LOOP {
        vec3 samplePos = first + t * rayDirection;
        vec4 voxel = getVoxel(volume0_, volumeParameters0_, samplePos);
        vec4 voxel1= getVoxel(volume1_, volumeParameters1_, samplePos);

        // apply masking
        if (RC_NOT_MASKED(samplePos, voxel.a)) {
            // calculate gradients
            voxel.xyz = RC_CALC_GRADIENTS(voxel.xyz, samplePos, volume0_, volumeParameters0_, t, rayDirection, entryPoints_);
            voxel1.xyz = RC_CALC_GRADIENTS(voxel1.xyz, samplePos, volume1_, volumeParameters1_, t, rayDirection, entryPoints_);

            // apply classification
            color = RC_APPLY_CLASSIFICATION(transferFunc_, voxel);
            color1 = RC_APPLY_CLASSIFICATION(transferFunc2_, voxel1);

            // apply shading
            color.rgb = RC_APPLY_SHADING(voxel.xyz, samplePos, volumeParameters0_, color.rgb, color.rgb,vec3(1.0,1.0,1.0));
            color1.rgb = RC_APPLY_SHADING(voxel1.xyz, samplePos, volumeParameters1_, color1.rgb, color1.rgb, vec3(1.0,1.0,1.0));


            // if opacity greater zero, apply compositing
            if ((color.a > 0.0) || (color1.a> 0.0)) {

                tempA=color.a+color1.a;
                color.rgb= (color.a/(tempA))*(color.rgb);
                color1.rgb=(color1.a/(tempA))*(color1.rgb);
                color.rgb=(color.rgb+color1.rgb);
                color.a=tempA-(color.a*color1.a);
                result = RC_APPLY_COMPOSITING_Multi01(result, color, samplePos, voxel.xyz, t,tDepth);

            }

        }
    } RC_END_LOOP(result);

}
void raycasting5(in vec3 first, in vec3 last){

    // calculate the required ray parameters
    float t     = 0.0;
    float tIncr = 0.0;
    float tEnd  = 1.0;
    vec3 rayDirection;
    raySetup(first, last, rayDirection, tIncr, tEnd);

        RC_BEGIN_LOOP {
            vec3 samplePos = first + t * rayDirection;
            vec4 voxel = getVoxel(volume0_, volumeParameters0_, samplePos);

            // apply masking
            if (RC_NOT_MASKED(samplePos, voxel.a)) {
                // calculate gradients
                voxel.xyz = RC_CALC_GRADIENTS(voxel.xyz, samplePos, volume0_, volumeParameters0_, t, rayDirection, entryPoints_);

                // apply classification
                vec4 color = RC_APPLY_CLASSIFICATION(transferFunc_, voxel);

                // apply shading
                color.rgb = RC_APPLY_SHADING(voxel.xyz, samplePos, volumeParameters0_, color.rgb, color.rgb, vec3(1.0,1.0,1.0));


                // if opacity greater zero, apply compositing
                if (color.a > 0.0) {
                    result = RC_APPLY_COMPOSITING(result, color, samplePos, voxel.xyz, t,tDepth);
                }

            }
        } RC_END_LOOP(result);


}

void raycasting6(in vec3 first, in vec3 last){
    // calculate the required ray parameters
    float t     = 0.0;
    float tIncr = 0.0;
    float tEnd  = 1.0;
    vec3 rayDirection;
    raySetup(first, last, rayDirection, tIncr, tEnd);

        RC_BEGIN_LOOP {
            vec3 samplePos = first + t * rayDirection;
            vec4 voxel = getVoxel(volume1_, volumeParameters1_, samplePos);

            // apply masking
            if (RC_NOT_MASKED(samplePos, voxel.a)) {
                // calculate gradients
                voxel.xyz = RC_CALC_GRADIENTS(voxel.xyz, samplePos, volume1_, volumeParameters1_, t, rayDirection, entryPoints_);

                // apply classification
                vec4 color = RC_APPLY_CLASSIFICATION(transferFunc_, voxel);

                // apply shading
                color.rgb = RC_APPLY_SHADING(voxel.xyz, samplePos, volumeParameters1_, color.rgb, color.rgb, vec3(1.0,1.0,1.0));

                // if opacity greater zero, apply compositing
                if (color.a > 0.0) {

                    result = RC_APPLY_COMPOSITING_1(result, color, samplePos, voxel.xyz, t,tDepth);
                }

            }
        } RC_END_LOOP(result);
}
void raycasting7(in vec3 first, in vec3 last){
    // calculate the required ray parameters
    float t     = 0.0;
    float tIncr = 0.0;
    float tEnd  = 1.0;
    vec3 rayDirection;
    raySetup(first, last, rayDirection, tIncr, tEnd);

        RC_BEGIN_LOOP {
            vec3 samplePos = first + t * rayDirection;
            vec4 voxel = getVoxel(volume2_, volumeParameters2_, samplePos);

            // apply masking
            if (RC_NOT_MASKED(samplePos, voxel.a)) {
                // calculate gradients
                voxel.xyz = RC_CALC_GRADIENTS(voxel.xyz, samplePos, volume2_, volumeParameters2_, t, rayDirection, entryPoints_);

                // apply classification
                vec4 color = RC_APPLY_CLASSIFICATION(transferFunc_, voxel);

                // apply shading
                color.rgb = RC_APPLY_SHADING(voxel.xyz, samplePos, volumeParameters2_, color.rgb, color.rgb, vec3(1.0,1.0,1.0));

                // if opacity greater zero, apply compositing
                if (color.a > 0.0) {
                    result = RC_APPLY_COMPOSITING_2(result, color, samplePos, voxel.xyz, t,tDepth);
                }

            }
        } RC_END_LOOP(result);
}

void rayTraversal(in vec3 first, in vec3 last, in float id){
    bool a=checkValue(0, id*256.0);
    bool b=checkValue(1, id*256.0);
    bool c=checkValue(2, id*256.0);

    vec4 tempro;
    if(SAMPLER_3D_NUMBER>0){
        if(a && b && c){
            raycasting1(first,last);
        }
        else if(a && c){
            raycasting2(first,last);

        }
        else if(b && c){
            raycasting3(first,last);

        }
        else if(a && b){
            raycasting4(first,last);
        }
        else if(a){
            raycasting5(first,last);
        }
        else if(b){
            raycasting6(first,last);
        }
        else if(c){
            raycasting7(first,last);

        }
        else{
            discard;

        }
    }

}


/***
 * The main method.
 ***/
void main(){

    vec3 frontPos = textureLookup2D(entryPoints_, gl_FragCoord.xy).rgb;
    vec3 backPos = textureLookup2D(exitPoints_, gl_FragCoord.xy).rgb;

    float ids=    textureLookup2D(entryPoints_, gl_FragCoord.xy).a;

    // initialize light and material parameters
    matParams = gl_FrontMaterial;
    // determine whether the ray has to be casted
    if (frontPos == backPos){ // background needs no raycasting
       discard;
    }
    else { // fragCoords are lying inside the bounding box
        rayTraversal(frontPos, backPos,ids);
    }

    gl_FragColor=result;
}


