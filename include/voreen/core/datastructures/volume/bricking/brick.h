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

#ifndef VRN_BRICK_H
#define VRN_BRICK_H

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumetexture.h"

namespace voreen {
    template<class T>
    class RamManager;

    /**
    * A Brick holds a certain amount of data at a certain position in a larger volume.
    * This class (or rather its subclasses) is used to divide large volumes into smaller ones.
    */
    class Brick {
    public:

        /**
        * Simple constructor.
        * @param pos The position of the brickin the original volume.
        * @param dimensions The dimensions this brick has.
        */
        Brick(tgt::ivec3 pos,tgt::ivec3 dimensions);

        virtual ~Brick();

        /**
        * Returns the position of this brick.
        */
        tgt::ivec3 getPosition();

        /**
        * Returns the dimensions of this brick.
        */
        tgt::ivec3 getDimensions();

        /*The following functions are virtual and have no effect in subclasses other than
        VolumeBrick. They are only    here in the superclass so that the templated VolumeBricks
        can be used easily in other non-templated classes. See documentation in the
        VolumeBrick Class for more information.
        */

        virtual char* getVolume();

        virtual void setCurrentLevelOfDetail(size_t i);

        virtual size_t getCurrentLevelOfDetail();

        virtual tgt::vec3 getLlf();

        virtual bool getLevelOfDetailChanged();

        virtual void setLevelOfDetailChanged(bool b);

        virtual size_t getOldLevelOfDetail();

        virtual void setOldLevelOfDetail(size_t lod);

        virtual void setAllVoxelsEqual(bool b);

        virtual bool getAllVoxelsEqual();

        virtual uint64_t getBvFilePosition();

        virtual void setBvFilePosition(uint64_t pos);

        virtual float getError(size_t levelOfDetail);

        virtual void addError(float error);

    protected:

        tgt::ivec3 dimensions_;            //The dimensions of this brick.
        tgt::ivec3 position_;            //The position brick in the original volume.

    private:

    }; //class Brick


    /**
    * This class is used for the purpose of packing several bricks of (maybe downsampled) volume
    * data into one volume. That volume is subdivided into several PackingBricks, which are
    * inserted into a list.
    */
    template<class T>
    class PackingBrick : public Brick {
    public:

        /**
        * @param pos The position of this brick in the volume that is to be filled with data
        * @param dimensions The dimensions of this brick
        * @param list The list holding all PackingBricks that are created. PackingBricks might
        * subdivide themselves into several smaller ones, and these smaller ones are then also
        * inserted into the list.
        */
        PackingBrick(tgt::ivec3 pos,tgt::ivec3 dimensions, std::list<Brick*>& list);

        /**
        * Sets the sourceVolume for this brick. If the volume doesn't fit, 0
        * is returned. If the volume fits perfectly, this brick removes
        * itself from the queue. If the volume does fit, but is smaller, this
        * brick subdivides itself into several smaller ones, until the volume fits
        * perfectly. The source volume holds the data that is to be inserted into
        * the volume subdivided by the PackingBricks. In order to save memory, the source
        * volume is only specified by data and dimensions, and not by a VolumeAtomic object.
        */
        PackingBrick<T>* setSourceVolume(T* v, tgt::ivec3 dims);

        /**
        * Returns the source volume.
        */
        T* getSourceVolume();

        /**
        * Sets the target volume. The target volume is the volume subdivided by the
        * PackingBricks.
        */
        void setTargetVolume(VolumeAtomic<T>* v);

        /**
        * Returns the target volume.
        */
        VolumeAtomic<T>* getTargetVolume();

        /**
        * Returns this bricks position in the list.
        */
        std::list<Brick*>::iterator getListPosition();

        /**
        * Writes the volume data from the source volume to the target volume.
        * This should only be done once. Once all PackingBricks have written
        * their source volume's data into the target volume (which is the same for
        * all PackingBricks), the target volume creates its opengl texture. To update
        * that texture, updateTexture(..) is used.
        */
        void write();

        /**
        * Updates the target volume's texture with this bricks current
        * source volume data.
        */
        void updateTexture(const VolumeTexture* texture);

    protected:

        /**
        * Splits this Brick into smaller ones and inserts them into the list at
        * this bricks position. Used when only a small part of this brick is filled
        * with VolumeData. The given volume is then inserted into one of the splitted
        * bricks. That brick is returned.
        * @param newBrickDimensions The new dimensions the newly created PackingBricks will have
        * once this PackingBrick subdivides itself.
        * @param volumeData The volume data to be inserted into this PackingBrick
        * @param volumeDims The dimensions of the volume data to be inserted into this PackingBrick.
        */
        PackingBrick<T>* split(tgt::ivec3 newBrickDimensions, T* volumeData, tgt::ivec3 volumeDims);

