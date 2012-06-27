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

#ifndef VRN_VOLUMEHANDLE_H
#define VRN_VOLUMEHANDLE_H

#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/modality.h"
#include "voreen/core/datastructures/volume/bricking/largevolumemanager.h"
#include "voreen/core/utils/observer.h"

#ifndef VRN_NO_OPENGL
#include "voreen/core/datastructures/volume/volumegl.h"
#endif


#include "voreen/core/io/serialization/serialization.h"

#include <set>
#include <string>

namespace voreen {

class VolumeHandle;

/**
 * Interface for volume handle observers.
 */
class VolumeHandleObserver : public Observer {

public:

    /**
     * This method is called by the observed VolumeHandle's destructor.
     *
     * @param source the calling VolumeHandle
     */
    virtual void volumeHandleDelete(const VolumeHandle* source) = 0;

    /**
     * This method is called by the observed VolumeHandle
     * after its member Volume object has changed.
     *
     * When this function is called, the new Volume object has
     * already been assigned. The former Volume object is still
     * valid at this point, but it is deleted immediately after
     * this function has been called.
     *
     * @param source the calling VolumeHandle
     */
    virtual void volumeChange(const VolumeHandle* source) = 0;

};

/**
 * Stores the source where the Volume is loaded from.
 *
 * TODO: Documentation
 */
struct VolumeOrigin : public Serializable {

    VolumeOrigin();
    VolumeOrigin(const VolumeOrigin& rhs);

    /// Constructs the origin from the passed URL.
    VolumeOrigin(const std::string& URL);

    /// Constructs the origin from the specified protocol string, filepath and optional search string
    VolumeOrigin(const std::string& protocol, const std::string& filepath, const std::string& searchString = "");

    virtual ~VolumeOrigin();

    VolumeOrigin& operator=(const VolumeOrigin& rhs);
    bool operator==(const VolumeOrigin& rhs) const;

    /// Returns the complete URL where volume is loaded from.
    const std::string& getURL() const;

    /// Returns the protocol portion of the URL. May be empty.
    std::string getProtocol() const;

    /// Returns the path portion of the URL, without a trailing search string.
    std::string getPath() const;

    /// Returns the file name component of the URL. May be empty, if URL does not reference a file.
    std::string getFilename() const;

    /// Returns the search string portion of the URL. May be empty.
    std::string getSearchString() const;

    /// Appends the given search parameter to the URL in the form: "key=value"
    void addSearchParameter(const std::string& key, const std::string& value);

    /// Returns the value corresponding to the passed key in the URL's search string.
    std::string getSearchParameter(std::string key) const;

    /**
     * @see Serializable::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Serializable::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

private:

    /// Replaces backslashes.
    void cleanupURL();

    /**
     * Copy VolumeOrigin object (used by operator=() and copy-ctor).
     */
    void clone(const VolumeOrigin& rhs) {
        url_ = rhs.getURL();
    }

    std::string url_;

    static const std::string loggerCat_;
};

/**
 * Class for handling different types and needs for volumes.
 *
 * Besides the hardware volume
 * this class holds information about the volume's origin, modality and timestep.
 * It is designed for being the only class which has to take care of
 * what kind of hardware volumes are used.
 */
class VolumeHandle : public Serializable, public Observable<VolumeHandleObserver> {

public:

    /**
     * All types of available hardware specializations.
     * This enum is used to create a mask by OR-ing the values.
     */
    enum HardwareVolumes {
        HARDWARE_VOLUME_NONE = 0,
        HARDWARE_VOLUME_GL = 1,
        HARDWARE_VOLUME_CUDA = 2,
        HARDWARE_VOLUME_ALL = 0xFFFF
    };

    /**
     * Default constructor.
     */
    VolumeHandle();

    /**
     * Constructor.
     *
     * @note No hardware specific volume data like VolumeGL are created initially. If you want
     *  to use hardware specific volume data / textures, call generateHardwareVolumes() or
     *  directly use getVolumeGL() which implicitly generates a hardware volume.
     *
     * @param   volume  The volume data for this VolumeHandle.
     * @param   time    The timestep for this VolumeHandle.
     */
    VolumeHandle(Volume* const volume, const float time = 0.f);

