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

#include "rptmainwindow.h"
#include "ui_aboutbox.h"
#include "rptpropertysetitem.h"
#include "voreen/core/vis/processors/networkserializer.h"
#include "voreen/core/vis/processors/processorfactory.h"
#include "voreen/core/vis/processors/propertyset.h"
#include "voreen/core/vis/processors/volumesetsourceprocessor.h"
#include "voreen/qt/widgets/showtexcontainerwidget.h"
#include "voreen/qt/widgets/widgetgenerator.h"
#include "voreen/qt/widgets/adddialog.h"

#include "voreen/qt/helpbrowser.h"

#include "voreen/core/volume/modality.h"

namespace voreen {

extern NetworkEvaluator* uglyglobalevaluator; // FIXME: Remove after we found a way to access the netev for tooltips

RptMainWindow::RptMainWindow()
    : QMainWindow()
    , ioSystem_(new IOSystem(this))
    , volumeSerializerPopulator_(ioSystem_->getObserver())
{
    setMinimumSize(800, 600);

#ifdef VRN_WITH_DCMTK
    dicomDirDialog_ = 0;
#endif

    // restore settings
    QSettings settings;
    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(1280, 800)).toSize());
    move(settings.value("pos", QPoint(0, 0)).toPoint());
    settings.endGroup();
    settings.beginGroup("Paths");
    networkPath_ = settings.value("network", "../../data/networks").toString();
    //FIXME: datasetPath_ is not used anywhere
    datasetPath_ = settings.value("dataset", "../../data").toString();
    settings.endGroup();

    copyCounter_ = 0;

    setWindowModified(false);

    setWindowTitle(tr("VoreenVE Beta [*]"));
    setWindowIcon(QIcon(":/vrn_app/icons/icon-64.png"));

    camera_ = new tgt::Camera(tgt::vec3(0.f,0.f,3.75f),tgt::vec3(0.f,0.f,0.f),tgt::vec3(0.f,1.f,0.f));

    graphWidget_ = new RptGraphWidget();
    setCentralWidget(graphWidget_);

    graphSerializer_ = new RptNetworkSerializerGui();
    networkserializer_ = new NetworkSerializer();
	evaluator_ = new NetworkEvaluator();
	uglyglobalevaluator = evaluator_; // FIXME: Remove after we found a way to access the netev for tooltips
	MsgDistr.insert(evaluator_);

#ifdef VRN_MODULE_GLYPHS
    datasourceContainer_ = new DataSourceContainer();
#endif

    // initialization of an empty geometry container
    //
    geoContainer_ = new GeometryContainer();
    evaluator_->setGeometryContainer(geoContainer_);

    // VolumeSet issues...
    // the widget containing all currently loaded volumesets must be created
    // BEFORE loading the first dataset on startup.
    //
    volsetContainer_ = new VolumeSetContainer();
    volumeSetWidget_ = new VolumeSetWidget(volsetContainer_, 0, VolumeSetWidget::LEVEL_ALL, Qt::Dialog);
    volumeSetWidget_->hide();

    createMenuAndToolBar();

    // create Canvas Widget before initGL is called
    canvasDock_ = new QDockWidget("Output", this);
    canvasDock_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    painterWidget_ = new RptPainterWidget();
    painterWidget_->eval = evaluator_;

    canvasTab_ = new QTabWidget(this);
    canvasTab_->insertTab(0, painterWidget_ , "Preview");
    canvasDock_->setWidget(canvasTab_);
    canvasDock_->setFeatures(QDockWidget::AllDockWidgetFeatures);
    addDockWidget(Qt::RightDockWidgetArea, canvasDock_);
    MsgDistr.insert(this);

}

RptMainWindow::~RptMainWindow() {
    //delete volumeContainer_;
    delete evaluator_;
    delete transferFuncPlugin_;
#ifdef VRN_MODULE_GLYPHS
    delete datasourceContainer_;
#endif

    delete geoContainer_;
    geoContainer_ = 0;

    delete volsetContainer_;
    volsetContainer_ = 0;
}

void RptMainWindow::openRecentFile() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        std::string file(action->data().toString().toStdString());
        if (file != "")
            openRecentFileSlot(file, QPoint(10, 10));
    }
}

void RptMainWindow::createConnections() {
    connect(graphWidget_, SIGNAL(processorAdded(Identifier, QPoint)), this, SLOT(addProcessorItem(Identifier, QPoint)));
    connect(graphWidget_, SIGNAL(aggregationAdded(std::string, QPoint)), this, SLOT(addAggregationSlot(std::string, QPoint)));
    connect(graphWidget_, SIGNAL(sendProcessor(Processor*, QVector<int>)), this, SLOT(sendProcessorToTable(Processor*, QVector<int>)));
    connect(graphWidget_, SIGNAL(copySignal()), this, SLOT(copyButtonPushed()));
    connect(graphWidget_, SIGNAL(pasteSignal()), this, SLOT(pasteButtonPushed()));
    connect(evaluatorAction_, SIGNAL(triggered()), this, SLOT(evaluatorButtonPushed()));
    connect(openFileAction_,SIGNAL(triggered()),this,SLOT(openFileButtonPushed()));
    connect(connectAction_,SIGNAL(triggered()),this,SLOT(connectButtonPushed()));
    connect(clearAction_,SIGNAL(triggered()),this,SLOT(clearNetwork()));
    connect(openNetworkFileAction_,SIGNAL(triggered()),this,SLOT(openNetworkFileButtonPushed()));
    connect(saveNetworkAction_,SIGNAL(triggered()),this,SLOT(saveNetworkButtonPushed()));
    connect(saveNetworkAsAction_,SIGNAL(triggered()),this,SLOT(saveNetworkAsButtonPushed()));
    connect(insertAggregationAction_, SIGNAL(triggered()), this, SLOT(insertAggregation()));
    connect(deaggregateAction_, SIGNAL(triggered()), this, SLOT(deaggregate()));
    connect(selectAllAction_, SIGNAL(triggered()), this, SLOT(selectAll()));
    connect(showAggregationContentAction_, SIGNAL(triggered()), this, SLOT(showAggregationContent()));
    connect(setReuseTargetsAction_, SIGNAL(triggered()), this, SLOT(setReuseTargets()));
    connect(copyAction_, SIGNAL(triggered()), this, SLOT(copyButtonPushed()));
    connect(pasteAction_, SIGNAL(triggered()), this, SLOT(pasteButtonPushed()));
    connect(createPropSetAction_, SIGNAL(triggered()), this, SLOT(createPropertySet()));
    connect(processorListWidget_, SIGNAL(itemSelectionChanged()), this, SLOT(changeProcessorInfo()));
    // iterate over recent files
    for (int i = 0; i < maxRecents; ++i)
        connect(recentFileActs[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));
    // if item is double clicked, show properties
    connect(graphWidget_, SIGNAL(showPropertiesSignal()), this, SLOT(showProperties()));

    // signals indicating a change in network
    connect(graphWidget_, SIGNAL(processorAdded(Identifier, QPoint)), this, SLOT(networkModified()));
    connect(graphWidget_, SIGNAL(pasteSignal()), this, SLOT(networkModified()));
    connect(graphWidget_, SIGNAL(aggregationAdded(std::string, QPoint)), this, SLOT(networkModified()));
    connect(insertAggregationAction_, SIGNAL(triggered()), this, SLOT(networkModified()));
    connect(deaggregateAction_, SIGNAL(triggered()), this, SLOT(networkModified()));
    connect(pasteAction_, SIGNAL(triggered()), this, SLOT(networkModified()));
    connect(createPropSetAction_, SIGNAL(triggered()), this, SLOT(networkModified()));
    connect(insertAggregationAction_, SIGNAL(triggered()), this, SLOT(networkModified()));
    connect(deaggregateAction_, SIGNAL(triggered()), this, SLOT(networkModified()));
}

int RptMainWindow::findProcessor(std::vector<RptProcessorItem*> vector, Processor* processor) {
	for (size_t i=0; i< vector.size(); i++) {
		if (processor == vector.at(i)->getProcessor())
			return i;
	}
	return -1;
}

void RptMainWindow::networkModified() {
        setWindowModified(true);
}