        T* sourceVolume_;                        //The volume from which the volumedata should be read.
        VolumeAtomic<T>* targetVolume_;            //The volume into which the volumedata should be written.
        std::list<Brick*>& list_;                //The list this PackingBrick is in.

    private:

    }; //class PackingBrick


    /**
    * VolumeBricks are used to divide a large volume into several smaller ones. A
    * VolumeBrick holds a part the original volumedata at (maybe) several resolution
    * levels.
    */
    template<class T>
    class VolumeBrick : public Brick {
    public:

        /**
        * @param position The position of this brick in the original volume (like <512,320,64>).
        * @param llf The VolumeBricks lower-left-front corner in world coordinates.
        * @param dimensions The dimensions of this brick.
        */
        VolumeBrick(tgt::ivec3 position, tgt::vec3 llf, tgt::ivec3 dimensions);

        /**
        * Deletes this VolumeBrick, including all data at all levels of detail.
        */
        ~VolumeBrick();

        /**
        * Returns the volumedata of this brick with resolution i (original resolution * 2^-i)
        */
        char* getLodVolume(size_t i);

        /**
        * Same as getLodVolume(0). This function only exist so you can
        * get the volumedata easily without templates.
        */
        char* getVolume();

        /**
        * Adds volumedata at a certain level of detail to the levelOfDetailMap_.
        */
        bool addLodVolume(char* volume, size_t lod);

        /**
        * Deletes volumdata from the levelOfDetailMap_, also frees the memory.
        */
        bool deleteLodVolume(size_t lod);

        /**
        * Sets the RamManager.
        */
        void setRamManager(RamManager<T>* ramManager);

        /**
        * Sets the level of detail at which this VolumeBrick should be rendered.
        */
        void setCurrentLevelOfDetail(size_t i);

        /**
        * Returns the level of detail at which this VolumeBrick is to be rendered.
        */
        size_t getCurrentLevelOfDetail();

        /**
        * Returns the VolumeBricks lower-left-front corner. Used during level of detail assignment.
        */
        tgt::vec3 getLlf();

        /**
        * Returns if the desired level of detail, at which this VolumeBrick should be rendered, has
        * changed.
        */
        bool getLevelOfDetailChanged();

        /**
        * Sets whether or not the desired level of detail at which this VolumeBrick
        * should be rendered has changed.
        */
        void setLevelOfDetailChanged(bool b);

        /**
        * If the level of detail at which this VolumeBrick should be rendered has
        * changed, returns the old one.
        */
        size_t getOldLevelOfDetail();

        /**
        * Sets the old level of detail if there is a new one.
        */
        void setOldLevelOfDetail(size_t lod);

        /**
        * Sets whether or not all voxels in this brick have the same value.
        */
        void setAllVoxelsEqual(bool b);

        /**
        * Returns if all voxels in the volumedata of this brick have the
        * same value.
        */
        bool getAllVoxelsEqual();

        /**
        * Sets the position at which the volumedata of this brick can be found
        * in the Bv File (Bricked Volume File).
        */
        void setBvFilePosition(uint64_t pos);

        /**
        * Gets the position at which the volumedata of this brick can be found
        * in the Bv File (Bricked Volume File).
        */
        uint64_t getBvFilePosition();

        /**
        * Returns the PackingBrick of this VolumeBrick.
        */
        PackingBrick<T>* getPackingBrick();

        /**
        * Sets the PackingBrick of this VolumeBrick.
        */
        void setPackingBrick(PackingBrick<T>* brick);

        /**
        * Returns the error the volume data at the given level of detail has
        * compared to the volume data at the highest level of detail.
        */
        float getError(size_t levelOfDetail);

        /**
        * Adds an error to the errors_ vector. This vector holds the errors of the
        * volume data at different levels of detail compared to the volume data
        * at highest resolution. As it is absolutely necessary to know the error of
        * each and every level of detail, this has been realized by a vector and not a
        * map. All errors have to be added in the correct order, starting with the error
        * of LOD 0 (which is obviously 0.0 every time).
        */
        void addError(float error);

    protected:

        /**
        * A map holding the different levels of detail of the volumedata for this brick.
        */
        std::map<size_t, char* > levelOfDetailMap_;

        /**
        * Have all voxels of this bricks' volumedata the same value?
        */
        bool allVoxelsEqual_;

        /**
        * The level of detail the volumdata in this brick should be rendered.
        */
        size_t currentLevelOfDetail_;

        /**
        * If this brick has been rendered at a different resolution previously,
        * this is its former level of detail.
        */
        size_t oldLevelOfDetail_;

