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

#ifndef VRN_TEXUNITMAPPER_H
#define VRN_TEXUNITMAPPER_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <stdexcept>
#include "tgt/types.h"
#include "voreen/core/vis/identifier.h"

namespace voreen {

class TexUnitMapper{
public:
    /**
     * Constructs an empty mapping.
     */
    TexUnitMapper();

    /**
     * Constructs a mapping with one identifier.
     */
    TexUnitMapper(const Identifier& ident);

    /**
     * Constructs the entire mapping with all useable identifiers.
     */
    TexUnitMapper(const std::vector<Identifier>& idents);

    ~TexUnitMapper();

    /**
     * Adds a texUnit to the map.
     */
    bool addTexUnit(const Identifier& ident);

    /**
     * Returns tex unit number mapped to indentifier.
     */
    uint getTexUnit(const Identifier& ident);

    /**
     * Returns OpenGL tex unit mapped to identifier.
     */
    uint getGLTexUnit(const Identifier& ident);

    /**
     * Sets the current system of texture units.
     */
    void registerUnits(const std::vector<Identifier>& idents);

    /**
     * Returns the OpenGL tex unit mapped to the texUnit number
     * use this function in combination with getFreeTexUnit().
	 * Maps numeric values (0...n) to the corresponding OpenGL values (GL_TEXTUREn).
     * @param texUnit texture unit number.
     */
    static uint getGLTexUnitFromInt(uint texUnit);

    /**
     * Sets tex unit number free.
     * Use this function only in combination with getFreeTexUnit().
     */
    void setTexUnitFree(uint texUnit);

    /**
     * Returns the current mapping.
     */
    std::map<Identifier, uint>* getCurrentSystem();

    /**
     * Removes a texture unit from registered units.
     */
    void removeTexUnit(const Identifier& ident);

private:
    /**
     * Saves use status of a GL_TEXTURE unit.
     */
    std::set<uint> texUnits_;

    /**
     * Calculates new mapping.
     */
    void remap();

    /**
     * Returns a free texture unit number (No OpenGL texture unit.)
     * Use this function only for debug purpose.
     * This function makes rendering slower than using the precomputed mapping.
     */
    uint getFreeTexUnit();

    /**
     * Maps identifiers to texture units.
     */
    std::map<Identifier, uint>* registeredUnits_;

    /**
     * Gets the next free texture unit number.
     */
    uint getFreeTexUnitInt();

    /**
     * Exception class for TexUnitMapper.
     */
    struct texUnit_Exception : public std::runtime_error {
        /**
         * Constructor for a texUnit_Exception.
         *
         * @param msg The error message.
         */
        texUnit_Exception(const std::string& msg) : std::runtime_error(msg) {}
    };
};

} //namespace voreen

#endif // VRN_TEXUNITMAPPER_H
