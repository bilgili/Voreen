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

#include "voreen/core/utils/GLSLparser/glslprogram.h"

#include "voreen/core/utils/GLSLparser/glsl/glslparser.h"
#include "voreen/core/utils/GLSLparser/glsl/glslvisitor.h"
#include "voreen/core/utils/GLSLparser/preprocessor/ppparser.h"
#include "voreen/core/utils/GLSLparser/preprocessor/ppvisitor.h"

namespace voreen {

namespace glslparser {

GLSLProgram::GLSLProgram(const std::string& fileName)
    : fileName_(fileName),
    is_(0),
    shaderHeader_(""),
    log_(std::ios_base::out | std::ios_base::binary)
{
}

GLSLProgram::~GLSLProgram() {
    freeDeclarations();
}

void GLSLProgram::freeDeclarations() {
    for (size_t i = 0; i < uniformDecls_.size(); ++i)
        delete uniformDecls_[i];

    uniformDecls_.clear();
}

// private methods
//

bool GLSLProgram::parse() {
    freeDeclarations();

    PreprocessorVisitor preprocessor;
    preprocessor.setShaderHeader(shaderHeader_);
    std::ostringstream& program = preprocessor.translate(fileName_);

    log_ << "parsing GLSL program in file '" << fileName_ << "'...\n\n";
    log_ << "Preprocessor:\n================\n";
    log_ << preprocessor.getLog().str() << "\n\n";

    /*
    std::ofstream pre("translated.txt", std::ios_base::binary | std::ios_base::out);
    pre << program.str();
    pre.close();
    */

    std::istream prog(program.rdbuf());
    GLSLParser glslParser(&prog);
    //glslParser.setDebugging(true);
    ParseTreeNode* root = glslParser.parse();
    log_ << "GLSL parser:\n================\n";
    log_ << glslParser.getLog().str() << "\n\n";

    bool res = false;
    if (root != 0) {
        res = true;
        GLSLVisitor glsl;
        glsl.visitAll(root);
        uniformDecls_ = glsl.getUniforms(false);
    } else
        std::cout << glslParser.getLog().str();
    delete root;

    return res;
}

}   // namespace glslparser

}   // namespace voreen