void RptMainWindow::copyButtonPushed() {
    //These are all used to store information about the part of the network that is to be copied.
    //We have to clear them everytime before copying.
    for (size_t i=0; i < copyPasteProcessors_.size(); i++) {
      delete copyPasteProcessors_[i];
    }
    copyPasteProcessors_.clear();
    copyPastePropertySets_.clear();
    copyPasteAggregations_.clear();
    copyPasteConnectionInfos_.clear();
    aggregationMap_.clear();
    aggregationNameMap_.clear();
    propertySetMap_.clear();
    propertySetNameMap_.clear();
    propertySetAggregationMap_.clear();
    numberOfAggregations_ = 0;
    numberOfPropertySets_ = 0;

    copyCounter_ = 0;

    //Get all the items that are to be copied.
    QList<QGraphicsItem*> selectedItems = graphWidget_->scene()->selectedItems();

    //Now check what type of items these are and put them in the appropriate vectors.
    for (int i=0; i<selectedItems.size(); i++) {
      if ( selectedItems.at(i)->type() == RptProcessorItem::Type ) {
          copyPasteProcessors_.push_back(static_cast<RptProcessorItem*>(selectedItems.at(i)));
      }
      else if (selectedItems.at(i)->type() == RptAggregationItem::Type ) {
          copyPasteAggregations_.push_back(static_cast<RptAggregationItem*>(selectedItems.at(i)));
      }
      else if (selectedItems.at(i)->type() == RptPropertySetItem::Type) {
          copyPastePropertySets_.push_back(static_cast<RptPropertySetItem*>(selectedItems.at(i)));
      }
    }

	//Go through every aggregation and do some stuff
	for (size_t i=0; i < copyPasteAggregations_.size(); i++) {
		RptAggregationItem* currentItem = copyPasteAggregations_.at(i);
		std::vector<RptProcessorItem*> currentVector = currentItem->getProcessorItems();
		std::vector<int> processorItemsInAggregation;

		//The aggregetionMap saves the information which processors are in which aggregation. Insert
		//the current aggregation number into the map as the key, and the vector as the value, which is empty in the beginning
		aggregationMap_.insert(std::pair<int, std::vector<int> >(numberOfAggregations_,processorItemsInAggregation));

		//now go through the processorItems in the aggregation and put them into the processors vector (all processors in that
		//vector will be copied) and their numbers into the aggregationMap. This number is the position the processor has in the
		//copyPasteProcessors_ vector.
		for (size_t j=0; j<currentVector.size(); j++) {

			//put the processor into the vector that holds all processors that are to be copied
			copyPasteProcessors_.push_back(currentVector.at(j));

			//The processor now has a number (its position in the copyPasteProcessors_ vector), and we store
			//that number in the vector in the aggregationMap.
			aggregationMap_[numberOfAggregations_].push_back(copyPasteProcessors_.size()-1);

			//Do the same for the aggregations name. (Yes, not nice, we could/should use maybe a struct, so that we only need one map)
			aggregationNameMap_.insert(std::pair<int,std::string>(numberOfAggregations_,currentItem->getName() ) );
		}

		numberOfAggregations_++;
	}

	//now save information about propertysets
	for (size_t i=0; i < copyPastePropertySets_.size(); i++) {

		//get all the guiItems to which the propertyset is connected
		std::vector<RptGuiItem*> guiItems = copyPastePropertySets_.at(i)->getGuiItems();

		//These two vectors hold the processors and the aggregations this propertyset is connected to.
		//They have to be handled differently, that's why we use two vectors.
		std::vector<int> currentProcessorVector;
		std::vector<int> currentAggregationVector;

		//Insert these two vectors into the two maps that hold the information to which processors and
		//aggregations the propertyset is connected to.
		propertySetMap_.insert(std::pair<int,std::vector<int> >(numberOfPropertySets_,currentProcessorVector) );
		propertySetAggregationMap_.insert(std::pair<int,std::vector<int> >(numberOfPropertySets_,currentAggregationVector) );

		//Now go through the guiItems and insert them into the appropriate vector, depending on their type (processor or aggregation)
		for (size_t j=0; j<guiItems.size(); j++) {

			//Is it a processor?
			RptProcessorItem* temp = dynamic_cast<RptProcessorItem*>(guiItems.at(j));
			if (temp) {

				//If yes, search for the processor in the copyPasteProcessors_ vector and return its position
				int number=findProcessor(copyPasteProcessors_,temp->getProcessor());
				if (number!= -1)

					//If that worked, we put that number into the vector in the map, thereby saving that this
					//processor was a member of the propertyset.
					propertySetMap_[numberOfPropertySets_].push_back(number);
			} else {

				//If it's not a processor, check if it's an aggregation.
				RptAggregationItem* temp2 = dynamic_cast<RptAggregationItem*>(guiItems.at(j));
				if (temp2) {
					int number=-1;

					//If yes, search for the aggregation in the copyPasteAggregations_ vector and get
					//its position.
					for (size_t k=0; k<copyPasteAggregations_.size(); k++) {
						if (copyPasteAggregations_.at(k) == temp2) {
							number=k;
							break;
						}
					}
					if (number != -1) {
						//If that worked, we save that position in the vector in the map, thereby saving that this
						//aggregation was a member of the propertyset.
						propertySetAggregationMap_[numberOfPropertySets_].push_back(number);
					}
				}
			}
		}
		numberOfPropertySets_++;
	}

	//Now every processor that is to be copied is in the copyPasteProcessors_ vector. (We had to put the
	//ones that were in aggregations in here, too). So now we can start saving the information about their
	//connections.
	for (size_t i=0; i<copyPasteProcessors_.size(); i++) {

		//Create a new object holding all the connection information for the current processor.
		ConnectionInfoForOneProcessor* currentInfo = new ConnectionInfoForOneProcessor();

		//The id this processor gets is its position in the copyPasteProcessors_ vector.
		currentInfo->id = i;

		//The vector that will hold all the input connections. We store the input connections and not the
		//output connections, because the order here is important.
		std::vector<PortConnection*> inputConnections;

		Processor* currentProcessor = copyPasteProcessors_.at(i)->getProcessor();

		//Go through all the inports and save the connection for each one
		for (size_t j=0; j<currentProcessor->getInports().size(); j++) {
			Port* currentPort = currentProcessor->getInports().at(j);

			//For every port we create a new PortConnection object.
			PortConnection* newConnection = new PortConnection();

			//Save the type of the port.
			newConnection->type=currentPort->getType();

			//Get all the ports that are connected to this one, because we have to save that.
			std::vector<Port*> connectedPorts = currentPort->getConnected();
			for (size_t k=0; k<connectedPorts.size(); k++) {

				//Get the processor of the connected port and find its position in the copyPasteProcessors_ vector, which is its id.
				int connectedProcessorNumber = findProcessor(copyPasteProcessors_,connectedPorts.at(k)->getProcessor());

				//if -1, the connectedProcessor was not copied, so we can't save that connection.
				if (connectedProcessorNumber != -1) {
					//save the id of the connected processor and the type of the connected port in the vector of the PortConnection object.
					ConnectedProcessor connectedProcessor;
					connectedProcessor.processorNumber = connectedProcessorNumber;
					connectedProcessor.portType = connectedPorts.at(k)->getType();
					newConnection->connectedProcessorsVector.push_back(connectedProcessor);
				}
			}

			//now put that PortConnection object into the vector holding all the input connections.
			inputConnections.push_back(newConnection);
		}

		//The exact same thing has to be done for the coprocessor inports. (Works just like above).
		for (size_t j=0; j<currentProcessor->getCoProcessorInports().size(); j++) {
			Port* currentPort = currentProcessor->getCoProcessorInports().at(j);
			PortConnection* newConnection = new PortConnection();
			newConnection->type=currentPort->getType();
			std::vector<Port*> connectedPorts = currentPort->getConnected();
			for (size_t k=0; k<connectedPorts.size(); k++) {
				int connectedProcessorNumber = findProcessor(copyPasteProcessors_,connectedPorts.at(k)->getProcessor());
				if (connectedProcessorNumber != -1) {
					ConnectedProcessor connectedProcessor;
					connectedProcessor.processorNumber = connectedProcessorNumber;
					connectedProcessor.portType = connectedPorts.at(k)->getType();
					newConnection->connectedProcessorsVector.push_back(connectedProcessor);
				}
			}
			inputConnections.push_back(newConnection);
		}

		currentInfo->inputs = inputConnections;
		copyPasteConnectionInfos_.push_back(currentInfo);
	}

    //Everything is duplicated, so assign the vector holding the duplicated processorItems to the copyPasteProcessors_ vector.
    //These processorItems are then added to the scene and connected as soon as the paste button is pushed.
    copyPasteProcessors_ = copyProcessors();

    // FIXME: reuseTCTargets is actually not necessary here and should not be set
    //graphSerializer_->serializeToXml(copyPasteProcessors_,copyPasteAggregations_,copyPastePropertySets_,false,"temp/cp.vnw");
}

std::vector<RptProcessorItem*> RptMainWindow::copyProcessors() {
    //Now we have all the connection information and can start duplicating the processor objects.
    //This vector will hold all the duplicated processorItems.
    std::vector<RptProcessorItem*> createdProcessors;
    //Go through all processors and duplicate them. We can't just use a copy constructor, because the messages
    //that are sent when changing a property wouldn't be sent, resulting in incorrect behavior.
    for (size_t i=0; i < copyPasteProcessors_.size();i++) {
        RptProcessorItem* newProcessorItem = new RptProcessorItem(copyPasteProcessors_.at(i)->getType());
        newProcessorItem->setPos(copyPasteProcessors_.at(i)->pos());
        newProcessorItem->moveBy(8 * copyCounter_, 8 * copyCounter_);

        Processor* currentProcessor = copyPasteProcessors_.at(i)->getProcessor();
        Processor* newProcessor = newProcessorItem->getProcessor();
        newProcessor->setTextureContainer(tc_);
        newProcessor->setCamera(camera_);

        std::vector<Property*> props = newProcessor->getProperties();

        for (size_t j=0; j<currentProcessor->getProperties().size(); j++) {
            Property* currentProp = currentProcessor->getProperties().at(j);
            int property_type = currentProp->getType();

            switch(property_type) {
                    case Property::FLOAT_PROP : {
                        try {
                            FloatProp* oldProp = dynamic_cast<FloatProp*>(currentProp);
                            FloatProp* prop = dynamic_cast<FloatProp*>(props.at(j));

                            if (prop) {
                                prop->set(oldProp->get());
                                newProcessor->postMessage(new FloatMsg(prop->getIdent(), oldProp->get()));
                            }
                        }
                        catch (const std::bad_cast& /*ex*/) {}
                        break;
                                                }
                    case Property::INT_PROP : {
                        try{
							IntProp* oldProp = dynamic_cast<IntProp*>(currentProp);
                            IntProp* prop = dynamic_cast<IntProp*>(props.at(j));

							if (prop) {
								prop->set(oldProp->get() );
								newProcessor->postMessage(new IntMsg(prop->getIdent(), oldProp->get()));
							}
						} catch (const std::bad_cast& /*ex*/) {
							//std::cout<< "args tot";
                        }
                        break;
                            }
											  //TODO: Check if this is working correctly
                    case Property::BOOL_PROP : {
                        try{
                            // first check if the property is a condition controller
                            // cause then it is a frame widget with checkbox
                            // otherwise normal bool plugin
                            if (props.at(j)->isConditionController())
                            {
								BoolProp* oldProp = dynamic_cast<BoolProp*>(currentProp);
                                BoolProp* prop = dynamic_cast<BoolProp*>(props.at(j));

								if(oldProp->get() ==1)
									if (prop) {
									   prop->set(true);
									   newProcessor->postMessage(new BoolMsg(prop->getIdent(), true));
									}
                                else
									if (prop) {
										prop->set(false);
										newProcessor->postMessage(new BoolMsg(prop->getIdent(), false));
									}
                            } else {
								BoolProp* oldProp = dynamic_cast<BoolProp*>(currentProp);
                                BoolProp* prop = dynamic_cast<BoolProp*>(props.at(j));

								if(oldProp->get()==1) {
									if (prop) {
										prop->set(true);
										newProcessor->postMessage(new BoolMsg(prop->getIdent(), true));
									}
								} else
									if (prop) {
									   prop->set(false);
									   newProcessor->postMessage(new BoolMsg(prop->getIdent(), false));
									}
							}
                        } catch (const std::bad_cast& /*ex*/) {

                        }
                            break;
                         }
                    case Property::COLOR_PROP : {
                        try{
							ColorProp* oldProp = dynamic_cast<ColorProp*>(currentProp);
                            ColorProp* prop = dynamic_cast<ColorProp*>(props.at(j));

							if (prop) {
								prop->set(oldProp->get());
								newProcessor->postMessage(new ColorMsg(prop->getIdent(), prop->get()));
							}
                        } catch (const std::bad_cast& /*ex*/) {

                        }
                       break;
                            }
                    case Property::ENUM_PROP : {
                        try{
							EnumProp* oldProp = dynamic_cast<EnumProp*>(currentProp);
                            EnumProp* prop = dynamic_cast<EnumProp*>(props.at(j));

							if (prop) {
								prop->set(oldProp->get() );
								if (prop->getSendStringMsg() ) {
									//newProcessor->postMessage(new StringMsg(prop->getIdent(), prop->getStrings().at(oldProp->get())));
								}
								else {
									newProcessor->postMessage(new IntMsg(prop->getIdent(), oldProp->get()));
								}
							}
                        } catch (const std::bad_cast& /*ex*/) {
                        }
                        break;
                            }
                    case Property::TRANSFUNC_PROP : {
                         try{
							 TransFuncProp* oldProp = dynamic_cast<TransFuncProp*>(currentProp);
							 TransFuncProp* prop = dynamic_cast<TransFuncProp*>(props.at(j));
							 TransFunc* temp = oldProp->get();
							 TransFuncIntensityKeys* oldTransFerFunc = dynamic_cast<TransFuncIntensityKeys*>(temp);
							 TransFuncIntensityKeys* tf = new TransFuncIntensityKeys();
							 tf->clearKeys();
							 for (size_t k=0; k<oldTransFerFunc->getKeys().size(); k++) {
								 TransFuncMappingKey* newKey = new TransFuncMappingKey(oldTransFerFunc->getKeys().at(k)->getIntensity(),oldTransFerFunc->getKeys().at(k)->getColorR());
								 newKey->setAlphaL(oldTransFerFunc->getKeys().at(k)->getAlphaL() );
								 newKey->setAlphaR(oldTransFerFunc->getKeys().at(k)->getAlphaR() );
								 newKey->setColorL(oldTransFerFunc->getKeys().at(k)->getColorL() );
								 newKey->setColorR(oldTransFerFunc->getKeys().at(k)->getColorR() );
								 tf->addKey(newKey);
							 }
							 tf->updateTexture();
							 prop->set(tf);
                             newProcessor->postMessage(new TransFuncPtrMsg(VolumeRenderer::setTransFunc_, tf) );

						 } // if (!pElem->NoChildren())
                         catch (const std::bad_cast& /*ex*/) {

                         }
                        break;
                            }
                    case Property::INTEGER_VEC2_PROP : {
                        try{
							IntVec2Prop* oldProp = dynamic_cast<IntVec2Prop*>(currentProp);
                            IntVec2Prop* prop = dynamic_cast<IntVec2Prop*>(props.at(j));

							if (prop) {
								prop->set(oldProp->get() );
								newProcessor->postMessage(new IVec2Msg(prop->getIdent(), oldProp->get()));
							}
                        } catch (const std::bad_cast& /*ex*/) {

                        }
                        break;
                            }
                    case Property::INTEGER_VEC3_PROP : {
                        try{
							IntVec3Prop* oldProp = dynamic_cast<IntVec3Prop*>(currentProp);
                            IntVec3Prop* prop = dynamic_cast<IntVec3Prop*>(props.at(j));

							if (prop) {
								prop->set(oldProp->get());
								newProcessor->postMessage(new IVec3Msg(prop->getIdent(), prop->get()));
							}
                        } catch (const std::bad_cast& /*ex*/) {
						}
                        break;
													   }
                    case Property::INTEGER_VEC4_PROP : {
                        try{
							IntVec4Prop* oldProp = dynamic_cast<IntVec4Prop*>(currentProp);
                            IntVec4Prop* prop = dynamic_cast<IntVec4Prop*>(props.at(j));

							if (prop) {
								prop->set(oldProp->get());
								newProcessor->postMessage(new IVec4Msg(prop->getIdent(), prop->get()));
							}
                        } catch (const std::bad_cast& /*ex*/) {

                        }
                        break;
													   }
                    case Property::FLOAT_VEC2_PROP : {
                        try{
							FloatVec2Prop* oldProp = dynamic_cast<FloatVec2Prop*>(currentProp);
                            FloatVec2Prop* prop = dynamic_cast<FloatVec2Prop*>(props.at(j));

							if (prop) {
								prop->set(oldProp->get() );
								newProcessor->postMessage(new Vec2Msg(prop->getIdent(), prop->get()));
							}
                        } catch (const std::bad_cast& /*ex*/) {
                        }
                        break;
                            }
                    case Property::FLOAT_VEC3_PROP : {
                        try{
                            FloatVec3Prop* oldProp = dynamic_cast<FloatVec3Prop*>(currentProp);
							FloatVec3Prop* prop = dynamic_cast<FloatVec3Prop*>(props.at(j));

							if (prop) {
								prop->set(oldProp->get() );
								newProcessor->postMessage(new Vec3Msg(prop->getIdent(), prop->get()));
							}
                        } catch (const std::bad_cast& /*ex*/) {
                        }
                        break;
                            }
                    case Property::FLOAT_VEC4_PROP : {
                        try{
                            FloatVec4Prop* oldProp = dynamic_cast<FloatVec4Prop*>(currentProp);
							FloatVec4Prop* prop = dynamic_cast<FloatVec4Prop*>(props.at(j));

							if (prop) {
								prop->set(oldProp->get() );
								newProcessor->postMessage(new Vec4Msg(prop->getIdent(), prop->get()));
							}
                        } catch (const std::bad_cast& /*ex*/) {
                        }
                        break;
                            }
				} // switch(property_type)
		}

		createdProcessors.push_back(newProcessorItem);
	}
    copyCounter_++;
    return createdProcessors;

}

