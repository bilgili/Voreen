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

#ifndef VRN_GLSLSTRUCTSPECIFIER_H
#define VRN_GLSLSTRUCTSPECIFIER_H

#include "voreen/core/utils/GLSLparser/glsl/glsldeclaration.h"

namespace voreen {

namespace glslparser {

class GLSLStructSpecifier : public GLSLTypeSpecifier {
public:
    GLSLStructSpecifier(IdentifierToken* const id, GLSLStructDeclaratorList* const decls)
        : GLSLTypeSpecifier(Token(GLSLTerminals::ID_STRUCT), 0, false, 0),
        identifier_(dynamic_cast<IdentifierToken*>(id->getCopy())),
        structDecls_(decls)
    {
    }

    virtual ~GLSLStructSpecifier() {
        delete identifier_;
        delete structDecls_;
    }

    virtual int getNodeType() const { return GLSLNodeTypes::NODE_STRUCT_SPECIFIER; }

    IdentifierToken* getStructName() const { return identifier_; }

protected:
    IdentifierToken* const identifier_;  // name of the struct
    GLSLStructDeclaratorList* const structDecls_;   // member declarations of the struct
};

}   // namespace glslparser

}   // namespace voreen

#endif
