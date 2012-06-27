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

#include "tgt/framebufferobject.h"
#include "tgt/logmanager.h"

namespace tgt {

const std::string FramebufferObject::loggerCat_("tgt.FramebufferObject");

FramebufferObject::FramebufferObject()
  : id_(0)
{
    generateId();
}

FramebufferObject::~FramebufferObject()
{
    glDeleteFramebuffersEXT(1, &id_);
}

void FramebufferObject::activate()
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id_);
}

void FramebufferObject::deactivate()
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void FramebufferObject::attachTexture(Texture* texture, GLenum attachment, int mipLevel, int zSlice)
{
    switch(texture->getType()) {
        case GL_TEXTURE_1D:
            glFramebufferTexture1DEXT( GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_1D, texture->getId(), mipLevel );
            break;
        case GL_TEXTURE_3D:
            glFramebufferTexture3DEXT( GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_3D, texture->getId(), mipLevel, zSlice );
            break;
        case GL_TEXTURE_2D_ARRAY_EXT:
            glFramebufferTextureLayerEXT( GL_FRAMEBUFFER_EXT, attachment, texture->getId(), mipLevel, zSlice );
            break;
        default: //GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE
            glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, attachment, texture->getType(), texture->getId(), mipLevel );
            break;
    }
    attachedTextures_[attachment] = texture;
}

Texture* FramebufferObject::getTextureAtAttachment(GLenum attachment) {
    std::map<GLenum, Texture*>::iterator iter = attachedTextures_.find(attachment);
    if( iter != attachedTextures_.end() ) {
        return attachedTextures_[attachment];
    }
    else
        return 0;
}

void FramebufferObject::detachTexture(GLenum attachment) {
    std::map<GLenum, Texture*>::iterator iter = attachedTextures_.find(attachment);
    if( iter != attachedTextures_.end() ) {
        attachedTextures_.erase(iter);
    }
    else {
        LWARNING("Trying to detach unknown texture!");
    }

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, GL_TEXTURE_2D, 0, 0);
}

void FramebufferObject::detachAll() {
    while(!attachedTextures_.empty()) {
        detachTexture(attachedTextures_.begin()->first);
    }
}

bool FramebufferObject::isComplete() const
{
  bool complete = false;

  GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  switch(status) {
      case GL_FRAMEBUFFER_COMPLETE_EXT:
          complete = true;
          break;
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
          LERROR("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT");
          break;
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
          LERROR("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT");
          break;
      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
          LERROR("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT");
          break;
      case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
          LERROR("GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT");
          break;
      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
          LERROR("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT");
          break;
      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
          LERROR("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT");
          break;
      case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
          LERROR("GL_FRAMEBUFFER_UNSUPPORTED_EXT");
          break;
      default:
          LERROR("Unknown error!");
  }
  return complete;
}

bool FramebufferObject::isActive() const {
    return ((getActiveObject() == id_) && (id_ != 0));
}

GLuint FramebufferObject::getActiveObject() {
    GLint fbo;
    glGetIntegerv (GL_FRAMEBUFFER_BINDING_EXT, &fbo);
    return static_cast<GLuint>(fbo);
}

GLuint FramebufferObject::generateId() {
    id_ = 0;
    glGenFramebuffersEXT(1, &id_);
    return id_;
}

} // namespace
