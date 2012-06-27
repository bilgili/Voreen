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

#ifndef TGT_SKYBOX_H
#define TGT_SKYBOX_H

#include <string>

#include "tgt/config.h"
#include "tgt/types.h"
#include "tgt/texture.h"
#include "tgt/renderable.h"
#include "tgt/bounds.h"

/* Skybox class */

namespace tgt {

/**
 * Rough Skybox implementation without checking texture sizes.
 * The class serves as a simple container for sky textures in the 6 different
 * directions.
 */
class Skybox : public Renderable {
private:

    union {
        struct {
            Texture *skyTop_;
            Texture *skyBottom_;
            Texture *skyLeft_;
            Texture *skyRight_;
            Texture *skyBack_;
            Texture *skyFront_;
        };
        Texture *skyTextures_[6];
    };

public:
    enum {
        TOP,
        BOTTOM,
        LEFT,
        RIGHT,
        BACK,
        FRONT
    };

    /**
        * the constructor receives the 6 texture names and loads them into the
        * texture manager.
        * @param texNames 6 String array of texture names.
        */
    Skybox(const std::string texNames[6], const Bounds& bounds);

    /**
    * dispose of sky textures.
    */
    ~Skybox();

    void init() {}
    void deinit() {}

    void render();
};

}

#endif //TGT_SKYBOX_H
