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

#include "voreen/core/utils/GLSLparser/parsetreenode.h"

#include <stack>

namespace voreen {

namespace glslparser {

void ParseTreeVisitor::visitAll(ParseTreeNode* const root) {
    if (root == 0)
        return;

    std::stack<ParseTreeNode*> tree;
    tree.push(root);
    while (! tree.empty()) {
        ParseTreeNode* const top = tree.top();
        tree.pop();
        if (top == 0)   // should never happen
            continue;

        visit(top);

        // Push the children in reversed order onto the stack to keep their
        // order.
        //
        const std::vector<ParseTreeNode*>& children = top->getChildren();
        for (size_t j = 0; j < children.size(); ++j)
            tree.push(children[children.size() - 1 - j]);
    }

}

// ============================================================================

ParseTreeNode::ParseTreeNode(const int symbolID)
    : symbolID_(symbolID)
{
}

ParseTreeNode::~ParseTreeNode() {
    for (size_t i = 0; i < children_.size(); ++i)
        delete children_[i];
}

void ParseTreeNode::addChild(ParseTreeNode* const child) {
    if (child != 0)
        children_.push_back(child);
}

}   // namespace glslparser

}   // namespace voreen
