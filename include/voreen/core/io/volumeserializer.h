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

#ifndef VRN_VOLUMESERIALIZER_H_
#define VRN_VOLUMESERIALIZER_H_

#include <map>
#include <vector>
#include <string>

#include "tgt/exception.h"

#ifndef VRN_VOLUMESET_H
#include "voreen/core/volume/volumeset.h"
#endif

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
 * You can register several VolumeReader instances in this class.
 * A call to \a load will then automatically select the proper VolumeReader
 * for you.
 */
class VolumeSerializer {
public:

    VolumeSerializer();
    ~VolumeSerializer();

    /**
     * Load one data set given by \p filename.
     *
     * @param filename The file name of the dataset
     */
    VolumeSet* load(const std::string& filename)
        throw(tgt::UnsupportedFormatException, tgt::CorruptedFileException, tgt::IOException, std::bad_alloc);

    /**
     * Saves a Volume to the given file.
     *
     * @param fileName The file name where the data should go.
     * @param volume The Volume that should be saved.
     */
    void save(const std::string& filename, Volume* volume)
        throw(tgt::UnsupportedFormatException, tgt::IOException);

    /**
     * Use this method to register a VolumeReader.
     *
     * @param vr The VolumeReader to be registered.
     */
    void registerReader(VolumeReader* vr)
        throw(FormatClashException);

    /**
     * Use this method to register a VolumeWriter.
     *
     * @param vw The VolumeWriter to be registered.
     */
    void registerWriter(VolumeWriter* vw)
        throw(FormatClashException);

private:

    typedef std::map<std::string, VolumeReader*> Readers;
    typedef std::map<std::string, VolumeWriter*> Writers;
    Readers readers_; ///< maps one or more extensions to a VolumeReader
    Writers writers_; ///< maps one or more extensions to a VolumeWriter
};

} // namespace voreen

#endif // VRN_VOLUMESERIALIZER_H_
