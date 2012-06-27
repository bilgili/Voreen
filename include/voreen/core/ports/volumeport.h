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

#ifndef VRN_VOLUMEPORT_H
#define VRN_VOLUMEPORT_H

#include "voreen/core/ports/genericport.h"
#include "voreen/core/datastructures/volume/volumehandle.h"

namespace voreen {

class VolumePort : public GenericPort<VolumeHandle>, public VolumeHandleObserver {
    public:
    explicit VolumePort(PortDirection direction, const std::string& name,
                        bool allowMultipleConnections = false,
                        Processor::InvalidationLevel invalidationLevel = Processor::INVALID_PROGRAM)
        : GenericPort<VolumeHandle>(direction, name, allowMultipleConnections, invalidationLevel),
          VolumeHandleObserver()
    {}

    /**
     * Assigns the passed volume handle to the port.
     */
    void setData(VolumeHandle* handle);

    /**
     * Assigns the passed volume handle to the port and
     * deletes the currently assigned one, if deletePrevious
     * is specified.
     */
    void setData(VolumeHandle* handle, bool deletePrevious);

    /**
     * Deletes the assigned volume handle and
     * assigns the null pointer to itself.
     */
    void deleteVolume();

    /**
     * Returns true, if the port contains a VolumeHandle object
     * and the VolumeHandle has a valid volume.
     */
     virtual bool isReady() const;

    /**
     * Implementation of VolumeHandleObserver interface.
     */
    virtual void volumeHandleDelete(const VolumeHandle* source);

    /**
     * Implementation of VolumeHandleObserver interface.
     */
    virtual void volumeChange(const VolumeHandle* source);

};

} // namespace

#endif // VRN_VOLUMEPORT_H