void RptMainWindow::pasteButtonPushed() {

    std::vector<RptProcessorItem*> createdProcessors = copyProcessors();

    //go through all processors that were previously duplicated and add them to the scene, insert them into the MsgDistr etc.
    for (size_t i=0; i< createdProcessors.size(); i++) {
        RptProcessorItem* newProcessorItem = createdProcessors.at(i);
        processors_.push_back(newProcessorItem);
        //insert them into the pastedProcessors_ vector aswell, we still have to be able to tell them apart from
        //the "old" processors.
        pastedProcessors_.push_back(newProcessorItem);

        //add them to the scene
        graphWidget_->addItem(newProcessorItem);
        MsgDistr.insert(newProcessorItem->getProcessor());

        //connect them to some functions, so that they can be deleted and aggregated etc.
        connect(newProcessorItem, SIGNAL(deleteSignal()), this, SLOT(deleteSlot()));
        connect(newProcessorItem, SIGNAL(aggregateSignal()), this, SLOT(insertAggregation()));
        connect(newProcessorItem, SIGNAL(changed()), this, SLOT(networkModified()));

    }

    //Now that they are added, they can be connected.

    RptProcessorItem* currentGuiItem;
    ConnectionInfoForOneProcessor* currentInfo;

    //Go through the connection information created when the copy button was pushed and connect
    //the processorItems.
    for (size_t i=0;i < copyPasteConnectionInfos_.size();i++) {

        //Each element in copyPasteConnectionInfos_ stores the connection info for one ProcessorItem
        //This ProcessorItem is searched by its id (its position in the pastedProcessors_) vector, which
        //is the same as its position in the copyPasteProcessors_ vector in the copyButtonPushed() function,
        //so this actually works)
        currentInfo = copyPasteConnectionInfos_.at(i);
        currentGuiItem = pastedProcessors_.at(currentInfo->id);

        if (!currentGuiItem)
            return;
        else {

        //Get the input connection informations for this processor
        std::vector<PortConnection*> in;
        in=currentInfo->inputs;

        //iterate throuh it and connect the ProcessorItems
        for (size_t j=0;j<in.size();j++) {
				PortConnection* con = in.at(j);

				//search the portItem of this processorItem, that is to be connected
				if (currentGuiItem->getPortItem(con->type) ) {

					//If we found that portItem, remember it.
					RptPortItem* destPort = currentGuiItem->getPortItem(con->type);

					//Get the vector containing the information to which ports of which processors this port is to be connected
					std::vector<ConnectedProcessor> connectedProcessors = con->connectedProcessorsVector;

					for (size_t k=0; k<connectedProcessors.size(); k++) {

						//The processorNumber was the position of the processor in the copyPasteProcessors_ vector, and the same
						//processor is at the same position in the pastedProcessors_ vector, so this works.
						RptProcessorItem* source = pastedProcessors_.at(connectedProcessors.at(k).processorNumber);

						//Now that we have the processor, get the port with the correct type
						RptPortItem* sourcePort = source->getPortItem(connectedProcessors.at(k).portType);
						if (sourcePort) {

							//We have everything we need and can connect the two processors and create the arrow
							source->connectAndCreateArrow(sourcePort,destPort);
						}
					}
				}
			}
		}
	} // for loop through copyPasteConnectionInfos

	//All the processors are created and connected, now aggregate them (if they were aggregated when they were copied)

	//this vector will hold all the aggregations we create now
	std::vector<RptAggregationItem*> createdAggregations;

	//Go through the map containing the information which processors should be aggregated
	for(std::map<int,std::vector<int> >::iterator i = aggregationMap_.begin(); i != aggregationMap_.end(); ++i) {

		//All the processors that are to be aggregated
		std::vector<RptProcessorItem*> aggregationMembers;

		//The numbers (positionNumbers) of the processors to be aggregated
		std::vector<int> processorNumbers = i->second;

		//Get all those processors and put them into the vector
		for (size_t j=0; j<processorNumbers.size(); j++) {

			//Just like farther above, the processorNumber indicates the processors position in the vector.
			aggregationMembers.push_back(pastedProcessors_.at(processorNumbers.at(j)));
		}

		//We have the processors to be aggregated, so do it.
		createAggregation(aggregationMembers,aggregationNameMap_[i->first] );

		//Save the created aggregation in a vector. This is important when creating the
		//propertysets.
		createdAggregations.push_back(aggregations_.at(aggregations_.size() -1));
	}

	//Now we can create the propertysets. This has to be done last, because the propertysets
	//might be connected to aggregations, so those had to be created first.

	for (int i=0; i< numberOfPropertySets_; i++) {

		//The vector containing all the guiItems (processorItems and aggregationItems)
		//that are to be connected to the propertyset
		std::vector<RptGuiItem*> currentVector;

		//The numbers (positionnumbers) of the processors that are to become a member
		//of this aggregation
		std::vector<int> processorNumbers = propertySetMap_[i];

		//Get all those processors and push them into the vector
		for(size_t j=0; j< processorNumbers.size(); j++) {
			currentVector.push_back(copyPasteProcessors_.at(processorNumbers.at(j) ) );
		}

		//The numbers (positionNumbers) of the aggregations that are to become a member
		//of this aggregation. Just like with processors, the number is the position in a vector,
		//thats why we had to remember them in a special vector when we created them.
		std::vector<int> aggregationNumbers = propertySetAggregationMap_[i];

		for (size_t j=0; j<aggregationNumbers.size(); j++) {
			currentVector.push_back(createdAggregations.at(aggregationNumbers.at(j) ) );
		}

		//Now we know all the members, so we can create the propertyset.
		if (currentVector.size() > 0) {
			RptPropertySetItem* propSetGuiItem = new RptPropertySetItem(currentVector,graphWidget_->getScene());
			propertySets_.push_back(propSetGuiItem);
			connect(propSetGuiItem, SIGNAL(deleteSignal()), this, SLOT(deleteSlot()));
		}
	}

	createdAggregations.clear();
	pastedProcessors_.clear();
}

/*
Volume* RptMainWindow::loadDataset(const std::string& filename) {
    VolumeSerializer* volumeSerializer = volumeSerializerPopulator_.getVolumeSerializer();

	ioSystem_->show();
	VolumeContainer* temp = 0;//volumeSerializer->load(filename);
	ioSystem_->hide();

    // TODO: remove this if VolumeContainer is about to be eliminated and the
    // serializers return VolumeSets*.
    //
    // ---------------------
    VolumeSet* volumeSet = volsetContainer_->insertContainer(temp, false);
    volumesetWidget_->updateContent();

    // do not delete temp anymore! this would cause the volume* contained
    // to become destroyed!
    //delete temp;
    if( volumeSet != 0 )
        return volumeSet->getFirstVolume();
    
    return 0;
    // ---------------------
}*/

Volume* RptMainWindow::loadDataset(const std::string& filename) {
    if( volumeSetWidget_ == 0 ) {
        printf("\tERROR: no VolumeSetWidget for loading Datasets was created!\n");
        return 0;
    }

    VolumeSet* volumeSet = volumeSetWidget_->loadVolumeSet(filename);

    if( volumeSet != 0 )
        return volumeSet->getFirstVolume();
    
    return 0;
}

void RptMainWindow::clearDataVectors()
{
    // Delete all processors FIRST
    //
    for( size_t i = 0; i < processors_.size(); i++ )
    {
        delete processors_[i];
        processors_[i] = 0;
    }
    processors_.clear();
    evaluator_->getProcessors().clear();

    // Delete all aggregations afterwards. Not doing this will crash the
    // crash the application. The reason therefore is still unknown to me.
    // (Dirk)
    //
    for( size_t i = 0; i < aggregations_.size(); i++ )
    {
        delete aggregations_[i];
        aggregations_[i] = 0;
    }
    aggregations_.clear();

    // Delete all property sets
    //
    for( size_t i = 0; i < propertySets_.size(); i++ )
    {
        delete propertySets_[i];
        propertySets_[i] = 0;
    }
    propertySets_.clear();
}

