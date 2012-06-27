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

#ifndef VRN_VOLUMECONTAINER_H
#define VRN_VOLUMECONTAINER_H

// Note: Please ensure that no OpenGL dependencies besides VolumeGL are added into this file.

#include <vector>
#include <map>

#include "voreen/core/vis/message.h"
#include "voreen/core/volume/modality.h"

namespace voreen {

// forward declarations
class Volume;
class VolumeGL;

/**
 * Container for multiple Volumes. Each entry has a modality and a time value associated
 * with it. Entries with equal time but different modalities are said to be in the same <i>time
 * step</i>. No two elements may have equal time <i>and</i> modality values.
 */
class VolumeContainer {
public:
    struct Entry {
        Entry(Volume* volume,
              const std::string& name,
              const Modality &modality = Modality::MODALITY_UNKNOWN,
              float time = 0.0,
              bool generateVolumeGL = true
#ifndef VRN_NO_OPENGL
              , VolumeGL* volumeGL = 0
#endif
        );

        /// Destroys \a volume_ and possibly \a volumeGL_
        void destroy();

        /// Entry objects are sorted by their time value
        bool operator < (const Entry& entry) const {
            return time_ < entry.time_;
        }

        Volume*     volume_;
#ifndef VRN_NO_OPENGL
        VolumeGL*   volumeGL_;
#endif
        Modality    modality_;
        float       time_;
        std::string name_;
    };

#ifndef VRN_NO_OPENGL
    /**
     * Determines the 3D-textures VolumeGL uses. On graphics boards capable of
     * non-power-of-two textures normal 3D textures are used, on other boards non-power-of-two
     * texture are resized to power-of-two and filled with zeros.
     */
    enum TextureType3D {
        VRN_TEXTURE_3D,                         /// GL_TEXTURE_3D
        VRN_TEXTURE_3D_POWER_OF_TWO_SCALED      /// volumes are resized to power-of-two and filled with zeros
    };
#endif

    typedef std::multimap<float, Entry> Entries;

    VolumeContainer();
    ~VolumeContainer();

    /**
     * Adds a Volume to the container.
     *
     * @param volume Volume to be inserted
     * @param modality Modality of the volume
     * @param time Time of the volume. If negative, time is automatically increased for
     *      a modality that already exists, else set to the maximum existing value.
     * @param generateVolumeGL If this is true a VolumeGL will be created,
     *      has no effect in the VRN_NO_OPENGL-Version!
     */
    void add(
        Volume* volume,
        const std::string& name,
        const Modality& modality = Modality::MODALITY_UNKNOWN,
        float time = -1.f,
        bool generateVolumeGL = true
#ifndef VRN_NO_OPENGL
        , VolumeGL* volumeGL = 0
#endif // VRN_NO_OPENGL
    );

    /**
     * Merges another VolumeContainer to this one.
     *
     * @param vc The VolumeContainer to be merged
     */
    void merge(VolumeContainer* vc);

    /**
     * Removes all elements with the given modality from the container.
     *
     * @note This does not release the elements' memory.
     */
    void remove(const Modality& mod);

    /**
     * Removes all elements from the container.
     *
     * @note This does not release the elements' memory.
     */
    void clear();

    /** 
     * Removes all Volumes from the container and deletes them.
     */
    void deleteVolumes();

    /**
     * Returns the total number of Volumes in the VolumeContainer.
     */
    size_t size() const { return entries_.size(); }

    /**
     * Change the modality of an entry.
     *
     * @param volume Volume whose modality should be changed
     * @param modality New modality
     */
    void changeModality(Volume* volume, const Modality& modality);
    /**
     * Change the time of an entry.
     */
    void changeTime(Volume* volume, float time);

#ifndef VRN_NO_OPENGL
    /**
     * Returns a volume of a certain modality.
     *
     * @param modality Modality of the volume, if "ANY" the first volume found will be returned.
     * @return The first volume found that has the specified modality, or 0.
     */
    VolumeGL* getVolumeGL(const Modality& modality = Modality::MODALITY_ANY) const;

    /**
     * Returns a volume with a certain time, if any.
     *
     * @param time Exact time of the volume.
     * @param modality  Modality of the volume, if "ANY" the first modality found at the time
     *                  will be returned.
     * @return The first volume found that has the specified time and modality, or 0.
     */
    VolumeGL* getVolumeGL(float time, const Modality& modality = Modality::MODALITY_ANY) const;

    /**
     * Returns a volume at a certain time step. Volumes belong to the same time step if their
     * time property is the same. Steps are counted starting from 0.
     *
     * @param step Requested time step.
     * @param modality  Modality of the volume, if "ANY" the first modality found at the time
     *                  step will be returned.
     * @return The first volume found that has the specified time step and modality, or 0.
     */
    VolumeGL* getVolumeGL(int step, const Modality& modality = Modality::MODALITY_ANY) const;

    /**
     * \see TextureType3D
     */
    TextureType3D getTextureType();

#endif // VRN_NO_OPENGL

    /**
     * Returns a volume of a certain modality.
     *
     * @param modality Modality of the volume, if "ANY" the first volume found will be returned.
     * @return The first volume found that has the specified modality, or 0.
     */
    Volume* getVolume(const Modality& modality = Modality::MODALITY_ANY) const;

    /**
     * Returns a volume with a certain time, if any.
     *
     * @param time Exact time of the volume.
     * @param modality  Modality of the volume, if "ANY" the first modality found at the time
     *                  will be returned.
     * @return The first volume found that has the specified time and modality, or 0.
     */
    Volume* getVolume(float time, const Modality& modality = Modality::MODALITY_ANY) const;

    /**
     * Returns a volume at a certain time step. Volumes belong to the same time step if their
     * time property is the same. Steps are counted starting from 0.
     *
     * @param step Requested time step.
     * @param modality  Modality of the volume, if "ANY" the first modality found at the time
     *                  step will be returned.
     * @return The first volume found that has the specified time step and modality, or 0.
     */
    Volume* getVolume(int step, const Modality& modality = Modality::MODALITY_ANY) const;

    /**
     * Returns the time step that corresponds to a time value.
     * @param time Time value. If no entry with the exact time is found, the return value is
     * undefined.
     */
    int getStep(float time) const;

    /**
     * Return the time that corresponds to a given step.
     *
     * @return Time value, returns -1.0 if the step does not exist.
     */
    float getTime(int step) const;

    /**
     * Returns maximum time value
     */
    float maxTime() const;

    /**
     * Returns number of time steps
     */
    int steps() const;

    /**
     * Sets the given modality for all entries in the VolumeContainer.
     */
    void setModality(const Modality& modality);

    /**
     * Returns all Entries in the volumeContainer
     */
    VolumeContainer::Entries getEntries();

    friend std::ostream& operator<<(std::ostream& os, const VolumeContainer& v);

protected:
    // internal helpers
    Entries::const_iterator getIterator(const Modality& modality = Modality::MODALITY_ANY) const;
    Entries::const_iterator getIterator(float time, const Modality& modality = Modality::MODALITY_ANY) const;

    std::string toString() const;

    Entries entries_;
#ifndef VRN_NO_OPENGL
    std::vector<VolumeGL*> volumeDatasets_; ///< TODO: deprecated
    TextureType3D textureType3D_;
#endif
    std::string loggerStr_;
};

typedef TemplateMessage<VolumeContainer*> VolumeContainerPtrMsg;

inline std::ostream& operator<<(std::ostream& os, const VolumeContainer& v) {
    return os << v.toString();
}

} // namespace voreen

#endif // VRN_VOLUMECONTAINER_H
