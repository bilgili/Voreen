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

#include "voreen/core/utils/GLSLparser/glsl/glslvisitor.h"

#include "voreen/core/utils/GLSLparser/annotations/annotationparser.h"
#include "voreen/core/utils/GLSLparser/annotations/annotationvisitor.h"

namespace voreen {

namespace glslparser {

GLSLVisitor::GLSLVisitor()
    : ParseTreeVisitor()
{
}

GLSLVisitor::~GLSLVisitor() {
}

void GLSLVisitor::printSymbolTable() const {
    typedef std::map<std::string, GLSLSymbol*> SymbolMap;
    const SymbolMap& symbols = symbols_.getSymbolsMap();

    for (SymbolMap::const_iterator it = symbols.begin(); it != symbols.end(); ++it) {
        std::cout << "name = '" << it->first << "', type = '" << it->second->getInternalType() << "'";
        std::cout << ", #elements = " << it->second->getNumInternalElements() << "\n";
    }
}

std::vector<GLSLVariableSymbol*> GLSLVisitor::getUniforms(const bool keepInTable) {
    typedef std::map<std::string, GLSLSymbol*> SymbolMap;
    const SymbolMap& symbols = symbols_.getSymbolsMap();

    std::vector<GLSLVariableSymbol*> uniforms;
    for (SymbolMap::const_iterator it = symbols.begin(); it != symbols.end(); ) {
        GLSLVariableSymbol* const varSymbol = dynamic_cast<GLSLVariableSymbol* const>(it->second);
        if (varSymbol == 0)
            continue;

        if (varSymbol->getStorageQualifier() == GLSLVariableSymbol::SQ_UNIFORM) {
            uniforms.push_back(varSymbol);
            if (keepInTable)
                ++it;
            else
                symbols_.removeSymbol((it++)->second);

        } else
            ++it;
    }

    return uniforms;
}

std::vector<GLSLVariableSymbol*> GLSLVisitor::getOuts(const bool keepInTable) {
    typedef std::map<std::string, GLSLSymbol*> SymbolMap;
    const SymbolMap& symbols = symbols_.getSymbolsMap();

    std::vector<GLSLVariableSymbol*> outs;
    for (SymbolMap::const_iterator it = symbols.begin(); it != symbols.end(); ) {
        GLSLVariableSymbol* const varSymbol = dynamic_cast<GLSLVariableSymbol* const>(it->second);
        if (varSymbol == 0)
            continue;

        if (varSymbol->getStorageQualifier() == GLSLVariableSymbol::SQ_OUT) {
            outs.push_back(varSymbol);
            if (keepInTable)
                ++it;
            else
                symbols_.removeSymbol((it++)->second);

        } else
            ++it;
    }

    return outs;
}

bool GLSLVisitor::visit(ParseTreeNode* const node) {
    if (node == 0)
        return false;

    bool result = true;
    switch (node->getNodeType()) {
        case GLSLNodeTypes::NODE_DECLARATION:
            visitNode(dynamic_cast<GLSLDeclaration* const>(node));
            break;

        case GLSLNodeTypes::NODE_DECLARATION_LIST:
            visitNode(dynamic_cast<GLSLDeclarationList* const>(node));
            break;
    }

    return result;
}

// protected methods
//

// Delcarations
//

void GLSLVisitor::visitNode(GLSLDeclaration* const decl) {
    if (decl == 0)
        return;
}

void GLSLVisitor::visitNode(GLSLDeclarationList* const decls) {
    if (decls == 0)
        return;

    GLSLTypeSpecifier* const typeSpec = decls->getTypeSpecifier();
    if (typeSpec == 0)
        throw std::runtime_error("visitNode(GLSLDeclarationList): given declarations are not specified!");

    try {
        GLSLVariableSymbol metaSymbol = visitNode(typeSpec);
        const std::vector<GLSLVariable*>& vars = decls->getVariables();

        for (size_t i = 0; i < vars.size(); ++i) {
            GLSLVariableSymbol* const symbol = new GLSLVariableSymbol(metaSymbol);

            symbol->setID(vars[i]->getName());
            symbol->setIsArray(vars[i]->isArray());
            int numArrayElements = 0;   // TODO: evaluate expression from vars[i] to determine array size
            symbol->setNumArrayElements(numArrayElements);

            std::vector<GLSLAnnotation*> leading = processAnnotation(decls->getLeadingAnnotation());
            std::vector<GLSLAnnotation*> trailing = processAnnotation(decls->getTrailingAnnotation());

            leading.insert(leading.end(), trailing.begin(), trailing.end());
            symbol->setAnnotations(leading);

            symbols_.insertSymbol(symbol);
        }
    } catch (std::runtime_error& e) {
        log_ << "  Exeption: " << e.what() << "\n";
    }
}

// Qualifiers
//

GLSLVariableSymbol::InterpolationQualifier GLSLVisitor::visitNode(
    GLSLInterpolationQualifier* const interpolation)
{
    if (interpolation == 0)
        return GLSLVariableSymbol::IQ_NONE;

    Token* const token = interpolation->getToken();
    if (token != 0) {
        switch (token->getTokenID()) {
            case GLSLTerminals::ID_FLAT:
                return GLSLVariableSymbol::IQ_FLAT;

            case GLSLTerminals::ID_SMOOTH:
                return GLSLVariableSymbol::IQ_SMOOTH;

            case GLSLTerminals::ID_NONPERSPECTIVE:
                return GLSLVariableSymbol::IQ_NOPERSPECTIVE;
        }
    }

    return GLSLVariableSymbol::IQ_NONE;
}

GLSLSymbol::PrecisionQualifier GLSLVisitor::visitNode(GLSLPrecisionQualifier* const precision) {
    if (precision == 0)
        return GLSLSymbol::PRECQ_NONE;

    Token* const token = precision->getToken();
    if (token != 0) {
        switch (token->getTokenID()) {
            case GLSLTerminals::ID_HIGHP:
                return GLSLSymbol::PRECQ_HIGH;

            case GLSLTerminals::ID_MEDIUMP:
                return GLSLSymbol::PRECQ_MEDIUM;

            case GLSLTerminals::ID_LOWP:
                return GLSLSymbol::PRECQ_LOW;
        }
    }

    return GLSLSymbol::PRECQ_NONE;
}

GLSLVariableSymbol::StorageQualifier GLSLVisitor::visitNode(GLSLStorageQualifier* const storage) {
    if (storage == 0)
        return GLSLVariableSymbol::SQ_NONE;

    Token* const token = storage->getToken();
    if (token != 0) {
        switch (token->getTokenID()) {
            case GLSLTerminals::ID_ATTRIBUTE:
                return GLSLVariableSymbol::SQ_ATTRIBUTE;

            case GLSLTerminals::ID_CONST:
                return GLSLVariableSymbol::SQ_CONST;

            case GLSLTerminals::ID_VARYING:
                return ((storage->hasCentroid()) ? GLSLVariableSymbol::SQ_CENTROID_VARYING
                    : GLSLVariableSymbol::SQ_VARYING);

            case GLSLTerminals::ID_IN:
                return ((storage->hasCentroid()) ? GLSLVariableSymbol::SQ_CENTROID_IN
                    : GLSLVariableSymbol::SQ_IN);

            case GLSLTerminals::ID_OUT:
                return ((storage->hasCentroid()) ? GLSLVariableSymbol::SQ_CENTROID_OUT
                    : GLSLVariableSymbol::SQ_OUT);

            case GLSLTerminals::ID_UNIFORM:
                return GLSLVariableSymbol::SQ_UNIFORM;
        }
    }

    return GLSLVariableSymbol::SQ_NONE;
}

// Specifiers
//

GLSLVariableSymbol GLSLVisitor::visitNode(GLSLTypeSpecifier* const typeSpec) {
    if (typeSpec == 0)
        throw std::runtime_error("visitNode(GLSLTypeSpecifier): given specifier is NULL!");

    Token* const token = typeSpec->getTypeToken();
    if (token == 0)
        throw std::runtime_error("visitNode(GLSLTypeSpecifier): specifier does not specify a type!");

    // Determine internal type of the declaration
    //
    GLSLSymbol::InternalType type = GLSLSymbol::INTERNAL_VOID;
    switch (token->getTokenID()) {
        case GLSLTerminals::ID_VOID:
            break;

        case GLSLTerminals::ID_STRUCT:
            type = GLSLSymbol::INTERNAL_STRUCT;
            break;

        case GLSLTerminals::ID_BOOL:
        case GLSLTerminals::ID_BVEC2:
        case GLSLTerminals::ID_BVEC3:
        case GLSLTerminals::ID_BVEC4:
            type = GLSLSymbol::INTERNAL_BOOL;
            break;

        case GLSLTerminals::ID_FLOAT:
        case GLSLTerminals::ID_VEC2:
        case GLSLTerminals::ID_VEC3:
        case GLSLTerminals::ID_VEC4:
        case GLSLTerminals::ID_MAT2:
        case GLSLTerminals::ID_MAT3:
        case GLSLTerminals::ID_MAT4:
        case GLSLTerminals::ID_MAT2X2:
        case GLSLTerminals::ID_MAT2X3:
        case GLSLTerminals::ID_MAT2X4:
        case GLSLTerminals::ID_MAT3X2:
        case GLSLTerminals::ID_MAT3X3:
        case GLSLTerminals::ID_MAT3X4:
        case GLSLTerminals::ID_MAT4X2:
        case GLSLTerminals::ID_MAT4X3:
        case GLSLTerminals::ID_MAT4X4:
            type = GLSLSymbol::INTERNAL_FLOAT;
            break;

        case GLSLTerminals::ID_INT:
        case GLSLTerminals::ID_IVEC2:
        case GLSLTerminals::ID_IVEC3:
        case GLSLTerminals::ID_IVEC4:
            type = GLSLSymbol::INTERNAL_INT;
            break;

        case GLSLTerminals::ID_UINT:
        case GLSLTerminals::ID_UVEC2:
        case GLSLTerminals::ID_UVEC3:
        case GLSLTerminals::ID_UVEC4:
            type = GLSLSymbol::INTERNAL_UINT;
            break;

        default:
            if ((token->getTokenID() >= GLSLTerminals::ID_SAMPLER1D)
                && (token->getTokenID() <= GLSLTerminals::ID_USAMPLER2DMSARRAY))
            {
                type = GLSLSymbol::INTERNAL_SAMPLER;
            }
            break;
    }

    // Determine number of elements of internal type
    //
    int numElements = 1;
    switch (token->getTokenID()) {
        case GLSLTerminals::ID_STRUCT:
            numElements = 0;
            break;

        case GLSLTerminals::ID_BVEC2:
        case GLSLTerminals::ID_VEC2:
        case GLSLTerminals::ID_IVEC2:
        case GLSLTerminals::ID_UVEC2:
            numElements = 2;
            break;

        case GLSLTerminals::ID_BVEC3:
        case GLSLTerminals::ID_VEC3:
        case GLSLTerminals::ID_IVEC3:
        case GLSLTerminals::ID_UVEC3:
            numElements = 3;
            break;

        case GLSLTerminals::ID_BVEC4:
        case GLSLTerminals::ID_VEC4:
        case GLSLTerminals::ID_IVEC4:
        case GLSLTerminals::ID_UVEC4:
        case GLSLTerminals::ID_MAT2:
        case GLSLTerminals::ID_MAT2X2:
            numElements = 4;
            break;

        case GLSLTerminals::ID_MAT2X3:
        case GLSLTerminals::ID_MAT3X2:
            numElements = 6;
            break;

        case GLSLTerminals::ID_MAT2X4:
        case GLSLTerminals::ID_MAT4X2:
            numElements = 8;
            break;

        case GLSLTerminals::ID_MAT3:
        case GLSLTerminals::ID_MAT3X3:
            numElements = 9;
            break;

        case GLSLTerminals::ID_MAT3X4:
        case GLSLTerminals::ID_MAT4X3:
            numElements = 12;
            break;

        case GLSLTerminals::ID_MAT4:
        case GLSLTerminals::ID_MAT4X4:
            numElements = 16;
            break;

        case GLSLTerminals::ID_SAMPLER1D:
        case GLSLTerminals::ID_SAMPLER1DSHADOW:
        case GLSLTerminals::ID_SAMPLER1DARRAY:
        case GLSLTerminals::ID_SAMPLER1DARRAYSHADOW:
        case GLSLTerminals::ID_ISAMPLER1D:
        case GLSLTerminals::ID_ISAMPLER1DARRAY:
        case GLSLTerminals::ID_USAMPLER1D:
        case GLSLTerminals::ID_USAMPLER1DARRAY:
            numElements = 1;
            break;

        case GLSLTerminals::ID_SAMPLER2D:
        case GLSLTerminals::ID_SAMPLER2DSHADOW:
        case GLSLTerminals::ID_SAMPLER2DARRAY:
        case GLSLTerminals::ID_SAMPLER2DARRAYSHADOW:
        case GLSLTerminals::ID_ISAMPLER2D:
        case GLSLTerminals::ID_ISAMPLER2DARRAY:
        case GLSLTerminals::ID_USAMPLER2D:
        case GLSLTerminals::ID_USAMPLER2DARRAY:
        case GLSLTerminals::ID_SAMPLER2DRECT:
        case GLSLTerminals::ID_SAMPLER2DRECTSHADOW:
        case GLSLTerminals::ID_ISAMPLER2DRECT:
        case GLSLTerminals::ID_USAMPLER2DRECT:
        case GLSLTerminals::ID_SAMPLER2DMS:
        case GLSLTerminals::ID_ISAMPLER2DMS:
        case GLSLTerminals::ID_USAMPLER2DMS:
        case GLSLTerminals::ID_SAMPLER2DMSARRY:
        case GLSLTerminals::ID_ISAMPLER2DMSARRAY:
        case GLSLTerminals::ID_USAMPLER2DMSARRAY:
            numElements = 2;
            break;

        case GLSLTerminals::ID_SAMPLER3D:
        case GLSLTerminals::ID_ISAMPLER3D:
        case GLSLTerminals::ID_USAMPLER3D:
            numElements = 3;
            break;

        case GLSLTerminals::ID_SAMPLERCUBE:
        case GLSLTerminals::ID_ISAMPLERCUBE:
        case GLSLTerminals::ID_USAMPLERCUBE:
            numElements = 6;
            break;

        case GLSLTerminals::ID_SAMPLERBUFFER:
        case GLSLTerminals::ID_ISAMPLERBUFFER:
        case GLSLTerminals::ID_USAMPLEBUFFER:
            numElements = 0;
            break;
    }

    GLSLVariableSymbol metaSymbol("", type, numElements);

    GLSLTypeQualifier* const typeQuali = typeSpec->getTypeQualifier();
    if (typeQuali != 0) {
        GLSLVariableSymbol::InterpolationQualifier iq = visitNode(typeQuali->getInterpolationQualifier());
        GLSLVariableSymbol::StorageQualifier sq = visitNode(typeQuali->getStorageQualifier());

        metaSymbol.setInterpolation(iq);
        metaSymbol.setStorage(sq);
    }

    GLSLSymbol::PrecisionQualifier pq = visitNode(typeSpec->getPrecisionQualifier());
    metaSymbol.setPrecision(pq);

    return metaSymbol;
}

std::vector<GLSLAnnotation*> GLSLVisitor::processAnnotation(AnnotationToken* const annotation)
{
    std::vector<GLSLAnnotation*> annotations;

    if (annotation != 0) {
        AnnotationParser annotationParser(annotation->getParts());
        //annotationParser.setDebugging(true);

        ParseTreeNode* const root = annotationParser.parse();
        if (root != 0) {
            AnnotationVisitor av;
            annotations = av.getAnnotations(root);
            delete root;
        } else {
            log_ << "Annotation parser log:\n------------------------\n";
            log_ << annotationParser.getLog().str() << "\n";
        }
    }

    return annotations;
}

}   // namespace glslparser

}   // namespace voreen