void RptMainWindow::clearNetwork() {
    if( isWindowModified() ) {
        QMessageBox msgBox;
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText("Save changes for the current network?");
        if( msgBox.exec() == QMessageBox::Yes )
        {
            saveNetworkAsButtonPushed();
            return;
        }
    }
    // clear all vectors containing network related items
    //
    clearDataVectors();

    // clear containers from existing parts of
    // previously rendered networks
    //
    geoContainer_->clearDeleting();

    graphWidget_->clearScene();
    currentFile_ = "";
    setWindowTitle(tr("%1[*] - %2").arg(tr("VoreenVE Beta")).arg(QString(currentFile_.c_str())));
    setWindowModified(false);
}

void RptMainWindow::finishOpen(NetworkInfos* infos, QPoint pos, bool clearScene) {
    if (clearScene) {
        // Ask user whether to save the network or not
        if (isWindowModified()) {
            QMessageBox msgBox;
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setIcon(QMessageBox::Question);
            msgBox.setText("Save changes for the current network?");
            if (msgBox.exec() == QMessageBox::Yes)
                saveNetworkAsButtonPushed();
        }
        addCurrentFileToRecents();
        setWindowTitle(tr("%1[*] - %2").arg(tr("VoreenVE Beta")).arg(QString(currentFile_.c_str())));

        // clear all vectors containing network related items
        clearDataVectors();

        // clear containers from existing parts of
        // previously rendered networks
        geoContainer_->clearDeleting();
        graphWidget_->clearScene();

        //clearNetwork(); // this method performs all the code from above! (dirk) 
                          // yes, but it additionally resets the current file stuff. this is not wanted here! (frank)
    }

    std::vector<RptProcessorItem*> processors = infos->processorItems;
    for (size_t i=0; i< processors.size(); i++) {
        graphWidget_->addItem(processors.at(i));

        processors.at(i)->getProcessor()->setTextureContainer(tc_);
        processors.at(i)->getProcessor()->setCamera(camera_);

        processors_.push_back(processors.at(i));
// FIXME: this should be task of the processor itself, shouldn't? (dirk)
//
        if (!MsgDistr.contains(processors[i]->getProcessor()))
            MsgDistr.insert(processors[i]->getProcessor());
        connect(processors[i], SIGNAL(deleteSignal()), this, SLOT(deleteSlot()));
        connect(processors[i], SIGNAL(aggregateSignal()), this, SLOT(insertAggregation()));
    }
    if (infos->version == 0)
        graphSerializer_->connectGuiItems();
    else if (infos->version == 1)
        graphSerializer_->connectGuiItemsVersion1();
        // FIXME: Maybe I do not always want this eg. copy and paste of partial networks
        setReuseTargetsAction_->setChecked(infos->reuseTCTargets);
        setReuseTargets();
    if (infos->aggroMap.size()>0) {
        if ((pos.x() == 10000) && (pos.y() == 10000))
            createLoadedAggregations(infos->aggroMap,infos->aggroNameMap);
        else
            createLoadedAggregations(infos->aggroMap,infos->aggroNameMap, pos);
    }
    for (size_t i=0; i< infos->propertySetInfos.size(); i++) {
        PropertySetInfos* currentPropSetInfo = infos->propertySetInfos.at(i);
        std::vector<RptGuiItem*> guiItems;
        for (size_t j=0; j<currentPropSetInfo->processorItems.size(); j++) {
            guiItems.push_back(currentPropSetInfo->processorItems.at(j));
        }
        RptPropertySetItem* propSetGuiItem = new RptPropertySetItem(guiItems,graphWidget_->getScene());
        propSetGuiItem->setPos(QPointF(currentPropSetInfo->xpos,currentPropSetInfo->ypos));
        propSetGuiItem->adjustArrows();
        propertySets_.push_back(propSetGuiItem);
        connect(propSetGuiItem, SIGNAL(deleteSignal()), this, SLOT(deleteSlot()));
    }

#ifdef VRN_MODULES_GLYPHS
    //propagate glyphprototype to processors (needed when glyphs are used in the network)
    MsgDistr.postMessage(new GlyphPrototypePtrMsg(Identifier("set.glyphPrototype"), glyphsPlugin_->getPrototype()));
#endif

    // also propagate the VolumeSetcontainer* to all VolumeSetSourceProcessors
    //
    MsgDistr.postMessage(new VolumeSetContainerMsg(VolumeSetContainer::msgSetVolumeSetContainer_, volsetContainer_), "VolumeSetSourceProcessor");

    //adjust scaling
    scaleView(1.2f);

    // placed here, because loading a network emits changed signals
    setWindowModified(false);
}

void RptMainWindow::finishOpen(RptNetwork& rptnet, QPoint, bool clearScene) {
    if (clearScene) {
        // Ask user whether to save the network or not
        if (isWindowModified()) {
            QMessageBox msgBox;
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setIcon(QMessageBox::Question);
            msgBox.setText("Save changes for the current network?");
            if (msgBox.exec() == QMessageBox::Yes) {
                saveNetworkAsButtonPushed();
            }
        }
        addCurrentFileToRecents();
        setWindowTitle(tr("%1[*] - %2").arg(QString(currentFile_.c_str())).arg(tr("VoreenVE Beta")));

        // clear all vectors containing network related items
        clearDataVectors();
    }

    // clear containers from existing parts of
    // previously rendered networks
    geoContainer_->clearDeleting();
    graphWidget_->clearScene();

    // Add the Items to the scene
    // ProcessorsItems
    std::vector<RptProcessorItem*> processorItems = rptnet.processorItems;
    for (size_t i=0; i< processorItems.size(); i++) {
        graphWidget_->addItem(processorItems.at(i));
        processorItems.at(i)->showAllArrows();

        processorItems.at(i)->getProcessor()->setTextureContainer(tc_);
        processorItems.at(i)->getProcessor()->setCamera(camera_);

        processors_.push_back(processorItems.at(i));
// FIXME: this should be task of the processor itself, shouldn't? (dirk)
//
        if (!MsgDistr.contains(processorItems[i]->getProcessor()))
            MsgDistr.insert(processorItems[i]->getProcessor());
        connect(processorItems[i], SIGNAL(deleteSignal()), this, SLOT(deleteSlot()));
        connect(processorItems[i], SIGNAL(aggregateSignal()), this, SLOT(insertAggregation()));
    }
    // PropertySetItems
    for (size_t i=0; i< rptnet.propertySetItems.size(); i++) {
        RptPropertySetItem* propertySetItem = rptnet.propertySetItems.at(i);
        graphWidget_->addItem(propertySetItem);
        propertySetItem->showAllArrows();
        propertySets_.push_back(propertySetItem);
        connect(propertySetItem, SIGNAL(deleteSignal()), this, SLOT(deleteSlot()));
    }
    // Aggregations
    for (size_t i=0; i< rptnet.aggregationItems.size(); i++) {
        RptAggregationItem* aggregationItem = rptnet.aggregationItems.at(i);
        // Hack to make it work now - show processors and add them as if not aggregated FIXME
        for (size_t j=0; j< aggregationItem->getProcessorItems().size(); j++) {
            graphWidget_->addItem(aggregationItem->getProcessorItems().at(j));
            aggregationItem->getProcessorItems().at(j)->showAllArrows();
            aggregationItem->getProcessorItems().at(j)->getProcessor()->setTextureContainer(tc_);
            aggregationItem->getProcessorItems().at(j)->getProcessor()->setCamera(camera_);
            if (!MsgDistr.contains(aggregationItem->getProcessorItems().at(j)->getProcessor()))
                MsgDistr.insert(aggregationItem->getProcessorItems().at(j)->getProcessor());
            connect(aggregationItem->getProcessorItems().at(j), SIGNAL(deleteSignal()), this, SLOT(deleteSlot()));
            connect(aggregationItem->getProcessorItems().at(j), SIGNAL(aggregateSignal()), this, SLOT(insertAggregation()));
        }
        graphWidget_->addItem(aggregationItem);
        aggregationItem->initialize(); // Belongs to the hack above
        aggregations_.push_back(aggregationItem);
        connect(aggregationItem, SIGNAL(deaggregateSignal()), this, SLOT(deaggregate()));
        connect(aggregationItem, SIGNAL(deleteSignal()), this, SLOT(deleteSlot()));
        connect(aggregationItem, SIGNAL(saveSignal(RptAggregationItem*)), this, SLOT(saveAggregationSlot(RptAggregationItem*)));
    }

    setReuseTargetsAction_->setChecked(rptnet.reuseTCTargets);
    setReuseTargets();

#ifdef VRN_MODULES_GLYPHS
    //propagate glyphprototype to processors (needed when glyphs are used in the network)
    MsgDistr.postMessage(new GlyphPrototypePtrMsg(Identifier("set.glyphPrototype"), glyphsPlugin_->getPrototype()));
#endif

    // also propagate the VolumeSetcontainer* to all VolumeSetSourceProcessors
    //
    MsgDistr.postMessage(new VolumeSetContainerMsg(VolumeSetContainer::msgSetVolumeSetContainer_, volsetContainer_), "VolumeSetSourceProcessor");

    //adjust scaling
    scaleView(1.2f);

    // placed here, because loading a network emits changed signals
    setWindowModified(false);
}

void RptMainWindow::scaleView(float maxFactor) {
    // justify graphWidget
    QRectF sceneRect = graphWidget_->getScene()->itemsBoundingRect();
    graphWidget_->getScene()->setSceneRect(sceneRect);
    // calculate scale factors
    float scaleFactor = std::min((float)(graphWidget_->width() / sceneRect.width()),
                                 (float)(graphWidget_->height() / sceneRect.height()));
    scaleFactor = std::min(scaleFactor, maxFactor);
    // scale smaller dimension
    graphWidget_->scale(scaleFactor, scaleFactor);
    // calculate center
    graphWidget_->setCenter(QPointF(sceneRect.left() + sceneRect.width() / 2,
                                    sceneRect.top() + sceneRect.height() / 2));
    graphWidget_->center();
    graphWidget_->invalidateScene();
}

void RptMainWindow::setReuseTargets() {
    MsgDistr.postMessage(new BoolMsg(NetworkEvaluator::setReuseTextureContainerTargets_, setReuseTargetsAction_->isChecked()),"evaluator");
}

void RptMainWindow::selectAll() {
	for (size_t i=0; i < processors_.size(); i++) {
		processors_.at(i)->setSelected(true);
	}
	for (size_t i=0; i< aggregations_.size(); i++)  {
		aggregations_.at(i)->setSelected(true);
	}
    for (size_t i=0; i< propertySets_.size(); i++)  {
		propertySets_.at(i)->setSelected(true);
	}
    graphWidget_->updateSelectedItems();
}

void RptMainWindow::saveAggregationSlot(RptAggregationItem* aggregation) {
    std::string filename = "../../data/networks/aggregations/";
    filename += aggregation->getName();
    filename += ".vnw";
    std::vector<RptProcessorItem*> r;
    std::vector<RptPropertySetItem*> p;
    std::vector<RptAggregationItem*> aggregations;
    aggregations.push_back(aggregation);
    //FIXME: no need to save reuseTargets here
    graphSerializer_->serializeToXml(r, aggregations, p, false, filename);
    aggregationListWidget_->buildItems();
}

