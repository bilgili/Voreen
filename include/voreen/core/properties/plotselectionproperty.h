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

#ifndef VRN_PLOTSELECTIONPROPERTY_H
#define VRN_PLOTSELECTIONPROPERTY_H

#include "tgt/vector.h"
#include "tgt/logmanager.h"
#include "voreen/core/properties/templateproperty.h"
#include "voreen/core/plotting/plotzoomstate.h"

#include <vector>
#include <string>
#include <algorithm>
#include <utility>

namespace voreen {

    /**
     * This property holds a vector of plotzoomstates.
     */
class PlotSelectionProperty : public TemplateProperty< std::vector< PlotZoomState > > {
public:

    PlotSelectionProperty(const std::string& id, const std::string& guiText, bool showThirdDimension,
             Processor::InvalidationLevel invalidationLevel = Processor::INVALID_RESULT);

    virtual std::string getTypeString() const;

    void push(const PlotZoomState& value);
    PlotZoomState pop();


    PlotZoomState top() const;

    void clear();

    const std::vector< PlotZoomState >& get() const;
    void set(const std::vector< PlotZoomState >& value);

    bool getShowThirdDimension() const;
    void setShowThirdDimension(bool value);

    int size() const;

    /**
     * @see Property::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Property::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

    PropertyWidget* createWidget(PropertyWidgetFactory* f);

    /// flag whether current data is from deserialization so that processors won't clear the property on reading new data
    bool isDeserialized_;

private:
    void notifyAll();

    static const std::string loggerCat_;

    bool showThirdDimension_;     ///< flag whether to show 2D or 3D zoom states - ONLY for display purposes and will neither be serialized nor be linked!

};

}   // namespace

#endif //VRN_PLOTSELECTIONPROPERTY_H
