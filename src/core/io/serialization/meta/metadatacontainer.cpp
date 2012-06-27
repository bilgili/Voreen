/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#include "voreen/core/io/serialization/meta/metadatacontainer.h"

#include "voreen/core/io/serialization/meta/positionmetadata.h"
#include "voreen/core/io/serialization/meta/windowstatemetadata.h"

namespace voreen {

MetaDataContainer::~MetaDataContainer() {
    clearMetaData();
}

void MetaDataContainer::addMetaData(const std::string& key, MetaDataBase* meta) {
    removeMetaData(key);
    metaData_[key] = meta;
}

bool MetaDataContainer::hasMetaData(const std::string& key) {
    return metaData_.find(key) != metaData_.end();
}

MetaDataBase* MetaDataContainer::getMetaData(const std::string& key) {
    if (hasMetaData(key))
        return metaData_[key];
    else
        return 0;
}

void MetaDataContainer::removeMetaData(const std::string& key) {
    if (!hasMetaData(key))
        return;

    delete metaData_[key];
    metaData_.erase(metaData_.find(key));
}

void MetaDataContainer::clearMetaData() {
    for (MetaDataMap::iterator it = metaData_.begin(); it != metaData_.end(); ++it)
        delete it->second;

    metaData_.clear();
}

void MetaDataContainer::serialize(XmlSerializer& s) const {
    if (metaData_.empty())
        return;

    // Initialize meta data factories, if it is not done already...
    if (!factoriesInitialized_)
        initializeFactories();

    // Register known meta data factories...
    for (FactoryCollection::const_iterator it = factories_.begin(); it != factories_.end(); ++it)
        s.registerFactory(*it);

    // Register factories for meta data to serialize
    for (MetaDataMap::const_iterator it = metaData_.begin(); it != metaData_.end(); ++it)
        s.registerFactory(it->second);

    s.serialize("MetaData", metaData_, "MetaItem", "name");
}

void MetaDataContainer::deserialize(XmlDeserializer& s) {
    if (!factoriesInitialized_)
        initializeFactories();

    // Register known meta data factories...
    for (FactoryCollection::const_iterator it = factories_.begin(); it != factories_.end(); ++it)
        s.registerFactory(*it);

    // Register actual containing meta data factories.
    for (MetaDataMap::const_iterator it = metaData_.begin(); it != metaData_.end(); ++it)
        s.registerFactory(it->second);

    clearMetaData();

    try {
        s.deserialize("MetaData", metaData_, "MetaItem", "name");
    } catch (XmlSerializationNoSuchDataException&) {
        // There were no meta data during serialization, so we can just ignore the exception...
        s.removeLastError();
    }
}

bool MetaDataContainer::factoriesInitialized_ = false;

MetaDataContainer::FactoryCollection MetaDataContainer::factories_;

void MetaDataContainer::initializeFactories() {
    factories_.push_back(new PositionMetaData());
    factories_.push_back(new WindowStateMetaData());

    factoriesInitialized_ = true;
}

} // namespace
