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

#line 1

uniform sampler3D shcoeffsI0_;
uniform sampler3D shcoeffsI1_;
uniform sampler3D shcoeffsI2_;
uniform sampler3D shcoeffsI3_;

#if defined(SH_REFLECT)
uniform sampler3D shcoeffsR0_;
uniform sampler3D shcoeffsR1_;
uniform sampler3D shcoeffsR2_;
uniform sampler3D shcoeffsG0_;
uniform sampler3D shcoeffsG1_;
uniform sampler3D shcoeffsB0_;
uniform sampler3D shcoeffsB1_;
//uniform sampler3D shcoeffsR3_;
#endif

#ifdef SH_BLEEDING
uniform sampler3D shcoeffsR0_;
uniform sampler3D shcoeffsR1_;
uniform sampler3D shcoeffsR2_;
uniform sampler3D shcoeffsR3_;
uniform sampler3D shcoeffsG0_;
uniform sampler3D shcoeffsG1_;
uniform sampler3D shcoeffsG2_;
uniform sampler3D shcoeffsG3_;
uniform sampler3D shcoeffsB0_;
uniform sampler3D shcoeffsB1_;
uniform sampler3D shcoeffsB2_;
uniform sampler3D shcoeffsB3_;
#endif

#if defined(SH_SUBSURFACE) || defined(SH_BLEED_SUB)
uniform sampler3D sssCoeffs0_;
uniform sampler3D sssCoeffs1_;
uniform sampler3D sssCoeffs2_;
uniform sampler3D sssCoeffs3_;
uniform sampler3D sssCoeffs4_;
uniform sampler3D sssCoeffs5_;
uniform sampler3D sssCoeffs6_;
#endif

#ifdef CONST_BLEEDING
uniform sampler3D bcVolume_;
uniform float bleedingIntensity_;
#endif

uniform mat4 lightMatsRTrans_[1];
#ifdef SH_LIGHTPROBE
uniform mat4 lightMatsGTrans_[1];
uniform mat4 lightMatsBTrans_[1];
#endif

//uniform bool interactiveMode_;

mat4 transCoeffsI_;

#ifdef SH_BLEEDING
mat4 transCoeffsR_;
mat4 transCoeffsG_;
mat4 transCoeffsB_;
#endif

