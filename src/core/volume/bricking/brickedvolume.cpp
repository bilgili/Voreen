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

#include "voreen/core/volume/bricking/brickedvolume.h"


namespace voreen {
 
	BrickedVolume::BrickedVolume(Volume* indexVolume, Volume* packedVolume, Volume* eepVolume) 
        : Volume(tgt::ivec3(0), packedVolume->getBitsStored(), tgt::vec3(1.f)) ,
        indexVolume_(indexVolume),
        packedVolume_(packedVolume),
        eepVolume_(eepVolume) 
    {
	}

	BrickedVolume::~BrickedVolume() {
		delete indexVolume_;
		delete packedVolume_;
		delete eepVolume_;
	}

	Volume* BrickedVolume::getIndexVolume() {
		return indexVolume_;
	}

	Volume* BrickedVolume::getPackedVolume() {
		return packedVolume_;
	}

	Volume* BrickedVolume::getEepVolume() {
		return eepVolume_;
	}

	void BrickedVolume::setIndexVolume(Volume *indexVolume) {
		indexVolume_ = indexVolume;
	}	

	void BrickedVolume::setPackedVolume(Volume *packedVolume) {
		packedVolume_ = packedVolume;
	}

	void BrickedVolume::setEepVolume(Volume *eepVolume) {
		eepVolume_ = eepVolume;
	}

	int BrickedVolume::getBitsStored() const {
		return packedVolume_->getBitsStored();
	}

} //namespace

