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

#ifndef VRN_VOLUMESERIALIZER_H_
#define VRN_VOLUMESERIALIZER_H_

#include <map>
#include <vector>
#include <string>

#include "tgt/exception.h"

#include "voreen/core/volume/volumehandle.h"
#include "voreen/core/volume/volumecollection.h"

namespace voreen {

// forward declarations
class FormatClashException;
class Volume;
class VolumeReader;
class VolumeWriter;
class VolumeSerializer;

/**
 * Thrown when a VolumeReader or VolumeWriter was tried to register for an
 * extension/extensions that has/have a handler/handlers.
 */
class FormatClashException : std::exception {
public:

    typedef std::vector<std::string> Extensions;

    /// @param extensions The extensions which have already been registered.
    FormatClashException(const Extensions& extensions);
    virtual ~FormatClashException() throw() {}

    /// Returns the extensions which have already been registered.
    const Extensions& getExtensions() const throw();

    virtual const char* what() const throw();

protected:

    Extensions extensions_; ///< The extensions which have already been registered.
};

//------------------------------------------------------------------------------

/**
 * You can register several VolumeReader and VolumeWriter instances in this class.
 * A call to \a load or \a save will then automatically select the proper VolumeReader /
 * VolumeWriter and delegate the loading / saving to it.
 */
class VolumeSerializer {
public:

    VolumeSerializer();
    ~VolumeSerializer();

    /**
     * Loads one or multiple volumes from the specified URL.
     *
     * @param url The URL the data set is to be read from.
     *      This may be a file path, e.g. /file/to/volume.dat,
     *      or a complete URL with resource type and inner path, e.g.
     *      zip://path/to/archive.zip/volume.dat
     * @return VolumeCollection containing all volumes read from the url.
     *      the caller is responsible for freeing the memory.
     */
    VolumeCollection* load(const std::string& url) const
        throw (tgt::FileException, std::bad_alloc);

    /**
     * Instead of reading the whole dataset, only some slices are read from file and written
     * to the newly built volume. This isn't supported by all file formats.
     *
     * @return new VolumeCollection, the caller is responsible for freeing the memory.
     */
    VolumeCollection* loadSlices(const std::string& url, size_t firstSlice, size_t lastSlice) const
        throw (tgt::FileException, std::bad_alloc);

    /**
     * Instead of reading the whole dataset, only a brick of volume data, specified by
     * its starting location and its dimensions, is read. This isn't supported by all file formats.
     *
     * @return new VolumeCollection, the caller is responsible for freeing the memory.
     */
    VolumeCollection* loadBrick(const std::string& url, tgt::ivec3 brickStartPos, int brickSize)  const
        throw (tgt::FileException, std::bad_alloc);

    /**
     * Reads a single volume from the specified origin.
     *
     * This function is mainly used for the deserialization
     * of VolumeHandles.
     *
     * @return VolumeHandle encapsulating the loaded volume.
     *      The caller is responsible for freeing the memory.
     */
    VolumeHandle* load(const VolumeOrigin& origin) const
        throw (tgt::FileException, std::bad_alloc);

    /**
     * Converts the file path of the passed origin to a path relative to the passed base path.
     * The conversion is delegated to the appropriate reader.
     *
     * @return origin with relative file path. If the passed origin already has a relative path or
     *      if it does not contain a file path at all, an identical copy is returned.
     */
    VolumeOrigin convertOriginToRelativePath(const VolumeOrigin& origin, std::string& basePath) const
        throw (tgt::FileException);

    /**
     * Converts the passed origin's file path from a path relative to the passed base path
     * to an absolute path. The conversion is delegated to the appropriate reader.
     *
     * @return origin with absolute file path. If the passed origin already has an absolute path or
     *      if it does not contain a file path at all, an identical copy is returned.
     */
    VolumeOrigin convertOriginToAbsolutePath(const VolumeOrigin& origin, std::string& basePath) const
        throw (tgt::FileException);

    /**
     * Saves a Volume to the given file.
     *
     * @param fileName The file name where the data should go.
     * @param volume The Volume that should be saved.
     */
    void save(const std::string& filename, VolumeHandle* volume) const throw (tgt::FileException);

    /**
     * Saves a Volume to the given file.
     *
     * @param fileName The file name where the data should go.
     * @param volume The Volume that should be saved.
     */
    void save(const std::string& filename, Volume* volume) const throw (tgt::FileException);

    /**
     * Use this method to register a VolumeReader.
     *
     * @param vr The VolumeReader to be registered.
     */
    void registerReader(VolumeReader* vr)
        throw (FormatClashException);

    /**
     * Use this method to register a VolumeWriter.
     *
     * @param vw The VolumeWriter to be registered.
     */
    void registerWriter(VolumeWriter* vw)
        throw (FormatClashException);

private:

    /// maps from filename extensions to the appropriate reader
    std::map<std::string, VolumeReader*> readersExtensionMap_;

    /// maps from protocol strings (e.g. zip, dicom) to the appropriate reader
    std::map<std::string, VolumeReader*> readersProtocolMap_;

    /// maps from filename extensions to the appropriate writer
    std::map<std::string, VolumeWriter*> writersMap_;

    /**
     * @brief Find a suitable VolumeReader for the specified URL.
     *
     * @return A VolumeReader that is able to read from the URL.
     */
    VolumeReader* getReader(const std::string& url) const
        throw (tgt::FileException, std::bad_alloc);

    /**
     * @brief Find a suitable VolumeWriter for the specified filename.
     *
     * @return A VolumeWriter that is able to write to this file.
     */
    VolumeWriter* getWriter(const std::string& filename) const
        throw (tgt::FileException, std::bad_alloc);
};

} // namespace voreen

#endif // VRN_VOLUMESERIALIZER_H_
