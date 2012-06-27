/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "voreen/core/volume/volume.h"

#ifndef VRN_NO_OPENGL
#include "voreen/core/volume/volumegl.h"
#endif

#ifdef VRN_MODULE_CUDA
#include "voreen/modules/cuda/volumecuda.h"
#endif

#include "voreen/core/xml/serializable.h"

#include <set>
#include <string>

namespace voreen {

class VolumeSet;
class VolumeSeries;
class Modality;

/**
 * Class for handling different types and needs for volumes. Besides this
 * class holds the current timestep of the selected volume in its VolumeSeries.
 * The class is designed for being the only class which has to take care of
 * what kind of hardware volumes are used.
 */
class VolumeHandle : public Serializable {
public:
    friend class VolumeSeries; // for setParentSeries()

    /**
     * Stores the source where the Volume was loaded from.
     */
    struct Origin : public Serializable {
        Origin() : filename(""), seriesname(""), timestep(-1.0) {}

        Origin(std::string fn, std::string sn, float ts)
            : filename(fn), seriesname(sn), timestep(ts)
        {}

        virtual ~Origin() {}

        bool operator==(const Origin& rhs) const;
        bool operator<(const Origin& rhs) const;

        virtual std::string getXmlElementName() const { return XmlElementName; }
        virtual TiXmlElement* serializeToXml() const;
        virtual void updateFromXml(TiXmlElement* elem);

        static void setBasePath(const std::string& basePath) { basePath_ = basePath; }
        
        std::string filename;
        std::string seriesname;
        float timestep;
        static const std::string XmlElementName;
        static std::string basePath_;
    };

    /** Holds the name of the xml element used when serializing the object */
    static const std::string XmlElementName;

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
     * Comparator structure for ensuring that the comparison of VolumeHandles used
     * by the std::set compares the dereferenced pointers and not the pointers
     * themselves!
     */
    struct VolumeHandleComparator {
        bool operator()(const VolumeHandle* const handle1, const VolumeHandle* const handle2) const {
            if (handle1 == 0 || handle2 == 0)
                return false;
            else
                return (*handle1 < *handle2);
        }
    };
    typedef std::set<VolumeHandle*, VolumeHandleComparator> HandleSet;

    /**
     * Constructor
     *
     * NOTE: No hardware specific volume data like VolumeGL are created initially. If you want
     * to use hardware specific volume data / textures, call generateHardwareVolumes() or
     * directly use getVolumeGL() which implicitly generates a hardware volume.
     *
     * @param   parentSeries The VolumeSeries containing this VolumeHandle. Should be
     *  non-NULL to prevent unpredictable results.
     * @param   volume  The volume data for this VolumeHandle.
     * @param   time    The timestep fot this VolumeHandle.
     */
    VolumeHandle(Volume* const volume, const float time = 0.f);

    /**
     * Copy constructor
     */
    VolumeHandle(const VolumeHandle& handle);

    /**
     * Delete all Volume pointers and the hardware specific ones, if they have been generated.
     */
    ~VolumeHandle();

    /**
     * Takes the values from the given handle and writes them into this instance
     */
    void CopyValuesFromVolumeHandle(const VolumeHandle* handle);

    /**
     * Comparison of the handles is defined as the comparison
     * of the timesteps held by the handles.
     */
    bool operator<(const VolumeHandle& handle) const;
    bool operator==(const VolumeHandle& handle) const;

    /**
     * Operator for convenience: a handle can be casted into its timestep.
     */
    operator float() const;

    /**
     * Compares the internal ID with the one of the given VolumeHandle.
     * The internal IDs are used to distinguish different objects: each
     * time you call the ctor you will get a different ID. This is done for
     * performance improvements and memory management.
     *
     * @return  "true" if both IDs and there both objects are identical, "false"
     *          otherwise.
     */
    bool isIdentical(const VolumeHandle& handle) const;
    bool isIdentical(VolumeHandle* const handle) const;

    /**
     * Returns the generic (Voreen) Volume.
     */
    Volume* getVolume() const;

    /**
     * (Re)Sets the volume for this handle and deletes the previous one.
     * Usually there should be no need for using this method as the volume
     * is initialized within the ctor, but some VolumeReaders need to modify
     * the read data.
     */
    void setVolume(Volume* const volume);