void RptMainWindow::saveNetworkButtonPushed() {
    if (!(currentFile_ == "")) {
        if (false)
            graphSerializer_->serializeToXml(processors_, aggregations_, propertySets_, setReuseTargetsAction_->isChecked(), currentFile_ );
        else {
            RptNetwork rptnet = RptNetwork();
            rptnet.processorItems = processors_;
            rptnet.aggregationItems = aggregations_;
            rptnet.propertySetItems = propertySets_;
            rptnet.reuseTCTargets = setReuseTargetsAction_->isChecked();
            networkserializer_->serializeToXml(graphSerializer_->makeProcessorNetwork(rptnet), currentFile_);
        }
        setWindowModified(false);
    }
    else
        saveNetworkAsButtonPushed();
}

void RptMainWindow::saveNetworkAsButtonPushed() {
    QFileDialog fileDialog(this,
                        tr("Save network as"),
                        networkPath_,
                        "Saved voreen network files (*.vnw)");
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setConfirmOverwrite(true);
    if (fileDialog.exec()) {
        currentFile_ = fileDialog.selectedFiles().at(0).toStdString();
        if (false)
            graphSerializer_->serializeToXml(processors_, aggregations_, propertySets_, setReuseTargetsAction_->isChecked(), currentFile_ );
        else {
            RptNetwork rptnet = RptNetwork();
            rptnet.processorItems = processors_;
            rptnet.aggregationItems = aggregations_;
            rptnet.propertySetItems = propertySets_;
            rptnet.reuseTCTargets = setReuseTargetsAction_->isChecked();
            networkserializer_->serializeToXml(graphSerializer_->makeProcessorNetwork(rptnet), currentFile_);
        }

        //update network path
        networkPath_ = fileDialog.directory().path();
        setWindowModified(false);

        // ensure that the file extension is correct (needed for recent file mechanism)
        std::string defFileExt_(".vnw");
        unsigned int len = currentFile_.length();   // unsigned to resolve conflict
        unsigned int extLen = defFileExt_.length();
        if( len >= extLen ) {  // check whether length may already be able to contain extension
            // append file extension to filename if not already contained
            std::string ext = currentFile_.substr(len - extLen, len);
            if( ext != defFileExt_ )
                currentFile_ += defFileExt_;
        }
        addCurrentFileToRecents();
        setWindowTitle(tr("%1[*] - %2").arg(QString(currentFile_.c_str())).arg(tr("VoreenVE Beta")));
    }
}

void RptMainWindow::openNetworkFileButtonPushed() {
    std::string directory = networkPath_.toStdString();
    std::string newFilename = openFileDialog(directory );
    if (newFilename == "")
        return;
    currentFile_ = newFilename;

    //update network path
    networkPath_ = QString(directory.c_str());
    if (graphSerializer_->readVersionFromFile(currentFile_) < 2) {
        NetworkInfos* infos = graphSerializer_->readNetworkFromFile(currentFile_);
        finishOpen(infos);
        delete infos;
        infos = 0;
    }
    else {
        RptNetwork rptnet = graphSerializer_->makeRptNetwork(networkserializer_->readNetworkFromFile(currentFile_));
        finishOpen(rptnet);
    }
}

void RptMainWindow::connectButtonPushed() {
    QList<QGraphicsItem*> selectedItems = graphWidget_->scene()->selectedItems();
    std::vector<RptGuiItem*> list;

    // get selected ProcessorItems
    for (int i=0; i<selectedItems.size(); i++) {
        if ( selectedItems.at(i)->type() == RptProcessorItem::Type
                || selectedItems.at(i)->type() == RptAggregationItem::Type )
            list.push_back(static_cast<RptGuiItem*>(selectedItems.at(i)));
    }

    if (list.size() < 2) return;

    // sort by position
    sortByPosition(list);

    // connect
    for (size_t i=1; i<list.size(); i++) {
        list.at(i-1)->connect(list.at(i));
    }
}

void RptMainWindow::createLoadedAggregations(std::map<int,std::vector<RptProcessorItem*>* > aggroList,std::map<int,std::string> aggroNameMap, QPoint pos) {
    int number=0;
    for(std::map<int,std::vector<RptProcessorItem*>*>::const_iterator i = aggroList.begin(); i != aggroList.end(); ++i) {
        createAggregation(*i->second,aggroNameMap[number], pos);
        number++;
    }
}

void RptMainWindow::sortByPosition(std::vector<RptGuiItem*> &v) {
    for (size_t i=0; i<v.size(); i++) {
        int pos=i;
        bool swap=false;
        for (size_t j=i; j<v.size(); j++) {
            if (v[j]->pos().y() < v[pos]->pos().y()) {
                pos=j;
                swap=true;
            }
        }
        if (swap) {
            RptGuiItem* swapper = v.at(pos);
            v.at(pos)=v.at(i);
            v.at(i) = swapper;
        }
    }
}

void RptMainWindow::evaluatorButtonPushed() {
    // set to a waiting cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    evaluator_->setTextureContainer(ProcessorFactory::getInstance()->getTextureContainer());

    evaluator_->setProcessors(getAllProcessors());

    try {
        if (evaluator_->analyze() >=0) {
            painterWidget_->setEvaluator(evaluator_);
        }
        dumpProcessorConnectionInfos();
    }
    catch (VoreenException e) {
        QMessageBox::warning(this, "Network evaluation", e.what(), QMessageBox::Ok);
    }

    transferFuncPlugin_->setEvaluator(evaluator_);
    QApplication::restoreOverrideCursor();
}

void RptMainWindow::openRecentFileSlot(std::string filename, QPoint position) {
    //FIXME: use same code with openNetworkFileButtonPushed
    currentFile_ = filename;
    if (graphSerializer_->readVersionFromFile(currentFile_) < 2) {
        NetworkInfos* infos = graphSerializer_->readNetworkFromFile(filename);
        finishOpen(infos, position);
        delete infos;
        infos = 0;
    }
    else {
        RptNetwork rptnet = graphSerializer_->makeRptNetwork(networkserializer_->readNetworkFromFile(filename));
        finishOpen(rptnet, position);
    }
}

void RptMainWindow::addAggregationSlot(std::string filename, QPoint position) {
    //FIXME: use same code with openNetworkFileButtonPushed
    currentFile_ = filename;
    if (graphSerializer_->readVersionFromFile(currentFile_) < 2) {
        NetworkInfos* infos = graphSerializer_->readNetworkFromFile(filename);
        finishOpen(infos, position, false);
        delete infos;
        infos = 0;
    }
    else {
        RptNetwork rptnet = graphSerializer_->makeRptNetwork(networkserializer_->readNetworkFromFile(filename));
        finishOpen(rptnet, position, false);
    }
}

void RptMainWindow::openFileButtonPushed() {
    if( volumeSetWidget_ == 0 )
        return;

    std::vector<std::string> files = volumeSetWidget_->openFileDialog();
    if( files.empty() == false ) {
        volumeSetWidget_->loadVolumeSet(files[0]);
    }
}

std::string RptMainWindow::openFileDialog(std::string &directory){

    QDir dir = QDir( QString(directory.c_str()) );

    QFileDialog fd(this, tr("Choose a file to open"), dir.absolutePath());
    QStringList filters;
    filters << "Saved voreen network files (*.vnw)"
            << "All files (*.*)";
    fd.setFilters(filters);
    fd.setViewMode(QFileDialog::Detail);

    if (fd.exec()) {
        if (fd.selectedFiles().size() > 1) {
            QMessageBox::information(0, "Voreen",
                tr("Multiple selection is not allowed for these filetypes."));
        }
        else if ( fd.selectedFiles().size() == 1){
            directory = fd.directory().path().toStdString();
            return fd.selectedFiles().at(0).toStdString();
        }
    }

    return "";

}

// show file open dialog, handle multiple selection (used only for dicom slices)
bool RptMainWindow::getFileDialog(QStringList& filenames, QDir& dir) {
    do {
        QFileDialog *fd = new QFileDialog(this, tr("Choose a Volume Dataset to Open"),
                                          dir.absolutePath());
        QStringList filters;
        filters << tr("Volume data (*.DAT *.I4D *.PVM *.RDM *.RDI *.HDR *.SW *.SEG *.TUV "
                      "*.ZIP *.TIFF *.TIF *.MAT *.HV *.NRRD *.NHDR)");
        fd->setFilters(filters);

        if (fd->exec()) {
            if (fd->selectedFiles().size() > 1) {
                QMessageBox::information(this, "Voreen",
                                         tr("Multiple selection is not allowed for these filetypes."));
            } else {
                filenames = fd->selectedFiles();
                QDir dir = fd->directory();
                fileDialogDir_.setPath(dir.absolutePath());
                return true;
            }
        } 
        else {
            return false;
        }

    } 
    while (true);
}

