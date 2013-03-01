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

#include "voreen/core/network/workspace.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/network/processornetwork.h"
#include "voreen/core/animation/animation.h"

#include "voreen/core/properties/link/linkevaluatorhelper.h"
#include "voreen/core/animation/animatedprocessor.h"

#include "tgt/filesystem.h"
#include "tgt/glcanvas.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <cstdio>
#include <sstream>

namespace voreen {

//-------------------------------------------------------------------------------------------------

const std::string Workspace::loggerCat_("voreen.Workspace");

namespace {
const int WORKSPACE_VERSION = 2;
} // namespace

Workspace::Workspace(tgt::GLCanvas* sharedContext)
    : version_(WORKSPACE_VERSION)
    , network_(new ProcessorNetwork())
    , animation_(0)
    , filename_("")
    , readOnly_(false)
    , description_("")
    , sharedContext_(sharedContext)
{
}

Workspace::~Workspace() {
    clear();
}

std::vector<std::string> Workspace::getErrors() const {
    return errorList_;
}

void Workspace::load(const std::string& filename, const std::string& workDir)
    throw (SerializationException)
{
    // open file for reading
    std::fstream fileStream(filename.c_str(), std::ios_base::in);
    if (fileStream.fail()) {
        //LERROR("Failed to open file '" << tgt::FileSystem::absolutePath(filename) << "' for reading.");
        throw SerializationException("Failed to open workspace file '" + tgt::FileSystem::absolutePath(filename) + "' for reading.");
    }

    std::string documentPath;
    if (!workDir.empty())
        documentPath = workDir;
    else
        documentPath = filename;

    // read data stream into deserializer
    XmlDeserializer d(documentPath);
    d.setUseAttributes(true);
    NetworkSerializer ser;
    try {
        d.read(fileStream, &ser);
    }
    catch (SerializationException& e) {
        throw SerializationException("Failed to read serialization data stream from workspace file '"
                                     + filename + "': " + e.what());
    }
    catch (...) {
        throw SerializationException("Failed to read serialization data stream from workspace file '"
                                     + filename + "' (unknown exception).");
    }

    // deserialize workspace from data stream
    try {
        d.deserialize("Workspace", *this);
        errorList_ = d.getErrors();
        setFilename(filename);
    }
    catch (std::exception& e) {
        throw SerializationException("Deserialization from workspace file '" + filename + "' failed: " + e.what());
    }
    catch (...) {
        throw SerializationException("Deserialization from workspace file '" + filename + "' failed (unknown exception).");
    }
}

void Workspace::save(const std::string& filename, bool overwrite, const std::string& workDir)
    throw (SerializationException)
{
    // check if file is already present
    if (!overwrite && tgt::FileSystem::fileExists(filename))
        throw SerializationException("File '" + filename + "' already exists.");

    std::string documentPath;
    if (!workDir.empty())
        documentPath = workDir;
    else
        documentPath = filename;

    // serialize workspace
    XmlSerializer s(documentPath);
    s.setUseAttributes(true);
    s.serialize("Workspace", *this);
    errorList_ = s.getErrors();

    // write serialization data to temporary string stream
    std::ostringstream textStream;

    try {
        s.write(textStream);
        if (textStream.fail())
            throw SerializationException("Failed to write serialization data to string stream.");
    }
    catch (std::exception& e) {
        throw SerializationException("Failed to write serialization data to string stream: " + std::string(e.what()));
    }
    catch (...) {
        throw SerializationException("Failed to write serialization data to string stream (unknown exception).");
    }

    // Now we have a valid StringStream containing the serialization data.
    // => Open output file and write it to the file.
    // For added data security we write to a temporary file and afterwards move it into place
    // (which should be an atomic operation).
    const std::string tmpfilename = filename + ".tmp";
    std::fstream fileStream(tmpfilename.c_str(), std::ios_base::out);
    if (fileStream.fail())
        throw SerializationException("Failed to open file '" + tmpfilename + "' for writing.");

    try {
        fileStream << textStream.str();
    }
    catch (std::exception& e) {
        throw SerializationException("Failed to write serialization data stream to file '"
                                     + tmpfilename + "': " + std::string(e.what()));
    }
    catch (...) {
        throw SerializationException("Failed to write serialization data stream to file '"
                                     + tmpfilename + "' (unknown exception).");
    }
    fileStream.close();

    // Finally move the temporary file into place. It is important that this happens in-place,
    // without deleting the old file first.
    bool success;
#ifdef WIN32
    // rename() does not replace existing files on Windows, so we have to use this
    success = (MoveFileEx(tmpfilename.c_str(), filename.c_str(),
                          MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED) != 0);

#else
    // atomic replace
    success = (rename(tmpfilename.c_str(), filename.c_str()) == 0);
#endif
    if (!success) {
#ifdef WIN32
        _unlink(tmpfilename.c_str()); // ignore failure here
#else
        unlink(tmpfilename.c_str()); // ignore failure here
#endif
        throw SerializationException("Failed to rename temporary file '" + tmpfilename + "' to '"
                                     + filename + "'");
    }

    // saving successful
    setFilename(filename);
}

void Workspace::clear() {

    bool glMode = VoreenApplication::app() && VoreenApplication::app()->isInitializedGL();
    if (!glMode)
        LDEBUG("clear() not in OpenGL mode");

    if (glMode) {
        if (sharedContext_)
            sharedContext_->getGLFocus();
        LGL_ERROR;
    }

    // network
    if (network_) {
        delete network_;
        if (glMode) {
            if (sharedContext_)
                sharedContext_->getGLFocus();
            LGL_ERROR;
        }
        network_ = 0;
    }

    // animation
    if (animation_) {
        if (glMode && sharedContext_)
            sharedContext_->getGLFocus();
        delete animation_;
        if (glMode) {
            if (sharedContext_)
                sharedContext_->getGLFocus();
            LGL_ERROR;
        }
        animation_ = 0;
    }

    errorList_.clear();
    readOnly_ = false;
}

bool Workspace::readOnly() const {
    return readOnly_;
}

ProcessorNetwork* Workspace::getProcessorNetwork() const {
    return network_;
}

void Workspace::setFilename(const std::string& filename) {
    filename_ = filename;

    // replace backslashes
    std::string::size_type pos = filename_.find("\\");
    while (pos != std::string::npos) {
        filename_[pos] = '/';
        pos = filename_.find("\\");
    }

}

std::string Workspace::getFilename() const {
    return filename_;
}

void Workspace::setProcessorNetwork(ProcessorNetwork* network) {
    network_ = network;
}

void Workspace::serialize(XmlSerializer& s) const {
    s.serialize("version", version_);

    // Serialize network...
    s.serialize("ProcessorNetwork", network_);

    // Serialize animation...
    s.serialize("Animation", animation_);

    s.serialize("GlobalDescription", description_);
}

void Workspace::deserialize(XmlDeserializer& s) {
    // clear existing network and containers
    clear();

    try {
        s.deserialize("readonly", readOnly_);
    }
    catch (XmlSerializationNoSuchDataException&) {
        s.removeLastError();
        readOnly_ = false;
    }

    // Deserialize network...
    s.deserialize("ProcessorNetwork", network_);

    // Deserialize animation if present...
    try {
        s.deserialize("Animation", animation_);
    } catch (XmlSerializationNoSuchDataException&) {
        s.removeLastError();
    }

    if (animation_) {
        animation_->setNetwork(network_);

        // register this as observer on all propertytimelines for undo/redo
        const std::vector<AnimatedProcessor*> animproc = this->animation_->getAnimatedProcessors();
        std::vector<AnimatedProcessor*>::const_iterator it;
        for (it = animproc.begin();it != animproc.end();it++)
        {
            const std::vector<PropertyTimeline*> timelines = (*it)->getPropertyTimelines();
            std::vector<PropertyTimeline*>::const_iterator it2;
            for (it2 = timelines.begin();it2 != timelines.end(); ++it2) {
                (*it2)->registerUndoObserver(this->animation_);
            }
        }

        // register this as observer in the processornetwork to register added and removed processors
        ProcessorNetwork* net = const_cast<ProcessorNetwork*>(network_);
        net->addObserver(this->animation_);
    }

    try {
        s.deserialize("GlobalDescription", description_);
    }
    catch (XmlSerializationNoSuchDataException&) {
        s.removeLastError();
        description_ = "";
    }
}

Animation* Workspace::getAnimation() const {
    return animation_;
}

void Workspace::setAnimation(Animation* anim) {
    animation_ = anim;
}

const std::string& Workspace::getDescription() const {
    return description_;
}

bool Workspace::hasDescription() const {
    return !description_.empty();
}

void Workspace::setDescription(const std::string& description) {
    description_ = description;
}

} // namespace
