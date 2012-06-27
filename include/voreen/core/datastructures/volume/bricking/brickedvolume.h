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

#ifndef VRN_BRICKEDVOLUME_H
#define VRN_BRICKEDVOLUME_H

#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"

namespace voreen {

    /**
    * BrickedVolumes are used when rendering large volumes with bricking.
    * A BrickedVolume is only a container for three other volumes,
    * the index volume, the packed volume, and the eep volume. This is
    * basically a hack in order to use bricking in standard networks.
    */
    class BrickedVolume : public Volume {

    public:
        BrickedVolume(Volume* indexVolume, Volume* packedVolume, Volume* eepVolume);

        ~BrickedVolume();

        Volume* getIndexVolume() const;

        Volume* getPackedVolume() const;

        Volume* getEepVolume() const;

        void setIndexVolume(Volume* indexVolume);

        void setPackedVolume(Volume* packedVolume);

        void setEepVolume(Volume* eepVolume);

        int getBitsStored() const;

        /// overwritten to redirect to the eepVolume (for use by proxy geometry)
        tgt::ivec3 getDimensions() const;
        /// overwritten to redirect to the eepVolume (for use by proxy geometry)
        tgt::vec3 getCubeSize() const;
        /// overwritten to redirect to the eepVolume (for use by proxy geometry)
        tgt::vec3 getLLF() const;
        /// overwritten to redirect to the eepVolume (for use by proxy geometry)
        tgt::vec3 getURB() const;

        virtual Volume* clone() const throw (std::bad_alloc) {
            return 0;
        }

        virtual Volume* clone(void*) const throw (std::bad_alloc) {
            return 0;
        }

        virtual int getBitsAllocated() const {
            return packedVolume_->getBitsAllocated();
        }

        virtual int getNumChannels() const {
            return packedVolume_->getNumChannels();
        }
        virtual int getBytesPerVoxel() const {
            return packedVolume_->getBytesPerVoxel();
        }

        /// Returns the number of bytes held in the \a data_ array
        virtual size_t getNumBytes() const {
            return packedVolume_->getNumBytes() + indexVolume_->getNumBytes();
        }

        virtual tgt::vec2 elementRange() const {
            return tgt::vec2(0.f, 1.f);
        }

        virtual float getVoxelFloat(const tgt::ivec3& , size_t ) const {
            return 0.0f;
        }

        virtual float getVoxelFloat(size_t , size_t , size_t , size_t ) const {
            return 0.0f;
        }

        virtual float getVoxelFloat(size_t , size_t) const {
            return 0.0f;
        }

        virtual void setVoxelFloat(float , const tgt::ivec3& , size_t ) {

        }

        virtual void setVoxelFloat(float , size_t , size_t , size_t , size_t ) {

        }

        virtual void setVoxelFloat(float, size_t, size_t ) {

        }

        virtual void clear() {

        }

        virtual void* getData() {
            return 0;
        }
        virtual Volume* mirrorZ() const {
            return 0;
        }
        virtual Volume* createSubset(tgt::ivec3, tgt::ivec3) const throw (std::bad_alloc){
            return 0;
        }

        virtual void resize(tgt::ivec3, bool /*allocMem = true*/)
            throw (std::bad_alloc) {
        }
        virtual Volume* resample(tgt::ivec3, Filter ) const
            throw (std::bad_alloc) {
            return 0;
        }

        virtual float calcError(Volume* ) {
            return 0.f;
        }

        virtual bool isUniform() const {
            return false;
        }


    protected:

        Volume* indexVolume_;
        Volume* packedVolume_;
        Volume* eepVolume_;

    private:

    }; //class BrickedVolume

} //namespace voreen

#endif
