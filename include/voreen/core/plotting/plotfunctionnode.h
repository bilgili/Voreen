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

#ifndef VRN_PLOTFUNCTIONNODE_H
#define VRN_PLOTFUNCTIONNODE_H

#include "voreen/core/utils/GLSLparser/parsetreenode.h"
#include "voreen/core/plotting/plotfunctionterminals.h"


namespace voreen {

namespace glslparser {



struct PlotFunctionNodeTypes {
    enum {
        NODE_PLOTFUNCTION,
        NODE_PLOTFUNCTION_TAGS,
        NODE_PLOTFUNCTION_TAG_NAME,
        NODE_PLOTFUNCTION_TAG_VALUES
    };
};

// ============================================================================

class PlotFunctionNode : public ParseTreeNode {
public:
    PlotFunctionNode(const int symbolID);

    virtual int getNodeType() const;
};

// ============================================================================

class PlotFunctionTagName : public PlotFunctionNode {
public:
    PlotFunctionTagName(IdentifierToken* const name);

    virtual ~PlotFunctionTagName();

    virtual int getNodeType() const;

    void addTagName(PlotFunctionTagName* const tagName);

    IdentifierToken* getName() const;

    const std::vector<PlotFunctionTagName*>& getMoreNames() const;

protected:
    IdentifierToken* const name_;
    std::vector<PlotFunctionTagName*> list_;
};

// ============================================================================

class PlotFunctionTagValues : public PlotFunctionNode {
public:
    PlotFunctionTagValues(Token* const token);

    virtual ~PlotFunctionTagValues();

    virtual int getNodeType() const;

    void addValue(Token* const token);

    void merge(PlotFunctionTagValues* const other);

    const std::vector<Token*>& getTokens() const;

protected:
    std::vector<Token*> values_;
};

// ============================================================================

class PlotFunctionTags : public PlotFunctionNode {
public:
    PlotFunctionTags(PlotFunctionTagName* const tagName, PlotFunctionTagValues* const tagValues);

    virtual ~PlotFunctionTags();

    virtual int getNodeType() const;

    void addTag(PlotFunctionTagName* const tagName, PlotFunctionTagValues* const tagValues);

    void merge(PlotFunctionTags* const other);

    const std::vector<std::pair<PlotFunctionTagName*, PlotFunctionTagValues*> >& getTags() const;

protected:
    typedef std::pair<PlotFunctionTagName*, PlotFunctionTagValues*> Tag;
    std::vector<Tag> tags_;
};

}   // namespace glslparser

}   // namespace voreen

#endif
