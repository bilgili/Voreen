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

#ifndef VRN_AGGREGATIONMETADATA_H
#define VRN_AGGREGATIONMETADATA_H

#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

class Processor;

class AggregationMetaData : public Serializable {
public:
    AggregationMetaData();
    virtual ~AggregationMetaData();

    virtual void serialize(XmlSerializer& s) const;
    virtual void deserialize(XmlDeserializer& s);

    void setProcessors(const std::vector<Processor*>& processors);
    std::vector<Processor*> getProcessors(bool recursive = false) const;

    void addAggregation(AggregationMetaData* aggregation);
    void setAggregations(const std::vector<AggregationMetaData*>& aggregations);
    const std::vector<AggregationMetaData*>& getAggregations() const;

    void setPosition(const int& x, const int& y);
    std::pair<int,int> getPosition() const;

private:
    std::vector<Processor*> processors_;
    std::vector<AggregationMetaData*> aggregations_;
    int positionX_;
    int positionY_;
};

//-------------------------------------------------------------------------------------------------

class AggregationMetaDataContainer : public MetaDataBase {
public:
    AggregationMetaDataContainer();
    virtual ~AggregationMetaDataContainer();

    virtual void serialize(XmlSerializer& s) const;
    virtual void deserialize(XmlDeserializer& s);

    virtual const std::string getTypeString(const std::type_info& type) const;
    virtual Serializable* createType(const std::string& typeString);

    void addAggregation(AggregationMetaData* aggregation);
    void removeAggregation(AggregationMetaData* aggregation);
    void clearAggregations();
    bool isEmpty() const;

    const std::vector<AggregationMetaData*>& getAggregations() const;

private:
    std::vector<AggregationMetaData*> aggregations_; ///< contains all stored aggregations and every processor within each aggregation
};

} // namespace

#endif // VRN_AGGREGATIONMETADATA_H
