/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "plottingmodule.h"

#include "processors/plotdatasource.h"
#include "processors/plotfunctionsource.h"
#include "processors/lineplot.h"
#include "processors/hemisphereplot.h"
#include "processors/barplot.h"
#include "processors/scatterplot.h"
#include "processors/surfaceplot.h"
#include "processors/plotdataselect.h"
#include "processors/plotdatagroup.h"
#include "processors/plotdataexport.h"
#include "processors/plotdataexporttext.h"
#include "processors/plotdatafitfunction.h"
#include "processors/plotdatamerge.h"
#include "processors/plotfunctiondiscret.h"
#include "processors/imageanalyzer.h"

#include "properties/link/plotlinkevaluatorfactory.h"

#include "utils/aggregationfunctionfactory.h"
#include "utils/plotpredicatefactory.h"

namespace voreen {

PlottingModule::PlottingModule(const std::string& modulePath)
    : VoreenModule(modulePath)
{
    setName("Plotting");

    registerProcessor(new PlotDataSource());
    registerProcessor(new PlotFunctionSource());
    registerProcessor(new BarPlot());
    registerProcessor(new HemispherePlot());
    registerProcessor(new ImageAnalyzer());
    registerProcessor(new LinePlot());
    registerProcessor(new PlotDataExport());
    registerProcessor(new PlotDataExportText());
    registerProcessor(new PlotDataFitFunction());
    registerProcessor(new PlotDataGroup());
    registerProcessor(new PlotDataMerge());
    registerProcessor(new PlotDataSelect());
    registerProcessor(new PlotFunctionDiscret());
    registerProcessor(new ScatterPlot());
    registerProcessor(new SurfacePlot());

    registerLinkEvaluatorFactory(new PlotLinkEvaluatorFactory());

    registerSerializerFactory(new AggregationFunctionFactory());
    registerSerializerFactory(PlotPredicateFactory::getInstance());
}

} // namespace
