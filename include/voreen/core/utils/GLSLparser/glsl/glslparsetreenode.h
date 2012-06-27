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

#ifndef VRN_GLSLPARSETREENODE_H
#define VRN_GLSLPARSETREENODE_H

#include "voreen/core/utils/GLSLparser/parsetreenode.h"

#include "voreen/core/utils/GLSLparser/glsl/glslterminals.h"
#include "voreen/core/utils/GLSLparser/glsl/glsltoken.h"

namespace voreen {

namespace glslparser {

struct GLSLNodeTypes {
    enum {
        NODE_GLSLNODE,
            NODE_VARIABLE,
            NODE_PARAMETER,
            NODE_DECLARATION,
                NODE_DECLARATION_LIST,
                NODE_STRUCT_DECLARATION,
                NODE_FIELD_DECLARATION,
                NODE_FUNCTION_DECLARATION,
            NODE_QUALIFIER,
                NODE_STORAGE_QUALIFIER,
                NODE_PRECISION_QUALIFIER,
                NODE_INTERPOLATION_QUALIFIER,
                NODE_PARAMETER_QUALIFIER,
                NODE_LAYOUT_QUALIFIER,
                NODE_TYPE_QUALIFIER,
            NODE_TYPE_SPECIFIER,
            NODE_STRUCT_SPECIFIER,
            NODE_EXPRESSION,
                NODE_ARRAY_OPERATION,
                NODE_ASSIGNMENT_EXPRESSION,
                NODE_BINARY_EXPRESSION,
                NODE_CONDITIONAL_EXPRESSION,
                NODE_EXPRESSION_LIST,
                NODE_FIELD_SELECTION,
                NODE_FUNCTION_CALL,
                NODE_PARENTHESIS_EXPRESSION,
                NODE_POSTFIX_OPERATION,
                NODE_UNARY_EXPRESSION,
            NODE_STATEMENT
    };
};

// ============================================================================

class GLSLNode : public ParseTreeNode {
public:
    GLSLNode(const int symbolID)
        : ParseTreeNode(symbolID)
    {
    }
};

}   // namespace glslparser

}   // namespace voreen

#endif
