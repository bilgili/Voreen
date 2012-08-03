/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#ifndef VRN_VOLUMEPORT_H
#define VRN_VOLUMEPORT_H

#include "voreen/core/ports/genericport.h"
#include "voreen/core/datastructures/volume/volume.h"

namespace voreen {

#ifdef DLL_TEMPLATE_INST
template class VRN_CORE_API GenericPort<VolumeBase>;
#endif

class VRN_CORE_API VolumePort : public GenericPort<VolumeBase>, public VolumeHandleObserver {
public:
    VolumePort(PortDirection direction, const std::string& name,
               bool allowMultipleConnections = false,
               Processor::InvalidationLevel invalidationLevel = Processor::INVALID_RESULT);

    /**
     * Assigns the passed volume handle to the port.
     */
    void setData(const VolumeBase* handle, bool takeOwnership = true);

    /**
     * Returns true, if the port contains a Volume object
     * and the Volume has a valid volume.
     */
     virtual bool isReady() const;

    /**
     * Implementation of VolumeHandleObserver interface.
     */
    virtual void volumeHandleDelete(const VolumeBase* source);

    /**
     * Implementation of VolumeHandleObserver interface.
     */
    virtual void volumeChange(const VolumeBase* source);

    /// This port type supports caching.
    virtual bool supportsCaching() const;

    /**
     * Returns an MD5 hash of the stored volume,
     * or and empty string, if no volume is assigned.
     */
    virtual std::string getHash() const;

    /**
     * Saves the assigned volume to the given path.
     * If a filename without extension is passed,
     * ".vvd" is appended to it.
     *
     * @throws VoreenException If saving failed or
     *      no volume is assigned.
     */
    virtual void saveData(const std::string& path) const
        throw (VoreenException);

    /**
     * Loads a volume from the given path and assigns it
     * to the port. If a filename without extension is passed,
     * ".vvd" is appended to it.
     *
     * @throws VoreenException if loading failed.
     */
    virtual void loadData(const std::string& path)
        throw (VoreenException);

    virtual tgt::col3 getColorHint() const;
};

} // namespace

#endif // VRN_VOLUMEPORT_H
