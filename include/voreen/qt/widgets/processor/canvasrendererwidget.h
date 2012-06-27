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

#ifndef VRN_CANVASRENDERERWIDGET_H
#define VRN_CANVASRENDERERWIDGET_H

#include "voreen/core/processors/canvasrenderer.h"
#include "voreen/qt/widgets/processor/qprocessorwidget.h"

#include "tgt/qt/qtcanvas.h"

namespace voreen {

class NetworkEvaluator;
class SnapshotPlugin;

class CanvasRendererWidget : public QProcessorWidget {
    Q_OBJECT
public:
    CanvasRendererWidget(QWidget* parent, CanvasRenderer* canvasRenderer, NetworkEvaluator* evaluator);
    virtual ~CanvasRendererWidget();

    void initialize();

    void showSnapshotTool();

    virtual void updateFromProcessor();
protected:
    void keyPressEvent(QKeyEvent*);
    void resizeEvent(QResizeEvent*);

    void createSnapshotTool();

private:
    tgt::QtCanvas* canvasWidget_;
    NetworkEvaluator* evaluator_;
    SnapshotPlugin* snapshotTool_;
};

} // namespace voreen

#endif
