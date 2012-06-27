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
class QLabel;

namespace voreen {

class NetworkEvaluator;
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

private slots:

    // update the intensity editor to the segmentation raycaster's state
    void updateIntensityEditor();

    // registers this plugin as listener at the segmentation raycaster's properties
    void registerAsListener();

    // is called when a relevant transfunc property was changed outside this plugin
    void transFuncChangedExternally();

    // callback for registered at segmentation raycaster's property
    void applySegmentationToggled();

    // callback gui checkbox
    void toggleApplySegmentation(bool);

    // gui callbacks
    void setCurrentSegment(int);
    void undoSegment();
    void clearSegment();
    void setSeed(bool checked);
    void saveSegmentation();
    void loadSegmentation();
    void clearSegmentation();

private:

    // Propagates the locally saved region growing parameters to the RegionGrowing processor
    void propagateRegionGrowingParams();

    // Retrieve processors from evaluator
    SegmentationRaycaster* getSegmentationRaycaster(bool suppressWarning = false);
    RegionGrowingProcessor* getRegionGrowingProcessor(bool suppressWarning = false);

    NetworkEvaluator* evaluator_;
    TransFuncEditorIntensity* intensityEditor_;

    QCheckBox* checkApplySegmentation_;
    QLabel* labelCurrentSegment_;
    QSpinBox* spinCurrentSegment_;

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

};

} // namespace voreen

#endif
