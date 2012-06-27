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

#include "voreen/core/volume/noisevolume.h"
#include <time.h>

namespace voreen {

	NoiseVolume::NoiseVolume(tgt::vec3 dimensions) {
		volumeGL_ = 0;
		volume_ = new Volume4xUInt8(dimensions, tgt::vec3(1.0), 8);

		srand((unsigned)time(0));
		for (int x=0;x<dimensions.x;x++) {
			//srand(32);
			for (int y=0;y<dimensions.y;y++) {
				for (int z=0;z<dimensions.z;z++) {
					volume_->setVoxelFloat(static_cast<float>(rand()%255), tgt::ivec3(x,y,z), 0);
					volume_->setVoxelFloat(static_cast<float>(rand()%255), tgt::ivec3(x,y,z), 1);
					volume_->setVoxelFloat(static_cast<float>(rand()%255), tgt::ivec3(x,y,z), 2);
					volume_->setVoxelFloat(static_cast<float>(rand()%255), tgt::ivec3(x,y,z), 3);
				}
			}
		}
	}

	NoiseVolume::~NoiseVolume() {
		delete volume_;
		delete volumeGL_;
	}

	VolumeGL* NoiseVolume::getVolumeGL() {
		if (!volumeGL_)
			volumeGL_ = new VolumeGL(volume_);
		return volumeGL_;
	}

} // namespace voreen
