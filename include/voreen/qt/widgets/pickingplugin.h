/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2008 Visualization and Computer Graphics Group, *
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

#ifndef PICKINGPLUGIN_H
#define PICKINGPLUGIN_H

#include "widgetplugin.h"
#include "tgt/event/eventlistener.h"
#include "voreen/core/opengl/texturecontainer.h"
#include "voreen/core/volume/volumecontainer.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/volume/volumeatomic.h"
#include "voreen/core/vis/messagedistributor.h"


#include <QtGui>


namespace voreen {


class PickingPlugin : public WidgetPlugin, tgt::EventListener {
    Q_OBJECT

    public:
        PickingPlugin(QWidget* parent = 0, MessageReceiver* msgReceiver = 0);
        virtual ~PickingPlugin() {
        }

        void mouseDoubleClickEvent(tgt::MouseEvent* e);
        void setTextureContainer(voreen::TextureContainer* tc); 
        void setDataset(int volume);
        void setVolumeContainer(voreen::VolumeContainer* volcont);
        void setLowerThreshold(float);
        void setUpperThreshold(float);


    protected:
        virtual void createWidgets();
        virtual void createConnections();
        void initiateFloodFill(tgt::ivec3 position);
        void floodFill(tgt::ivec3 position);



    public slots:
        
         void evaluateThreshold();

        
    private:
        voreen::TextureContainer* tc_;  
        QGroupBox* thresholdBox_;
        QGroupBox* segmentBox_;

        QLabel*  thresholdStandardDerivationLbl_;
        QLabel* thresholdStandardDerivationValueLbl_;
        QLabel* thresholdAverageLbl_;
        QLabel* thresholdAverageValueLbl_;

        QLabel* segmentCapacityLbl_;
        QLabel* segmentCapacityValueLbl_ ;

        QLabel* segmentAverageLbl_;
        QLabel* segmentAverageValueLbl_;
        QLabel* segmentStandardDerivationLbl_;
        QLabel* segmentStandardDerivationValueLbl_;
       
        
        QPushButton* thresholdCalcBt_;

        voreen::Volume* dataset_;
        voreen::VolumeContainer* volumeContainer_; 

        tgt::vec2 thresholdValues_;
        
};

    } // namespace voreen

#endif






