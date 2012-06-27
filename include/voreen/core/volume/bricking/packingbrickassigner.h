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

#ifndef VRN_PACKINGBRICKASSIGNER_H
#define VRN_PACKINGBRICKASSIGNER_H

#include "voreen/core/volume/bricking/rammanager.h"
#include "voreen/core/volume/bricking/brickinginformation.h"

namespace voreen {

    /**
    * Assigns VolumeBricks to PackingBricks. VolumeBricks hold the data
    * of the original volume and PackingBricks hold space in the packed volume,
    * into which the VolumeBricks' data is to be written. So the task of this
    * class is to assign the VolumeBricks their position (or rather the position
    * of the data they are holding) in the packed volume (which is subdivided into
    * PackingBricks).
    */
    template<class T>
    class PackingBrickAssigner {

    public:

        /**
        * @param brickingInformation The BrickingInformation holds all the information neccessary
        * for bricking, including the vector holding the PackingBricks.
        * @param indexVolume The information where the VolumeBricks' data can be found in the packed
        * volume is stored in this index volume.
        */
        PackingBrickAssigner(BrickingInformation& brickingInformation, Volume4xUInt16* indexVolume);

        /**
        * Assigns a VolumeBrick to a PackingBrick. The reason for not doing this for all
        * VolumeBricks at once is that some VolumeBricks (those holding parts of the original
        * volume in which all voxels are the same) should be immediately deleted after they
        * have been assigned. And that should happen before all VolumeBricks are created,
        * because having too many VolumeBricks causes memory problems. So if the VolumeBrick
        * has only voxels of the same value, it is assigned to a PackingBrick and its volume data
        * written to the packed volume immeadiately, after which both VolumeBrick and PackingBrick
        * are deleted. The index volume is also updated in that case.
        *
        * @param emptyVolumeBrick   Have all voxels in the VolumeBrick the same value?
        * @param packedVolume       The packed volume into which the volume data is written.
        */
        void assignVolumeBrickToPackingBrick(VolumeBrick<T>* volumeBrick, bool emptyVolumeBrick = false,
            VolumeAtomic<T>* packedVolume = 0);

        /**
        * Creates a backup of all packing bricks that DON'T hold volume bricks with all voxels
        * having the same value. The backup is written to brickingInformation_.packingBrickBackups.
        */
        void createPackingBrickBackups();

        /**
        * Deletes all packing bricks in brickingInformation_.packingBricks and
        * brickingInformation_.bricksWithTheData.
        */
        void deletePackingBricks();

        /**
        * Copies all packingBricks from brickingInformation_.packingBrickBackups to
        * brickingInformation_.packingBricks.
        */
        void createPackingBricksFromBackup();



    protected:

        /**
        * Updates the index volume, saving where the VolumeBricks data can be found in the packed
        * volume. Used by assignEmptyVolumeBrickToPackingBrick()
        */
        void updateIndexVolume(VolumeBrick<T>* volBrick, PackingBrick<T>* packBrick);

        BrickingInformation& brickingInformation_;
        Volume4xUInt16* indexVolume_;

    private:

    }; //class


    template<class T>
    PackingBrickAssigner<T>::PackingBrickAssigner(BrickingInformation &brickingInformation,
        Volume4xUInt16* indexVolume)
        : brickingInformation_ (brickingInformation),
        indexVolume_(indexVolume)
    {
    }


    template<class T>
    void PackingBrickAssigner<T>::updateIndexVolume(VolumeBrick<T>* volBrick, PackingBrick<T>* packBrick) {

        int scaleFactor = static_cast<int>(pow(2.f, (int)volBrick->getCurrentLevelOfDetail() ));
        tgt::ivec3 indexVolumePosition = volBrick->getPosition() / brickingInformation_.brickSize;
        tgt::ivec4 indexVolumeValue = tgt::ivec4(packBrick->getPosition(),scaleFactor);

        indexVolume_->voxel(indexVolumePosition ) = indexVolumeValue;
    }

