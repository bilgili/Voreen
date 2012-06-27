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
#include "voreen/core/io/serialization/serialization.h"

namespace tgt {
    class GLCanvas;
}

namespace voreen {

class ProcessorNetwork;
class VolumeContainer;
class Animation;
class ScriptManagerLinking;

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

    /**
     * Deletes the current network and the resource containers,
     * and clears the serialization error collector.
     *
     * The workspace's filename is not cleared.
     */
    void clear();

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
     * Returns the errors that have occurred during serialization.
     */
    std::vector<std::string> getErrors() const;

    /**
     * Exports the workspace and all associated resources, such as volumes,
     * to a Zip archive. File paths are adjusted, such that the contained vws-file
     * can be directly opened after manually extracting the exported archive.
     *
     * @param archive Full path to archive to be created
     * @param overwrite determines, whether an existing archive should be overwritten
     *
     * @return True, if the export has been successful
     */
    bool exportToZipArchive(const std::string& archive, bool overwrite = true)
        throw (SerializationException);

    /// Flag prompting the application to not overwrite the workspace file, not used by serialize()
    bool readOnly() const;

    ProcessorNetwork* getProcessorNetwork() const;
    void setProcessorNetwork(ProcessorNetwork* network);

    void setVolumeContainer(VolumeContainer* volumeContainer);
    VolumeContainer* getVolumeContainer() const;

    void setFilename(const std::string& filename);
    std::string getFilename() const;

    Animation* getAnimation() const;
    void setAnimation(Animation* anim);

    /// @see Serializable::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Serializable::deserialize
    virtual void deserialize(XmlDeserializer& s);

private:
    int version_;
    ProcessorNetwork* network_;
    VolumeContainer* volumeContainer_;
    Animation* animation_;
    std::string filename_;
    bool readOnly_;

    tgt::GLCanvas* sharedContext_;

    std::vector<std::string> errorList_;

    ScriptManagerLinking* scriptManagerLinking_;

    static const std::string loggerCat_;
};

} // namespace

#endif //VRN_WORKSPACE_H
