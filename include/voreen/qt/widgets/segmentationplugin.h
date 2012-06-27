/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
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

#ifndef SEGMENTATIONPLUGIN_H
#define SEGMENTATIONPLUGIN_H

#include "widgetplugin.h"
#include "tgt/event/eventlistener.h"
#include "voreen/core/vis/processors/render/segmentationraycaster.h"
#include "voreen/core/vis/processors/volume/regiongrowing.h"
#include "voreen/core/vis/message.h"
#include "voreen/core/vis/properties/transferfuncproperty.h"
#include "voreen/qt/widgets/transfunc/transfunceditorintensity.h"

class QComboBox;
class QPushButton;
class QSpinBox;
class QDoubleSpinBox;
class QGroupBox;
class QComboBox;

namespace voreen {

class NetworkEvaluator;
class SimpleSegmentationProcessor;
class ThresholdWidget;

class SegmentationPlugin : public WidgetPlugin, MessageReceiver, tgt::EventListener {
    Q_OBJECT
public:
    SegmentationPlugin(QWidget* parent, NetworkEvaluator* evaluator);
    ~SegmentationPlugin();

    void createWidgets();
    void createConnections();

    void processMessage(Message* msg, const Identifier& dest);

    virtual void mousePressEvent(tgt::MouseEvent* e);

    bool usable(const std::vector<Processor*>& processors);
                                                    
public slots:
    void toggleApplySegmentation(bool);
    void setCurrentSegment(int);
    void toggleSegmentVisible(bool);
    void undoSegment();
    void clearSegment();
    void setSeed(bool checked);
    void setThresholds(int lower, int upper);
    void saveSegmentation();
    void clearSegmentation();

private:

    enum TransFuncSyncDirection {
        PUSH,   // push local transfunc to segmentation raycaster
        PULL    // pull transfunc from segmentation raycaster
    };

    // is called when the user has modified the transfer function via the widget's editor
    void transFuncChanged();
    // Synchronizes the widget's transfer function with the SegmentationRaycaster's.
    void synchronizeTransFuncs(TransFuncSyncDirection syncDir);
    // Propagates the locally saved region growing parameters to the RegionGrowing processor
    void propagateRegionGrowingParams();

    // Retrieve processors from evaluator
    SegmentationRaycaster* getSegmentationRaycaster();
    RegionGrowingProcessor* getRegionGrowingProcessor();

    NetworkEvaluator* evaluator_;
    TransFuncProp transFuncProp_;
    TransFuncEditorIntensity* intensityEditor_;

    QCheckBox* checkApplySegmentation_;
    QSpinBox* spinCurrentSegment_;
    QCheckBox* checkSegmentVisible_;

    QPushButton* markSeedButton_;
    QPushButton* undoButton_;
    QSpinBox* outputSegment_;
    QCheckBox* checkApplyThresholds_;
    QDoubleSpinBox* spinStrictness_;
    QComboBox* comboCostFunction_;
    QCheckBox* checkAdaptive_;
    QSpinBox* spinMaxSeedDist_;

    QPushButton* clearSegmentButton_;
    QPushButton* clearSegmentationButton_;
    QPushButton* saveSegmentationButton_;
    QPushButton* loadSegmentationButton_;

    QGroupBox* renderingBox_;
    QGroupBox* regionGrowingBox_;

    ThresholdWidget* thresholdWidget_;
};

} // namespace voreen

#endif
