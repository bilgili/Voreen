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

/*******************/
/* Declare Inports */
/*******************/
//$ @name = "inport", @depthtex = "inputTexDepth_", @params = "inputTexParams_"
uniform sampler2D inputTex_;
uniform sampler2D inputTexDepth_;
//uniform TEXTURE_PARAMETERS inputTexParams_;

//$ @name = "volumeport", @params = "volParams_"
uniform sampler3D volume_;
//uniform VOLUME_PARAMETERS volParams_;


/********************/
/* Declare Outports */
/********************/

//$ @name = "outport0"
out vec4 FragData;

//$ @name = "ouport1"
out vec4 FragData1;


/**********************/
/* Declare Properties */
/**********************/
//$ @name = "test", @label = "Test property"
uniform float test_;

uniform vec2 screenDimRCP_;


/***
 * The main method.
 ***/
void main() {
    vec2 p = gl_FragCoord.xy * screenDimRCP_;
    FragData = vec4(0.0, test_, 0.0, 1.0);
    FragData1 = vec4(1.0, 1.0, 0.0, 1.0)*texture2D(inputTex_, p);
    gl_FragDepth = texture2D(inputTexDepth_, p).z;
}
