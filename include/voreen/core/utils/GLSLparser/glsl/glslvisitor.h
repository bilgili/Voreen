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

#ifndef VRN_GLSLVISITOR_H
#define VRN_GLSLVISITOR_H

#include "voreen/core/utils/GLSLparser/symboltable.h"
#include "voreen/core/utils/GLSLparser/glsl/glsltoken.h"
#include "voreen/core/utils/GLSLparser/glsl/glslstructspecifier.h"
#include "voreen/core/utils/GLSLparser/glsl/glslsymbol.h"

#include <sstream>

namespace voreen {

namespace glslparser {

class GLSLVisitor : public ParseTreeVisitor {
public:
    GLSLVisitor();
    virtual ~GLSLVisitor();

    virtual bool visit(ParseTreeNode* const node);

    void printSymbolTable() const;

    /**
     * Returns all declarations of uniform symbols within this GLSL program.
     *
     * The returned symbols are no long kept within the symbol table if the
     * parameter keepInTable is set to true and the caller must delete the
     * returned pointers using C++ delete operator in order to prevent memory
     * leaks.
     * If keepInTable is false, the pointers are kept within the table, but
     * become deleted invalid as soon as the dtor of GLSLVistitor is called,
     * for it deletes its symbol table and all contained symbols.
     */
    std::vector<GLSLVariableSymbol*> getUniforms(const bool keepInTable);

private:
    // Expressions
    //

    // Declarations
    //
    void visitNode(GLSLDeclaration* const decl);
    void visitNode(GLSLDeclarationList* const decls);
    //void visitNode(GLSLStructDeclaratorList* const decls);
    //void visitNode(GLSLFieldDeclaration* const decl);
    //void visitNode(GLSLFunctionDeclaration* const decl);

    // Qualifiers
    //
    GLSLVariableSymbol::InterpolationQualifier visitNode(GLSLInterpolationQualifier* const interpolation);
    GLSLSymbol::PrecisionQualifier visitNode(GLSLPrecisionQualifier* const precision);
    GLSLVariableSymbol::StorageQualifier visitNode(GLSLStorageQualifier* const storage);

    // Specifiers
    //
    GLSLVariableSymbol visitNode(GLSLTypeSpecifier* const typeSpec);

    std::vector<GLSLAnnotation*> processAnnotation(AnnotationToken* const annotation);

protected:
    typedef SymbolTable<GLSLSymbol> GLSLSymbolMap;
    GLSLSymbolMap symbols_;
};

}   // namespace glslparser

}   // namespace voreen

#endif