        /**
        * If the bricks level of detail has changed this variable is set to true.
        */
        bool levelOfDetailChanged_;

        /**
        * The bricks lower-left-front corner in world coordinates.
        */
        tgt::vec3 llf_;

        /**
        * The PackingBrick this volumebrick is inserted into.
        */
        PackingBrick<T>* packingBrick_;

        /**
        * The position at which the volumedata of this brick can be found
        * in the Bv File (Bricked Volume File).
        */
        uint64_t bvFilePosition_;

        /**
        * The RamManager. Whenever volumedata at a specific level of detail of this brick
        * is accessed and it's not currently in the RAM, the RamManager will load it from disk.
        */
        RamManager<T>* ramManager_;

        /*This vector holds the errors of the volume data at different levels of detail
        * compared to the volume data at highest resolution. As it is absolutely necessary to
        * know the error of each and every level of detail, this has been realized by a vector and not a
        * map. All errors have to be added in the correct order, starting with the error
        * of LOD 0 (which is obviously 0.0 every time).
        */
        std::vector<float> errors_;


    private:

    }; //class VolumeBrick;


    template<class T>
    VolumeBrick<T>::VolumeBrick(tgt::ivec3 pos, tgt::vec3 llf, tgt::ivec3 dims)
        : Brick(pos, dims),
          llf_(llf),
          bvFilePosition_(0),
          ramManager_(0)
    {

    }

    template<class T>
    VolumeBrick<T>::~VolumeBrick() {
        //delete all volumedata
        std::map<size_t,char* >::iterator it = levelOfDetailMap_.begin();
        for ( ; it != levelOfDetailMap_.end(); ++it) {
            delete it->second;
        }
    }

    template<class T>
    void VolumeBrick<T>::setRamManager(RamManager<T>* ramManager) {
        ramManager_ = ramManager;
    }

    template<class T>
    void VolumeBrick<T>::setCurrentLevelOfDetail(size_t lod) {
        oldLevelOfDetail_ = currentLevelOfDetail_;
        currentLevelOfDetail_ = lod;
        if (currentLevelOfDetail_ != oldLevelOfDetail_) {
            levelOfDetailChanged_ = true;
        } else {
            levelOfDetailChanged_ = false;
        }
    }

    template<class T>
    size_t VolumeBrick<T>::getCurrentLevelOfDetail() {
        return currentLevelOfDetail_;
    }

    template<class T>
    tgt::vec3 VolumeBrick<T>::getLlf() {
        return llf_;
    }

    template<class T>
    bool VolumeBrick<T>::getLevelOfDetailChanged(){
        return levelOfDetailChanged_;
    }

    template<class T>
    void VolumeBrick<T>::setLevelOfDetailChanged(bool b){
        levelOfDetailChanged_ = b;
    }

    template<class T>
    size_t VolumeBrick<T>::getOldLevelOfDetail(){
        return oldLevelOfDetail_;
    }

    template<class T>
    void VolumeBrick<T>::setOldLevelOfDetail(size_t lod){
        oldLevelOfDetail_ = lod;
    }

    template<class T>
    void VolumeBrick<T>::setAllVoxelsEqual(bool b) {
        allVoxelsEqual_ = b;
    }

    template<class T>
    bool VolumeBrick<T>::getAllVoxelsEqual() {
        return allVoxelsEqual_;
    }

    template<class T>
    void VolumeBrick<T>::setPackingBrick(PackingBrick<T>* brick){
        packingBrick_ = brick;
    }

    template<class T>
    PackingBrick<T>* VolumeBrick<T>::getPackingBrick(){
        return packingBrick_;
    }

    template<class T>
    char* VolumeBrick<T>::getLodVolume(size_t i) {
        std::map<size_t,char* >::iterator it;
        it = levelOfDetailMap_.find(i);
        if (it != levelOfDetailMap_.end() ) {
            return it->second;
        } else {
            //If the volumedata isn't in memory, load it from disk.
            ramManager_->readBrickFromDisk(this,i);
            return levelOfDetailMap_[i];
        }
    }

    template<class T>
    char* VolumeBrick<T>::getVolume() {
        return levelOfDetailMap_[0];
    }

    template<class T>
    bool VolumeBrick<T>::addLodVolume(char *volume, size_t lod) {

        //insert returns a pair, the second element of which indicates success
        bool success = levelOfDetailMap_.insert( std::pair<size_t,char* >(lod,volume) ).second;
        return success;
    }

    template<class T>
    bool VolumeBrick<T>::deleteLodVolume(size_t lod) {

        char* vol = levelOfDetailMap_[lod];
        levelOfDetailMap_.erase(lod);
        delete vol;
        vol = 0;
        return true;
    }

