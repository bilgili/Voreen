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

#ifndef VRN_BRICKEDVOLUMEWRITER_H
#define VRN_BRICKEDVOLUMEWRITER_H

#include "voreen/core/io/volumewriter.h"
#include "voreen/core/volume/bricking/brickinginformation.h"

namespace voreen {

    /**
    * This writer writes a bricked volume into a single file. That way
    * bricks can be read with a BrickedVolumeReader directly from the file,
    * together with information of the brick contains an empty volume.
    */
    class BrickedVolumeWriter : public VolumeWriter {
    public:
        BrickedVolumeWriter(BrickingInformation& brickingInformation);

        ~BrickedVolumeWriter();

        void setBrickingInformation(BrickingInformation& brickingInformation);

        /**
        * Opens the files to which the bricks and the volume information (dimensions etc) will be written.
        */
        bool openFile(std::string filename);

        /* creates the information file (.bvi = bricked volume information) in which dimensions,
        * format etc are stored.
        */
        void writeBviFile();

        /**
        * Creates all levels of detail of the given volume and writes
        * them to the end of the currently open file, including the information
        * whether or not all voxels are equal in the volume.
        */
        void writeVolume(VolumeHandle* volumeHandle);

        /**
        * Closes the currently open file.
        */
        void closeFile();

        /**
        * This function is inherited from VolumeWriter, but won't be used.
        */
        virtual void write(const std::string& filename, VolumeHandle* volumeHandle)
            throw (tgt::IOException);

    protected:
        /**
        * In here are all the neccessary informations, like VolumeDimensions,
        * the bricks, etc.
        */
        BrickingInformation& brickingInformation_;

        std::fstream* bviout_;
        std::fstream* bvout_;
        std::fstream* bpiout_;

        FILE* bvFile_;

        std::string bviname_;
        std::string bvname_;
        std::string bpiname_;

        uint64_t bvPosition_;
        int sizeOfUint64_;

        uint64_t* positionArray_;
        char* allVoxelsEqualArray_;
        float* errorArray_;
        uint64_t currentBrick_;
        uint64_t errorArrayPosition_;

    private:
        static const std::string loggerCat_;
    };

} // namespace

#endif //
