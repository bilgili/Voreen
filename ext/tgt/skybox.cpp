/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#include "tgt/skybox.h"

#include "tgt/tgt_gl.h"
#include "tgt/texturemanager.h"

namespace tgt {

Skybox::Skybox(const std::string texNames[6], const Bounds& bounds)
  : Renderable(bounds, true) {
    for (size_t i = 0; i < 6; i++)
        skyTextures_[i] = TexMgr.load(texNames[i], Texture::LINEAR);
}

Skybox::~Skybox() {
    for (size_t i = 0; i < 6; i++)
        TexMgr.dispose(skyTextures_[i]);
}

void Skybox::render() {
    glEnable(GL_TEXTURE_2D );
    glColor4f(1.f, 1.f, 1.f, 1.f);

    vec3 llf = boundingBox_.getLLF();
    vec3 urb = boundingBox_.getURB();

    /* render TOP sky texture */
    if (skyTextures_[TOP]) skyTextures_[TOP]->bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBegin(GL_QUADS);
    glNormal3f(0.f, -1.f, 0.f);
    glTexCoord2f( 0.f , 0.f);
    glVertex3f( llf.x, urb.y, llf.z );

    glTexCoord2f( 1.f, 0.f );
    glVertex3f( urb.x, urb.y, llf.z );

    /* coincides with urb */
    glTexCoord2f( 1.f, 1.f );
    glVertex3f( urb.x, urb.y, urb.z );

    glTexCoord2f( 0.f, 1.f );
    glVertex3f( llf.x, urb.y, urb.z );
    glEnd();

	/* front */
    if (skyTextures_[FRONT]) skyTextures_[FRONT]->bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBegin(GL_QUADS);
    glNormal3f(0.f, 0.f, 1.f);

    /* lies at the same point as llf */
    glTexCoord2f( 0.f, 0.f );
    glVertex3f( llf.x, llf.y, llf.z );

    glTexCoord2f( 1.f, 0.f );
    glVertex3f( urb.x, llf.y, llf.z );

    glTexCoord2f( 1.f, 1.f );
    glVertex3f( urb.x, urb.y, llf.z );

    glTexCoord2f( 0.f, 1.f );
    glVertex3f( llf.x, urb.y, llf.z );
    glEnd();

    /* bottom */
    if (skyTextures_[BOTTOM]) skyTextures_[BOTTOM]->bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBegin(GL_QUADS);

    glNormal3f(0.f, 1.f, 0.f);
    glTexCoord2f( 0.f , 0.f);
    glVertex3f( llf.x, llf.y, llf.z );

    glTexCoord2f( 1.f, 0.f );
    glVertex3f( urb.x, llf.y, llf.z );

    glTexCoord2f( 1.f, 1.f );
    glVertex3f( urb.x, llf.y, urb.z );

    glTexCoord2f( 0.f, 1.f );
    glVertex3f( llf.x, llf.y, urb.z );
    glEnd();

    /* back */
    if (skyTextures_[BACK]) skyTextures_[BACK]->bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBegin(GL_QUADS);

    glNormal3f(0.f, 0.f, -1.f);
    glTexCoord2f( 0.f, 0.f );
    glVertex3f( urb.x, llf.y, urb.z );

    glTexCoord2f( 1.f, 0.f );
    glVertex3f( llf.x, llf.y, urb.z );

    glTexCoord2f( 1.f, 1.f );
    glVertex3f( llf.x, urb.y, urb.z );

    glTexCoord2f( 0.f, 1.f );
    glVertex3f( urb.x, urb.y, urb.z );

    glEnd();

    /* left */
    if (skyTextures_[LEFT]) skyTextures_[LEFT]->bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBegin(GL_QUADS);
    glNormal3f(1.f, 0.f, 0.f);

    glTexCoord2f( 0.f, 0.f );
    glVertex3f( llf.x, llf.y, urb.z );

    glTexCoord2f( 1.f, 0.f );
    glVertex3f( llf.x, llf.y, llf.z );

    glTexCoord2f( 1.f, 1.f );
    glVertex3f( llf.x, urb.y, llf.z );

    glTexCoord2f( 0.f, 1.f );
    glVertex3f( llf.x, urb.y, urb.z );
    glEnd();

    /* right */
    if (skyTextures_[RIGHT]) skyTextures_[RIGHT]->bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBegin(GL_QUADS);

    glNormal3f( -1.f, 0.f, 0.f );
    glTexCoord2f( 0.f, 0.f );
    glVertex3f(urb.x, llf.y, llf.z );

    glTexCoord2f( 1.f, 0.f );
    glVertex3f( urb.x, llf.y, urb.z );

    glTexCoord2f( 1.f, 1.f );
    glVertex3f( urb.x, urb.y, urb.z );

    glTexCoord2f( 0.f, 1.f );
    glVertex3f( urb.x, urb.y, llf.z );
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

} // namespace