    /**
     * Returns the associated Timestep for the selected volume from the VolumeSeries
     * containing the object of this class.
     */
    float getTimestep() const;

    /**
     * Sets the Timestep for this VolumeHandle. This is only need on insertion into
     * VolumeSeries if a VolumeHandle is already contained within the series.
     * The timestep can only be set if the parent VolumeSeries (if existing) contains
     * no other VolumeHandle with the same timestep as the given one. If it does, the
     * method will fail and return "false".
     * If setting the timestep succeeds, "true" is returned.
     * If there is no parent VolumeSeries, setting the timestep always succeeds.
     */
    bool setTimestep(const float timestep);

    /**
     * Returns the VolumeSeries which this VolumeHandle belongs to.
     */
    VolumeSeries* getParentSeries() const;

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

    /**
     * Returns the first pointer to a VolumeHandle object of the given
     * modality and the same timestep as this VolumeHandle from all
     * VolumeSeries know to its parent VolumeSet or 0 if no
     * such series, timestep or volume exist.
     * This is useful for finding other series (former "modalities")
     * of the same scan at the same timestep.
     */
    VolumeHandle* getRelatedVolumeHandle(const Modality& modality) const;

    /**
     * Returns the first pointer to a Volume object of the given
     * modality and the same timestep as this VolumeHandle from all
     * VolumeSeries know to its parent VolumeSet or 0 if no
     * such series, timestep or volume exist.
     * This is useful for finding other series (former "modalities")
     * of the same scan at the same timestep.
     */
    Volume* getRelatedVolume(const Modality& modality) const;

    /**
     * Returns the object ID. Whenever the ctor is called, a new ID is created.
     */
    unsigned int getObjectID() const;

    void setOrigin(const std::string& filename, const std::string& seriesname, const float& timestep);
    void setOrigin(const Origin& origin);
    const Origin& getOrigin() const;

    /**
     * Returns the name of the xml element used when serializing the object
     */
    virtual std::string getXmlElementName() const { return XmlElementName; }

    /**
     * Serializes the object to XML.
     */
    virtual TiXmlElement* serializeToXml() const;

    virtual void updateFromXml(TiXmlElement* elem);

    static std::string getFileNameFromXml(TiXmlElement* elem);


#ifndef VRN_NO_OPENGL
    /**
     * Returns the first pointer to a VolumeGL object of the given
     * modality and the same timestep as this VolumeHandle from all
     * VolumeSeries know to its parent VolumeSet or 0 if no
     * such series, timestep or volume exist.
     * This is useful for finding other series (former "modalities")
     * of the same scan at the same timestep.
     */
    VolumeGL* getRelatedVolumeGL(const Modality& modality);

    /**
     * Returns an OpenGL hardware volume for this volume, generating a new one via
     * generateHardwareVolumes() if it does not already exist. You can use hasHardwareVolumes() to
     * check if such a hardware volume already exists.
     */
    VolumeGL* getVolumeGL();
#endif

#ifdef VRN_MODULE_CUDA
    /**
     * Returns a CUDA hardware volume for this volume, generating a new one via
     * generateHardwareVolumes() if it does not already exist. You can use hasHardwareVolumes()
     * to check if such a hardware volume already exists.
     */
    VolumeCUDA* getVolumeCUDA();
#endif

protected:
    /**
     * Sets the given Series as the new parent Series for this VolumeHandle.
     * This method is usually only called by class VolumeSeries on inserting
     * a VolumeHandle from <code>VolumeSeries::addVolumeHandle()</code>
     * so use with caution in order to prevent unpredictable behavior.
     */
    void setParentSeries(VolumeSeries* const series);

    Volume* volume_;
    float time_;
    int hardwareVolumeMask_;
    VolumeSeries* parentSeries_;    ///< VolumeSeries containing this object
    unsigned int objectID_;         ///< unique ID for each INSTANCE of this class

#ifndef VRN_NO_OPENGL
    VolumeGL* volumeGL_;
#endif

#ifdef VRN_MODULE_CUDA
    VolumeCUDA* volumeCUDA_;
#endif

    Origin origin_;
    static const std::string loggerCat_;
    static unsigned int nextObjectID_;
};

} // namespace

#endif // VRN_VOLUMEHANDLE_H
