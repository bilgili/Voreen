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

#ifndef VRN_ERRORLODSELECTOR_H
#define VRN_ERRORLODSELECTOR_H

#include "voreen/core/volume/bricking/bricklodselector.h"

namespace voreen {

    /**
    * A structure holding information about how to best improve the LOD of a
    * brick. This is used by the ErrorLodSelector.
    */
    struct ErrorStruct {

        /**
        * The error improvement per byte the improvement requires. This is the
        * measure by which it is decided which brick gets improved next.
        */
        float improvementPerByte;

        /**
        * The memory the improvement requires.
        */
        int memRequiredForImprovement;

        /**
        * The number of voxels the brick will have after the improvement.
        */
        int numVoxels;

        /**
        * The level of detail the brick will be improved to.
        */
        int nextLod;

        /**
        * The brick this struct is about.
        */
        Brick* brick;

        /**
        * Different instances of this struct should be compared by the
        * error reduction per byte.
        */
        bool operator < (ErrorStruct const & other) const {
            return improvementPerByte < other.improvementPerByte;
        }

        bool operator > (ErrorStruct const & other) const {
            return improvementPerByte > other.improvementPerByte;
        }

        /**
        * Instances of the struct should be equal if they handle the same brick.
        */
        bool operator = (ErrorStruct const & other) const {
            return brick > other.brick;
        }
    };

    /**
    * The level of detail selector assigns a level of detail to the VolumeBricks
    * based on the error of their low resolution bricks compared to their high
    * resolution versions.
    */
    class ErrorLodSelector : public BrickLodSelector {

    public:

        ErrorLodSelector(BrickingInformation& brickingInformation);

        /**
        * Assigns a level of detail to every VolumeBrick based on the error of its
        * low resolution version compared to its high resolution version.
        */
        virtual void selectLods();

    protected:

        /**
        * Creates an ErrorStruct for every VolumeBrick and puts them into the errorSet_.
        */
        void initializeErrorSet();

        /**
        * Calculates to which level of detail the brick in the errorStruct should be improved.
        * This depends on how much the error would be reduced and on how much memory would be
        * needed.
        */
        void calculateNextImprovement(ErrorStruct errorStruct);

        /**
        * A set holding all the ErrorStructs, sorted by the ErrorStruct with the largest
        * error reduction per byte. The first element in the set is therefore the brick
        * that is improved next.
        */
        std::set<ErrorStruct, std::greater<ErrorStruct> > errorSet_;

        /**
        * The size of a voxel in byte.
        */
        int voxelSizeInByte_;

        /**
        * The memory availabe for all the bricks' volume data.
        */
        int availableMemoryInByte_;

        /**
        * The memory already consumed by the bricks' volume data.
        */
        int usedMemoryInByte_;

    private:



    }; //end of class



} //namespace




#endif
