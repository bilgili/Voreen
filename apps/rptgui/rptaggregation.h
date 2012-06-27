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

#ifndef VRN_RPTAGGREGATION_H
#define VRN_RPTAGGREGATION_H

#include <QtGui>
#include "voreen/rptgui/rptrendererguiitem.h"

namespace voreen {

class RptAggregationGuiItem;
class RptRendererWrapperGui;

class RptAggregation {

public:
    RptAggregation(std::vector<RptRendererWrapperGui*> wrappers, QGraphicsScene* scene, std::string name = "Aggregation");
    ~RptAggregation();

    RptAggregationGuiItem* getGuiItem() { return guiItem_;}
    std::string getName() { return name_; };
    void setName(std::string name);
    std::vector<RptRendererWrapperGui*> getRendererWrappers() {return rendererWrappers_;}
    std::vector<RptPortItem*> getUnconnectedPortItems();
    std::vector<RptPortItem*> getOutwardsConnectedPortItems();
    std::vector<RptPortItem*> getUnconnectedCoProcessorPortItems();
    std::vector<RptPortItem*> getOutwardsConnectedCoProcessorPortItems();
    bool contains(RptGuiItem* item);
    void setRendererWrappers(std::vector<RptRendererWrapperGui*> wrappers);
    std::vector<RptRendererWrapperGui*>& deaggregate();

private:
    RptAggregationGuiItem* guiItem_;
    std::vector<RptRendererWrapperGui*> rendererWrappers_;
    std::string name_;

};

} // namespace voreen

#endif //VRN_RPTAGGREGATION_H