vec3 shShading(in vec3 gradient, in vec3 samplePos, in vec3 view, in vec3 color) {

    #ifdef SH_UNSHADOWED
        vec4 normal = vec4(normalize(gradient), 1.0);
        vec3 rad = vec3(dot(normal, lightMatsRTrans_[0]*normal));
        #ifdef SH_LIGHTPROBE
        rad.g = dot(normal, lightMatsGTrans_[0]*normal);
        rad.b = dot(normal, lightMatsBTrans_[0]*normal);
        #endif
        return color*rad;
    #else

    mat4 lightMatR = 0.33*mat4(
        vec4(3.54491, 0.0200977, -0.00693648, 0.0250663),
        vec4(-0.0329919, -0.0122785, 0.0196336, 0.0214612),
        vec4(0.0402781, -0.00008, 0.00994587, -0.0158844),
        vec4(-0.0179197, 0.0186624, 0.0150356, -0.0564452)
    );

    vec4 temp;
    vec3 intensityFacs;

    transCoeffsI_ = mat4(texture3D(shcoeffsI0_, samplePos),
                         texture3D(shcoeffsI1_, samplePos),
                         texture3D(shcoeffsI2_, samplePos),
                         texture3D(shcoeffsI3_, samplePos));
    transCoeffsI_ = 2.0*transCoeffsI_ - 1.0;

    // apply shading
    //mat4 finalIntensityCoeffs = matrixCompMult(transCoeffsI_, transpose(lightMatsRTrans_[0]));
    mat4 finalIntensityCoeffs = matrixCompMult(transCoeffsI_, lightMatsRTrans_[0]);
    temp = finalIntensityCoeffs[0] + finalIntensityCoeffs[1] + finalIntensityCoeffs[2] + finalIntensityCoeffs[3];
    intensityFacs = vec3(temp.x + temp.y + temp.z + temp.w);

    // 1 band version for testing
    //intensityFacs = vec3(finalIntensityCoeffs[0].x);

    // 2 bands version for testing
    //temp = finalIntensityCoeffs[0];
    //intensityFacs = vec3(temp.x + temp.y + temp.z + temp.w);

    // 3 bands version for testing
    //temp = finalIntensityCoeffs[0] + finalIntensityCoeffs[1];
    //intensityFacs = vec3(temp.x + temp.y + temp.z + temp.w + finalIntensityCoeffs[2].x);

    #ifdef SH_LIGHTPROBE
    finalIntensityCoeffs = matrixCompMult(transCoeffsI_, lightMatsGTrans_[0]);
    temp = finalIntensityCoeffs[0] + finalIntensityCoeffs[1] + finalIntensityCoeffs[2] + finalIntensityCoeffs[3];
    intensityFacs.y = temp.x + temp.y + temp.z + temp.w;
    finalIntensityCoeffs = matrixCompMult(transCoeffsI_, lightMatsBTrans_[0]);
    temp = finalIntensityCoeffs[0] + finalIntensityCoeffs[1] + finalIntensityCoeffs[2] + finalIntensityCoeffs[3];
    intensityFacs.z = temp.x + temp.y + temp.z + temp.w;
    #endif

    #ifdef CONST_BLEEDING
    vec4 bleedColor = texture3D(bcVolume_, samplePos);
    color += bleedingIntensity_ * bleedColor.rgb;
    //color = bleedingIntensity_ * bleedColor.rgb;
    #endif

    #ifndef SH_SUBSURFACE
    color *= intensityFacs;
    #endif
    //color = intensityFacs;
    //color = 2.0*texture3D(shcoeffsI0_, samplePos).xyz - 1.0;
    //color = texture3D(shcoeffsI0_, samplePos).xyz;

    #ifdef SH_BLEEDING
        transCoeffsR_ = mat4(texture3D(shcoeffsR0_, samplePos),
                             texture3D(shcoeffsR1_, samplePos),
                             texture3D(shcoeffsR2_, samplePos),
                             texture3D(shcoeffsR3_, samplePos));
        transCoeffsR_ = 2.0*transCoeffsR_ - 1.0;
        transCoeffsG_ = mat4(texture3D(shcoeffsG0_, samplePos),
                             texture3D(shcoeffsG1_, samplePos),
                             texture3D(shcoeffsG2_, samplePos),
                             texture3D(shcoeffsG3_, samplePos));
        transCoeffsG_ = 2.0*transCoeffsG_ - 1.0;
        transCoeffsB_ = mat4(texture3D(shcoeffsB0_, samplePos),
                             texture3D(shcoeffsB1_, samplePos),
                             texture3D(shcoeffsB2_, samplePos),
                             texture3D(shcoeffsB3_, samplePos));
        transCoeffsB_ = 2.0*transCoeffsB_ - 1.0;

        mat4 finalColorCoeffs;
        vec3 colorFacs;

        #ifdef SH_LIGHTPROBE
        finalColorCoeffs = matrixCompMult(transCoeffsR_, lightMatsRTrans_[0]);
        temp = finalColorCoeffs[0] + finalColorCoeffs[1] + finalColorCoeffs[2] + finalColorCoeffs[3];
        colorFacs.x = temp.x + temp.y + temp.z + temp.w;
        finalColorCoeffs = matrixCompMult(transCoeffsG_, lightMatsGTrans_[0]);
        temp = finalColorCoeffs[0] + finalColorCoeffs[1] + finalColorCoeffs[2] + finalColorCoeffs[3];
        colorFacs.y = temp.x + temp.y + temp.z + temp.w;
        finalColorCoeffs = matrixCompMult(transCoeffsB_, lightMatsBTrans_[0]);
        temp = finalColorCoeffs[0] + finalColorCoeffs[1] + finalColorCoeffs[2] + finalColorCoeffs[3];
        colorFacs.z = temp.x + temp.y + temp.z + temp.w;
        #else

        //finalColorCoeffs = matrixCompMult(transCoeffsR_, shId);
        finalColorCoeffs = matrixCompMult(transCoeffsR_, lightMatsRTrans_[0]);
        temp = finalColorCoeffs[0] + finalColorCoeffs[1] + finalColorCoeffs[2] + finalColorCoeffs[3];
        colorFacs.x = temp.x + temp.y + temp.z + temp.w;

        //finalColorCoeffs = matrixCompMult(transCoeffsG_, shId);
        finalColorCoeffs = matrixCompMult(transCoeffsG_, lightMatsRTrans_[0]);
        temp = finalColorCoeffs[0] + finalColorCoeffs[1] + finalColorCoeffs[2] + finalColorCoeffs[3];
        colorFacs.y = temp.x + temp.y + temp.z + temp.w;

        //finalColorCoeffs = matrixCompMult(transCoeffsB_, shId);
        finalColorCoeffs = matrixCompMult(transCoeffsB_, lightMatsRTrans_[0]);
        temp = finalColorCoeffs[0] + finalColorCoeffs[1] + finalColorCoeffs[2] + finalColorCoeffs[3];
        colorFacs.z = temp.x + temp.y + temp.z + temp.w;
        #endif // SH_LIGHTPROBE

        //color += intensityFacs*colorFacs;
        //color *= colorFacs;
        color = colorFacs;

        //color = colorFacs;
        //color = texture3D(shcoeffsB3_, samplePos).xyz;
    #endif // SH_BLEEDING

    #if defined(SH_SUBSURFACE) || defined(SH_BLEED_SUB)

    vec4 coeffs0 = texture3D(sssCoeffs0_, samplePos);
    vec4 coeffs1 = texture3D(sssCoeffs1_, samplePos);
    vec4 coeffs2 = texture3D(sssCoeffs2_, samplePos);
    vec4 coeffs3 = texture3D(sssCoeffs3_, samplePos);
    vec4 coeffs4 = texture3D(sssCoeffs4_, samplePos);
    vec4 coeffs5 = texture3D(sssCoeffs5_, samplePos);
    vec4 coeffs6 = texture3D(sssCoeffs6_, samplePos);

    mat4 subCoeffsR = mat4(coeffs0, coeffs1, vec4(coeffs2.x, vec3(0.5)), vec4(0.5));
    mat4 subCoeffsG = mat4(vec4(coeffs2.yzw, coeffs3.x), vec4(coeffs3.yzw, coeffs4.x), vec4(coeffs4.y, vec3(0.5)), vec4(0.5));
    mat4 subCoeffsB = mat4(vec4(coeffs4.zw, coeffs5.xy), vec4(coeffs5.zw, coeffs6.xy), vec4(coeffs6.z, vec3(0.5)), vec4(0.5));

    subCoeffsR = 2.0*subCoeffsR - 1.0;
    subCoeffsG = 2.0*subCoeffsG - 1.0;
    subCoeffsB = 2.0*subCoeffsB - 1.0;

    #ifndef SH_LIGHTPROBE
    vec3 finalSubColor;
    finalIntensityCoeffs = matrixCompMult(lightMatsRTrans_[0], subCoeffsR);
    //finalIntensityCoeffs = matrixCompMult(lightMatR, subCoeffsR);
    temp = finalIntensityCoeffs[0] + finalIntensityCoeffs[1] + finalIntensityCoeffs[2];
    finalSubColor.r = temp.x + temp.y + temp.z + temp.w;
    //finalSubColor.r = finalIntensityCoeffs[0].x + finalIntensityCoeffs[0].y + finalIntensityCoeffs[0].z + finalIntensityCoeffs[0].w;

    finalIntensityCoeffs = matrixCompMult(lightMatsRTrans_[0], subCoeffsG);
    //finalIntensityCoeffs = matrixCompMult(lightMatR, subCoeffsG);
    temp = finalIntensityCoeffs[0] + finalIntensityCoeffs[1] + finalIntensityCoeffs[2];
    finalSubColor.g = temp.x + temp.y + temp.z + temp.w;
    //finalSubColor.g = finalIntensityCoeffs[0].x + finalIntensityCoeffs[0].y + finalIntensityCoeffs[0].z + finalIntensityCoeffs[0].w;

    finalIntensityCoeffs = matrixCompMult(lightMatsRTrans_[0], subCoeffsB);
    //finalIntensityCoeffs = matrixCompMult(lightMatR, subCoeffsB);
    temp = finalIntensityCoeffs[0] + finalIntensityCoeffs[1] + finalIntensityCoeffs[2];
    finalSubColor.b = temp.x + temp.y + temp.z + temp.w;
    //finalSubColor.b = finalIntensityCoeffs[0].x + finalIntensityCoeffs[0].y + finalIntensityCoeffs[0].z + finalIntensityCoeffs[0].w;

    //finalSubColor = 2.0*vec3(coeffs0.xyz)-1.0;
    #else
    vec3 finalSubColor;
    finalIntensityCoeffs = matrixCompMult(lightMatsRTrans_[0], subCoeffsR);
    temp = finalIntensityCoeffs[0] + finalIntensityCoeffs[1] + finalIntensityCoeffs[2];
    finalSubColor.r = temp.x + temp.y + temp.z + temp.w;

    finalIntensityCoeffs = matrixCompMult(lightMatsGTrans_[0], subCoeffsG);
    temp = finalIntensityCoeffs[0] + finalIntensityCoeffs[1] + finalIntensityCoeffs[2];
    finalSubColor.g = temp.x + temp.y + temp.z + temp.w;

    finalIntensityCoeffs = matrixCompMult(lightMatsBTrans_[0], subCoeffsB);
    temp = finalIntensityCoeffs[0] + finalIntensityCoeffs[1] + finalIntensityCoeffs[2];
    finalSubColor.b = temp.x + temp.y + temp.z + temp.w;
    #endif

    float eta = 1.0 / 1.3;
    //float eta = 1.5;
    //float eta = 1.3;

    //float expo = 5.0;
    float expo = 2.0;
    float R0 = pow(1.0 - eta, 2.0) / pow(1.0 + eta, 2.0);
    float fres = R0 + (1.0-R0) * pow(1.0-dot(normalize(gradient), view), expo);
    //fres = 1.0 - fres;
    //float fres = R0 + (1.0-R0) * pow(1.0-cos(asin(1.3*sin(acos(dot(normalize(gradient), view))))), expo);
    //float fres = 1.0;

    //float dp = dot(normalize(gradient), view);
    //float fresRe = 0.5*(1.0 - dp);
    //float fresTr = 0.5*(1.0 + dp);
    //color = finalSubColor*fresTr + color*intensityFacs*fresRe;

    //float ext_i = eta;
    //float cos_theta2 = dot(normalize(gradient), view);
    ////float cos_theta1 = cos(asin((1.0/eta)*sin(acos(cos_theta2))));
    //float cos_theta1 = cos(asin(eta*sin(acos(cos_theta2))));
    //float fresnel_rs = (ext_i * cos_theta1 - cos_theta2 ) / (ext_i * cos_theta1 + cos_theta2);
    //float fresnel_rp = (cos_theta1 - ext_i * cos_theta2 ) / (cos_theta1 + ext_i * cos_theta2);
    //float fres =((1.0-fresnel_rs) * (1.0-fresnel_rs) + (1.0-fresnel_rp) * (1.0-fresnel_rp)) / 2.0;
    //fres = clamp(fres, 0.0, 1.0);

    //float eta = 1.0 / 1.3;
    ////const vec3 fres_dr = -1.44/(eta*eta) + 0.710/eta + 0.668 + 0.0636*eta;
    //float fres = -0.4399 + (0.7099 / eta) - (0.3319 / (eta*eta)) + (0.0636 / (eta*eta*eta));
    //fres = 1.0 - fres;

    //color *= 0.5*intensityFacs;
    //color += 0.1*fres*finalSubColor;
    //color += 0.5 * fres * finalSubColor;
    //color = color*0.5 + 0.5*finalSubColor;
    //color = mix(finalSubColor, color, fres)*intensityFacs;

    //color = mix(finalSubColor, intensityFacs*color, fres);

    //color = mix(finalSubColor, color, fres);
    //color = fres*finalSubColor + (1.0-fres)*intensityFacs;
    //color = intensityFacs*(color + 2.0*fres*finalSubColor);
    //color += color*fres*finalSubColor;

    //color *= 5.0;
    //color = clamp(color, 0.0, 1.0);
    //if(color.x < 0.8)
    //    color.x = 0.0;
    //if(color.y < 0.8)
    //    color.y = 0.0;
    //if(color.z < 0.8)
    //    color.z = 0.0;
    //color += finalSubColor;

    //color *= intensityFacs;
    //color += fres*finalSubColor;

    #ifdef SH_BLEED_SUB
    color = finalSubColor;
    #else
    color = 0.5*(finalSubColor + intensityFacs) + color * intensityFacs;
    //color = finalSubColor;
    #endif

    //color = finalSubColor + color * intensityFacs;
    //color += finalSubColor;
    //color += finalSubColor;
    //color = fres*finalSubColor;
    //color = (finalSubColor+intensityFacs)*color;

    //color = 0.5*(color*intensityFacs+finalSubColor);

    //color *= intensityFacs;//*finalSubColor;

    //color = vec3(finalSubColor.b);

    //color = finalSubColor;
    //color = intensityFacs;
    //color = texture3D(sss, samplePos).yzw - 1.0;

    //color *= fres*finalSubColor;
    //color = fres*finalSubColor;
    //color = vec3(fres);
    //color.b = 0.0;
    //color.g = 0.0;
    //color = mix(fres*finalSubColor, color, 0.5);

    //color = vec3(subCoeffs[0].r, subCoeffs[1].r, subCoeffs[2].r);

    //color = 2.0*texture3D(shcoeffsR0_, samplePos).yzw - 1.0;
    //color = 2.0*texture3D(shcoeffsI0_, samplePos).yzw - 1.0;
    //color = finalSubColor * fres;
    //color = finalSubColor;
    //color = vec3(finalSubColor.x)*fres;
    //color += finalSubColor * fres;
    //color += fres * finalSubColor;
    //color *= 0.25;
    //color += 0.75 * finalSubColor * fres;
    //color = intensityFacs;
    //color += (color/intensityFacs)*finalSubColor;

    //color = intensityFacs + fres*finalSubColor;
    #endif

    #ifdef SH_REFLECT
    mat3x4 refCoeffs = mat3x4(texture3D(shcoeffsR0_, samplePos),
                              texture3D(shcoeffsG0_, samplePos),
                              texture3D(shcoeffsB0_, samplePos));
    refCoeffs = 2.0*refCoeffs - 1.0;
    #ifndef SH_LIGHTPROBE
    vec3 finalRefColor = lightMatsRTrans_[0][0]*refCoeffs;
    //vec3 finalRefColor = lightMatsRTrans_[0][0][0]*vec3(refCoeffs[0][0], refCoeffs[1][0], refCoeffs[2][0]);
    #else
    vec3 finalRefColor;
    finalRefColor.x = dot(lightMatsRTrans_[0][0], refCoeffs[0]);
    finalRefColor.y = dot(lightMatsGTrans_[0][0], refCoeffs[1]);
    finalRefColor.z = dot(lightMatsBTrans_[0][0], refCoeffs[2]);
    #endif
    //color = finalRefColor;
    color += finalRefColor;
    //color = 2.0*texture3D(shcoeffsR0_, samplePos).yzw - 1.0;
    #endif

    return color;
    #endif // SH_UNSHADOWED
}

