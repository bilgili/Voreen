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

#ifndef VRN_SIMPLETEXTURE_H
#define VRN_SIMPLETEXTURE_H

#include "tgt/vector.h"

namespace voreen {

template<typename T>
class SimpleTexture {
public:
    typedef T ElemType;

public:
    SimpleTexture(const tgt::ivec2& dimensions, const bool autoFree = true);
    SimpleTexture(const tgt::ivec3& dimensions, const bool autoFree = true);
    virtual ~SimpleTexture();

    virtual void free();

    const T* getData() const { return data_; }

    const tgt::ivec3 getDimensions() const { return dimensions_; }
    size_t getNumElements() const { return numElements_; }

    bool doesAutoFree() const { return autoFree_; }
    void setAutoFree(const bool autoFree) { autoFree_ = autoFree; }

    const T& operator[](size_t index) const;
    const T& operator[](const tgt::ivec2& position) const;
    const T& operator[](const tgt::ivec3& position) const;

    T& operator[](size_t index);
    T& operator[](const tgt::ivec2& position);
    T& operator[](const tgt::ivec3& position);

    void createWhiteNoise();

protected:
    size_t positionToIndex(const tgt::ivec2& position) const;
    size_t positionToIndex(const tgt::ivec3& position) const;

protected:
    const tgt::ivec3 dimensions_;
    const size_t numElements_;
    T* const data_;
    bool autoFree_;

private:
    SimpleTexture(const SimpleTexture&);
    SimpleTexture& operator=(const SimpleTexture&);
};

}   // namespace

#endif