    template<class T>
    uint64_t VolumeBrick<T>::getBvFilePosition() {
        return bvFilePosition_;
    }

    template<class T>
    void VolumeBrick<T>::setBvFilePosition(uint64_t pos) {
        bvFilePosition_ = pos;
    }





    template<class T>
    PackingBrick<T>::PackingBrick(tgt::ivec3 pos,tgt::ivec3 dimensions, std::list<Brick*>& list)
        : Brick(pos,dimensions),
          sourceVolume_(0),
          list_(list)
    {
    }

    template<class T>
    PackingBrick<T>* PackingBrick<T>::setSourceVolume(T* v , tgt::ivec3 volumeDims) {

        if (volumeDims.x > dimensions_.x) {
            return 0;
        } else if (volumeDims == dimensions_) {
            //The volumedata fits perfectly into this PackingBrick.
            sourceVolume_=v;
            list_.remove(this);
            return this;
        } else if ( volumeDims.x < dimensions_.x) {
            //That means a brick of data smaller than this brick is to be inserted.
            //In order not to waste space we divide this brick into several smaller ones.
            tgt::ivec3 newBrickDimensions = dimensions_ / 2;
            return split(newBrickDimensions,v,volumeDims);
        } else {
            //Not possible to go here unless you do something incredibly wrong.
            return 0;
        }
    }

    template<class T>
    T* PackingBrick<T>::getSourceVolume() {
        return sourceVolume_;
    }

    template<class T>
    void PackingBrick<T>::setTargetVolume(VolumeAtomic<T>* v) {
        targetVolume_=v;
    }

    template<class T>
    VolumeAtomic<T>* PackingBrick<T>::getTargetVolume() {
        return targetVolume_;
    }

    template<class T>
    std::list<Brick*>::iterator PackingBrick<T>::getListPosition() {
        std::list<Brick*>::iterator iter;
        for (iter = list_.begin(); iter != list_.end(); iter++) {
            if (*iter == this) {
                break;
            }
        }
        return iter;
    }

    template<class T>
    void VolumeBrick<T>::addError(float error) {
        errors_.push_back(error);
    }

    template<class T>
    float VolumeBrick<T>::getError(size_t levelOfDetail) {
        if (levelOfDetail < errors_.size()) {
            return errors_.at(levelOfDetail);
        } else {
            return -1.0f;
        }
    }

    template<class T>
    PackingBrick<T>* PackingBrick<T>::split(tgt::ivec3 newBrickDimensions,T* v, tgt::ivec3 volumeDims) {

        int blocksize=newBrickDimensions.x;

        tgt::ivec3 numblocks = dimensions_/blocksize;

        PackingBrick<T>* newBrick;
        PackingBrick<T>* returnBrick = 0;
        bool volumeInserted = false;

        int xpos,ypos,zpos;
        for (int i=0; i < numblocks.z; i++) {
            for (int j=0; j < numblocks.y; j++) {
                for (int k=0; k < numblocks.x; k++) {
                    xpos=k*blocksize;
                    ypos=j*blocksize;
                    zpos=i*blocksize;

                    newBrick=new PackingBrick<T>(position_+tgt::ivec3(xpos,ypos,zpos),tgt::ivec3(blocksize),list_);

                    std::list<Brick*>::iterator insertPos = this->getListPosition();

                    if (insertPos != list_.end() ) {
                        list_.insert(insertPos,newBrick);
                    }

                    if (!volumeInserted) {
                        returnBrick = newBrick->setSourceVolume(v,volumeDims);
                        volumeInserted=true;
                    }

                }
            }
        }

        list_.remove(this);
        return returnBrick;

    }

    template<class T>
    void PackingBrick<T>::write() {
        tgt::ivec3 dims = dimensions_;

        T* data = sourceVolume_;

        for (int i=0;i <dims.x; i++) {
            for (int j=0;j <dims.y; j++) {
                for (int k=0;k <dims.z; k++) {
                    int pos = k*dims.x*dims.y + j*dims.x+i;
                    targetVolume_->voxel(tgt::ivec3(position_.x +i,position_.y+j,position_.z +k)) = data[pos];
                }
            }
        }
    }

    template<class T>
    void PackingBrick<T>::updateTexture(const VolumeTexture* texture) {
        GLubyte* data = (GLubyte*) sourceVolume_;

        glTexSubImage3D(GL_TEXTURE_3D,0,position_.x, position_.y, position_.z,
                        dimensions_.x, dimensions_.y , dimensions_.z,texture->getFormat(),texture->getDataType(),data);
    }

} //namespace voreen

#endif
