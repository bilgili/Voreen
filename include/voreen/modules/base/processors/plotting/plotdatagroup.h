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

#ifndef VRN_PLOTDATAGROUP_H
#define VRN_PLOTDATAGROUP_H

#include "voreen/core/ports/plotport.h"
#include "voreen/core/processors/processor.h"
#include "voreen/core/plotting/plotdata.h"
#include "voreen/core/plotting/aggregationfunction.h"

#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/intproperty.h"

namespace voreen {

class Serializable;

/**
  * This class describes the Processor who implements the GroupBy-Function
  * on PlotData-Objectes similar to the GroupBy-Function of SQL.
  */
class PlotDataGroup : public Processor {

public:
    PlotDataGroup();
    ~PlotDataGroup();
    virtual Processor* create() const;

    virtual std::string getCategory() const  { return "Plotting"; }
    virtual std::string getClassName() const { return "PlotDataGroup"; }
    virtual CodeState getCodeState() const   { return CODE_STATE_TESTING; }
    virtual std::string getProcessorInfo() const;
    virtual bool isEndProcessor() const;
    virtual bool isReady() const;

    /// @see Serializer::serialize
    virtual void serialize(XmlSerializer& s) const;

    /// @see Deserializer::deserialize
    virtual void deserialize(XmlDeserializer& s);

    /// this function give the PlotData after grouping, nessasary for the widget to get the PlotData
    const PlotData* getPlotData() const;

    /// delete the whole ResetList.
    void resetAllData();
    /// delete the last Elements in the ResetList.
    void resetLastData();
    /// function to group the plotdata on inport by the column
    void groupbyWidget(int column, const std::vector< std::pair<int,AggregationFunction*> >& functions);

protected:
    virtual void process();
    virtual void initialize() throw (VoreenException);
    virtual void deinitialize() throw (VoreenException);

private:
    typedef std::vector< std::pair< int, AggregationFunction*> > AggregationVector;

    struct GroupValues : public Serializable {
    public:
        int groupbyColumn;
        AggregationVector aggregationVector;

        /// @see Serializer::serialize
        virtual void serialize(XmlSerializer& s) const;

        /// @see Deserializer::deserialize
        virtual void deserialize(XmlDeserializer& s);

    };

    void setOutPortData();
    void readData();
    void calculate();
    void deleteList();
    void updateView();
    PlotData* PlotDataGroupby(PlotData* actPlotData);
    void fillResetList(int groupbyColumn, const AggregationVector& aggregationVector);

    PlotPort inPort_;
    PlotPort outPort_;

    PlotData* pData_;
    PlotData* pDataOut_;

    std::vector<GroupValues> resetList_;

    IntProperty maxTableColumnWidth_;

    FloatVec4Property keyColumnColor_;
    FloatVec4Property dataColumnColor_;

    static const std::string loggerCat_;
}; //namespace

}
#endif // VRN_PLOTDATAGROUP_H

