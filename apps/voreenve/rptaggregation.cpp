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

#include "voreen/rptgui/rptaggregation.h"
#include "voreen/rptgui/rptaggregationguiitem.h"
#include "voreen/rptgui/rptrendererwrappergui.h"

namespace voreen {

RptAggregation::RptAggregation(std::vector<RptRendererWrapperGui*> wrappers, QGraphicsScene* scene, std::string name)
:   name_(name),
    rendererWrappers_(wrappers)
{
    guiItem_ = new RptAggregationGuiItem(this, scene, name);
  //  scene->addItem(guiItem_);
  //  guiItem_->createPorts();

  //  // position guiItem_
  //  QPoint pos = QPoint(0,0);
  //  for (size_t i=0; i<rendererWrappers_.size(); i++) {
		//pos += rendererWrappers_[i]->getGuiItem()->pos().toPoint();
  //  }
  //  pos /= rendererWrappers_.size();
  //  guiItem_->setPos(pos);
  //  scene->removeItem(guiItem_);
}

RptAggregation::~RptAggregation() {
    guiItem_->disconnectAll();
    delete(guiItem_);
    for (size_t i=0; i<rendererWrappers_.size(); i++) {
        delete(rendererWrappers_[i]);
    }
}

void RptAggregation::setRendererWrappers(std::vector<RptRendererWrapperGui*> wrappers) {
    rendererWrappers_ = wrappers;
    guiItem_->createPorts();
}

void RptAggregation::setName(std::string name) {
    name_ = name;
    guiItem_->setName(name);
}

std::vector<RptPortItem*> RptAggregation::getUnconnectedPortItems() {
    std::vector<RptPortItem*> ports;
    for (size_t i=0; i<rendererWrappers_.size(); i++) {
        std::vector<RptPortItem*> portVector = rendererWrappers_.at(i)->getGuiItem()->getInports();
        for (size_t j=0; j<portVector.size(); j++) {
            if (portVector[j]->getConnected().size() == 0)
                ports.push_back(portVector[j]);
        }
        portVector = rendererWrappers_.at(i)->getGuiItem()->getOutports();
        for (size_t j=0; j<portVector.size(); j++) {
            if (portVector[j]->getConnected().size() == 0)
                ports.push_back(portVector[j]);
        }
    }
    return ports;
}

std::vector<RptPortItem*> RptAggregation::getUnconnectedCoProcessorPortItems() {
    std::vector<RptPortItem*> ports;
    for (size_t i=0; i<rendererWrappers_.size(); i++) {
        std::vector<RptPortItem*> portVector = rendererWrappers_.at(i)->getGuiItem()->getCoProcessorInports();
        for (size_t j=0; j<portVector.size(); j++) {
            if (portVector[j]->getConnected().size() == 0)
                ports.push_back(portVector[j]);
        }
        portVector = rendererWrappers_.at(i)->getGuiItem()->getCoProcessorOutports();
        for (size_t j=0; j<portVector.size(); j++) {
            if (portVector[j]->getConnected().size() == 0)
                ports.push_back(portVector[j]);
        }
    }
    return ports;
}

std::vector<RptPortItem*> RptAggregation::getOutwardsConnectedPortItems() {
    std::vector<RptPortItem*> ports;

    for (size_t i=0; i<rendererWrappers_.size(); i++) {
        // go through all inports
        std::vector<RptPortItem*> portVector = rendererWrappers_.at(i)->getGuiItem()->getInports();
        for (size_t j=0; j<portVector.size(); j++) {
            // check if connectedPorts' parentItems are in rendererWrappers
            for (size_t k=0; k<portVector[j]->getConnected().size(); k++) {
                if (!this->contains(portVector[j]->getConnected()[k]->getParent())) {
                    ports.push_back(portVector[j]);
                    break;  // so don't push_back a port twice
                }
            }
        }
        // go through all outports
        portVector = rendererWrappers_.at(i)->getGuiItem()->getOutports();
        for (size_t j=0; j<portVector.size(); j++) {
            // check if connectedPorts' parentItems are in rendererWrappers
            for (size_t k=0; k<portVector[j]->getConnected().size(); k++) {
                if (!this->contains(portVector[j]->getConnected()[k]->getParent())) {
                    ports.push_back(portVector[j]);
                    break;  // so don't push_back a port twice
                }
            }
        }
    }

    return ports;
}

std::vector<RptPortItem*> RptAggregation::getOutwardsConnectedCoProcessorPortItems() {
    std::vector<RptPortItem*> ports;

    for (size_t i=0; i<rendererWrappers_.size(); i++) {
        // go through all inports
        std::vector<RptPortItem*> portVector = rendererWrappers_.at(i)->getGuiItem()->getCoProcessorInports();
        for (size_t j=0; j<portVector.size(); j++) {
            // check if connectedPorts' parentItems are in rendererWrappers
            for (size_t k=0; k<portVector[j]->getConnected().size(); k++) {
                if (!this->contains(portVector[j]->getConnected()[k]->getParent())) {
                    ports.push_back(portVector[j]);
                    break;  // so don't push_back a port twice
                }
            }
        }
        // go through all outports
        portVector = rendererWrappers_.at(i)->getGuiItem()->getCoProcessorOutports();
        for (size_t j=0; j<portVector.size(); j++) {
            // check if connectedPorts' parentItems are in rendererWrappers
            for (size_t k=0; k<portVector[j]->getConnected().size(); k++) {
                if (!this->contains(portVector[j]->getConnected()[k]->getParent())) {
                    ports.push_back(portVector[j]);
                    break;  // so don't push_back a port twice
                }
            }
        }
    }

    return ports;
}

bool RptAggregation::contains(RptGuiItem* item) {
    for (size_t i=0; i<rendererWrappers_.size(); i++) {
        if (rendererWrappers_[i]->getGuiItem() == static_cast<RptRendererGuiItem*>(item)) {
            return true;
        }
    }
    return false;
}

std::vector<RptRendererWrapperGui*>& RptAggregation::deaggregate() {
    guiItem_->deaggregate();
    for (size_t i=0; i<rendererWrappers_.size(); i++) {
        rendererWrappers_[i]->getGuiItem()->adjustArrows();
    }
    //rendererWrappers_.clear();
    //delete(this);

    return rendererWrappers_;

    //// outward ports
    //std::vector<RptPortItem*> inports;
    //std::vector<RptPortItem*> outports;

    //// disconnect ports of aggregation
    //for (size_t i=0; i<guiItem_->getInports().size(); i++) {
    //    inports.push_back(guiItem_->getInports()[i];
    //    for (size_t j=0; j<guiItem_->getInports()[i]->getConnected()->size(); j++) {
    //        guiItem_->getInports()[i]->getConnected()->at(j)->disconnect
    //            (guiItem_->getInports()[i]->getConnected()->at(j)->getOutport
    //                (guiItem_->getInports()[i]), guiItem_, guiItem_->getInports()[i]);
    //    }
    //}
    //for (size_t i=0; i<guiItem_->getOutports().size(); i++) {
    //    outports.push_back(guiItem_->getOutports()[i];
    //    for (size_t j=0; j<guiItem_->getOutports()[i]->getConnected()->size(); j++) {
    //        for (size_t k=0; k<guiItem_->getOutports()[i]->getConnected()->at(j)->getInports().size(); k++) {
    //            guiItem_->disconnect(guiItem_->getOutports()[i],
    //                guiItem_->getOutports()[i]->getConnected()->at(j),
    //                guiItem_->getOutports()[i]->getConnected()->at(j)->getInports(guiItem_->getOutports()[i])[k]);
    //        }
    //    }
    //}

    //// therefore connect outward ports of rendererWrappers_
    //for (size_t i=0; i<inports.size(); i++) {
    //    // find matching inports in rendererWrappers_
    //    for (size_t j=0; j<rendererWrappers_.size(); j++) {
    //        for (size_t k=0; k<rendererWrappers_[j]->getGuiItem()->getInports().size(); k++) {
    //            if (rendererWrappers_[j]->getGuiItem()->getInports()[k]->getPort() == inports[i]->getPort()) {
    //                // get the connected outport and connect if exists
    //                for (size_t m=0; m<inports[i]->getConnected()->size(); m++) {
    //                    if (inports[i]->getConnected()->at(m)->getOutport(rendererWrappers_[j]->getGuiItem()->getInports()[k]) != 0)
    //                        static_cast<RptGuiItem*>(inports[i]->parentItem())->connectAndCreateArrow(
    //                            inports[i]->getConnected()->at(m)->getOutport(rendererWrappers_[j]->getGuiItem()->getInports()[k]),
    //                            rendererWrappers_[j]->getGuiItem(),
    //                            rendererWrappers_[j]->getGuiItem()->getInports()[k]);
    //                }
    //            }
    //        }
    //    }
    //}
    
    

    // delete aggregation and remove aggregation from aggregations_ in mainwindow

    // put rendererwrappers back to rendererwrappers in mainwindow and back to scene
}

}