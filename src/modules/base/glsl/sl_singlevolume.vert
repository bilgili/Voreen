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

// uniforms needed for clipping code
uniform int frontIdx_;
uniform vec3 vecView_;
uniform float camDistance_;
uniform float dPlaneIncr_;
uniform int[64] nSequence_;
uniform vec3[8] vecVertices_;
uniform int[24] v1_;
uniform int[24] v2_;

uniform float dummy_;

void main() {
    vec3 position;
    vec3 frontPos = vecVertices_[frontIdx_];
    float dPlaneIncr = dPlaneIncr_ * 15.0; // FIXME: constant needed for diagonal vecView_
    frontPos = frontPos - (camDistance_+length(vecView_))*normalize(vecView_);
    vec3 slicePos = frontPos + ((gl_Vertex.y * dPlaneIncr) * normalize(vecView_));
    float dPlane = dot(slicePos,normalize(vecView_));

    for (int e=0; e<4; e++) {
        int vidx1 = nSequence_[int(frontIdx_*8 + v1_[int(gl_Vertex.x*4 + e)])];
        int vidx2 = nSequence_[int(frontIdx_*8 + v2_[int(gl_Vertex.x*4 + e)])];
        vec3 vecV1 = vecVertices_[vidx1];
        vec3 vecV2 = vecVertices_[vidx2];
        vec3 vecDir = vecV2-vecV1;
        float denom = dot(vecDir,vecView_);
        float lambda = -1.0;
        if (denom != 0.0)
            lambda =  (dPlane-dot(vecV1,vecView_))/denom;
        if (lambda >= 0.0 && lambda <= 1.0) {
            position = vecV1 + lambda * vecDir;
            break;
        }
    }
    gl_TexCoord[0] = vec4((position+vec3(1.0))/vec3(2.0), 1.0);
    gl_Position =  gl_ModelViewProjectionMatrix * vec4(position, 1.0);
}
