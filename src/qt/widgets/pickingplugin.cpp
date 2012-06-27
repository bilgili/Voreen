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

#include "voreen/qt/widgets/pickingplugin.h"

#include <QColorDialog>
#include <QPixmap>
#include <QVBoxLayout>
#include <cmath>

#include "voreen/core/vis/lightmaterial.h"

using tgt::vec2;

namespace voreen {

PickingPlugin::PickingPlugin(QWidget* parent, MessageReceiver* msgReceiver) :
    WidgetPlugin(parent, msgReceiver),
    thresholdValues_(0,255)
       
   
{
    setObjectName(tr("Picking"));
    icon_ = QIcon(":/icons/bulb.png");

    // lighting model settings
   
}



void PickingPlugin::createWidgets() {
    resize(600,300);
    

    QVBoxLayout* mainLayout = new QVBoxLayout();
    QGridLayout* thresholdBoxLayout = new QGridLayout();
    QGridLayout* segmentBoxLayout = new QGridLayout();

    thresholdBox_  = new QGroupBox(tr("Threshold"));
    thresholdBox_->setLayout(thresholdBoxLayout);    
    segmentBox_ = new QGroupBox(tr("Segmentation"));
    segmentBox_->setLayout(segmentBoxLayout); 
     
    thresholdBox_->setMinimumWidth(200);

    thresholdAverageLbl_ = new QLabel("average:");
    thresholdAverageValueLbl_ = new QLabel("0");

    thresholdStandardDerivationLbl_ = new QLabel("standard derivation:");
    thresholdStandardDerivationValueLbl_ = new QLabel("0");

    thresholdCalcBt_ = new QPushButton(tr("analyze voxels"));

    thresholdBoxLayout->addWidget(thresholdAverageLbl_,0,0);
    thresholdBoxLayout->addWidget(thresholdAverageValueLbl_,0,1);
    thresholdBoxLayout->addWidget(thresholdStandardDerivationLbl_,1,0);
    thresholdBoxLayout->addWidget(thresholdStandardDerivationValueLbl_,1,1);
    thresholdBoxLayout->addWidget(thresholdCalcBt_,2,0,1,2);


    segmentCapacityLbl_ = new QLabel(tr("capacity:"));
    segmentCapacityValueLbl_ = new QLabel(tr("0"));

    segmentAverageLbl_ = new QLabel(tr("average:"));
    segmentAverageValueLbl_ = new QLabel(tr("0"));
    segmentStandardDerivationLbl_ = new QLabel(tr("standard derivation:"));
    segmentStandardDerivationValueLbl_ = new QLabel(tr("0"));
      
    
    segmentBoxLayout->addWidget(segmentCapacityLbl_,0,0);
    segmentBoxLayout->addWidget(segmentCapacityValueLbl_,0,1);
    segmentBoxLayout->addWidget(segmentAverageLbl_,1,0);
    segmentBoxLayout->addWidget(segmentAverageValueLbl_,1,1);
    segmentBoxLayout->addWidget(segmentStandardDerivationLbl_,2,0);
    segmentBoxLayout->addWidget(segmentStandardDerivationValueLbl_,2,1);
    
    mainLayout->addWidget(thresholdBox_);
    mainLayout->addWidget(segmentBox_);

    setLayout(mainLayout);

}

void PickingPlugin::createConnections() {

 connect(thresholdCalcBt_,SIGNAL(clicked()),this,SLOT(evaluateThreshold()));

}

void PickingPlugin::setDataset(int volume){
    dataset_ = volumeContainer_->getVolume(volume);
    
}
void PickingPlugin::setVolumeContainer(voreen::VolumeContainer* volcont){
    volumeContainer_ = volcont;
}
void PickingPlugin::setLowerThreshold(float threshold){
    thresholdValues_.x = threshold;
}

void PickingPlugin::setUpperThreshold(float threshold){
    thresholdValues_.y = threshold;
}



void PickingPlugin::mouseDoubleClickEvent(tgt::MouseEvent* e){
   tgt::ivec2 size = tc_->getSize(); 
   float* color = new float[4];
   int posX = e->x();
   int posY = e->y();
   tgt::ivec3 position;

   int maxY = tc_->getSize().y ;
   posY = maxY - posY; 
 
   color = tc_->getTargetAsFloats(3,posX,posY);
   float red = color[0];
   float green = color[1];
   float blue = color[2];
   float alpha = color[3];


     if (!(alpha > 0)) {
          QMessageBox::warning(this, "Voreen", tr("you've selected the background!")); 
          return;
     }
    
     position = tgt::ivec3(red*dataset_->getDimensions().x,green*dataset_->getDimensions().y,blue*dataset_->getDimensions().z);

     if (position.x <= 1 || position.x >= dataset_->getDimensions().x-1 || 
         position.y <= 1 || position.y >= dataset_->getDimensions().y-1 ||
	     position.z <= 1 || position.z >= dataset_->getDimensions().z-1){
             QMessageBox::warning(this, "Voreen", tr("you've selected the border lines, try another angle or threshold"));
	         return ;
     }

     floodFill(position); 
}
 
void PickingPlugin::initiateFloodFill(tgt::ivec3 position){
    
    

}

int max(int x, int y) {
 if (x >= y) return x; 
 else return y;
}


void PickingPlugin::floodFill(tgt::ivec3 position){
    float xl,yl,zl;
    float xr,yr,zr;

    float count = 0;
    float sum = 0.f;
    float derivationSum = 0.f;
    float density = 0.f;
    float lowerThreshold;
    float upperThreshold;
    
    VolumeUInt8* vol = dynamic_cast<VolumeUInt8*>(dataset_);
 

    tgt::ivec3 datasetDimension = dataset_->getDimensions();
    VolumeAtomic<bool> markedVoxels(dataset_->getDimensions(),tgt::vec3(1),1); 

    for(int x = 0; x< markedVoxels.getDimensions().x; x++)
        for(int y = 0; y< markedVoxels.getDimensions().y; y++)
            for(int z = 0; z< markedVoxels.getDimensions().z; z++)
                markedVoxels.voxel(x,y,z) = false;

    tgt::ivec3 val = tgt::ivec3(0);
    std::stack<tgt::ivec3> voxelStack;

    voxelStack.push(tgt::ivec3(position));
    voxelStack.push(tgt::ivec3(position.x + 2,position.y,position.z));  
    voxelStack.push(tgt::ivec3(position.x - 2,position.y,position.z));  

    voxelStack.push(tgt::ivec3(position.x,position.y+2,position.z));  
    voxelStack.push(tgt::ivec3(position.x,position.y-2,position.z));  

    voxelStack.push(tgt::ivec3(position.x,position.y,position.z+2));  
    voxelStack.push(tgt::ivec3(position.x,position.y,position.z-2));  

    lowerThreshold = thresholdValues_.x; 
    upperThreshold = thresholdValues_.y;

    density = dataset_->getVoxelFloat(tgt::ivec3(position),0);

    tgt::ivec3 lowerLeftCorner(position);
    tgt::ivec3 upperRightCorner(position);  

   
    xl = lowerLeftCorner.x;
    yl = lowerLeftCorner.y;
    zl = lowerLeftCorner.z;

    xr = upperRightCorner.x;
    yr = upperRightCorner.y;
    zr = upperRightCorner.z;


//   if (density < lowerThreshold)  lowerThreshold = density;  
       while(!voxelStack.empty()) {
          val = voxelStack.top();
          voxelStack.pop();
          density = dataset_->getVoxelFloat(val,0);
      	   
	      if ( (val.x > 2) && (val.x < datasetDimension.x-2) && (val.y > 2) && (val.y < datasetDimension.y -2)
		       && (val.z > 2) && (val.z < datasetDimension.z-2) && (markedVoxels.voxel(val.x,val.y,val.z)==false)&&
		       (density >= lowerThreshold) && (density < upperThreshold) )
	       {

                  markedVoxels.voxel(val.x,val.y,val.z) = true;	

                  if (val.x < xl) xl = val.x;
                  if (val.y < yl) yl = val.y;
                  if (val.z < zl) zl = val.z;

                  if (val.x > xr) xr = val.x;
                  if (val.y > yr) yr = val.y;
                  if (val.z > zr) zr = val.z;

                  if (markedVoxels.voxel(val.x+1,val.y,val.z)==false) 
                         voxelStack.push(tgt::ivec3(val.x+1,val.y,val.z));
	   		                      
                  if (markedVoxels.voxel(val.x-1,val.y,val.z)==false) 
                         voxelStack.push(tgt::ivec3(val.x-1,val.y,val.z));
	   		      
                  if (markedVoxels.voxel(val.x,val.y+1,val.z)==false) 
                        voxelStack.push(tgt::ivec3(val.x,val.y+1,val.z));
			     
                  if (markedVoxels.voxel(val.x,val.y-1,val.z)==false) 
                        voxelStack.push(tgt::ivec3(val.x,val.y-1,val.z)); 	   		

                  if (markedVoxels.voxel(val.x,val.y,val.z+1)==false) 
                        voxelStack.push(tgt::ivec3(val.x,val.y,val.z+1));
                 
                  if (markedVoxels.voxel(val.x,val.y,val.z-1)==false) 
                        voxelStack.push(tgt::ivec3(val.x,val.y,val.z-1));

            }
       } 


   // draw rectangle 
     float maxSideLength = max(max(dataset_->getDimensions().x,dataset_->getDimensions().y),dataset_->getDimensions().z);
      //xl = (xl / ((float)dataset_->getDimensions().x))* (( 1 / ((float)dataset_->getDimensions().x)) *  maxSideLength)    *2 - 1;
      //xr = (xr / ((float)dataset_->getDimensions().x))* (( 1 / ((float)dataset_->getDimensions().x)) *  maxSideLength)    *2 - 1;


      //yl = (yl / ((float)dataset_->getDimensions().y))* (( 1 / ((float)dataset_->getDimensions().y)) *  maxSideLength)    *2 - 1;
      //yr = (yr / ((float)dataset_->getDimensions().y))* (( 1 / ((float)dataset_->getDimensions().y)) *  maxSideLength)    *2 - 1;

      //zl = (zl / ((float)dataset_->getDimensions().z))* (( 1 / ((float)dataset_->getDimensions().z)) *  maxSideLength)    *2 - 1;
      //zr = (zr / ((float)dataset_->getDimensions().z))* (( 1 / ((float)dataset_->getDimensions().z)) *  maxSideLength)    *2 - 1;

      xl = xl / ((float)dataset_->getDimensions().x); 
      xr = xr / ((float)dataset_->getDimensions().x);

      yl = yl / ((float)dataset_->getDimensions().y); 
      yr = yr / ((float)dataset_->getDimensions().y);

      zl = zl / ((float)dataset_->getDimensions().z); 
      zr = zr / ((float)dataset_->getDimensions().z);
 
      xl = (2.f*xl-1.f)*(dataset_->getDimensions().x / maxSideLength);
      xr = (2.f*xr-1.f)*(dataset_->getDimensions().x / maxSideLength);
 
      yl = (2.f*yl-1.f)*(dataset_->getDimensions().y / maxSideLength);
      yr = (2.f*yr-1.f)*(dataset_->getDimensions().y / maxSideLength);

      zl = (2.f*zl-1.f)*(dataset_->getDimensions().z / maxSideLength);
      zr = (2.f*zr-1.f)*(dataset_->getDimensions().z / maxSideLength);

      
      
      tgt::vec4 llf(xl,yl,zl,1.f);
      tgt::vec4 urb(xr,yr,zr,1.f);

      llf = dataset_->meta().getTransformation() * llf;
      urb = dataset_->meta().getTransformation() * urb;


      MsgDistr.postMessage(new Vec3Msg("set.PickingBoundingBox_lowerLeftFront",tgt::vec3(llf.x,llf.y,llf.z)));
      MsgDistr.postMessage(new Vec3Msg("set.PickingBoundingBox_upperRightBack",tgt::vec3(urb.x,urb.y,urb.z)));
      MsgDistr.postMessage(new BoolMsg("set.PickingBoundingBox_hide",false));


   //floodfill examination
  
      for (int x =0;x<markedVoxels.getDimensions().x;x++)
          for (int y =0;y<markedVoxels.getDimensions().y;y++)
              for (int z =0;z<markedVoxels.getDimensions().z;z++)
                if ( markedVoxels.voxel(x,y,z))
                {
                    count++;
                    sum+= vol->voxel(x,y,z); 
                }
          
       if (count > 0) sum /= count;
       else { 
           sum = 0.f;
           segmentAverageValueLbl_->setText(QString::number(sum));
           return;
       }

   segmentAverageValueLbl_->setText(QString::number(sum));
         
 
    for (int x =0;x<markedVoxels.getDimensions().x;x++)
          for (int y =0;y<markedVoxels.getDimensions().y;y++)
              for (int z =0;z<markedVoxels.getDimensions().z;z++)
                if (markedVoxels.voxel(x,y,z))
                {
                    derivationSum += (sum - vol->voxel(x,y,z))*(sum - vol->voxel(x,y,z));
                }

                derivationSum  = sqrt( (1/count) * derivationSum);
                segmentStandardDerivationValueLbl_->setText(QString::number(derivationSum));
}


void PickingPlugin::setTextureContainer(voreen::TextureContainer* tc){
tc_ = tc;
}



void PickingPlugin::evaluateThreshold(){

  VolumeUInt8* vol = dynamic_cast<VolumeUInt8*>(dataset_);
  float count = 0;
  float sum = 0.f;
  float derivationSum = 0.f;
  
  for (int x =0;x<vol->getDimensions().x;x++)
      for (int y =0;y<vol->getDimensions().y;y++)
          for (int z =0;z<vol->getDimensions().z;z++){
            if ( (thresholdValues_.x < dataset_->getVoxelFloat(x,y,z))  && (dataset_->getVoxelFloat(x,y,z) < thresholdValues_.y))
            {
                count++;
                sum+= vol->voxel(x,y,z); 
            }

          }
      
   if (count > 0) sum /= count;
   else { 
       sum = 0.f;
       return;
   }

   thresholdAverageValueLbl_->setText(QString::number(sum));

 for (int x =0;x<vol->getDimensions().x;x++)
      for (int y =0;y<vol->getDimensions().y;y++)
          for (int z =0;z<vol->getDimensions().z;z++)
          if ( (thresholdValues_.x < dataset_->getVoxelFloat(x,y,z))  && (dataset_->getVoxelFloat(x,y,z) < thresholdValues_.y))
              {
                derivationSum += (sum - vol->voxel(x,y,z))*(sum - vol->voxel(x,y,z));
            }

            derivationSum  = sqrt( (1/count) * derivationSum);

            thresholdStandardDerivationValueLbl_->setText(QString::number(derivationSum));

}



} // namespace voreen
