/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2007 Visualization and Computer Graphics Group, *
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
 * in the file "license.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * information about commercial licencing please contact the authors. *
 *                                                                    *
 **********************************************************************/

#include "voreen/core/io/brickedvolumereader.h"

#include "voreen/core/volume/bricking/largevolumemanager.h"
#include "voreen/core/volume/volumehandle.h"
#include "tgt/gpucapabilities.h"
#ifdef _MSC_VER
#include "tgt/gpucapabilitieswindows.h"
#endif

namespace voreen {

size_t LargeVolumeManager::maxMemory_ = 256;
size_t LargeVolumeManager::maxGpuMemory_ = 0;

	LargeVolumeManager::LargeVolumeManager(VolumeHandle*, BrickedVolumeReader*) {
	}
	
	LargeVolumeManager::~LargeVolumeManager() {
	}

	void LargeVolumeManager::processMessage(Message*, const Identifier&) {
	}

    void LargeVolumeManager::changeBrickResolutionCalculator(std::string) {
    }

    void LargeVolumeManager::changeBrickLodSelector(std::string) {
    }

	void LargeVolumeManager::setUpdateBricks(bool) {
    }

    void LargeVolumeManager::addBoxBrickingRegion(int, tgt::vec3, tgt::vec3) {

    }

size_t LargeVolumeManager::estimateMaxGpuMemory() {
    /**
     * The total size of the memory on the gpu.
     */
    size_t gpuMemorySize = 256;

    /**
     * A reserve for the memory on the graphics card. The brickingmanager
     * will try to fill the graphics card memory as much as possible in order
     * to produce an image with the best possible quality. But the dataset won't
     * be on the graphics card exclusively, we have to account for transfer functions,
     * rendertargets etc. That's why we introduce this reserve that the brickingmanager
     * won't touch.
     */   
    const size_t gpuMemoryReserve = 100;

#ifdef _MSC_VER
    if (GpuCapsWin.getVideoRamSize() > 100)
        gpuMemorySize = std::min<int>(GpuCapsWin.getVideoRamSize(), 256);
#endif

    if (gpuMemoryReserve > gpuMemorySize)
        return 0;
    else    
        return gpuMemorySize - gpuMemoryReserve;
}

} // namespace voreen

