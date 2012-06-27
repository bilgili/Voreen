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

#ifndef VRN_GLSLEXPRESSION_H
#define VRN_GLSLEXPRESSION_H

#include "voreen/core/utils/GLSLparser/glsl/glslparsetreenode.h"

namespace voreen {

namespace glslparser {

class GLSLExpression : public GLSLNode {
public:
    GLSLExpression(const Token& token)
        : GLSLNode(token.getTokenID()),
        token_(token.getCopy())
    {
    }

    virtual ~GLSLExpression() {
        delete token_;
    }

    virtual int getNodeType() const { return GLSLNodeTypes::NODE_EXPRESSION; }

    Token* getToken() const { return token_; }

protected:
    Token* const token_;
};

// ============================================================================

class GLSLParenthesisExpression : public GLSLExpression {
public:
    GLSLParenthesisExpression(GLSLExpression* const expr)
        : GLSLExpression(Token(GLSLTerminals::ID_LPAREN)),
        expr_(expr)
    {
    }

    virtual ~GLSLParenthesisExpression() {
        delete expr_;
    }

    virtual int getNodeType() const { return GLSLNodeTypes::NODE_PARENTHESIS_EXPRESSION; }

protected:
    GLSLExpression* const expr_;
};

// ============================================================================

class GLSLPostfixOperation : public GLSLExpression {
public:
    GLSLPostfixOperation(Token* const token, GLSLExpression* const operand)
        : GLSLExpression(*token),
        operand_(operand)
    {
    }

    virtual ~GLSLPostfixOperation() {
        delete operand_;
    }

    virtual int getNodeType() const { return GLSLNodeTypes::NODE_POSTFIX_OPERATION; }

protected:
    GLSLExpression* const operand_;
};

// ============================================================================

class GLSLArrayOperation : public GLSLExpression {
public:
    GLSLArrayOperation(GLSLExpression* const arr, GLSLExpression* const intExpr)
        : GLSLExpression(Token(GLSLTerminals::ID_LBRACKET)),
        arr_(arr),
        intExpression_(intExpr)
    {
    }

    virtual ~GLSLArrayOperation() {
        delete arr_;
        delete intExpression_;
    }

    virtual int getNodeType() const { return GLSLNodeTypes::NODE_ARRAY_OPERATION; }

protected:
    GLSLExpression* const arr_;
    GLSLExpression* const intExpression_;
};

// ============================================================================

class GLSLFieldSelection : public GLSLExpression {
public:
    GLSLFieldSelection(GLSLExpression* const operand, IdentifierToken* const field)
        : GLSLExpression(Token(GLSLTerminals::ID_DOT)),
        operand_(operand),
        field_(dynamic_cast<IdentifierToken* const>(field->getCopy()))
    {
    }

    virtual ~GLSLFieldSelection() {
        delete operand_;
        delete field_;
    }

    virtual int getNodeType() const { return GLSLNodeTypes::NODE_FIELD_SELECTION; }

protected:
    GLSLExpression* const operand_;
    IdentifierToken* const field_;
};

// ============================================================================

class GLSLUnaryExpression : public GLSLExpression {
public:
    GLSLUnaryExpression(GLSLExpression* const expr)
        : GLSLExpression(*expr),
        expression_(0)
    {
    }

    GLSLUnaryExpression(Token* const token, GLSLExpression* const expression)
        : GLSLExpression(*token),
        expression_(expression)
    {
    }

    virtual ~GLSLUnaryExpression() {
        delete expression_;
    }

    virtual int getNodeType() const { return GLSLNodeTypes::NODE_UNARY_EXPRESSION; }

protected:
    GLSLExpression* const expression_;
};

// ============================================================================

class GLSLBinaryExpression : public GLSLExpression {
public:
    GLSLBinaryExpression(Token* const token, GLSLExpression* const lhs, GLSLExpression* const rhs)
        : GLSLExpression(*token),
        lhs_(lhs),
        rhs_(rhs)
    {
    }

    virtual ~GLSLBinaryExpression() {
        delete lhs_;
        delete rhs_;
    }

    virtual int getNodeType() const { return GLSLNodeTypes::NODE_BINARY_EXPRESSION; }

protected:
    GLSLExpression* const lhs_;
    GLSLExpression* const rhs_;
};

// ============================================================================

class GLSLAssignmentExpression : public GLSLExpression {
public:
    GLSLAssignmentExpression(Token* const token, GLSLUnaryExpression* const lvalue,
        GLSLExpression* const rvalue)
        : GLSLExpression(*token),
        lvalue_(lvalue),
        rvalue_(rvalue)
    {
    }

    virtual ~GLSLAssignmentExpression() {
        delete lvalue_;
        delete rvalue_;
    }

    virtual int getNodeType() const { return GLSLNodeTypes::NODE_ASSIGNMENT_EXPRESSION; }

protected:
    GLSLUnaryExpression* const lvalue_;
    GLSLExpression* const rvalue_;
};

// ============================================================================

class GLSLConditionalExpression : public GLSLExpression {
public:
    GLSLConditionalExpression(GLSLExpression* const condExpr,
        GLSLExpression* const trueExpr, GLSLExpression* const falseExpr)
        : GLSLExpression(Token(GLSLTerminals::ID_QUESTION)),
        condition_(condExpr),
        trueExpression_(trueExpr),
        falseExpression_(falseExpr)
    {
    }

    virtual ~GLSLConditionalExpression() {
        delete condition_;
        delete trueExpression_;
        delete falseExpression_;
    }

    virtual int getNodeType() const { return GLSLNodeTypes::NODE_CONDITIONAL_EXPRESSION; }

protected:
    GLSLExpression* const condition_;
    GLSLExpression* const trueExpression_;
    GLSLExpression* const falseExpression_;
};

// ============================================================================

class GLSLExpressionList : public GLSLExpression {
public:
    GLSLExpressionList(GLSLExpression* const expr)
        : GLSLExpression(*(expr->getToken()))
    {
        addExpression(expr);
    }

    virtual ~GLSLExpressionList() {
        for (size_t i = 0; i < expressions_.size(); ++i)
            delete expressions_[i];
    }

    virtual int getNodeType() const { return GLSLNodeTypes::NODE_EXPRESSION_LIST; }

    void addExpression(GLSLExpression* const expr) {
        if (expr != 0)
            expressions_.push_back(expr);
    }

protected:
    std::vector<GLSLExpression*> expressions_;
};

}   // namespace glslparser

}   // namespace voreen

#endif
