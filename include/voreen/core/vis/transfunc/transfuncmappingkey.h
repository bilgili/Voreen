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

#ifndef VRN_TRANSFUNCMAPPINGKEY_H
#define VRN_TRANSFUNCMAPPINGKEY_H

#include "tgt/vector.h"

namespace voreen {


/**
*   One of multiple Keys that define a TransfuncIntensityKeys.
*   Each key has a intensity at which it is located and can be split in two parts
*   with different colors.
*/
class TransFuncMappingKey {
public:
    TransFuncMappingKey(float i, const tgt::col4& color);
    virtual ~TransFuncMappingKey();
    
    void setColorL(const tgt::col4& color);
    void setColorL(const tgt::ivec4& color);
    tgt::col4& getColorL();
    void setColorR(const tgt::col4& color);
    void setColorR(const tgt::ivec4& color);
    tgt::col4& getColorR();
    ///Set Alpha as float (0.0->1.0)
    void setAlphaR(float a);
    ///Set Alpha as float (0.0->1.0)
    void setAlphaL(float a);
    ///Get Alpha as float (0.0->1.0)
    float getAlphaR();
    ///Get Alpha as float (0.0->1.0)
    float getAlphaL();
    /**
     * Returns wether the key is split or not.
     *
     * @return The key is split.
     */
    virtual bool isSplit();
    
    /**
     * Split/unsplit the current key.
     *
     * @param split \c true to split, \c false to merge.
     * @param useLeft In case of joining: use the left color (else use right)
     */
    virtual void setSplit(bool split, bool useLeft=true);
    virtual float getIntensity();
    ///Set the intensity at which the key is located.
    ///Note: TFIK has to be sorted
    virtual void setIntensity(float i);

private:
    float intensity_;
    tgt::col4 colorL_;
    tgt::col4 colorR_;
    bool split_;
};

} // namespace voreen

#endif // VRN_TRANSFUNCMAPPINGKEY_H
