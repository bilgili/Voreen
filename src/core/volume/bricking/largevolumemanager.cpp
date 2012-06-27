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

namespace voreen {

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

} //namespace voreen

