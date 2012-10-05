/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#ifndef VRN_PREINTEGRATIONTABLE_H
#define VRN_PREINTEGRATIONTABLE_H

//#include "tgt/types.h"
#include "tgt/vector.h"
#include "tgt/texture.h"
#include "voreen/core/datastructures/transfunc/transfunc1dkeys.h"

namespace voreen {

/**
 * This realizes a pre-integration table (pre-integrated transfer function) as described by Engel et al., 2001.
 */
class VRN_CORE_API PreIntegrationTable
{
public:

    /**
     * Constructor, automatically calls computeTable at the end.
     *
     * @param transFunc the transfer function for with the pre-integration table is computed
     * @param resolution the resolution of the table in both directions (if <= 1 the resolution is set to 256)
     * @param d the segment length (= sampling step size) for which the pre-integration table is computed (if <= 0 the segment length is set to 1.0)
     * @param useIntegral Use integral to compute pre-integration table, which is faster but not quite as accurate.
     */
    PreIntegrationTable(const TransFunc1DKeys* transFunc, size_t resolution = 256, float d = 1.f, bool useIntegral = true);

    ~PreIntegrationTable();

    /**
     * Compute the pre-integrated table for the given transfer function
     */
    void computeTable();

    /**
     * Return the value in the pre-integration table.
     * This will return a vec4(0, 0, 0, 0) if the access is out of range [0, 1] or if no table is found.
     *
     * @param fs normalized intensity value at the start of the segment
     * @param fe normalized intensity value at the end of the segment
     */
    tgt::vec4 classify(float fs, float fe) const;

    /**
     * Returns the texture of the pre-integration table.
     * If no texture has been created yet, createTex() is called.
     */
    const tgt::Texture* getTexture() const;

    float getSamplingStepSize() const;

    bool usesIntegral() const;

    size_t getDimension() const;

private:

    // no default constructor
    PreIntegrationTable();

    /// Returns the RGBA values for a segment.
    tgt::vec4 apply1DTF(const tgt::Texture* tfTexture, float intensity);

    /// Generates the texture for the pre-integration table.
    virtual void createTex() const;

    const TransFunc1DKeys* transFunc_; ///< the 1D transfer function that is used to compute the pre-integration table

    size_t resolution_; ///< resolution of the pre-integrated table
    float samplingStepSize_; ///< length of the segments
    bool useIntegral_;

    tgt::vec4* table_; ///< the actual pre-integration table in row-major order

    mutable tgt::Texture* tex_; ///< texture for the pre-integration table, is generated internally

};

} //namespace

#endif // VRN_PREINTEGRATIONTABLE_H
