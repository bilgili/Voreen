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

#include "voreen/core/network/workspace.h"

#include "voreen/core/io/datvolumereader.h"
#include "voreen/core/io/datvolumewriter.h"
#include "voreen/core/datastructures/volume/volumecontainer.h"
#include "voreen/modules/base/processors/datasource/volumesource.h"
#include "voreen/core/properties/link/linkevaluatorfactory.h"
#include "voreen/core/properties/filedialogproperty.h"  // needed for zip-export
#include "tgt/ziparchive.h"
#include "voreen/core/animation/animatedprocessor.h"

#ifdef WIN32
#include <windows.h>
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
    , volumeContainer_(new VolumeContainer())
    , filename_("")
    , readOnly_(false)
    , sharedContext_(sharedContext)
    , animation_(0)
    , scriptManagerLinking_(new ScriptManagerLinking())
{
    LinkEvaluatorFactory::getInstance()->setScriptManager(scriptManagerLinking_);
}

Workspace::~Workspace() {
    clear();
}

std::vector<std::string> Workspace::getErrors() const {
    return errorList_;
}

#ifndef VRN_WITH_ZLIB
bool Workspace::exportZipped(const std::string& /*exportName*/, bool /*overwrite*/)
    throw (SerializationException)
{
    LERROR("exportZipped(): Cannot export workspace, because Voreen was compiled without "
           "support for handling zip archives!");
    return false;
#else
bool Workspace::exportZipped(const std::string& exportName, bool overwrite)
    throw (SerializationException)
{
    if (!volumeContainer_)
        throw SerializationException("No volume container present");

    // check if archive already exists
    if (tgt::FileSystem::fileExists(exportName)) {
        if (overwrite) {
            // try to delete it
            if (!tgt::FileSystem::deleteFile(exportName)) {
                LERROR("Unable to delete file " << exportName);
                throw SerializationException("Unable to overwrite file " + exportName);
            }
        }
        else {
            LWARNING("Archive " << exportName << " already exists.");
            throw SerializationException("Archive " + exportName + " already exists.");
        }
    }

    // Save the original VolumeOrigin itself
    //
    std::vector<VolumeOrigin> previousOrigins;
    for (size_t i = 0; i < volumeContainer_->size(); ++i) {
        VolumeHandle* handle = volumeContainer_->at(i);
        previousOrigins.push_back(handle->getOrigin());

        // Get the old file name, remove file extension if present and
        // set a new origin with that new file name for the exported
        // version of the workspace.
        // .flow files need special treatment for they cannot be converted
        // to .dat/.raw files and are therefore added "as they are".
        //
        std::string fileName = handle->getOrigin().getFilename();
        if (tgt::FileSystem::fileExtension(fileName) != "flow") {
            fileName = fileName.substr(0, fileName.rfind("."));
            handle->setOrigin(VolumeOrigin(("./data/" + fileName + ".dat")));
        } else
            handle->setOrigin(VolumeOrigin(("./data/" + fileName)));
    }

    // Now collect all FileDialogProperties within the network, save their
    // original file names and adjust the names to match the future paths
    // within the zip archive before serializing the workspace.
    //
    typedef std::map<FileDialogProperty*, std::string> FileDialogMap;
    FileDialogMap fdps;
    const std::vector<Processor*>& processors = network_->getProcessors();
    for (size_t i = 0; i < processors.size(); ++i) {
        const std::vector<Property*>& props = processors[i]->getProperties();
        for (size_t j = 0; j < props.size(); ++j) {
            FileDialogProperty* fdp = dynamic_cast<FileDialogProperty*>(props[j]);
            if ((fdp == 0) || (fdp->get().empty() == true))
                continue;

            std::pair<FileDialogMap::iterator, bool> res =
                fdps.insert(std::make_pair(fdp, fdp->get()));
            if (res.second == true)
                fdp->set("./files/" + tgt::FileSystem::fileName(fdp->get()));
        }   // for (j
    }   // for (i

    // Serialize the adjusted version of the workspace to a stringstream.
    //
    std::ostringstream stringStream;
    XmlSerializer serializer;
    serializer.setUseAttributes(true);
    serializer.serialize("Workspace", *this);
    serializer.write(stringStream);
    std::string data = stringStream.str();

    // generate filename of vws file from current filename
    std::string vwsfilename = filename_;
    if (vwsfilename.find(".zip") != std::string::npos) {
        vwsfilename = vwsfilename.substr(0, vwsfilename.find(".zip")) + ".vws";
    }
    if (vwsfilename.empty())
        vwsfilename = "workspace.vws";

    tgt::ZipArchive zip(exportName);
    size_t size = (stringStream.tellp() > 0) ? static_cast<size_t>(stringStream.tellp()) : 0;
    tgt::MemoryFile memFile(const_cast<char*>(data.c_str()), size, vwsfilename, false);
    bool result = zip.addFile(&memFile);

    // Export all volume data currently present in the VolumeContainer to the zip
    // archive by storing them inside as .dat and .raw files. This facilitates
    // exporting and importing workspaces, because external data, e.g. from other
    // zip files are handled uniformly.
    //
    DatVolumeWriter dvw;
    std::vector<tgt::MemoryFile*> datFiles;
    std::vector<tgt::MemoryFile*> rawFiles;
    for (size_t i = 0; ((result == true) && (i < volumeContainer_->size())); ++i) {
        VolumeHandle* handle = volumeContainer_->at(i);

        // get the file name only and remove file extension, if it is not
        // a .flow file, which need "special" treatment
        //
        std::string fileName = handle->getOrigin().getFilename();
        if (tgt::FileSystem::fileExtension(previousOrigins[i].getFilename()) != "flow") {
            fileName = fileName.substr(0, fileName.rfind("."));
            std::string rawFileName = fileName + ".raw";

            // Get information about the volume and a .dat file, as well as the
            // correct size of the volume data and a pointer to them
            //
            char* rawData = 0;
            size_t numBytesRaw = 0;
            std::string datFileString = dvw.getDatFileString(handle, rawFileName, &rawData, numBytesRaw);

            // Add the .dat file to the zip-archive as a memory-mapped file. The string needs to be
            // copied, because the memory file does not copy the passed data.
            //
            size_t numBytesDat = datFileString.size();
            char* datFileData = new char[numBytesDat];
            memcpy(datFileData, datFileString.c_str(), numBytesDat);
            datFiles.push_back(new tgt::MemoryFile(datFileData, numBytesDat, (fileName + ".dat"), true));
            result = zip.addFile(datFiles.back(), "data/");

            // Pack the raw data in also into a memory file and add them to the archive - no
            // further memory (except the one for the file handle, but not for the data!)
            // should be required.
            //
            if (result) {
                rawFiles.push_back(new tgt::MemoryFile(rawData, numBytesRaw, rawFileName, false));
                result = zip.addFile(rawFiles.back(), "data/");
            }
        }
        else    // if (fileExtension == "flow"
            result = zip.addFile(previousOrigins[i].getPath(), "data/");
    }   // for (i

    // Add the files from the FileDialogProperty to the archive and restore the
    // original names/pathes to their files.
    //
    for (FileDialogMap::iterator it = fdps.begin(); fdps.empty() == false;
        fdps.erase(it), it = fdps.begin())
    {
        bool added = zip.addFile(it->second, "files/");
        if (added == false) {
            LERROR("Failed to add file '" << it->second << "' from FileDialogProperty ");
            LERROR("to zip archive '" << exportName << "'!");
        }
        it->first->set(it->second); // first = FileDialogProperty*, second = std::string
    }

    if (result == true)
        result = zip.save();

    if (result == true) {
        LINFO("Exported workspace successfully to '" << exportName << "'.");
        setFilename(exportName);
    }
    else
        LERROR("Export of workspace to file '" << exportName << "' failed!");

    // free temporary memory files from .dat export
    //
    for (size_t i = 0; i < datFiles.size(); ++i)
        delete datFiles[i];
    for (size_t j = 0; j < rawFiles.size(); ++j)
        delete rawFiles[j];

    // Restore VolumeOrigins and base path
    //
    for (size_t i = 0; ((i < volumeContainer_->size()) && (i < previousOrigins.size())); ++i)
        volumeContainer_->at(i)->setOrigin(previousOrigins[i]);

    return result;
#endif // VRN_WITH_ZLIB
}

void Workspace::load(const std::string& filename)
    throw (SerializationException)
{
    // open file for reading
    std::fstream fileStream(filename.c_str(), std::ios_base::in);
    if (fileStream.fail()) {
        LERROR("Failed to open file '" << filename << "' for reading.");
        throw SerializationException("Failed to open file '" + filename + "' for reading.");
    }

    // read data stream into deserializer
    XmlDeserializer d(filename);
    d.setUseAttributes(true);
    NetworkSerializer ser;
    try {
        d.read(fileStream, &ser);
    }
    catch (SerializationException& e) {
        throw SerializationException("Failed to read serialization data stream from file '"
                                     + filename + "': " + e.what());
    }
    catch (...) {
        throw SerializationException("Failed to read serialization data stream from file '"
                                     + filename + "' (unknown exception).");
    }

    // deserialize workspace from data stream
    try {
        d.deserialize("Workspace", *this);
        errorList_ = d.getErrors();
        setFilename(filename);
    }
    catch (std::exception& e) {
        throw SerializationException("Deserialization from file '" + filename + "' failed: " + e.what());
    }
    catch (...) {
        throw SerializationException("Deserialization from file '" + filename + "' failed (unknown exception).");
    }
}

void Workspace::save(const std::string& filename, bool overwrite)
    throw (SerializationException)
{
    // check if file is already present
    if (!overwrite && tgt::FileSystem::fileExists(filename))
        throw SerializationException("File '" + filename + "' already exists.");

    // serialize workspace
    XmlSerializer s(filename);
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

    if (sharedContext_)
        sharedContext_->getGLFocus();
    LGL_ERROR;

    // network
    if (network_) {
        delete network_;
        if (sharedContext_)
            sharedContext_->getGLFocus();
        LGL_ERROR;
        network_ = 0;
    }

    // volume container
    if (volumeContainer_) {
        if (sharedContext_)
            sharedContext_->getGLFocus();
        delete volumeContainer_;
        if (sharedContext_)
            sharedContext_->getGLFocus();
        LGL_ERROR;
        volumeContainer_ = 0;
    }

    // animation
    if (animation_) {
        if (sharedContext_)
            sharedContext_->getGLFocus();
        delete animation_;
        if (sharedContext_)
            sharedContext_->getGLFocus();
        LGL_ERROR;
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

VolumeContainer* Workspace::getVolumeContainer() const {
    return volumeContainer_;
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

void Workspace::setVolumeContainer(VolumeContainer* volumeContainer) {
    volumeContainer_ = volumeContainer;
}

void Workspace::serialize(XmlSerializer& s) const {
    s.serialize("version", version_);

    // Serialize volumecontainer...
    s.serialize("VolumeContainer", volumeContainer_);

    // Serialize ScriptManagerLinking
    s.serialize("ScriptManagerLinking", scriptManagerLinking_);

    // Serialize network...
    s.serialize("ProcessorNetwork", network_);

    // Serialize animation...
    s.serialize("Animation", animation_);
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

    // Deserialize volume-container...
    s.deserialize("VolumeContainer", volumeContainer_);

    // Deserialize ScriptManagerLinking
    try {
        s.deserialize("ScriptManagerLinking", scriptManagerLinking_);
    }
    catch(XmlSerializationNoSuchDataException&) {
        s.removeLastError();
    }
    LinkEvaluatorFactory::getInstance()->setScriptManager(scriptManagerLinking_);

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
}

Animation* Workspace::getAnimation() const {
    return animation_;
}

void Workspace::setAnimation(Animation* anim) {
    animation_ = anim;
}

} // namespace