void RptMainWindow::fileOpenDicomFiles() {
#ifdef VRN_WITH_DCMTK
    QString tmp = QFileDialog::getExistingDirectory(
        this,
        "Choose a Directory",
        "../../data",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (tmp != "") {
#ifdef WIN32
        if (!tmp.endsWith("/"))
            tmp += "/";
#endif
        voreen::DicomVolumeReader volumeReader;
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        setUpdatesEnabled(false);
        VolumeSet* volumeSet = volumeReader.read(tmp.toStdString());
        if (volumeSet != 0) {
            volsetContainer_->addVolumeSet(volumeSet);
            volumeSetWidget_->updateContent();
        }
        QApplication::restoreOverrideCursor();
    }
#else
    QMessageBox::information(this, "Voreen", tr("Application was compiled without DICOM support."));
#endif // VRN_WITH_DCMTK
}

void RptMainWindow::fileOpenDicomDir() {
#ifdef VRN_WITH_DCMTK
    QString tmp = QFileDialog::getOpenFileName(
        this,
        "Choose a File to Open",
        "../../data",
        "DICOMDIR File");
    if (tmp == "")
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    setUpdatesEnabled(false);
    voreen::DicomVolumeReader volumeReader;
    std::vector<voreen::DicomSeriesInfo> series = volumeReader.listSeries(tmp.toStdString());
    QApplication::restoreOverrideCursor();

    if (series.size() > 0) {
        if (dicomDirDialog_)
            delete  dicomDirDialog_;
        dicomDirDialog_ = new DicomDirDialog();
        connect(dicomDirDialog_, SIGNAL(dicomDirFinished()), this, SLOT(dicomDirFinished()));
        dicomDirDialog_->setSeries(series, tmp.toStdString());
        dicomDirDialog_->show();
    }
    else
        QMessageBox::warning(this, "Voreen", "No DICOM series found.");
#else
    QMessageBox::information(this, "Voreen", tr("Application was compiled without DICOM support."));
#endif // VRN_WITH_DCMTK
}


void RptMainWindow::dicomDirFinished() {
#ifdef VRN_WITH_DCMTK
    voreen::DicomVolumeReader volumeReader;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    setUpdatesEnabled(false);
    VolumeSet* volumeSet = volumeReader.read(dicomDirDialog_->getFilename());
    if (volumeSet != 0) {
        volsetContainer_->addVolumeSet(volumeSet);
        volumeSetWidget_->updateContent();
    }
    QApplication::restoreOverrideCursor();
    setUpdatesEnabled(true);
#endif
}

void RptMainWindow::addProcessorItem(Identifier type, QPoint pos) {
    RptProcessorItem* rgi = new RptProcessorItem(type);
    Processor* proc = (rgi->getProcessor());

    // special treatment for VolumeSetSourceProcessors:
    // those processors need to know the VolumeSetContainer so it is assigned here.
    // this has no effect on serialization.
    //
    VolumeSetSourceProcessor* vssp = dynamic_cast<VolumeSetSourceProcessor*>(proc);
    if( vssp != 0 )
    {
        vssp->setVolumeSetContainer(volsetContainer_);
    }

    proc->setTextureContainer(tc_);
	proc->setCamera(camera_);

    processors_.push_back(rgi);
    graphWidget_->addItem(rgi, pos);
    if (!MsgDistr.contains(proc))
	    MsgDistr.insert(proc);
    connect(rgi, SIGNAL(deleteSignal()), this, SLOT(deleteSlot()));
    connect(rgi, SIGNAL(aggregateSignal()), this, SLOT(insertAggregation()));
}

void RptMainWindow::createPropertySet() {
    std::vector<QGraphicsItem*> selectedItems = graphWidget_->scene()->selectedItems().toVector().toStdVector();
    std::vector<RptGuiItem*> selected;

    // select only ProcessorItems and AggregationGuiItems
    for (size_t i=0; i<selectedItems.size(); i++) {
        if (selectedItems[i]->type() == RptProcessorItem::Type) {
            selected.push_back(static_cast<RptProcessorItem*>(selectedItems[i]));
        }
        else if (selectedItems[i]->type() == RptAggregationItem::Type) {
            selected.push_back(static_cast<RptAggregationItem*>(selectedItems[i]));
        }
    }

    RptPropertySetItem* propSet;
    if (selected.size() < 1)
        propSet = new RptPropertySetItem(graphWidget_->getScene());
    else
        propSet = new RptPropertySetItem(selected, graphWidget_->getScene());

	propertySets_.push_back(propSet);
    connect(propSet, SIGNAL(deleteSignal()), this, SLOT(deleteSlot()));
}

void RptMainWindow::showProperties(){
    // At the moment the only way to set propertyDockWidget_ on top (Qt 4.4):
    contentTab_->setCurrentIndex(1);
}

void RptMainWindow::createAggregation(std::vector<RptProcessorItem*> items, std::string name, QPoint pos) {
	if (items.size() < 2)
        return;

    RptAggregationItem* aggro = new RptAggregationItem(items, graphWidget_->scene(), name);

	if (pos.x() != 10000 && pos.y() != 10000)
        graphWidget_->addItem(aggro, pos-aggro->scenePos().toPoint());
    else
        graphWidget_->addItem(aggro);

    aggro->adjustArrows();
    aggro->setSelected(true);
    aggregations_.push_back(aggro);

    for (size_t i=0; i<items.size(); i++) {
        for (size_t j=0; j<processors_.size(); j++) {
            if (processors_.at(j) == items[i]) {
                processors_.at(j)->removeFromScene();
                processors_.erase(processors_.begin() + j);
                break;
            }
        }
    }

    connect(aggro, SIGNAL(deaggregateSignal()), this, SLOT(deaggregate()));
    connect(aggro, SIGNAL(deleteSignal()), this, SLOT(deleteSlot()));
    connect(aggro, SIGNAL(saveSignal(RptAggregationItem*)), this, SLOT(saveAggregationSlot(RptAggregationItem*)));
}

void RptMainWindow::insertAggregation() {
    std::vector<QGraphicsItem*> selectedItems = graphWidget_->scene()->selectedItems().toVector().toStdVector();
    std::vector<RptProcessorItem*> items;

    // select only ProcessorItems that are not already aggregated
    for (size_t i=0; i<selectedItems.size(); i++) {
        if (selectedItems[i]->type() == RptProcessorItem::Type) {
            if (!(selectedItems[i]->parentItem() && selectedItems[i]->parentItem()->type() == RptAggregationItem::Type)) {
                items.push_back(static_cast<RptProcessorItem*>(selectedItems[i]));
            }
        }
    }

    createAggregation(items, "Aggregation");

}

void RptMainWindow::editAggregation() {}

void RptMainWindow::deaggregate() {
    // get selected items
    std::vector<QGraphicsItem*> selectedItems = graphWidget_->scene()->selectedItems().toVector().toStdVector();
    std::vector<RptAggregationItem*> aggregations;

    // select only AggregationItems & deaggregate them
    for (size_t i=0; i<selectedItems.size(); i++) {
        if (selectedItems[i]->type() == RptAggregationItem::Type) {
            aggregations.push_back(static_cast<RptAggregationItem*>(selectedItems[i]));
        }
    }

    for (size_t i=0; i<aggregations.size(); i++) {
        if (aggregations[i]->isShowContent())
            aggregations[i]->showContent(false);
        const std::vector<RptProcessorItem*> &items = aggregations[i]->deaggregate();
        aggregations[i]->clear();
        for (size_t j=0; j<items.size(); j++) {
            processors_.push_back(items[j]);
        }

        for (size_t j=0; j<aggregations_.size(); j++) {
            if (aggregations_[j] == aggregations[i]) {
                delete(aggregations_[j]);
                aggregations_.erase(aggregations_.begin() + j);
                break;
            }
        }

    }

}

void RptMainWindow::showAggregationContent() {
    // get selected items
    std::vector<QGraphicsItem*> selectedItems = graphWidget_->scene()->selectedItems().toVector().toStdVector();
    std::vector<RptAggregationItem*> aggregations;

    // select only AggregationItems
    for (size_t i=0; i<selectedItems.size(); i++) {
        if (selectedItems[i]->type() == RptAggregationItem::Type) {
            aggregations.push_back(static_cast<RptAggregationItem*>(selectedItems[i]));
        }
    }

    for (size_t i=0; i<aggregations.size(); i++) {
        if (showAggregationContentAction_->isChecked()) {
            aggregations[i]->showContent(true);
        }
        else {
            aggregations[i]->showContent(false);
        }
    }
}

void RptMainWindow::sendProcessorToTable(Processor* processor, QVector<int> unequalEntries){
	transferFuncPlugin_->findAndSetProcessor(processor);
    propertyListWidget_->setProcessor(processor, unequalEntries);
	changeProcessorInfo();
}

void RptMainWindow::initTextureContainer(){
    ShdrMgr.addPath("../../src/core/vis/glsl");
    ShdrMgr.setPath("../../src/core/vis/glsl");
	int finalTarget = 30;


	tc_ = TextureContainer::createTextureContainer(finalTarget + 1);//, false, TextureContainer::VRN_TEXTURE_CONTAINER_RTT);
    tc_->setFinalTarget(20);
    if (tc_->initializeGL() == false) {
        delete tc_;
        tc_ = 0;
    }

    id1_.setTC(tc_);

#ifndef __APPLE__
    int renderTargetType =
        TextureContainer::VRN_RGBA_FLOAT16 |
        TextureContainer::VRN_DEPTH |
        TextureContainer::VRN_DEPTH_TEX;
#else
    // FIXME: support for depth textures on Apple ;)
    int renderTargetType =
        TextureContainer::VRN_RGBA_FLOAT16 |
        //    TextureContainer::VRN_DEPTH |
        TextureContainer::VRN_DEPTH_TEX;
#endif

    for (int i=0; i < finalTarget; i++) {
        tc_->initializeTarget(i, renderTargetType);
    }
    tc_->initializeTarget(finalTarget, TextureContainer::VRN_FRAMEBUFFER);
    tc_->setFinalTarget(finalTarget);
    MsgDistr.postMessage(new BoolMsg(NetworkEvaluator::setReuseTextureContainerTargets_,false),"evaluator");

    id1_.initNewRendering();

    painterWidget_->init(tc_,camera_);
    loadDataset("../../data/nucleon.dat");
}

void RptMainWindow::createDockWidgets() {
    // TODO: i suspect these lines of not being suitable placed here,
    // because the actually do nothing for creating dockwidgets.
    // but placing them within the ctor somehow causes application crashes.
    // (dirk)
    //
    ProcessorFactory::getInstance()->setTextureContainer(tc_);
	//ProcessorFactory::getInstance()->initializeClassList();

    // ShowTextureContainerWidget
    ShowTexContainerWidget* showTextureContainer = new ShowTexContainerWidget(painterWidget_);
    showTextureContainer->setTextureContainer(tc_);
    //tcDockWidget_ = new QDockWidget("Texture Container", this);
    //tcDockWidget_->setAllowedAreas(Qt::TopDockWidgetArea | Qt::RightDockWidgetArea);
    //tcDockWidget_->setWidget(showTextureContainer);
    canvasTab_->insertTab(1, showTextureContainer, "TextureContainer");

    // processor tab
    processorLayout_ = new LayoutContainer(this);

    // Processor List Widget
    processorListWidget_ = new RptProcessorListWidget();
    processorLayout_->addWidget(processorListWidget_);
    graphWidget_->addAllowedWidget(processorListWidget_);

     // Aggregation List Widget
    aggregationListWidget_ = new RptAggregationListWidget();
    processorLayout_->addWidget(aggregationListWidget_);
    graphWidget_->addAllowedWidget(aggregationListWidget_); // 2.

    //Info Widget containing the informations for the selected Processor
    processorInfoBox_ = new QTextBrowser(0);
    processorLayout_->addWidget(processorInfoBox_);
    processorInfoBox_->setText("This box contains the info for the selected processor");

    // property tab
    propertyLayout_ = new LayoutContainer(this);

    // Property List Widget
    propertyListWidget_ = new RptPropertyListWidget();
	propertyListWidget_->setPainter(dynamic_cast<VoreenPainter*>(painterWidget_->getPainter()));
    propertyLayout_->addWidget(propertyListWidget_);

	// transferfunc widget
    transferFuncPlugin_ = new TransFuncPlugin(this, painterWidget_->painter_, 0, Qt::Horizontal);
	transferFuncPlugin_->createWidgets();
    transferFuncPlugin_->createConnections();
//transferFuncPlugin_->dataSourceChanged(volumeContainer_->getVolume(0));
	propertyLayout_->addWidget(transferFuncPlugin_);

#ifdef VRN_MODULE_GLYPHS
    //glyphsplugin
    glyphsPlugin_ = new GlyphsPlugin(this, painterWidget_->painter_, painterWidget_);
    glyphsPlugin_->setDataSourceContainer(datasourceContainer_);
    glyphsPlugin_->setShowPlacingMethods(false);
    ((WidgetPlugin*)glyphsPlugin_)->createWidgets();
    ((WidgetPlugin*)glyphsPlugin_)->createConnections();
#endif

    // tabbed widget for content dock
    contentTab_ = new QTabWidget(this);
    contentTab_->insertTab(0, processorLayout_, "Processors");
    contentTab_->insertTab(1, propertyLayout_, "Properties");
#ifdef VRN_MODULE_GLYPHS
    contentTab_->insertTab(2, glyphsPlugin_, "Glyphs");
#endif
    contentTab_->insertTab(3, volumeSetWidget_, "Volume Sets");
    contentDock_ = new QDockWidget("Processors", this);
    contentDock_->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    contentDock_->setWidget(contentTab_);
    contentDock_->setFeatures(QDockWidget::AllDockWidgetFeatures);
    addDockWidget(Qt::RightDockWidgetArea, contentDock_);

	/*QDockWidget* pipelineDock = new QDockWidget(this);
    QToolBox* pipelineBox = new QToolBox(pipelineDock);*/

    /*WidgetGenerator* guiGen = new WidgetGenerator(0, this);
	std::vector<Processor*> temp;
	temp.push_back(new FancyRaycaster());
	guiGen->createAllWidgets(temp,static_cast<VoreenPainter*>(painterWidget_->getPainter())->getOverlayMgr());

	static_cast<WidgetPlugin*>(guiGen)->createWidgets();
	static_cast<WidgetPlugin*>(guiGen)->createConnections();
	pipelineBox->addItem(static_cast<WidgetPlugin*>(guiGen), static_cast<WidgetPlugin*>(guiGen)->getIcon(), static_cast<WidgetPlugin*>(guiGen)->objectName());*/

	//multi view creation
	//MultiViewWidget* multiView  = new MultiViewWidget(painterWidget_->getCamera(),(tgt::EventListener*)(transferFuncPlugin_->getIntensityPlugin()), volumeContainer_, this);
 //   // set color processing mode to 'no shading' for multiview 3D renderer
 //   MsgDistr.postMessage( new StringMsg(FancyRaycaster::setShadingMode_, "No Shading"), "mv1" );
 //   // disable clipping for multiview 3D view
 //   MsgDistr.postMessage( new BoolMsg(ProxyGeometry::setUseClipping_, false), "mv1" );
 //   QDockWidget* multiViewDock = new QDockWidget(tr("MultiView"), this);
	//multiViewDock->setWidget(multiView);
	//multiViewDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	//addDockWidget(Qt::RightDockWidgetArea,multiViewDock);

	/*pipelineDock->setWidget(pipelineBox);
	pipelineDock->setMinimumWidth(300);
    addDockWidget(Qt::RightDockWidgetArea, pipelineDock);*/

	//transferDock_->hide();
	//tcDockWidget_->hide();

								//aggroDock->hide();

    //showTextureContainer->resize(512, 512);
    //showTextureContainer->show();

}

void RptMainWindow::createMenuAndToolBar() {
	menu_ = menuBar();						//create menuBar
	toolBar_ = addToolBar(tr("ToolBar"));	//create the ToolBar

	fileMenu_ = menu_->addMenu(tr("&File"));
	editMenu_ = menu_->addMenu(tr("&Edit"));
	optionsMenu_ = menu_->addMenu(tr("&Options"));
    helpMenu_ = menu_->addMenu(tr("&Help"));

	selectAllAction_ = new QAction(tr("Select all"),this);
	selectAllAction_->setShortcut(tr("Ctrl+A"));

	copyAction_ = new QAction(tr("Copy"),this);
	copyAction_->setShortcut(tr("Ctrl+C"));

	pasteAction_ = new QAction(tr("Paste"),this);
	pasteAction_->setShortcut(tr("Ctrl+V"));

    QAction* deleteAction = new QAction(QIcon(":/vrn_app/icons/eraser.png"),tr("Delete"),this);
    deleteAction->setShortcut(tr("Del"));

    insertAggregationAction_ = new QAction(tr("Aggregate"),this);

    showAggregationContentAction_ = new QAction("Show Aggregation Content",this);
    showAggregationContentAction_->setCheckable(true);
	showAggregationContentAction_->setChecked(false);

    deaggregateAction_ = new QAction(tr("Deaggregate"),this);

    createPropSetAction_ = new QAction(tr("Create Property Set"), this);

    connectAction_ = new QAction(QIcon(":/icons/mapping-function.png"),tr("Connect"), this);
    connectAction_->setShortcut(tr("Alt+C"));
    connectAction_->setStatusTip(tr("Connects selected Processor"));

    aboutAction_ = new QAction(QIcon(":/vrn_app/icons/about.png"), tr("&About"), this);
    aboutAction_->setStatusTip(tr("Show the application's About box"));
    aboutAction_->setToolTip(tr("Show the application's About box"));
    connect(aboutAction_, SIGNAL(triggered()), this, SLOT(helpAbout()));
    helpMenu_->addAction(aboutAction_);

    helpFirstStepsAct_ = new QAction(QIcon(":/vrn_app/icons/wizard.png"),
                                        tr("Getting Started"), this);
    connect(helpFirstStepsAct_, SIGNAL(triggered()), this, SLOT(helpFirstSteps()));
    helpMenu_->addAction(helpFirstStepsAct_);

    editMenu_->addAction(selectAllAction_);
	editMenu_->addAction(copyAction_);
	editMenu_->addAction(pasteAction_);
    editMenu_->addAction(deleteAction);
    editMenu_->addSeparator();
    editMenu_->addAction(insertAggregationAction_);
    editMenu_->addAction(showAggregationContentAction_);
    editMenu_->addAction(deaggregateAction_);
    editMenu_->addSeparator();
    editMenu_->addAction(connectAction_);
    editMenu_->addAction(createPropSetAction_);



    setReuseTargetsAction_ = new QAction("Reuse TC targets (needs rebuild)",this);
    setReuseTargetsAction_->setCheckable(true);
    setReuseTargetsAction_->setChecked(false);

    optionsMenu_->addAction(setReuseTargetsAction_);

    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteSlot()));

    //actions to be put into the toolbar
    evaluatorAction_ = new QAction(QIcon(":/icons/player_play.png"),"Use the current network to render the data set.",this);
    evaluatorAction_->setShortcut(tr("Ctrl+N"));


    clearAction_ = new QAction(QIcon("icons/clear.png"),tr("New Network"),this);
    clearAction_->setStatusTip(tr("Create a new network"));

    openFileAction_ = new QAction(QIcon(":/vrn_app/icons/open_dicom.png"),"Open Data set",this);
    openFileAction_->setShortcut(tr("Ctrl+O"));
    openFileAction_->setStatusTip(tr("Open a volume data set"));

    openDicomDirAct_ = new QAction(QIcon(":/vrn_app/icons/open_dicom.png"), tr("&Open DICOMDIR Data set..."), this);
    openDicomDirAct_->setStatusTip(tr("Open an existing DICOMDIR file"));
    openDicomDirAct_->setToolTip(tr("Open an existing DICOMDIR file"));
    connect(openDicomDirAct_, SIGNAL(triggered()), this, SLOT(fileOpenDicomDir()));

  	openDicomFilesAct_ = new QAction(QIcon(":/vrn_app/icons/open_dicom.png"), tr("Open DICOM Slices..."), this);
  	openDicomFilesAct_->setStatusTip(tr("Open DICOM slices"));
  	openDicomFilesAct_->setToolTip(tr("Open existing DICOM slices"));
  	connect(openDicomFilesAct_, SIGNAL(triggered()), this, SLOT(fileOpenDicomFiles()));

    quitAction_ = new QAction(QIcon(":/vrn_app/icons/exit.png"), tr("&Quit"), this);
    quitAction_->setShortcut(tr("Ctrl+Q"));
    quitAction_->setStatusTip(tr("Exit the application"));
    quitAction_->setToolTip(tr("Exit the application"));
    connect(quitAction_, SIGNAL(triggered()), this, SLOT(close()));
    
    openNetworkFileAction_ = new QAction(QIcon(":/vrn_app/icons/openNetwork.png"),tr("Open Network"),this);
    saveNetworkAction_ = new QAction(QIcon(":/vrn_app/icons/save.png"),tr("Save Network"),this);
    saveNetworkAction_->setToolTip(tr("Save Current Network"));
    saveNetworkAsAction_ = new QAction(tr("Save Network As..."),this);

    rebuildShadersAction_ = new QAction( QIcon("icons/rebuildshaders.png"), tr("Rebuild All Shaders"),  this);
    rebuildShadersAction_->setShortcut(tr("F5"));
    rebuildShadersAction_->setStatusTip(tr("Reloads all shaders currently loaded from file and rebuilds them"));
    rebuildShadersAction_->setToolTip(tr("Rebuilds all currently loaded shaders"));
    connect(rebuildShadersAction_, SIGNAL(triggered()), this, SLOT(rebuildShaders()));

    for (int i = 0; i < maxRecents; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
    }

    fileMenu_->addAction(clearAction_);
    fileMenu_->addAction(openNetworkFileAction_);
    fileMenu_->addAction(saveNetworkAction_);
    fileMenu_->addAction(saveNetworkAsAction_);
    fileMenu_->addSeparator();
    fileMenu_->addAction(openFileAction_);
    dicomMenu_ = fileMenu_->addMenu(tr("Open &DICOM Dataset..."));
  	dicomMenu_->addAction(openDicomDirAct_);
  	dicomMenu_->addAction(openDicomFilesAct_);
    fileMenu_->addSeparator();
    fileMenu_->addAction(quitAction_);

    fileMenu_->addSeparator();
    for (int i = 0; i < maxRecents; ++i)
        fileMenu_->addAction(recentFileActs[i]);
    updateRecentFileActions();

    //put the actions into the toolbar
    toolBar_->addAction(clearAction_);
    toolBar_->addAction(openNetworkFileAction_);
    toolBar_->addAction(saveNetworkAction_);
    toolBar_->addAction(openFileAction_);
    toolBar_->addSeparator();
    toolBar_->addAction(deleteAction);
    toolBar_->addAction(rebuildShadersAction_);
    toolBar_->addAction(connectAction_);
    toolBar_->addSeparator();
    toolBar_->addAction(evaluatorAction_);
}

