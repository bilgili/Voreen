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

#ifndef VRN_PLOTFUNCTIONVISITOR_H
#define VRN_PLOTFUNCTIONVISITOR_H

#include "voreen/core/utils/GLSLparser/parsetreenode.h"

#include "voreen/core/plotting/parser/plotfunctionnode.h"
#include "voreen/core/plotting/parser/plotfunctionterminals.h"

namespace voreen {

namespace glslparser {


class PlotFunctionVisitor : public ParseTreeVisitor {
public:
    // represents a function and its domain as vector of Token
    struct TokenVector {
    public:
        // vector of tokens who represents the partial function
        std::vector<Token*> function;
        // vector of tokens who represents the domain of the above function
        std::vector<Token*> interval;
    };

    // constructor
    PlotFunctionVisitor();
    // destructor
    virtual ~PlotFunctionVisitor();
    // see @ ParseTreeVisitor
    virtual bool visit(ParseTreeNode* const node);
    // returns the vector of Tokenvectors. The function is devide in her parts and domains.
    const std::vector<TokenVector>& getPlotFunctionToken(ParseTreeNode* const root);

private:

    struct PartialTree {
    public:
        ParseTreeNode* function;
        ParseTreeNode* interval;
    };

    void visitNode(PlotFunctionNode* const node);
    void visitNode(PlotFunctionTerm* const node);
    void visitNode(PlotFunctionRangeTerm* const node);
    void visitNode(PlotFunctionFunctionTerm* const node);
    void visitNode(PlotFunctionTags* const node);
    void visitNode(PlotFunctionTagName* const node);
    void visitNode(PlotFunctionTagValues* const node);
    void visitNode(PlotFunctionIsolatorTerm* const node);
    void visitNode(PlotFunctionCenterOperation* const node);
    void visitNode(PlotFunctionPreOperation* const node);
    void visitNode(PlotFunctionTermList* const node);

    void devideTree(ParseTreeNode* const root);
    void getPartialTrees(ParseTreeNode* const mainRoot);

    std::vector<Token*> temptokens_;
    std::vector<TokenVector> tokens_;
    std::vector<PartialTree> partialTrees_;

};

}   // namespace glslparser

}   // namespace voreen

#endif
