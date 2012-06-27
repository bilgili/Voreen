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

#ifndef VRN_MESHCOLORDATA_H
#define VRN_MESHCOLORDATA_H

#include <vector>
#include "tgt/vector.h"

namespace voreen {

/**
 * A class that encapsulates the colordata of a mesh. The data is stored in a vector of floats.
 * You can retrieve normalized values of the colordata. That means that the minimum value in the vector
 * is mapped to 0 and the maximum value is mapped to 1. If you want to map the floats to rgb-color
 * use a transferfunction or implement a mapping by yourself.
 */
class MeshColorData {

public:

    /**
     * Constructor
     *
     * @param caption caption of the object. used for display in gui
     * @param dimension dimension of the colordata
     */
    MeshColorData(const std::string caption, tgt::ivec2 dimension);

    /**
     * Destructor - does nothing
     */
    ~MeshColorData();

    /**
     * @return dimension of colordata
     */
    tgt::ivec2 getDimension();

    /**
     * This method returns the pure colordata.
     *
     * @return unnormalized colordata
     */
    const std::vector<float>& getData();

    /**
     * This method returns the normalized colordata, e.g. all values are lying in the interval [0,1]
     * where 0 is the minimum and 1 is the maximum of whole colordata.
     * 
     * @return returns the colordata in normalized range. That means.
     */
    const std::vector<float> getNormalizedData();

    /**
     * @return returns the caption
     */
    const std::string getCaption();

    /**
     * This method sets the data for the MeshColorData-object
     *
     * @param data the colordata that is stored in this object
     */
    void setData(std::vector<float> data);

    /**
     * Sets the colorvalue at a specific position in the vector.
     *
     * @param position position in the vector where the value should be set.
     * @param value new value.
     */
    void setData(int position, float value);

private:

    tgt::ivec2 dimension_;     ///< dimension of colordata (usually 25x16)
    std::string caption_;      ///< string that is displayed in the gui
    std::vector<float> data_;  ///< the colordata
};

} //namespace voreen

#endif // VRN_MESHCOLORDATA_H
