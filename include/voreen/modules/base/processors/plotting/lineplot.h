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

#ifndef VRN_LINEPLOT_H
#define VRN_LINEPLOT_H

#include "voreen/modules/base/processors/plotting/plotprocessor.h"

#include "voreen/core/properties/floatproperty.h"

namespace voreen {

class LinePlot : public PlotProcessor {

public:
    LinePlot();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "LinePlot"; }
    virtual CodeState getCodeState() const   { return CODE_STATE_TESTING; }
    virtual std::string getProcessorInfo() const;

protected:
     virtual void initialize() throw (VoreenException);
     virtual void deinitialize() throw (VoreenException);

private:
    // inherited methods
    virtual void render();
    virtual void renderData();
    virtual void renderAxes();
    virtual void setPlotStatus();
    virtual void readFromInport();
    virtual void calcDomains();
    virtual void toggleProperties();
    virtual void createPlotLabels();

    /// create line labels
    void createLineLabels();

    // properties
    FloatProperty lineWidth_;
    FloatProperty pointSize_;
    BoolProperty logXAxis_;
    BoolProperty logYAxis_;
    BoolProperty renderLineLabel_;

    static const std::string loggerCat_;

    GLuint dataList_;         ///< display list of the data
    GLuint pickingList_;      ///< display list of the picking data
};

}   //namespace

#endif // VRN_LINEPLOT_H
