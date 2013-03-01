/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "voreen/core/datastructures/meta/aggregationmetadata.h"

#include "voreen/core/ports/port.h"
#include "voreen/core/processors/processor.h"

namespace voreen {

AggregationMetaData::AggregationMetaData()
    : Serializable()
    , name_("")
{}

AggregationMetaData::~AggregationMetaData() {}

void AggregationMetaData::serialize(XmlSerializer& s) const {
    s.serialize("Name", name_);
    s.serialize("Processors", processors_);
    s.serialize("SubAggregations", aggregations_);
    s.serialize("PortConnections", portConnections_);
    s.serialize("x-Position", positionX_);
    s.serialize("y-Position", positionY_);
}

void AggregationMetaData::deserialize(XmlDeserializer& s) {
    s.deserialize("Name", name_);
    s.deserialize("Processors", processors_);
    s.deserialize("SubAggregations", aggregations_);
    s.deserialize("PortConnections", portConnections_);
    s.deserialize("x-Position", positionX_);
    s.deserialize("y-Position", positionY_);
}

void AggregationMetaData::setProcessors(const std::vector<Processor*>& processors) {
    processors_ = processors;
}

std::vector<Processor*> AggregationMetaData::getProcessors(bool recursive) const {
    if (recursive) {
        std::vector<Processor*> result = processors_;

        for (size_t i = 0; i < aggregations_.size(); ++i) {
            std::vector<Processor*> r = aggregations_[i]->getProcessors(true);
            result.insert(result.end(), r.begin(), r.end());
        }

        return result;
    }
    else {
        return processors_;
    }
}

void AggregationMetaData::addAggregation(AggregationMetaData* aggregation) {
    aggregations_.push_back(aggregation);
}

void AggregationMetaData::setAggregations(const std::vector<AggregationMetaData*>& aggregations) {
    aggregations_ = aggregations;
}

const std::vector<AggregationMetaData*>& AggregationMetaData::getAggregations() const {
    return aggregations_;
}

void AggregationMetaData::addPortConnection(Port* outport, Port* inport) {
    PortConnection connection(outport, inport);
    portConnections_.push_back(connection);
}

const std::vector<PortConnection>& AggregationMetaData::getPortConnections() const {
    return portConnections_;
}

void AggregationMetaData::setPosition(const int& x, const int& y) {
    positionX_ = x;
    positionY_ = y;
}

std::pair<int,int> AggregationMetaData::getPosition() const {
    return std::make_pair(positionX_, positionY_);
}

void AggregationMetaData::setName(const std::string& name) {
    name_ = name;
}

const std::string& AggregationMetaData::getName() const {
    return name_;
}

//-------------------------------------------------------------------------------------------------

AggregationMetaDataContainer::AggregationMetaDataContainer() {}

AggregationMetaDataContainer::~AggregationMetaDataContainer() {}

void AggregationMetaDataContainer::serialize(XmlSerializer& s) const {
    s.serialize("Aggregations", aggregations_);
}

void AggregationMetaDataContainer::deserialize(XmlDeserializer& s) {
    s.deserialize("Aggregations", aggregations_);
}

std::string AggregationMetaDataContainer::toString() const{
    return "";
}

std::string AggregationMetaDataContainer::toString(const std::string& /*component*/) const{
    return toString();
}

void AggregationMetaDataContainer::addAggregation(AggregationMetaData* aggregation) {
    aggregations_.push_back(aggregation);
}

void AggregationMetaDataContainer::removeAggregation(AggregationMetaData* aggregation) {
    for (size_t i = 0; i < aggregations_.size(); ++i)
        if (aggregations_[i] == aggregation) {
            aggregations_.erase(aggregations_.begin() + i);
            break;
        }
}

void AggregationMetaDataContainer::clearAggregations() {
    aggregations_.clear();
}

bool AggregationMetaDataContainer::isEmpty() const {
    return aggregations_.empty();
}

const std::vector<AggregationMetaData*>& AggregationMetaDataContainer::getAggregations() const {
    return aggregations_;
}

MetaDataBase* AggregationMetaDataContainer::clone() const {
    AggregationMetaDataContainer* md = new AggregationMetaDataContainer();
    md->aggregations_ = aggregations_;
    return md;
}

MetaDataBase* AggregationMetaDataContainer::create() const {
    return new AggregationMetaDataContainer();
}

} // namespace
