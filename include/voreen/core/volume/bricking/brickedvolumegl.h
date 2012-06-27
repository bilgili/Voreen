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
#include "voreen/core/volume/volumegl.h"

namespace voreen {

    /**
    * BrickedVolumeGls are used when rendering large volumes with bricking.
    * A BrickedVolumeGL has the additional capacity to store two other VolumeGLs.
    * This is basically a hack in order to use bricking in standard networks.
    */
    class BrickedVolumeGL : public VolumeGL {

    public:
        /**
        * Creates three VolumeGLs. The eep volume in the brickedVolume is used to create
        * this VolumeGL, the other two volumes are used to create the packedVolumeGL and the
        * indexVolumeGL.
        */
        BrickedVolumeGL(BrickedVolume* brickedVolume);

        ~BrickedVolumeGL();

        VolumeGL* getPackedVolumeGL();

        VolumeGL* getIndexVolumeGL();

        void setIndexVolumeGL(VolumeGL*);

        void setPackedVolumeGL(VolumeGL*);

    protected:

        VolumeGL* packedVolumeGL_;
        VolumeGL* indexVolumeGL_;

    private:

    };



} //namespace
