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

#ifndef VRN_VOLUMESOURCEPROCESSOR_H
#define VRN_VOLUMESOURCEPROCESSOR_H

#include "voreen/core/processors/processor.h"
#include "voreen/core/ports/allports.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/properties/volumehandleproperty.h"

namespace voreen {

class Volume;

/**
 * Volume data set supplier in the network.
 */
class VolumeSource : public Processor, public VolumeHandleObserver {

public:
    VolumeSource();
    virtual Processor* create() const;

    virtual std::string getClassName() const    { return "VolumeSource";    }
    virtual std::string getCategory() const     { return "Data Source";     }
    virtual CodeState getCodeState() const      { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;

    virtual void invalidate(int inv = INVALID_RESULT);

    /**
     * Loads the volume specified by filename. The loading is
     * delegated to the processor's VolumeHandleProperty.
     *
     * @param filename the volume to load
     */
    void loadVolume(const std::string& filename)
        throw (tgt::FileException, std::bad_alloc);

    /**
     * Clears the loaded volume.
     */
    void clearVolume();

    /**
     * Assigns a volume handle to this processors.
     *
     * @param handle the handle to assign, is written to the processor's outport
     */
    void setVolumeHandle(VolumeHandle* handle);

    /**
     * Returns a reference to the handle of the loaded volume. May be null.
     */
    const VolumeHandle* getVolumeHandle() const;

    /**
     * Assigns the null pointer to its VolumeHandleProperty and its outport.
     *
     * @see VolumeHandleObserver
     */
    virtual void volumeHandleDelete(const VolumeHandle* source);

    /**
     * Noop.
     *
     * @see VolumeHandleObserver
     */
    virtual void volumeChange(const VolumeHandle* source);

protected:
    virtual void process();
    virtual void initialize() throw (VoreenException);

    VolumeHandleProperty volumeHandle_;

    /// The volume port the loaded data set is written to.
    VolumePort outport_;

    static const std::string loggerCat_;
};

} // namespace

#endif
