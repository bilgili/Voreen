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

#ifndef VRN_VOLUMECACHE_H
#define VRN_VOLUMECACHE_H

#include "voreen/core/io/cache.h"

namespace voreen {

class VolumeHandle;

/**
 * Class for handling caching of volumes. This done by caching the wrapper
 * classes VolumeHandle for Volume objects.
 *
 * @author  Dirk Feldmann, June/July 2009
 */
class VolumeCache : public Cache<VolumeHandle> {
public:
    VolumeCache();

    virtual ~VolumeCache() { /* dtor has nothing to do */ }

protected:
    /**
     * Inherited from <code>Cache&lt;T&gt;</code>.
     */
    //virtual VolumeHandle* getPortData(Port* const port) const;

    /**
     * Inherited from <code>Cache&lt;T&gt;</code>.
     */
    virtual VolumeHandle* loadObject(const std::string& filename) const;

    /**
     * Inherited from <code>CacheBase</code>.
     * The returned string the form of
     *
     * "[complete filename]([timestep])", e.g. "D:/data/nucleon.dat(0)"
     */
    virtual std::string portContentToString(Port* const port) const;

    /**
     * Inherited from <code>Cache&lt;T&gt;</code>.
     */
    virtual std::string saveObject(VolumeHandle* object, const std::string& directory,
        const std::string& filename);
};

}   // namespace

#endif
