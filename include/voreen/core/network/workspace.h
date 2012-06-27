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

#ifndef VRN_WORKSPACE_H
#define VRN_WORKSPACE_H

#include "voreen/core/network/networkserializer.h"
#include "voreen/core/network/processornetwork.h"

#include "voreen/core/io/serialization/serialization.h"
#include "voreen/core/animation/animation.h"

#include "voreen/core/properties/link/scriptmanagerlinking.h"

namespace tgt {
    class GLCanvas;
}

namespace voreen {

class VolumeContainer;

class Workspace : public Serializable {
public:

    /**
     * Constructor.
     *
     * @param sharedContext Is used for operations that require
     *      an active OpenGL context, such as deletion of textures.
     *      Providing a shared context is optional, but strongly recommended,
     *      since OpenGL resources might not be released properly without it.
     */
    Workspace(tgt::GLCanvas* sharedContext = 0);

    /**
     * Calls clear().
     */
    ~Workspace();

    std::vector<std::string> getErrors() const;

    bool exportZipped(const std::string& exportName, bool overwrite = true)
        throw (SerializationException);

    /**
     * Updates the workspace from the specified file.
     *
     * Non-fatal errors occuring during workspace load are saved
     * and can be requested using @c getErrors().
     *
     * @throw SerializationException on serialization errors
     */
    void load(const std::string& filename) throw (SerializationException);

    /**
     * Saves the workspace to the specified file.
     *
     * @throw SerializationException on serialization errors
     */
    void save(const std::string& filename, bool overwrite = true) throw (SerializationException);

    /**
     * Deletes the current network and the resource containers,
     * and clears the serialization error collector.
     *
     * The workspace's filename is not cleared.
     */
    void clear();

    // flag for app to not overwrite this file, not used by serialize()
    bool readOnly() const;

    ProcessorNetwork* getProcessorNetwork() const;
    void setProcessorNetwork(ProcessorNetwork* network);

    void setVolumeContainer(VolumeContainer* volumeContainer);
    VolumeContainer* getVolumeContainer() const;

    void setFilename(const std::string& filename);
    std::string getFilename() const;

    /**
     * @see Serializable::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Serializable::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

    Animation* getAnimation() const;
    void setAnimation(Animation* anim);

private:
    int version_;
    ProcessorNetwork* network_;
    VolumeContainer* volumeContainer_;
    std::string filename_;
    bool readOnly_;

    tgt::GLCanvas* sharedContext_;

    std::vector<std::string> errorList_;
    Animation* animation_;
    ScriptManagerLinking* scriptManagerLinking_;

private:
    static const std::string loggerCat_;
};

} // namespace

#endif //VRN_WORKSPACE_H
