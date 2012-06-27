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

#ifndef VRN_INTERPOLATIONFUNCTIONBASE_H
#define VRN_INTERPOLATIONFUNCTIONBASE_H

#include <string>
#include "tgt/tgt_gl.h"
#include "tgt/vector.h"

namespace voreen {

/**
 * This class is an non-templated interface for interpolation functions.
 */
class InterpolationFunctionBase {
public:

    virtual ~InterpolationFunctionBase() {}

    /**
     * Returns the name of the interpolation.
     */
    virtual std::string getName() const = 0;

    /**
     * Returns the identifier of the interpolation.
     */
    virtual std::string getIdentifier() const = 0;

    /**
     * Returns the mode of the interpolation.
     */
    virtual std::string getMode() const = 0;

protected:
    static GLubyte* convertTextureToRGBA(tgt::ivec3 dim, GLubyte* textur, GLuint inputformat);
    static GLubyte* changeTextureDimension(tgt::ivec3 in_dim, tgt::ivec3 out_dim, GLubyte* indata);
};

} // namespace voreen

#endif
