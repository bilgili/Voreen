/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
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

#ifndef VRN_WORKSPACE_H
#define VRN_WORKSPACE_H

#include "voreen/core/network/networkserializer.h"
#include "voreen/core/io/serialization/serialization.h"

namespace tgt {
    class GLCanvas;
}

namespace voreen {

class ProcessorNetwork;
class Animation;

class VRN_CORE_API Workspace : public Serializable {
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
     * Non-fatal errors occurring during workspace load are saved
     * and can be requested using @c getErrors().
     *
     * @param filename the workspace file to load
     * @param workDir Absolute working directory of the workspace, used for relative-to-absolute path conversions.
     *      Is passed as document path to the XMLDeserializer. If an empty string is passed, the location
     *      of the workspace file is used as working directory.
     *
     * @throw SerializationException on serialization errors
     */
    void load(const std::string& filename, const std::string& workDir = "")
        throw (SerializationException);

    /**
     * Saves the workspace to the specified file.
     *
     * @param filename the file the workspace will be written to
     * @param if true, an existing file will be overwritten, otherwise an exception is thrown when the file already exists
     * @param workDir Absolute working directory of the workspace, used for absolute-to-relative path conversions.
     *      Is passed as document path to the XMLSerializer. If an empty string is passed, the output location
     *      of the workspace file is used as working directory.
     *
     * @throw SerializationException on serialization errors
     */
    void save(const std::string& filename, bool overwrite = true, const std::string& workDir = "") throw (SerializationException);

    /**
     * Returns the errors that have occurred during serialization.
     */
    std::vector<std::string> getErrors() const;

    /// Flag prompting the application to not overwrite the workspace file, not used by serialize()
    bool readOnly() const;

    ProcessorNetwork* getProcessorNetwork() const;
    void setProcessorNetwork(ProcessorNetwork* network);

    void setFilename(const std::string& filename);
    std::string getFilename() const;

    Animation* getAnimation() const;
    void setAnimation(Animation* anim);

    const std::string& getDescription() const;
    bool hasDescription() const;
    void setDescription(const std::string& description);

    /// @see Serializable::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Serializable::deserialize
    virtual void deserialize(XmlDeserializer& s);

private:
    int version_;
    ProcessorNetwork* network_;
    Animation* animation_;
    std::string filename_;
    bool readOnly_;
    std::string description_;

    tgt::GLCanvas* sharedContext_;

    std::vector<std::string> errorList_;

    static const std::string loggerCat_;
};

} // namespace

#endif //VRN_WORKSPACE_H
