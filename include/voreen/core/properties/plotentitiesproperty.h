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

#ifndef VRN_PLOTENTITIESPROPERTY_H
#define VRN_PLOTENTITIESPROPERTY_H

#include "voreen/core/properties/templateproperty.h"
#include "voreen/core/plotting/colormap.h"
#include "voreen/core/plotting/plotdata.h"
#include "voreen/core/plotting/plotentitysettings.h"

#include <vector>

namespace voreen {
/**
 * Property encapsulating a vector of PlotEntitySettings.
 */
class PlotEntitiesProperty : public TemplateProperty<std::vector<PlotEntitySettings> > {
public:
    PlotEntitiesProperty(const std::string& id, const std::string& guiText, PlotEntitySettings::Entity entities,
       std::vector<PlotEntitySettings> value = std::vector<PlotEntitySettings>(), Processor::InvalidationLevel invalidationLevel = Processor::INVALID_RESULT);

    virtual std::string getTypeString() const;

    /**
     * Executes all member actions that belong to the property. Generally the owner of the
     * property is invalidated.
     */
    //void notifyChange();

    virtual PropertyWidget* createWidget(PropertyWidgetFactory* f);

    /**
     * @see Property::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Property::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);


    PlotEntitySettings::Entity getEntities() const;
    void setEntities(PlotEntitySettings::Entity entities);

    ///current selected x axis
    int getXColumnIndex() const;
    bool setXColumnIndex(int index);

    //call only if entities == SURFACE or SCATTER
    ///current selected y axis
    int getYColumnIndex() const;
    bool setYColumnIndex(int index);

    /**
     * Returns a vector of the main indices of all held PlotEntities
     **/
    std::vector<int> getDataColumnIndices() const;

    const ColorMap& getColorMap() const;
    void setColorMap(ColorMap cm);

    const PlotData* getPlotData() const;
    void setPlotData(const PlotData* data);

    void setPlotEntitySettings(PlotEntitySettings settings, int index);
    void deletePlotEntitySettings(int index);

    /// returns the dataEmptyFlag_
    bool dataEmpty() const;

    /// returns the dataValidFlag_
    bool dataValid() const;

    ///applies the colormap to the entities
    void applyColormap();

    ///creates entity settings
    PlotEntitySettings createEntitySettings() const;

    ///creates all entity settings
    std::vector<PlotEntitySettings> createAllEntitySettings() const;

private:
    void notifyAll();       ///< invalidates every owner and executes links

    /// executes links if all properties use the same PlotData
    //virtual void executeLinks(const std::vector<PlotEntitySettings>& prevValue, const std::vector<PlotEntitySettings>& curValue);

    ///returns true, if the column of data_ with index index can represent a x axis
    bool possibleXAxis(int index);
    ///returns true, if the column of data_ with index index can represent a y axis
    bool possibleYAxis(int index);

    /// type of the entities
    PlotEntitySettings::Entity entities_;

    int xColumnIndex_;                                ///< column index of x axis (ALL)
    int yColumnIndex_;                                ///< column index of y axis (SURFACE and SCATTER)
    ColorMap colorMap_;

    bool dataEmptyFlag_;  ///< true if the data is empty
    bool dataValidFlag_;  ///< false if there is not enough data to plot something

    const PlotData* data_;      ///< pointer to plotdata

};

} // namespace voreen

#endif // VRN_PLOTENTITIESPROPERTY_H
