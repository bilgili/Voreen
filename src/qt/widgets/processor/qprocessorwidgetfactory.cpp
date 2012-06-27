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

#ifdef VRN_MODULE_BASE
#include "voreen/modules/base/processors/render/dynamicglslprocessor.h"
#include "voreen/qt/widgets/processor/dynamicglslwidget.h"
#endif
#include "voreen/qt/widgets/processor/qprocessorwidgetfactory.h"
#include "voreen/qt/widgets/processor/qprocessorwidget.h"
#include "voreen/qt/widgets/processor/canvasrendererwidget.h"
#include "voreen/qt/widgets/processor/plotdataprocessorwidget.h"
#include "voreen/qt/widgets/processor/imageanalyzerwidget.h"
#include "voreen/qt/widgets/processor/plotdataselectwidget.h"
#include "voreen/qt/widgets/processor/plotdatagroupwidget.h"
#include "voreen/qt/widgets/processor/plotdatafitfunctionwidget.h"
#include "voreen/qt/widgets/processor/plotdatamergewidget.h"


namespace voreen {

QProcessorWidgetFactory::QProcessorWidgetFactory(QWidget* parent, NetworkEvaluator* nwe) : nwe_(nwe) {
        parent_ = parent;
}

ProcessorWidget* QProcessorWidgetFactory::createWidget(Processor* processor) const {

    if (dynamic_cast<CanvasRenderer*>(processor))
        return new CanvasRendererWidget(parent_, static_cast<CanvasRenderer*>(processor), nwe_);
#ifdef VRN_MODULE_BASE
    else if (dynamic_cast<DynamicGLSLProcessor*>(processor))
        return new DynamicGLSLWidget(parent_, static_cast<DynamicGLSLProcessor*>(processor));
#endif
    else if (dynamic_cast<ImageAnalyzer*>(processor))
        return new ImageAnalyzerWidget(parent_, static_cast<ImageAnalyzer*>(processor));
    else if (dynamic_cast<PlotDataSelect*>(processor))
       return new PlotDataSelectWidget(parent_, static_cast<PlotDataSelect*>(processor));
    else if (dynamic_cast<PlotDataGroup*>(processor))
       return new PlotDataGroupWidget(parent_, static_cast<PlotDataGroup*>(processor));
    else if (dynamic_cast<PlotDataFitFunction*>(processor))
       return new PlotDataFitFunctionWidget(parent_, static_cast<PlotDataFitFunction*>(processor));
    else if (dynamic_cast<PlotDataMerge*>(processor))
       return new PlotDataMergeWidget(parent_, static_cast<PlotDataMerge*>(processor));
    else
        return 0;
}
} // namespace voreen
