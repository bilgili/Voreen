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

#ifndef VRN_PLOTDATAEXPORT_H
#define VRN_PLOTDATAEXPORT_H

#include "voreen/core/ports/plotport.h"
#include "voreen/core/processors/processor.h"

#include "voreen/core/properties/stringproperty.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/buttonproperty.h"

namespace voreen {

class PlotData;
class PlotFunction;

class PlotDataExport : public Processor {

public:
    PlotDataExport();
    ~PlotDataExport();
    virtual Processor* create() const;

    virtual std::string getCategory() const  { return "Export"; }
    virtual std::string getClassName() const { return "PlotDataExport"; }
    virtual CodeState getCodeState() const   { return CODE_STATE_TESTING; }
    virtual std::string getProcessorInfo() const;
    virtual bool isEndProcessor() const;
    virtual bool isReady() const;

protected:
    virtual void process();
    virtual void initialize() throw (VoreenException);
    virtual void deinitialize() throw (VoreenException);

private:


    void exportFile();

    PlotPort inPort_;

    FileDialogProperty outputFile_;   ///< filename of the file the value information should be saved in

    StringProperty separator_;
    BoolProperty valueBracket_;
    PlotData* pData_;
    PlotFunction* pPlotFunction_;

    ButtonProperty rewrite_;

    static const std::string loggerCat_;
};

} //namespace
#endif // VRN_PLOTDATAEXPORT_H

