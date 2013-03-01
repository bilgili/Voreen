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

    virtual std::string getClassName() const { return "SelectionMetaData<T>"; }
    virtual MetaDataBase* create() const {
        return new SelectionMetaData<T>();
    }
    virtual MetaDataBase* clone() const {
        return new SelectionMetaData<T>(values_);
    }

    virtual std::string toString() const {
        return "";
    }

    virtual std::string toString(const std::string& /*component*/) const {
        return toString();
    }

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
