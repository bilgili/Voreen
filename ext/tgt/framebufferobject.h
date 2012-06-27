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

#ifndef TGT_FRAMEBUFFEROBJECT_H
#define TGT_FRAMEBUFFEROBJECT_H

#include "tgt/texture.h"
#include "tgt/types.h"

#include <map>

namespace tgt {

class TGT_API FramebufferObject {
public:
    FramebufferObject();
    virtual ~FramebufferObject();

    void activate();
    static void deactivate();

    bool isComplete() const;

    bool isActive() const;

    /// Bind a texture to the "attachment" point of this FBO
    void attachTexture(Texture* texture,
                       GLenum attachment = GL_COLOR_ATTACHMENT0_EXT,
                       int mipLevel      = 0,
                       int zSlice        = 0);

    void detachTexture(GLenum attachment);

    void detachAll();

    Texture* getTextureAtAttachment(GLenum attachment);

    static GLuint getActiveObject();

protected:
    GLuint generateId();

    GLuint id_;
    std::map<GLenum, Texture*> attachedTextures_;

    static const std::string loggerCat_; ///< category used in logging
};

} // namespace tgt

#endif // TGT_FRAMEBUFFEROBJECT_H