    /**
     * Delete all Volume pointers and the hardware specific ones, if they have been generated.
     */
    virtual ~VolumeHandle();

    /**
     * Gives up ownership of associated volumes without deleting them.
     * Calls this in order to prevent deletion of the volumes on destruction
     * of the handle.
     */
    void releaseVolumes();

    /**
     * Returns the generic Volume.
     */
    Volume* getVolume() const;

    /**
     * (Re)Sets the volume for this handle and deletes the previous one.
     * Usually there should be no need for using this method as the volume
     * is initialized within the ctor, but some VolumeReaders need to modify
     * the read data.
     */
    void setVolume(Volume* const volume);

    void setModality(Modality modality);
    Modality getModality() const;

    /**
     * Returns the associated timestep of this volume handle.
     */
    float getTimestep() const;

    /**
     * Sets the timestep for this VolumeHandle.
     */
    void setTimestep(float timestep);

    void setOrigin(const VolumeOrigin& origin);
    const VolumeOrigin& getOrigin() const;

    /**
     * Reloads the volume from its origin, usually from the
     * hard disk, and regenerates the dependent hardware volumes.
     *
     * @note The Volume object as well as the dependent hardware volume objects
     *       are replaced during this operation.
     *
     * After a successful reload, volumeChanged() is called on the registered observers.
     * In case the reloading failed, the VolumeHandle's state remains unchanged.
     *
     * @return true, if the volume could be successfully reloaded.
     */
    bool reloadVolume();

    /**
     * Returns the mask indicating what hardware volume are currently used
     * by this handle.
     * The used types can be extracted by AND-ing the mask with the desired bit(s)
     * like usual.
     */
    int getHardwareVolumeMask() const;

    /**
     * Returns whether the specified types of hardware volumes exists.
     */
    bool hasHardwareVolumes(int volumeMask) const;

    /**
     * Generate hardware volumes. If the hardware volume already exists, it is rebuild.
     *
     * @param volumeMask OR-junction of HardwareVolumes enum determining what kind of hardware
     *    volumes to create.
     */
    void generateHardwareVolumes(int volumeMask);

    /**
     * Remove hardware volumes and free memory.
     *
     * @param volumeMask OR-junction of HardwareVolumes enum determining which hardware
     *    volumes to delete.
     */
    void freeHardwareVolumes(int volumeMask = HARDWARE_VOLUME_ALL);

    LargeVolumeManager* getLargeVolumeManager();
    void setLargeVolumeManager(LargeVolumeManager* largeVolumeManager);


    /**
     * @see Serializable::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Serializable::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

#ifndef VRN_NO_OPENGL
    /**
     * Returns an OpenGL hardware volume for this volume, generating a new one via
     * generateHardwareVolumes() if it does not already exist. You can use hasHardwareVolumes() to
     * check if such a hardware volume already exists.
     */
    VolumeGL* getVolumeGL();
#endif


protected:

    /**
     * Notifies the registered VolumeHandleObservers about the pending
     * deletion of the VolumeHandle.
     */
    void notifyDelete();

    /**
     * Notifies the registered VolumeHandleObservers that a reload
     * of the volume was done.
     */
    void notifyReload();

    VolumeOrigin origin_;

    Volume* volume_;
    float time_;
    Modality modality_;
    int hardwareVolumeMask_;

    LargeVolumeManager* largeVolumeManager_; /** In case this handle holds a bricked
                                                 volume, this is the Manager responsible
                                                 for updating bricks etc. 0 if no bricked
                                                 volume is present.*/

#ifndef VRN_NO_OPENGL
    VolumeGL* volumeGL_;
#endif


    static const std::string loggerCat_;

private:
    friend class KeyValueFactory;
};

} // namespace

#endif // VRN_VOLUMEHANDLE_H