void RptMainWindow::updateRecentFileActions() {
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)maxRecents);
    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < maxRecents; ++j)
        recentFileActs[j]->setVisible(false);
    //separatorAct->setVisible(numRecentFiles > 0);
}

void RptMainWindow::addCurrentFileToRecents() {
    if (currentFile_ != "") {       // probably check is redundant
        QString fileName(currentFile_.c_str());
        QSettings settings;
        QStringList files = settings.value("recentFileList").toStringList();
        files.removeAll("");        // delete empty entries
        files.removeAll(fileName);
        files.prepend(fileName);
        while (files.size() > maxRecents)
            files.removeLast();

        settings.setValue("recentFileList", files);
        updateRecentFileActions();
    }
}

void RptMainWindow::rebuildShaders() {
    // set to a waiting cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	if (ShdrMgr.rebuildAllShadersFromFile()) {
        LINFOC("RptMainWindow", "Shaders reloaded")
		#ifdef WIN32
			Beep(100,100);
		#endif
	} else {
        LWARNINGC("RptMainWindow", "Shader reloading failed");
		#ifdef WIN32
			Beep(10000,100);
		#endif
	}
    QApplication::restoreOverrideCursor();
}

void RptMainWindow::deleteSlot() {

   QList<QGraphicsItem*> selectedItems = graphWidget_->scene()->selectedItems();
    // check for text item (indicates del shortcut during renaming)
   for (int i=0; i < selectedItems.size(); i++) {
       if (selectedItems.at(i)->hasFocus() && selectedItems.at(i)->type() == RptTextItem::Type) {
            /*RptTextItem* item = static_cast<RptTextItem*>(selectedItems.at(i));
            QKeyEvent* event = new QKeyEvent(QEvent::KeyRelease, Qt::Key_Delete, Qt::NoModifier);
            QCoreApplication::postEvent(item, event);*/
            return;
        }
    }

   transferFuncPlugin_->setEnabled(false);


        // sort selectedItems by their type to not delete a port/arrow-item
        // that has already been deleted indirectly with the guiitem
        // so at first kick out the ports:
        for (int i=0; i < selectedItems.size(); i++) {
            if (selectedItems.at(i)->type() == RptPortItem::Type) {
                selectedItems.removeAt(i);
                i--;
            }
        }
        for (int i=0; i < selectedItems.size(); i++) {
            if (selectedItems.at(i)->type() == RptPropertyPort::Type) {
                selectedItems.removeAt(i);
                i--;
            }
        }
        for (int i=0; i < selectedItems.size(); i++) {
            if (selectedItems.at(i)->type() == RptTextItem::Type) {
                selectedItems.removeAt(i);
                i--;
            }
        }


    // next delete arrows
    for (int i=0; i<selectedItems.size(); i++) {
        if (selectedItems[i]) {
            QGraphicsItem* item = selectedItems[i];
            if (selectedItems.at(i)->type() == RptArrow::Type) {

                RptArrow* arrow = static_cast<RptArrow*>(item);
                if (arrow->getDestNode() != 0) {
                    // arrow between ports or arrow between property set and guiitem
                    if (arrow->getDestNode()->type() == RptPortItem::Type) {
                        static_cast<RptPortItem*>(arrow->getSourceNode())->getParent()
                            ->disconnect(static_cast<RptPortItem*>(arrow->getSourceNode()), static_cast<RptPortItem*>(arrow->getDestNode()));
                    }
                    else if (arrow->getSourceNode()->type() == RptPropertyPort::Type) {
                        static_cast<RptPropertySetItem*>(arrow->getSourceNode()->parentItem())->disconnectGuiItem(arrow->getDestNode());
                    }
                }

                selectedItems.removeAt(i);
                i--;
            }
        }
    }

    // eventually delete the guiitems
    for (int i=0;i<selectedItems.size();i++) {
        if (selectedItems[i]) {
            QGraphicsItem* item = selectedItems[i];
            if (item->type() == RptProcessorItem::Type) {
                RptProcessorItem* guiItem = static_cast<RptProcessorItem*>(item);
                if (!(guiItem->parentItem() && guiItem->parentItem()->type() == RptAggregationItem::Type)) {

                    for (size_t j=0; j<processors_.size(); j++) {
                        RptProcessorItem* temp = processors_.at(j);
                        if (temp == guiItem) {
                            delete(processors_.at(j));
                            processors_.at(j) = 0;
                            processors_.erase(processors_.begin()+j);
                            j--;
                            break;
                        }
                    }
                }
                selectedItems.removeAt(i);
                i--;
            }
        }
    }

    // and delete the aggregationitems
    for (int i=0; i<selectedItems.size();i++) {
        if (selectedItems[i]) {
            if (selectedItems[i]->type() == RptAggregationItem::Type) {
                RptAggregationItem* aggrItem = static_cast<RptAggregationItem*>(selectedItems[i]);

                for (size_t j=0; j<aggregations_.size(); j++) {
                    if (aggregations_[j] == aggrItem) {
                        //delete(aggrItem);
                        aggregations_[j]->disconnectAll();
                        delete(aggregations_[j]);
                        aggregations_[j] = 0;
                        aggregations_.erase(aggregations_.begin() + j);
                        j--;
                    }
                }
                selectedItems.removeAt(i);
                i--;
            }
        }
    }

    // eventually delete the property sets
    for (int i=0; i<selectedItems.size();i++) {
        if (selectedItems[i]) {
            if (selectedItems[i]->type() == RptPropertySetItem::Type) {
                RptPropertySetItem* prop = static_cast<RptPropertySetItem*>(selectedItems[i]);

                for (size_t j=0; j<propertySets_.size(); j++) {
                    if (propertySets_[j] == prop) {
                        delete(propertySets_[j]);
                        propertySets_[j] = 0;
                        propertySets_.erase(propertySets_.begin() + j);
                        j--;
                    }
                }
                selectedItems.removeAt(i);
                i--;
            }
        }
    }

    //evaluator_->analyze();
    setWindowModified(true);
}

