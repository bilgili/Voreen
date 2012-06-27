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

#include "voreen/core/plotting/plotfunctionnode.h"

//#include "voreen/core/plotting/plotfunctionterminals.h"

namespace voreen {

namespace glslparser {


PlotFunctionNode::PlotFunctionNode(const int symbolID)
    :ParseTreeNode(symbolID)
{}

int PlotFunctionNode::getNodeType() const
{
    return PlotFunctionNodeTypes::NODE_PLOTFUNCTION;
}

// ============================================================================

PlotFunctionTagName::PlotFunctionTagName(IdentifierToken* const name)
    : PlotFunctionNode(name->getTokenID()),
    name_(dynamic_cast<IdentifierToken* const>(name->getCopy()))
{}

PlotFunctionTagName::~PlotFunctionTagName() {
    delete name_;
    for (size_t i = 0; i < list_.size(); ++i)
        delete list_[i];
}

int PlotFunctionTagName::getNodeType() const {
    return PlotFunctionNodeTypes::NODE_PLOTFUNCTION_TAG_NAME;
}

void PlotFunctionTagName::addTagName(PlotFunctionTagName* const tagName) {
    if (tagName != 0)
        list_.push_back(tagName);
}

IdentifierToken* PlotFunctionTagName::getName() const {
    return name_;
}

const std::vector<PlotFunctionTagName*>& PlotFunctionTagName::getMoreNames() const {
    return list_;
}

// ============================================================================

PlotFunctionTagValues::PlotFunctionTagValues(Token* const token)
    : PlotFunctionNode(token->getTokenID())
{
    addValue(token);
}

PlotFunctionTagValues::~PlotFunctionTagValues() {
    for (size_t i = 0; i < values_.size(); ++i)
        delete values_[i];
}

int PlotFunctionTagValues::getNodeType() const {
    return PlotFunctionNodeTypes::NODE_PLOTFUNCTION_TAG_VALUES;
}

void PlotFunctionTagValues::addValue(Token* const token) {
    if (token != 0)
        values_.push_back(token->getCopy());
}

void PlotFunctionTagValues::merge(PlotFunctionTagValues* const other) {
    if (other != 0) {
        values_.insert(values_.end(), other->values_.begin(), other->values_.end());
        other->values_.clear();
    }
}

const std::vector<Token*>& PlotFunctionTagValues::getTokens() const {
    return values_;
}

// ============================================================================

PlotFunctionTags::PlotFunctionTags(PlotFunctionTagName* const tagName, PlotFunctionTagValues* const tagValues)
    : PlotFunctionNode(PlotFunctionTerminals::ID_FUNCTION)
{
    addTag(tagName, tagValues);
}

PlotFunctionTags::~PlotFunctionTags() {
    for (size_t i = 0; i < tags_.size(); ++i) {
        delete tags_[i].first;
        delete tags_[i].second;
    }
}

int PlotFunctionTags::getNodeType() const {
    return PlotFunctionNodeTypes::NODE_PLOTFUNCTION_TAGS;
}

void PlotFunctionTags::addTag(PlotFunctionTagName* const tagName, PlotFunctionTagValues* const tagValues) {
    if ((tagName != 0) && (tagValues != 0))
        tags_.push_back(std::make_pair(tagName, tagValues));
}

void PlotFunctionTags::merge(PlotFunctionTags* const other) {
    if (other != 0) {
        tags_.insert(tags_.end(), other->tags_.begin(), other->tags_.end());
        other->tags_.clear();
    }
}

const std::vector<std::pair<PlotFunctionTagName*, PlotFunctionTagValues*> >& PlotFunctionTags::getTags() const {
    return tags_;
}



}   // namespace glslparser
}   // namespace voreen
