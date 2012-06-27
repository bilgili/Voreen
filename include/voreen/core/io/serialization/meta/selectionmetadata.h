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

#ifndef VRN_SELECTIONMETADATA_H
#define VRN_SELECTIONMETADATA_H

#include "voreen/core/io/serialization/serialization.h"

#include "voreen/core/processors/processor.h"

namespace voreen {

/**
 * The @c SelectionMetaData class stores a vector of selected entities (e.g. processors).
 * In the case of processors the template parameter would be @see Processor
 *
 * @see MetaDataBase
 */

template<class T>
class SelectionMetaData : public MetaDataBase {
public:
    SelectionMetaData<T>(const std::vector<T>& values = std::vector<T>()) : values_(values) {}
    virtual ~SelectionMetaData() {}

    /**
     * @see Serializable::serialize
     */
    virtual void serialize(XmlSerializer& s) const {
        s.serialize("values", values_);
    }

    /**
     * @see Serializable::deserialize
     */
    virtual void deserialize(XmlDeserializer& s) {
        s.deserialize("values", values_);
    }

    /**
     * @see SerializableFactory::getTypeString
     */
    virtual const std::string getTypeString(const std::type_info& type) const {
        if (type == typeid(SelectionMetaData<Processor*>))
            return "SelectionMetaData::Processor";
        else
            return "";
    }

    /**
     * @see SerializableFactory::createType
     */
    virtual Serializable* createType(const std::string& typeString) {
        if (typeString == "SelectionMetaData::Processor")
            return new SelectionMetaData<Processor*>;
        else
            return 0;
    }


    /**
     * Sets the selected value.
     *
     * @param value The selected value
     */
    void setValues(const std::vector<T>& value) {
        values_ = value;
    }

    /**
     * Returns the selected value.
     *
     * @return the selected value
     */
    std::vector<T> getValues() const {
        return values_;
    }

    /**
     * Adds a single selection entity
     *
     * @param value The entity
     */
    void addValue(const T& value) {
        values_.push_back(value);
    }

private:
    std::vector<T> values_;
};

} // namespace

#endif // VRN_SELECTIONMETADATA_H