    template<class T>
    void PackingBrickAssigner<T>::assignVolumeBrickToPackingBrick(VolumeBrick<T> *volumeBrick,
        bool emptyVolumeBrick, VolumeAtomic<T>* packedVolume) {

        std::list<Brick*>::iterator iter = brickingInformation_.packingBricks.begin();
        PackingBrick<T>* brickWithTheData;

        bool success=false;

        while (!success) {
            //As long as the volume brick wasn't assigned to a packing brick, try the next packing brick.
            PackingBrick<T>* packBrick = static_cast<PackingBrick<T>*>((*iter));
            size_t levelOfDetail = volumeBrick->getCurrentLevelOfDetail();

            //Check which dimensions a brick of the current level of detail has.
            tgt::ivec3 dims = brickingInformation_.lodToDimensionsMap[levelOfDetail];

            //If the PackingBrick is too big for the VolumeBricks data, the PackingBrick
            //is subdivided into several smaller ones, therefore the returned PackingBrick
            //can be different from the one that called setSourceVolume
            brickWithTheData = packBrick->setSourceVolume(
                (T*)volumeBrick->getLodVolume(levelOfDetail), dims);

            if (brickWithTheData != 0) {
                //This means that the data wasn't too big for the brick
                brickingInformation_.packingBricks.remove(brickWithTheData);
                success=true;
            } else {
                //The data was too big for the brick at the iterator position. Check if there are
                //more bricks in the list and try the next one
                if (iter == brickingInformation_.packingBricks.end() ) {
                    //TODO: Throw some kind of exception
                    break;
                } else {
                    iter++;
                }
            }
        }

        if (brickWithTheData != 0) {
            if (!emptyVolumeBrick) {
                volumeBrick->setPackingBrick(brickWithTheData);
                brickingInformation_.packingBricksWithData.push_back( brickWithTheData);
                updateIndexVolume(volumeBrick,brickWithTheData);
            } else {
                updateIndexVolume(volumeBrick, brickWithTheData);

                //Write the data to the packed volume and then delete both sorts
                //of bricks, as they are not needed in the level of detail
                //calculations.
                brickWithTheData->setTargetVolume(packedVolume);
                brickWithTheData->write();

                delete volumeBrick;
                delete brickWithTheData;
            }
        }
    }

    template<class T>
    void PackingBrickAssigner<T>::createPackingBrickBackups() {

        brickingInformation_.packingBrickBackups.clear();
        std::list<Brick*>::iterator listIterator = brickingInformation_.packingBricks.begin();

        PackingBrick<T>* newBrick;

        while (listIterator != brickingInformation_.packingBricks.end() ) {
            tgt::ivec3 pos = (*listIterator)->getPosition();
            tgt::ivec3 size = (*listIterator)->getDimensions();
            newBrick = new PackingBrick<T>(pos,size,brickingInformation_.packingBrickBackups);
            brickingInformation_.packingBrickBackups.push_back(newBrick);
            listIterator++;
        }
    }

    template<class T>
    void PackingBrickAssigner<T>::deletePackingBricks() {
        std::list<Brick*>::iterator listIterator = brickingInformation_.packingBricks.begin();

         while (listIterator != brickingInformation_.packingBricks.end() ) {
            delete (*listIterator);
            listIterator++;
         }

         brickingInformation_.packingBricks.clear();

         for (size_t i=0; i< brickingInformation_.packingBricksWithData.size(); i++) {
            delete brickingInformation_.packingBricksWithData.at(i);
         }

         brickingInformation_.packingBricksWithData.clear();
    }

    template<class T>
    void PackingBrickAssigner<T>::createPackingBricksFromBackup() {
        deletePackingBricks();

        std::list<Brick*>::iterator listIterator = brickingInformation_.packingBrickBackups.begin();

        PackingBrick<T>* newBrick;

        while (listIterator != brickingInformation_.packingBrickBackups.end() ) {
            tgt::ivec3 pos = (*listIterator)->getPosition();
            tgt::ivec3 size = (*listIterator)->getDimensions();
            newBrick = new PackingBrick<T>(pos,size,brickingInformation_.packingBricks);
            brickingInformation_.packingBricks.push_back(newBrick);
            listIterator++;
        }

    }


} //namespace

#endif