void RptMainWindow::keyPressEvent(QKeyEvent *event) {
    QMainWindow::keyPressEvent(event);
}

void RptMainWindow::closeEvent(QCloseEvent *event) {
    // storing setings
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.endGroup();
    settings.beginGroup("Paths");
    settings.setValue("network", networkPath_);
    settings.setValue("dataset", datasetPath_);
    settings.endGroup();

    if (isWindowModified()) {
        switch (QMessageBox::information(this, "VoreenVE Beta",
                                         "Do you want to save the changes to the current network?",
                                         "Yes", "No", "Cancel", 0, 1))
        {
        case 0:
            saveNetworkAsButtonPushed();
            event->accept();
            break;
        case 1:
            event->accept();
            break;
        case 2:
        default: // just for sanity
            event->ignore();
            break;
        }
    }
}

void RptMainWindow::dumpProcessorConnectionInfos() {
	processorInfoBox_->clear();
	std::string s;
	for (size_t i=0;i<evaluator_->getProcessors().size();i++) {
		Processor* processor = evaluator_->getProcessors().at(i);
		s="<b>";
		s.append(processor->getClassName().getName().c_str());
		s.append(": </b>");
		processorInfoBox_->append(QString(s.c_str())) ;
		std::string prio="Priority: ";
		processorInfoBox_->append(QString(prio.c_str()));
		processorInfoBox_->append(QString::number(evaluator_->getPiorityMap()[processor]));
		for (size_t j=0;j<processor->getInports().size();j++) {
			s="Inport: ";
			s.append(processor->getInports().at(j)->getType().getName());
			s.append(" is connected to: ");
			std::vector<Port*> targets = processor->getInports().at(j)->getConnected();
			std::string targetString = "";
			for (size_t k=0;k<targets.size();k++) {
				targetString.append(targets.at(k)->getProcessor()->getClassName().getName().c_str());
				targetString.append(" , ");
			}
			s.append(targetString);
			processorInfoBox_->append(QString(s.c_str()));
		}
		for (size_t j=0;j<processor->getOutports().size();j++) {
			s="Outport: ";
			s.append(processor->getOutports().at(j)->getType().getName());
			s.append(" is connected to: ");
			std::vector<Port*> targets = processor->getOutports().at(j)->getConnected();
			std::string targetString = "";
			for (size_t k=0;k<targets.size();k++) {
				targetString.append(targets.at(k)->getProcessor()->getClassName().getName().c_str());
				targetString.append(" , ");
			}
			s.append(targetString);
			processorInfoBox_->append(QString(s.c_str()));
		}
		processorInfoBox_->append(QString(""));
	}
	//processorInfoBox_->append(QString("Now the portmaps!"));
	//std::map<Port*,TextureContainer::TargetConfiguration>::iterator iter;
	//int t = evaluator_->getPortMap().size();
	//for(iter = evaluator_->getPortMap().begin(); iter != evaluator_->getPortMap().end(); iter++) {
	//	Port* p = iter->first;
	//	processorInfoBox_->append(QString(p->getType().getName().c_str()));
	//	TextureContainer::TargetConfiguration tc = iter->second;
	//	processorInfoBox_->append(QString(tc.type_.getName().c_str()));
	//	processorInfoBox_->append(QString(""));
	//}
}

std::vector<Processor*> RptMainWindow::getAllProcessors() {
	std::vector<Processor*> processors;
	for (size_t i=0; i<processors_.size(); i++) {
		processors.push_back(processors_.at(i)->getProcessor());
	}
    for (size_t i=0; i<aggregations_.size(); i++) {
        std::vector<RptProcessorItem*> tmp = aggregations_[i]->getProcessorItems();
        for (size_t j=0; j<tmp.size(); j++) {
            processors.push_back(tmp[j]->getProcessor());
        }
    }
	return processors;
}

void RptMainWindow::processMessage(Message* msg, const Identifier& /*ident*/) {
	if (msg->id_ == Processor::delete_) {
		transferFuncPlugin_->removeProcessor(msg->getValue<Processor*>());
	}
#ifdef VRN_MODULE_GLYPHS
    else if (msg->id_ == "set.glyphDatasourceNames") {
        datasourceContainer_->clear();
        if (!volsetContainer_)
            return;
        std::vector<std::string> names = msg->getValue<std::vector<std::string> >();
        for (size_t i = 0; i < names.size(); ++i) {
            VolumeSet* volset = volsetContainer_->findVolumeSet(names[i]);
            if (!volset)
                continue;
            VolumeSeries::SeriesSet series_set = volset->getSeries();            
            VolumeSeries::SeriesSet::iterator it_series;
            for (it_series = series_set.begin(); it_series != series_set.end(); ++it_series){
                for (int i = 0; i < (*it_series)->getNumVolumeHandles(); ++i) {
                    Volume* volume = (*it_series)->getVolumeHandle(i)->getVolume();
                    std::stringstream s;
                    s << (*it_series)->getVolumeHandle(i)->getTimestep();
                    std::string caption = VolumeMetaData::getFileNameWithoutPath(volume->meta().getFileName());
                    caption += "_" + (*it_series)->getName() + "_";
                    caption += s.str();

                    DataSource* dataSource = new DataSourceImmediateFloat(volume, caption);
                    datasourceContainer_->addDataSource(dataSource);

                    dataSource = new DataSourceDerivedGradient((DataSourceImmediateFloat *)dataSource, caption);
                    datasourceContainer_->addDataSource(dataSource);

                    dataSource = new DataSourceDerivedMagnitude((DataSourceDerivedGradient *)dataSource, caption);
                    datasourceContainer_->addDataSource(dataSource);
                }
            }
        }
        glyphsPlugin_->refreshDataSources();
    }
#endif
}

void RptMainWindow::changeProcessorInfo() {
	//clear info box
	//processorInfoBox_->clear(); not needed anymore since box is updated by ..._->setHtml();
	QString text;// =  QString("");

	//get current list item
	//prevent showing information twice after drag'n drop
	if (processorListWidget_->hasFocus()) {
		//check if a item is selected
		if (!(processorListWidget_->currentItem() == NULL)) {
			RptProcessorListItem* selectedTreeWidgetItem = dynamic_cast<RptProcessorListItem*>(processorListWidget_->currentItem());
			//check if item is a processor
			if (selectedTreeWidgetItem) {
				Identifier id = selectedTreeWidgetItem->getId();
				text += QString("<b>") + QString(id.getName().c_str()) + QString("</b><br /> ");
				text += QString(ProcessorFactory::getInstance()->getProcessorInfo(id).c_str() + QString("<hr />"));
			}
		}
	}

	//get selected graphics items
	QList<QGraphicsItem*> selectedGraphItems = graphWidget_->scene()->selectedItems();
	//check what type of items these are and extract processors from list
	for (int i=0; i<selectedGraphItems.size(); i++) {
		if ( selectedGraphItems.at(i)->type() == RptProcessorItem::Type ) {
			RptProcessorItem* item = static_cast<RptProcessorItem*>(selectedGraphItems.at(i));
			text += QString("<b><font color='green'>") + QString(item->getName().c_str()) + QString("</font></b><br /> ");
			text += QString(item->getProcessor()->getProcessorInfo().c_str()) + QString("<hr />");
		}
	}

	// insert text in info box
	processorInfoBox_->setHtml(text);
}

void RptMainWindow::helpAbout() {
    QDialog* window = new QDialog(this);
    Ui::VoreenAboutBox ui;
    ui.setupUi(window);
#ifndef WIN32
    // On Unix the windows manager should take care of this
    int posX = pos().x() + (width() - window->width()) / 2;
    int posY = pos().y() + (height() - window->height()) / 2;
    window->move(posX, posY);
#endif
    window->setWindowIcon(QIcon(":/vrn_app/icons/icon-64.png"));
    setDisabled(true);
    window->setDisabled(false);
    window->exec();
    setDisabled(false);
}

void RptMainWindow::helpFirstSteps() {
    HelpBrowser* help = new HelpBrowser("file:///" + QDir::currentPath() + "/doc/gsg.html");
    help->resize(900, 600);
    help->show();
}

//================================================================================================================

LayoutContainer::LayoutContainer(QWidget *parent, QBoxLayout::Direction direction)
    : QWidget(parent)
{
    layout_ = new QBoxLayout(direction, this);
}


LayoutContainer::~LayoutContainer() {
    delete layout_;
}

void LayoutContainer::addWidget(QWidget* widget) {
    layout_->addWidget(widget);
}


} // namespace
