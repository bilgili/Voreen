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

#ifndef VRN_OMEVOLUMEREADER_H
#define VRN_OMEVOLUMEREADER_H

#include "voreen/core/io/volumereader.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"

#include <tiffio.h>

class TiXmlNode;
class TiXmlElement;

namespace voreen {

/**
 * Reader for microscopy data stored in the OME-TIFF format.
 *
 * A specific channel and timestep in the 5D OME stack can be selected in the URL
 * via the "channel" and "timestep" search parameters, respectively. 
 * If no channel/timestep is selected, the entire stack is loaded and put out as volume list.
 *
 * Supported extensions:
 * - ome.tiff
 * - ome.tif
 *
 * Used protocol: ome-tiff
 *
 * @note Requires the tiff library.
 */
class OMETiffVolumeReader : public VolumeReader {
public:
    OMETiffVolumeReader(ProgressBar* progress = 0);
    ~OMETiffVolumeReader() {}
    virtual VolumeReader* create(ProgressBar* progress = 0) const;

    virtual std::string getClassName() const         { return "OMETiffVolumeReader";      }
    virtual std::string getFormatDescription() const { return "OME-TIFF microscopy data"; }

    virtual VolumeList* read(const std::string& url)
        throw (tgt::FileException, std::bad_alloc);

    virtual VolumeBase* read(const VolumeURL& origin)
        throw (tgt::FileException, std::bad_alloc);

    virtual std::vector<VolumeURL> listVolumes(const std::string& url) const
        throw (tgt::FileException);

private:
    /// Represents one tiff file on the disk.
    struct OMETiffFile {
        OMETiffFile(std::string filename, size_t numDirectories, size_t firstZ, size_t firstT, size_t firstC);
        std::string toString() const;

        std::string filename_;   //< absolute path to tiff file
        size_t numDirectories_;  //< number of directories, i.e. slices, contained by the file
        size_t firstZ_;          //< z coordinate of the first slice in the file
        size_t firstT_;          //< t coordinate of the first slice in the file
        size_t firstC_;          //< c coordinate of the first slice in the file
    };

    /// Extracts the OME-XML from the passed opened Tiff file and parses it for the dataset information.
    void extractMetaData(TIFF* tiffFile, const std::string& path, std::string& dimensionOrder, tgt::svec3& dimensions, int& sizeC, int& sizeT, 
        std::string& dataType, tgt::vec3& spacing, std::vector<OMETiffFile>& files) const
        throw (tgt::Exception);

    /** 
     * Determines the number of directories of each of the passed tiff files.
     * Note: Requires to open each file and iterate over its directories.
     */
    void determineDirectoryCount(std::vector<OMETiffFile>& files, size_t& totalNumberOfSlices)
        throw (tgt::Exception);

    /**
     * Reads the pixel data from the current directory of the passed opened Tiff file 
     * and copies it to the dest buffer.
     */
    void readTiffDirectory(TIFF* tiffFile, const std::string& dataType, const tgt::svec3& volumeDim, void* destBuffer) const
        throw (tgt::Exception);

    // XPath-like XML helper functions
    const TiXmlNode* getXMLNode(const TiXmlNode* parent, const std::string& path) const 
        throw (tgt::Exception);
    const TiXmlElement* getXMLElement(const TiXmlNode* parent, const std::string& path) const 
        throw (tgt::Exception);
    std::vector<const TiXmlNode*> getXMLNodeList(const TiXmlNode* parent, const std::string& path, const std::string& nodeName) const
        throw (tgt::Exception);
    std::vector<const TiXmlElement*> getXMLElementList(const TiXmlNode* parent, const std::string& path, const std::string& nodeName) const
        throw (tgt::Exception);

    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_OMEVOLUMEREADER_H
