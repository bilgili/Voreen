/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

/*
    TODO
    - Why reinvent the wheel with this setType and getType stuff?
    Use typeid and dynamic_casts instead.

    - Furthermore large chunks of this file is copy&paste code. One Method
    with this functionallity is _MUCH_ easier to maintain (and faster, less buggier, ...).
*/

#include "voreen/core/vis/networkanalyzer.h"

/* Property-Helper
* Analyzes the Pipeline and configures all properties properly
*/

namespace voreen {

const std::string NetworkAnalyzer::loggerCat_("voreen.core.vis.NetworkAnalyzer");

std::pair<std::vector<ConditionProp*>, std::vector<GroupProp*> > NetworkAnalyzer::findCondProps(std::vector<Processor*> rend) {
    std::vector<ConditionProp*> ret;
    std::vector<GroupProp*> ret2;
    for (size_t i=0; i<rend.size(); ++i) {
        const Properties& props_tmp = rend.at(i)->getProperties();
        for (size_t j=0; j<props_tmp.size(); ++j) {
            if (props_tmp.at(j)->getType() == Property::CONDITION_PROP)
                ret.push_back(dynamic_cast<ConditionProp*>(props_tmp.at(j)));
            else if (props_tmp.at(j)->getType() == Property::GROUP_PROP)
                ret2.push_back(dynamic_cast<GroupProp*>(props_tmp.at(j)));
        }
    }
    std::pair<std::vector<ConditionProp*>, std::vector<GroupProp*> > right_ret;
    right_ret.first = ret;
    right_ret.second = ret2;
    return right_ret;
}

void NetworkAnalyzer::associateCondProps(std::vector<ConditionProp*> cond_props, Property* prop) {
    Identifier ident = prop->getCondControllerIdent();
    Identifier destIdent = prop->getMsgDestination();
    for (size_t k=0; k< cond_props.size(); ++k) {
        if (cond_props.at(k)->getCondIdent() == ident && cond_props.at(k)->getCondControllerProp()->getMsgDestination() == destIdent) {
            cond_props.at(k)->addCondProp(prop);
        }
    }
}

void NetworkAnalyzer::associateGroupProps(std::vector<GroupProp*> group_props, Property* prop) {
    Identifier ident = prop->getGroup();
    Identifier destIdent = prop->getMsgDestination();
    for (size_t k=0; k<group_props.size(); ++k) {
        if (group_props.at(k)->getGroupIdent() == ident && group_props.at(k)->getMsgDestination() == destIdent) {
            group_props.at(k)->addGroupedProp(prop);
        }
    }
}

void NetworkAnalyzer::connectOverlayProps(std::vector<VoreenPainterOverlay*> overlays) {
    std::vector<ConditionProp*> cond_props;
    std::vector<GroupProp*> group_props;

    for (size_t i=0; i<overlays.size(); ++i) {
        const Properties& props_tmp = overlays.at(i)->getProperties();
        for (unsigned j=0; j<props_tmp.size(); ++j) {
            if (props_tmp.at(j)->getType() == Property::CONDITION_PROP)
                cond_props.push_back(dynamic_cast<ConditionProp*>(props_tmp.at(j)));
            if (props_tmp.at(j)->getType() == Property::GROUP_PROP)
                group_props.push_back(dynamic_cast<GroupProp*>(props_tmp.at(j)));
        }
    }
    for (size_t i=0; i<overlays.size(); ++i) {
        const Properties& props_tmp = overlays.at(i)->getProperties();
        for (size_t j=0; j<props_tmp.size(); ++j) {
            if (props_tmp.at(j)->isConditioned()) {
                associateCondProps(cond_props,props_tmp.at(j));
            }//conditioned
            else if (props_tmp.at(j)->isGrouped()) {
                associateGroupProps(group_props,props_tmp.at(j));
            }//grouped
        }
    }
}

void NetworkAnalyzer::markCondProps(std::vector<Processor*> rend, std::vector<ConditionProp*> conds, std::vector<GroupProp*> groups) {
    for (size_t i=0; i<rend.size(); ++i) {
        const Properties& props_tmp = rend.at(i)->getProperties();
        for (size_t j=0; j<props_tmp.size(); ++j) {
            if (props_tmp.at(j)->isConditioned()) {
                associateCondProps(conds,props_tmp.at(j));
            }//conditioned
            else if (props_tmp.at(j)->isGrouped()) {
                associateGroupProps(groups,props_tmp.at(j));
            }//grouped
        }
    }
}


}//namespace
