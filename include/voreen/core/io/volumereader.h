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

#ifndef VRN_VOLUMEREADER_H
#define VRN_VOLUMEREADER_H

#include <string>
#include <vector>

#include "voreen/core/datastructures/volume/volumecollection.h"
#include "voreen/core/io/ioprogress.h"

#include "tgt/exception.h"

namespace voreen {

// forward declarations
class Volume;

/**
 * Reads a volume dataset.
 * Implement this class in order to support a new format.
 */
class VolumeReader {
public:
    VolumeReader(IOProgress* progress = 0);
    virtual ~VolumeReader() {}

    /**
     * Virtual constructor.
     */
    virtual VolumeReader* create(IOProgress* progress = 0) const = 0;

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
    virtual VolumeCollection* read(const std::string& url) throw (tgt::FileException, std::bad_alloc) = 0;

    /**
     * Instead of reading a whole dataset, only some slices are read from file and written
     * to the newly built volume.
     *
     * Override this function in order to provide a brick-wise loading routine.
     * The default implementation throws an exception.
     */
    virtual VolumeCollection* readSlices(const std::string& url, size_t firstSlice = 0, size_t lastSlice = 0)
        throw(tgt::FileException, std::bad_alloc);

    /**
     * Instead of reading the whole dataset, only a brick of volumedata, specified by
     * its starting location and its dimensions, is read.
     *
     * Override this function in order to provide a brick-wise loading routine.
     * The default implementation throws an exception.
     */
    virtual VolumeCollection* readBrick(const std::string& url, tgt::ivec3 start, int dimensions)
        throw(tgt::FileException, std::bad_alloc);

    /**
     * Loads a single volume from the passed origin.
     *
     * The default implementation calls the read() function with the
     * origin's path. Override it in order to access the VolumeOrigin directly.
     *
     * @param origin The origin the data set is to be read from.
     * @return VolumeHandle encapsulating the loaded volume. The caller is responsible for freeing the memory.
     */
    virtual VolumeHandle* read(const VolumeOrigin& origin)
        throw (tgt::FileException, std::bad_alloc);

    /**
     * Returns whether this reader is persistent and should not be deleted after read() has
     * completed. This is necessary in case a reader want to do incremental loading, for example,
     * based on some view parameters. Then this reader must be freed through some other means.
     */
    virtual bool isPersistent() const;

    /**
     * Converts the file path of the passed origin to a path relative to the passed base path.
     *
     * @note The default implementation performs the conversion under the assumption that the origin's path
     *      refers to a physical file. Override this function in order to perform a custom conversion.
     *
     * @return origin with relative file path. If the passed origin already has a relative path or
     *      if it does not contain a file path at all, an identical copy is returned.
     */
    virtual VolumeOrigin convertOriginToRelativePath(const VolumeOrigin& origin, std::string& basePath) const;

    /**
     * Converts the passed origin's file path from a path relative to the passed base path
     * to an absolute path.
     *
     * @note The default implementation performs the conversion under the assumption that the origin's path
     *      refers to a physical file. Override this function in order to perform a custom conversion.
     *
     * @return origin with absolute file path. If the passed origin already has an absolute path or
     *      if it does not contain a file path at all, an identical copy is returned.
     */
    virtual VolumeOrigin convertOriginToAbsolutePath(const VolumeOrigin& origin, std::string& basePath) const;

    /**
     * Returns a list of filename extensions that are supported by this reader.
     */
    const std::vector<std::string>& getExtensions() const;

    /**
     * Returns a list of protocols that are supported by this reader.
     */
    const std::vector<std::string>& getProtocols() const;

protected:
    void read(Volume* volume, FILE* fin);
    IOProgress* getProgress() const { return progress_; }

    /**
     * Reverses the order of the slice in x-direction. This method
     * is called, when the .dat file contains "SliceOrder: -x" in one line.
     */
    void reverseXSliceOrder(Volume* const volume) const;

    /**
     * Reverses the order of the slice in y-direction. This method
     * is called, when the .dat file contains "SliceOrder: -y" in one line.
     */
    void reverseYSliceOrder(Volume* const volume) const;

    /**
     * Reverses the order of the slice in z-direction. This method
     * is called, when the .dat file contains "SliceOrder: -z" in one line.
     */
    void reverseZSliceOrder(Volume* const volume) const;

    /// List of filename extensions supported by the reader.
    std::vector<std::string> extensions_;

    /// List of protocols supported by the reader.
    std::vector<std::string> protocols_;

    static const std::string loggerCat_;

private:

    IOProgress* progress_;

};

} // namespace voreen

#endif // VRN_VOLUMEREADER_H
