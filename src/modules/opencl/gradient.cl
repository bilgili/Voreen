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

const sampler_t smp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

float4 readFloat(image3d_t img, sampler_t smp, float4 p) {
    if(get_image_channel_data_type(img) == CLK_UNSIGNED_INT16) {
        uint4 tmp = read_imagef(img, smp, p);
        float4 ret;
        ret.x = tmp.x / 65535.0f;
        ret.y = tmp.y / 65535.0f;
        ret.z = tmp.z / 65535.0f;
        ret.w = tmp.w / 65535.0f;
        return ret;
    }
    else {
        return read_imagef(img, smp, p);
    }
}

int getIndex(int x, int y, int z, int4 dim) {
    return x + y*dim.x + z*dim.x*dim.y;
}

bool border(int x, int y, int z, int4 dim) {
    return (x == 0 || x >= dim.x-1 || y == 0 || y >= dim.y-1 || z == 0 || z >= dim.z-1);
}

int4 readVoxel(const uchar* volume, int index) {
    return (int4)(volume[3*index], volume[3*index+1], volume[3*index+2], 0);
}

void writeVoxel(uchar* volume, int index, int x, int y, int z) {
    volume[3*index] = x;
    volume[3*index+1] = y;
    volume[3*index+2] = z;
    volume[3*index+3] = 0;
}


__kernel void gradient(__global const uchar* inputVolume, __global uchar* outputVolume, int copyIntensityChannel)
{
    int tidX = get_global_id(0), tidY = get_global_id(1), tidZ = get_global_id(2);
    int4 dim = (int4)(get_global_size(0), get_global_size(1), get_global_size(2), 1);

    int index = getIndex(tidX, tidY, tidZ, dim);

    int numChannels = (copyIntensityChannel == 1) ? 4 : 3;

    if (border(tidX, tidY, tidZ, dim)) {
        outputVolume[numChannels*index] = 128;
        outputVolume[numChannels*index+1] = 128;
        outputVolume[numChannels*index+2] = 128;
    }
    else {
        int dX = inputVolume[getIndex(tidX+1, tidY, tidZ, dim)] - inputVolume[getIndex(tidX-1, tidY, tidZ, dim)];
        int dY = inputVolume[getIndex(tidX, tidY+1, tidZ, dim)] - inputVolume[getIndex(tidX, tidY-1, tidZ, dim)];
        int dZ = inputVolume[getIndex(tidX, tidY, tidZ+1, dim)] - inputVolume[getIndex(tidX, tidY, tidZ-1, dim)];
        outputVolume[numChannels*index] = (dX / 2) + 128;
        outputVolume[numChannels*index+1] = (dY / 2) + 128;
        outputVolume[numChannels*index+2] = (dZ / 2) + 128;
    }

    if (copyIntensityChannel == 1)
        outputVolume[numChannels*index+3] = inputVolume[getIndex(tidX, tidY, tidZ, dim)];

}

