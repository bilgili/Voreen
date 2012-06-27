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

#include "rptaggregationlistwidget.h"
#include "rptnetworkserializergui.h"
#include "rptpainterwidget.h"
#include "rptpropertylistwidget.h"

#include "voreen/core/geometry/geometrycontainer.h"
#include "voreen/core/vis/processors/processorfactory.h"
#include "voreen/core/vis/transfunc/transfuncmappingkey.h"

#include "voreen/qt/aboutbox.h"
#include "voreen/qt/helpbrowser.h"
#include "voreen/qt/opennetworkfiledialog.h"
#include "voreen/qt/widgets/canvasmodifier.h"
#include "voreen/qt/widgets/showtexcontainerwidget.h"
#include "voreen/qt/widgets/volumesetwidget.h"
#include "voreen/qt/widgets/transfunc/transfuncplugin.h"
#include "voreen/qt/widgets/voreentoolwindow.h"
#include "voreen/qt/widgets/voreentoolbar.h"
#include "voreen/qt/widgets/orientationplugin.h"
#include "voreen/qt/widgets/animationplugin.h"

#include "voreen/core/version.h"

#ifdef VRN_MODULE_GLYPHS
#include "voreen/modules/glyphs/datasource.h"
#include "voreen/modules/glyphs/glyphsplugin.h"
#endif

#ifdef VRN_MODULE_MEASURING
#include "voreen/modules/measuring/selectionprocessorlistwidget.h"
#include "voreen/modules/measuring/measuringprocessorlistwidget.h"
#endif

#ifdef VRN_WITH_PYTHON
#include "tgt/scriptmanager.h"
#endif // VRN_WITH_PYTHON

namespace voreen {

RptMainWindow::RptMainWindow(const std::string& network, const std::string& dataset)
    : QMainWindow()
    , numberOfPropertySets_(0)
    , loadVolumeSetContainer_(false)
{
    setMinimumSize(300, 200);   

    resetSettings_ = settings_.value("ResetSettings", false).toBool();

    if (!network.empty())
        defaultNetwork = network.c_str();
    else
        defaultNetwork = "../../data/networks/standard.vnw";

    if (!dataset.empty())
        defaultDataset = dataset.c_str();
    else
        defaultDataset = "../../data/nucleon.dat";
    
    // set defaults
    networkPath_ = "../../data/networks";
    QString datasetPath = "../../data";
    QSize windowSize = QSize(800, 600);
    
    // restore settings
    if (!resetSettings_) {
        settings_.beginGroup("MainWindow");
        windowSize = settings_.value("size", windowSize).toSize();
        move(settings_.value("pos", QPoint(0, 0)).toPoint());
        settings_.endGroup();

        settings_.beginGroup("Paths");
        networkPath_ = settings_.value("network", networkPath_).toString();
        datasetPath = settings_.value("dataset", datasetPath).toString();
        settings_.endGroup();
    }
    resize(windowSize);
    
    copyCounter_ = 0;

    setWindowModified(false);
    setAcceptDrops(true);

    updateWindowTitle();
    setWindowIcon(QIcon(":/vrn_app/icons/icon-64.png"));

    camera_ = new tgt::Camera(tgt::vec3(0.f,3.5f,0.f),tgt::vec3(0.f,0.f,0.f),tgt::vec3(0.f,0.f,1.f));

    graphSerializer_ = new RptNetworkSerializerGui();
    networkserializer_ = new NetworkSerializer();
    evaluator_ = new NetworkEvaluator();
    MsgDistr.insert(evaluator_);

    // initialization of an empty geometry container
    //
    geoContainer_ = new GeometryContainer();
    evaluator_->setGeometryContainer(geoContainer_);

    // The widget containing all currently loaded volumesets must be created before loading the
    // first dataset on startup.
    volsetContainer_ = new VolumeSetContainer();
    volumeSetWidget_ = new VolumeSetWidget(volsetContainer_, 0, VolumeSetWidget::LEVEL_ALL);
    volumeSetWidget_->setCurrentDirectory(datasetPath.toStdString());

    // Create Canvas Widget before initGL() is called.
    // Disable rendering updates until initGL() is finished to prevent
    // rendering of garbage of Mac OS, happening when the widgets is
    // rendererd with no painter attached.
    painterWidget_ = new RptPainterWidget(this);
    painterWidget_->setUpdatesEnabled(false); // enabled when fully initialized
    painterWidget_->eval = evaluator_;
    painterWidget_->setFocusPolicy(Qt::ClickFocus);
    setCentralWidget(painterWidget_);

    canvasMod_ = new CanvasModifier(painterWidget_, true);

    createMenuAndToolBar();
}

RptMainWindow::~RptMainWindow() {
    clearScene(); //FIXME: needed to prevent crash with Qt 4.3 on exit when loading multiple networks
    delete camera_;
    delete graphWidget_; // this also deletes all processors (happens when processoritem is deleted)
    delete painterWidget_;
    delete networkserializer_;
    delete graphSerializer_;
    delete evaluator_;
    delete transferFuncPlugin_;
    delete geoContainer_;
    delete volsetContainer_;
}

void RptMainWindow::createConnections() {

    // create tool windows now, after everything is initialized
    createToolWindows();
    
    // restore session
    if (sessionArray_.size() > 0)
        restoreState(sessionArray_);

    // create connections
    connect(graphWidget_, SIGNAL(processorAdded(Identifier, QPoint)), this, SLOT(addProcessorItem(Identifier, QPoint)));
    connect(graphWidget_, SIGNAL(aggregationAdded(std::string, QPoint)), this, SLOT(addAggregationSlot(std::string, QPoint)));
    connect(graphWidget_, SIGNAL(processorSelected(Processor*)), this, SLOT(processorSelected(Processor*)));
    connect(graphWidget_, SIGNAL(copySignal()), this, SLOT(copyButtonPushed()));
    connect(graphWidget_, SIGNAL(pasteSignal()), this, SLOT(pasteButtonPushed()));

    connect(evaluatorAction_, SIGNAL(triggered()), this, SLOT(evaluatorButtonPushed()));
    connect(openFileAction_,SIGNAL(triggered()), this,SLOT(openFileButtonPushed()));
    connect(connectAction_,SIGNAL(triggered()), this,SLOT(connectButtonPushed()));
    connect(clearAction_,SIGNAL(triggered()), this,SLOT(clearNetwork()));
    connect(openNetworkFileAction_,SIGNAL(triggered()), this,SLOT(openNetworkFileButtonPushed()));
    connect(saveNetworkAction_,SIGNAL(triggered()), this,SLOT(saveNetworkButtonPushed()));
    connect(saveNetworkAsAction_,SIGNAL(triggered()), this,SLOT(saveNetworkAsButtonPushed()));
    connect(insertAggregationAction_, SIGNAL(triggered()), this, SLOT(insertAggregation()));
    connect(deaggregateAction_, SIGNAL(triggered()), this, SLOT(deaggregate()));
    connect(selectAllAction_, SIGNAL(triggered()), this, SLOT(selectAll()));
    connect(showAggregationContentAction_, SIGNAL(triggered()), this, SLOT(showAggregationContent()));
    connect(setReuseTargetsAction_, SIGNAL(triggered()), this, SLOT(setReuseTargets()));
    connect(setLoadVolumeSetContainerAction_, SIGNAL(triggered()), this, SLOT(setLoadVolumeSetContainer()));
    connect(copyAction_, SIGNAL(triggered()), this, SLOT(copyButtonPushed()));
    connect(pasteAction_, SIGNAL(triggered()), this, SLOT(pasteButtonPushed()));
    connect(createPropSetAction_, SIGNAL(triggered()), this, SLOT(createPropertySet()));
    connect(processorListWidget_, SIGNAL(itemSelectionChanged()), this, SLOT(changeProcessorInfo()));
    connect(navigationGroup_, SIGNAL(triggered(QAction*)), this, SLOT(navigationActionTriggered(QAction*)));

    // iterate over recent files
    for (int i = 0; i < maxRecents; ++i)
        connect(recentFileActs[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));

    // if item is double clicked, show properties
    connect(graphWidget_, SIGNAL(showPropertiesSignal()), this, SLOT(showProperties()));
    connect(connectCanvasModAct_, SIGNAL(toggled(bool)), this, SLOT(connectCanvasModifier(bool)));
    connect(resetSettingsAct_, SIGNAL(toggled(bool)), this, SLOT(setResetSettings(bool)));

    // signals indicating a change in network
    connect(graphWidget_, SIGNAL(processorAdded(Identifier, QPoint)), this, SLOT(networkModified()));
    connect(graphWidget_, SIGNAL(pasteSignal()), this, SLOT(networkModified()));
    connect(graphWidget_, SIGNAL(aggregationAdded(std::string, QPoint)), this, SLOT(networkModified()));
    connect(insertAggregationAction_, SIGNAL(triggered()), this, SLOT(networkModified()));
    connect(deaggregateAction_, SIGNAL(triggered()), this, SLOT(networkModified()));
    connect(pasteAction_, SIGNAL(triggered()), this, SLOT(networkModified()));
    connect(createPropSetAction_, SIGNAL(triggered()), this, SLOT(networkModified()));

    //
    // now the GUI is complete
    //
    
    // load an initial network
    if (!defaultNetwork.isEmpty()) {
        qApp->processEvents(); // workaround for initial rendering problem
        openNetworkFile(defaultNetwork);
    }

    // load an initial dataset
    if (!defaultDataset.isEmpty())
        loadDataset(defaultDataset.toStdString(), false);    
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
    for (size_t i=0; i < copyPasteProcessors_.size(); i++)
      delete copyPasteProcessors_[i];

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

    // Get all the items that are to be copied.
    QList<QGraphicsItem*> selectedItems = graphWidget_->scene()->selectedItems();

    // Now check what type of items these are and put them in the appropriate vectors.
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

	// Go through every aggregation and do some stuff
	for (size_t i=0; i < copyPasteAggregations_.size(); i++) {
		RptAggregationItem* currentItem = copyPasteAggregations_.at(i);
		std::vector<RptProcessorItem*> currentVector = currentItem->getProcessorItems();
		std::vector<int> processorItemsInAggregation;

		// The aggregationMap saves the information which processors are in which aggregation.
		// Insert the current aggregation number into the map as the key, and the vector as the
		// value, which is empty in the beginning
		aggregationMap_.insert(std::pair<int, std::vector<int> >(numberOfAggregations_,processorItemsInAggregation));

		// Now go through the processorItems in the aggregation and put them into the
		// processors vector (all processors in that vector will be copied) and their numbers
		// into the aggregationMap. This number is the position the processor has in the
		// copyPasteProcessors_ vector.
		for (size_t j=0; j<currentVector.size(); j++) {

			// Put the processor into the vector that holds all processors that are to be copied
			copyPasteProcessors_.push_back(currentVector.at(j));

			// The processor now has a number (its position in the copyPasteProcessors_
			//vector), and we store that number in the vector in the aggregationMap.
			aggregationMap_[numberOfAggregations_].push_back(copyPasteProcessors_.size()-1);

			// Do the same for the aggregations name. (Yes, not nice, we could/should use maybe
			// a struct, so that we only need one map)
			aggregationNameMap_.insert(std::pair<int,std::string>(numberOfAggregations_,currentItem->getName() ) );
		}

		numberOfAggregations_++;
	}

	// Now save information about propertysets
	for (size_t i=0; i < copyPastePropertySets_.size(); i++) {

		// Get all the guiItems to which the propertyset is connected
		std::vector<RptGuiItem*> guiItems = copyPastePropertySets_.at(i)->getGuiItems();

		// These two vectors hold the processors and the aggregations this propertyset is
		// connected to. They have to be handled differently, that's why we use two vectors.
		std::vector<int> currentProcessorVector;
		std::vector<int> currentAggregationVector;

		// Insert these two vectors into the two maps that hold the information to which
		//processors and aggregations the propertyset is connected to.
		propertySetMap_.insert(std::pair<int,std::vector<int> >(numberOfPropertySets_,currentProcessorVector) );
		propertySetAggregationMap_.insert(std::pair<int,std::vector<int> >(numberOfPropertySets_,currentAggregationVector) );

		// Now go through the guiItems and insert them into the appropriate vector, depending
		// on their type (processor or aggregation)
		for (size_t j=0; j<guiItems.size(); j++) {

			// Is it a processor?
			RptProcessorItem* temp = dynamic_cast<RptProcessorItem*>(guiItems.at(j));
			if (temp) {

				// If yes, search for the processor in the copyPasteProcessors_ vector and return its position
				int number=findProcessor(copyPasteProcessors_,temp->getProcessor());
				if (number!= -1)

					// If that worked, we put that number into the vector in the map, thereby
					// saving that this processor was a member of the propertyset.
					propertySetMap_[numberOfPropertySets_].push_back(number);
			} else {
				// If it's not a processor, check if it's an aggregation.
				RptAggregationItem* temp2 = dynamic_cast<RptAggregationItem*>(guiItems.at(j));
				if (temp2) {
					int number=-1;

					// If yes, search for the aggregation in the copyPasteAggregations_ vector
					// and get its position.
					for (size_t k=0; k<copyPasteAggregations_.size(); k++) {
						if (copyPasteAggregations_.at(k) == temp2) {
							number=k;
							break;
						}
					}
					if (number != -1) {
						// If that worked, we save that position in the vector in the map,
						// thereby saving that this aggregation was a member of the
						// propertyset.
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

				// Get the processor of the connected port and find its position in the
				// copyPasteProcessors_ vector, which is its id.
				int connectedProcessorNumber = findProcessor(copyPasteProcessors_,connectedPorts.at(k)->getProcessor());

				// If -1, the connectedProcessor was not copied, so we can't save that connection.
				if (connectedProcessorNumber != -1) {
					// Save the id of the connected processor and the type of the connected
					// port in the vector of the PortConnection object.
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

    // Everything is duplicated, so assign the vector holding the duplicated processorItems to
    // the copyPasteProcessors_ vector. These processorItems are then added to the scene and
    // connected as soon as the paste button is pushed.
    copyPasteProcessors_ = copyProcessors();
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

								if (oldProp->get() ==1) {
									if (prop) {
									   prop->set(true);
									   newProcessor->postMessage(new BoolMsg(prop->getIdent(), true));
									}
                                } else {
									if (prop) {
										prop->set(false);
										newProcessor->postMessage(new BoolMsg(prop->getIdent(), false));
									}
                                }
                            } else {
								BoolProp* oldProp = dynamic_cast<BoolProp*>(currentProp);
                                BoolProp* prop = dynamic_cast<BoolProp*>(props.at(j));

								if (oldProp->get()==1) {
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
							 TransFuncIntensity* oldTransFerFunc = dynamic_cast<TransFuncIntensity*>(temp);
							 TransFuncIntensity* tf = new TransFuncIntensity();
							 tf->clearKeys();
							 for (size_t k=0; k<oldTransFerFunc->getKeys().size(); k++) {
								 TransFuncMappingKey* newKey
                                     = new TransFuncMappingKey(oldTransFerFunc->getKeys().at(k)->getIntensity(),
                                                               oldTransFerFunc->getKeys().at(k)->getColorR());
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
        if (!MsgDistr.contains(newProcessorItem->getProcessor()))
            MsgDistr.insert(newProcessorItem->getProcessor());

        //connect them to some functions, so that they can be deleted and aggregated etc.
        connect(newProcessorItem, SIGNAL(deleteSignal()), this, SLOT(deleteSlot()));
        connect(newProcessorItem, SIGNAL(aggregateSignal()), this, SLOT(insertAggregation()));
        connect(newProcessorItem, SIGNAL(changed()), this, SLOT(networkModified()));

    }

    //Now that they are added, they can be connected.

    RptProcessorItem* currentGuiItem;
    ConnectionInfoForOneProcessor* currentInfo;

    // Go through the connection information created when the copy button was pushed and connect
    // the processorItems.
    for (size_t i=0;i < copyPasteConnectionInfos_.size();i++) {

        //Each element in copyPasteConnectionInfos_ stores the connection info for one ProcessorItem
        //This ProcessorItem is searched by its id (its position in the pastedProcessors_) vector, which
        //is the same as its position in the copyPasteProcessors_ vector in the copyButtonPushed() function,
        //so this actually works)
        currentInfo = copyPasteConnectionInfos_.at(i);
        currentGuiItem = pastedProcessors_.at(currentInfo->id);

        if (!currentGuiItem)
            return;

        // Get the input connection informations for this processor
        std::vector<PortConnection*> in;
        in=currentInfo->inputs;

        //iterate throuh it and connect the ProcessorItems
        for (size_t j=0; j<in.size(); j++) {
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
	} // for loop through copyPasteConnectionInfos

	//All the processors are created and connected, now aggregate them (if they were aggregated when they were copied)

	//this vector will hold all the aggregations we create now
	std::vector<RptAggregationItem*> createdAggregations;

	//Go through the map containing the information which processors should be aggregated
	for (std::map<int,std::vector<int> >::iterator i = aggregationMap_.begin(); i != aggregationMap_.end(); ++i) {

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
		for (size_t j=0; j< processorNumbers.size(); j++) {
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

Volume* RptMainWindow::loadDataset(const std::string& filename, bool showProgress) {
    if (volumeSetWidget_ == 0)
        return 0;

    if (!showProgress)
        volumeSetWidget_->setUseProgress(false);  
    VolumeSet* volumeSet = volumeSetWidget_->loadVolumeSet(filename);
    if (!showProgress)
        volumeSetWidget_->setUseProgress(true);

    if (volumeSet != 0)
        return volumeSet->getFirstVolume();
    else
        return 0;
}

void RptMainWindow::clearDataVectors() {
    // Delete all processors FIRST
    //
    for (size_t i = 0; i < processors_.size(); i++) {
        delete processors_[i];
        processors_[i] = 0;
    }
    processors_.clear();
    evaluator_->getProcessors().clear();

    // Delete all aggregations afterwards. Not doing this will crash the
    // crash the application. The reason therefore is still unknown to me.
    // (Dirk)
    //
    for (size_t i = 0; i < aggregations_.size(); i++) {
        delete aggregations_[i];
        aggregations_[i] = 0;
    }
    aggregations_.clear();

    // Delete all property sets
    //
    for (size_t i = 0; i < propertySets_.size(); i++) {
        delete propertySets_[i];
        propertySets_[i] = 0;
    }
    propertySets_.clear();
}

void RptMainWindow::clearNetwork() {
    if (isWindowModified()) {
        QMessageBox msgBox;
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText("Save changes for the current network?");
        if (msgBox.exec() == QMessageBox::Yes) {
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
    updateWindowTitle();
    setWindowModified(false);
}

void RptMainWindow::addToScene(RptNetwork& rptnet) {
    // Use new VolumeSetContainer if there is one in the Network
    if (rptnet.volumeSetContainer) {
        delete volsetContainer_;
        volsetContainer_ = rptnet.volumeSetContainer;
        volumeSetWidget_->updateContent(volsetContainer_);
    }

    // ProcessorsItems
    std::vector<RptProcessorItem*> processorItems = rptnet.processorItems;
    for (size_t i=0; i< processorItems.size(); i++) {
        initializeProcessorItem(processorItems[i]);
        processorItems[i]->showAllArrows();
    }

    // PropertySetItems
    for (size_t i=0; i< rptnet.propertySetItems.size(); i++) {
        RptPropertySetItem* propertySetItem = rptnet.propertySetItems.at(i);
        graphWidget_->addItem(propertySetItem);
        propertySetItem->showAllArrows();
        propertySets_.push_back(propertySetItem);
        connect(propertySetItem, SIGNAL(deleteSignal()), this, SLOT(deleteSlot()));
    }

    addAggregationsToScene(rptnet);

    setReuseTargetsAction_->setChecked(rptnet.reuseTCTargets);
    setReuseTargets();

    //adjust scaling
    scaleView(0.85f);
    
    // placed here, because loading a network emits changed signals
    setWindowModified(false);
}

void RptMainWindow::addAggregationsToScene(RptNetwork& rptnet, QPoint pos) {
    // Use new VolumeSetContainer if there is one in the Network
    if (rptnet.volumeSetContainer) {
        delete volsetContainer_;
        volsetContainer_ = rptnet.volumeSetContainer;
        volumeSetWidget_->updateContent(volsetContainer_);
    }

    for (size_t i=0; i< rptnet.aggregationItems.size(); i++) {
        RptAggregationItem* aggregationItem = rptnet.aggregationItems.at(i);
        std::vector<RptProcessorItem*> procItems = aggregationItem->getProcessorItems();
        //init all processors in the aggregation
        for (size_t j=0; j<procItems.size(); j++) {
            initializeProcessor(procItems[j]->getProcessor());
        }
        if (pos == QPoint(0, 0))
            graphWidget_->addItem(aggregationItem);
        else {
            aggregationItem->setPos(0, 0);
            graphWidget_->addItem(aggregationItem, pos);
        }
        aggregationItem->initialize();
        aggregations_.push_back(aggregationItem);
        connect(aggregationItem, SIGNAL(deaggregateSignal()), this, SLOT(deaggregate()));
        connect(aggregationItem, SIGNAL(deleteSignal()), this, SLOT(deleteSlot()));
        connect(aggregationItem, SIGNAL(saveSignal(RptAggregationItem*)),
                this, SLOT(saveAggregationSlot(RptAggregationItem*)));
    }
}

void RptMainWindow::scaleView(float maxFactor) {
    // justify graphWidget
    QRectF sceneRect = graphWidget_->getScene()->itemsBoundingRect();
    graphWidget_->getScene()->setSceneRect(sceneRect);
    // calculate scale factors
    float scaleFactor = std::min(static_cast<float>(graphWidget_->width() / sceneRect.width()),
                                 static_cast<float>(graphWidget_->height() / sceneRect.height()));
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
    MsgDistr.postMessage(new BoolMsg(NetworkEvaluator::setReuseTextureContainerTargets_,
                                     setReuseTargetsAction_->isChecked()), "evaluator");
}

void RptMainWindow::setLoadVolumeSetContainer() {
    loadVolumeSetContainer_ = setLoadVolumeSetContainerAction_->isChecked();
}

void RptMainWindow::selectAll() {
	for (size_t i=0; i < processors_.size(); i++)
		processors_.at(i)->setSelected(true);

    for (size_t i=0; i< aggregations_.size(); i++)
		aggregations_.at(i)->setSelected(true);

    for (size_t i=0; i< propertySets_.size(); i++) 
		propertySets_.at(i)->setSelected(true);

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

    RptNetwork rptnet = RptNetwork();
    rptnet.processorItems = r;
    rptnet.aggregationItems.push_back(aggregation);
    rptnet.propertySetItems = p;
    rptnet.reuseTCTargets = false; // FIXME: dont want to set this
    networkserializer_->serializeToXml(graphSerializer_->makeProcessorNetwork(rptnet), filename);
    
    aggregationListWidget_->buildItems();
}

void RptMainWindow::saveNetworkButtonPushed() {
    if (!currentFile_.isEmpty()) {
        saveCurrentNetwork();
        setWindowModified(false);
    }
    else
        saveNetworkAsButtonPushed();
}

void RptMainWindow::saveNetworkAsButtonPushed() {
    QFileDialog fileDialog(this,
                           tr("Save network as"),
                           networkPath_,
                           tr("Voreen network files (*.vnw)"));
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setConfirmOverwrite(true);
    if (fileDialog.exec()) {
        currentFile_ = fileDialog.selectedFiles().at(0);

        // ensure file extension if none was given

        //FIXME: when the user specifies "foobar" this will overwrite an existing "foobar.vnw"
        // without asking the user.
        QFileInfo f(currentFile_);
        if (f.suffix().isEmpty())
            currentFile_ += ".vnw";

        saveCurrentNetwork();

        networkPath_ = fileDialog.directory().path();
        setWindowModified(false);
        updateWindowTitle();       
        addCurrentFileToRecents();
    }
}

void RptMainWindow::saveCurrentNetwork() {
    RptNetwork rptnet = RptNetwork();
    rptnet.processorItems = processors_;
    rptnet.aggregationItems = aggregations_;
    rptnet.propertySetItems = propertySets_;
    rptnet.reuseTCTargets = setReuseTargetsAction_->isChecked();
    rptnet.volumeSetContainer = volsetContainer_;
    rptnet.serializeVolumeSetContainer = true;
    networkserializer_->serializeToXml(graphSerializer_->makeProcessorNetwork(rptnet),
                                       currentFile_.toStdString());
}

void RptMainWindow::openNetworkFileButtonPushed() {
    askSaveNetwork();
    OpenNetworkFileDialog fd(this, tr("Choose a file to open"), QDir(networkPath_).absolutePath());
    fd.setLoadVolumeSetContainer(loadVolumeSetContainer_);
    if (fd.exec()) {
        networkPath_ = fd.directory().path();
        setLoadVolumeSetContainerAction_->setChecked(fd.loadVolumeSetContainer());
        setLoadVolumeSetContainer();
        openNetworkFile(fd.selectedFiles().at(0), fd.loadVolumeSetContainer());
    }
}

void RptMainWindow::openNetworkFile(const QString& filename, bool loadVolumeSetContainer) {
    RptNetwork rptnet;
    clearScene();
    try {
        rptnet = graphSerializer_->makeRptNetwork(networkserializer_->readNetworkFromFile(filename.toStdString(),
                                                                                          loadVolumeSetContainer));
        currentFile_ = filename;
        addCurrentFileToRecents();
        updateWindowTitle();
        addToScene(rptnet);
        showNetworkErrors(rptnet);
    } catch (SerializerException e) {
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage(e.what());
        LWARNINGC("VoreenVe.RptMainWindow", e.what());
    }
}

void RptMainWindow::clearScene() {
    // clear all vectors containing network related items
    clearDataVectors();
    // clear containers from existing parts of
    // previously rendered networks
    geoContainer_->clearDeleting();
    graphWidget_->clearScene();
}

void RptMainWindow::askSaveNetwork() {
    if (isWindowModified()) {
        QMessageBox msgBox;
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText("Save changes for the current network?");
        if (msgBox.exec() == QMessageBox::Yes) {
            saveNetworkAsButtonPushed();
        }
    }
}

bool RptMainWindow::askLoadVolumeSetContainer(QString filename) {
    if (networkserializer_->hasVolumeSetContainer(filename.toStdString())) {
        QMessageBox msgBox;
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setText("This network has associated Datasets. Do you want to load them?");
        return (msgBox.exec() == QMessageBox::Yes);
    }
    return false;
}

void RptMainWindow::showNetworkErrors(const RptNetwork& rptnet) {
    //alert about errors in the Network
    // TODO: good error classes and proper display
    if (!rptnet.errors.empty()) {
        std::stringstream errormessage;
        errormessage << "There were " << rptnet.errors.size() << " errors loading the network.";
        QMessageBox::warning(this, "Errors", errormessage.str().c_str(), QMessageBox::Ok);
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

void RptMainWindow::createLoadedAggregations(std::map<int,std::vector<RptProcessorItem*>* > aggroList,
                                             std::map<int,std::string> aggroNameMap, QPoint pos)
{
    int number=0;
    for (std::map<int,std::vector<RptProcessorItem*>*>::const_iterator i = aggroList.begin(); i != aggroList.end(); ++i) {
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

    if (evaluator_->analyze() >= 0) {
        if (!painterWidget_->setEvaluator(evaluator_)) {
            QApplication::restoreOverrideCursor();
            QMessageBox::critical(this, tr("Error"),
                                  tr("Initialization of one or more processors failed.\n"
                                     "Please check the console or log file for error messages."),
                                  QMessageBox::Ok);
        }
    }
    dumpProcessorConnectionInfos();

#ifdef VRN_MODULE_MEASURING
	selectionProcessorListWidget_->setEvaluator(evaluator_);
    measuringProcessorListWidget_->setEvaluator(evaluator_);
#endif
    transferFuncPlugin_->setEvaluator(evaluator_);
    QApplication::restoreOverrideCursor();
}

void RptMainWindow::openRecentFile() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        QString file(action->data().toString());
        if (!file.isEmpty())
            openNetworkFile(file, loadVolumeSetContainer_);
    }
}

void RptMainWindow::addAggregationSlot(std::string filename, QPoint pos) {
    try {
        RptNetwork rptnet = graphSerializer_->makeRptNetwork(networkserializer_->readNetworkFromFile(filename, false));
        addAggregationsToScene(rptnet, pos);
    } catch (SerializerException e) {
        QErrorMessage* errorMessageDialog = new QErrorMessage(this);
        errorMessageDialog->showMessage(e.what());
        LWARNINGC("VoreenVe.RptMainWindow", e.what());
    }
}

void RptMainWindow::openFileButtonPushed() {
    if (!volumeSetWidget_)
        return;

    std::vector<std::string> files = volumeSetWidget_->openFileDialog();
    if (!files.empty())
        volumeSetWidget_->addVolumeSets(files);
}

void RptMainWindow::initializeProcessorItem(RptProcessorItem* item, QPoint pos) {
    initializeProcessor(item->getProcessor());

    processors_.push_back(item);
    if (pos == QPoint(0, 0))
        graphWidget_->addItem(item);
    else
        graphWidget_->addItem(item, pos);

    connect(item, SIGNAL(deleteSignal()), this, SLOT(deleteSlot()));
    connect(item, SIGNAL(aggregateSignal()), this, SLOT(insertAggregation()));
}

void RptMainWindow::initializeProcessor(Processor* processor) {
    VolumeSetSourceProcessor* vssp = dynamic_cast<VolumeSetSourceProcessor*>(processor);
    if (vssp != 0) {
        vssp->setVolumeSetContainer(volsetContainer_);
    }

    processor->setTextureContainer(tc_);
	processor->setCamera(camera_);

    if (!MsgDistr.contains(processor))
	    MsgDistr.insert(processor);
}

void RptMainWindow::addProcessorItem(Identifier type, QPoint pos) {
    RptProcessorItem* rgi = new RptProcessorItem(type);
    initializeProcessorItem(rgi, pos);
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
    //contentTab_->setCurrentIndex(1);
	propertyListTool_->setVisible(true);
}

void RptMainWindow::createAggregation(std::vector<RptProcessorItem*> items,
                                      std::string name, QPoint pos)
{
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
            if (selectedItems[i]->parentItem() && selectedItems[i]->parentItem()->type() != RptAggregationItem::Type) {
                items.push_back(static_cast<RptProcessorItem*>(selectedItems[i]));
            }
        }
    }

    createAggregation(items, "Aggregation");
}

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
        for (size_t j=0; j<items.size(); j++)
            processors_.push_back(items[j]);

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

void RptMainWindow::processorSelected(Processor* processor){   
	transferFuncPlugin_->findAndSetProcessor(processor);
    propertyListWidget_->setProcessor(processor);
	changeProcessorInfo();
}

void RptMainWindow::runScript() {
#ifdef VRN_WITH_PYTHON
    QString filename = QFileDialog::getOpenFileName(this, tr("Run script"), "../../data/scripts",
        "Python scripts (*.py)");
    if (!filename.isEmpty()) {
        tgt::Script* script = ScriptMgr.load(filename.toStdString(), false);
        if (script->compile()) {
            if (!script->run())
                QMessageBox::warning(this, "Voreen", tr("Python runtime error (see stdout)"));

        } else {
            QMessageBox::warning(this, "Voreen", tr("Python compile error (see stdout)"));
        }
        ScriptMgr.dispose(script);
    }
#else
    QMessageBox::warning(this, "Voreen", tr("Voreen and tgt have been compiled without "
                                            "Python support\n"));
#endif // VRN_WITH_PYTHON
}

void RptMainWindow::navigationActionTriggered(QAction* /*action*/) {

    if (trackballNaviAction_->isChecked())
        painterWidget_->setCurrentNavigation(RptPainterWidget::TRACKBALL_NAVIGATION);
    else
        painterWidget_->setCurrentNavigation(RptPainterWidget::FLYTHROUGH_NAVIGATION);

}

void RptMainWindow::initGL(){
    ShdrMgr.addPath("../../src/core/vis/glsl");
    const int finalTarget = 20;
    tc_ = evaluator_->initTextureContainer(finalTarget);

    id1_.setTC(tc_);

    painterWidget_->init(tc_, camera_);
    painterWidget_->makeCurrent();

    // initialize OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // now we can activate rendering in the widget
    painterWidget_->setUpdatesEnabled(true);
}

VoreenToolWindow* RptMainWindow::addToolWindow(QAction* action, QWidget* widget, const QString& name) {
    action->setCheckable(true);
    toolsMenu_->addAction(action);
    VoreenToolWindow* window = new VoreenToolWindow(action, this, widget, name);
    toolbar_->registerToolWindow(window);
    window->adjustSize(); // prevents strange sizes written to config file

    toolWindows_ << window;
    return window;
}


void RptMainWindow::createToolWindows() {

    ProcessorFactory::getInstance()->setTextureContainer(tc_);

    // network editor
    graphWidget_ = new RptGraphWidget();
    addToolWindow(new QAction(QIcon(":/icons/mapping-function-inverted.png"), tr("Network Editor"), this),
                  graphWidget_, "NetworkEditor");
                
	// property list
	propertyListWidget_ = new RptPropertyListWidget();
	propertyListWidget_->setPainter(dynamic_cast<VoreenPainter*>(painterWidget_->getPainter()));
    propertyListTool_ = addToolWindow(new QAction(QIcon(":/icons/information.png"), tr("Property Editor"), this),
                                      propertyListWidget_, "PropertyList");

	// transfer function
	transferFuncPlugin_ = new TransFuncPlugin(this, dynamic_cast<VoreenPainter*>(painterWidget_->getPainter()),
                                              0, Qt::Horizontal);
	transferFuncPlugin_->createWidgets();
	transferFuncPlugin_->createConnections();
    addToolWindow(new QAction(QIcon(":/icons/colorize.png"), tr("Transfer Function Editor"), this),
                  transferFuncPlugin_, "TransferFunction");
    
	// processor list
	processorListWidget_ = new RptProcessorListWidget();
	graphWidget_->addAllowedWidget(processorListWidget_);
    processorListWidget_->setMinimumSize(200, 200);
    addToolWindow(new QAction(QIcon(":/icons/console.png"), tr("Processors"), this),
                  processorListWidget_, "ProcessorList");

	// processor info box
	processorInfoBox_ = new QTextBrowser(0);
	processorInfoBox_->setText("This box contains the info for the selected processor");
    processorInfoBox_->setMinimumSize(100, 100);
    addToolWindow(new QAction(QIcon(":/icons/eye.png"), tr("Processor Info"), this),
                  processorInfoBox_, "ProcessorInfo");

    // aggregation list
	aggregationListWidget_ = new RptAggregationListWidget();
	graphWidget_->addAllowedWidget(aggregationListWidget_);
    aggregationListWidget_->setMinimumSize(100, 200);
    addToolWindow(new QAction(QIcon(":/icons/segmentation.png"), tr("Aggregations"), this),
                  aggregationListWidget_, "AggregationList");

    // volumes
    addToolWindow(new QAction(QIcon(":/icons/paper-clip.png"), tr("Volumes"), this),
                  volumeSetWidget_, "Volumes");

    // orientation
    OrientationPlugin* orientationPlugin
        = new OrientationPlugin(this, dynamic_cast<VoreenPainter*>(painterWidget_->getPainter()), 
                                painterWidget_, painterWidget_->getTrackballNavigation()->getTrackball());
    orientationPlugin->createWidgets();
    orientationPlugin->createConnections();
    painterWidget_->getTrackballNavigation()->addReceiver(orientationPlugin);
    addToolWindow(new QAction(QIcon(":/icons/trackball-reset-inverted.png"), tr("Camera Orientation"), this),
                  orientationPlugin, "Orientation");

    // animation
    AnimationPlugin* animationPlugin = new AnimationPlugin(this, camera_, painterWidget_);
    animationPlugin->createWidgets();
    animationPlugin->createConnections();
    addToolWindow(new QAction(QIcon(":/icons/movie.png"), tr("Animation"), this), animationPlugin, "Animation");

    // texture container
	ShowTexContainerWidget* texContainerWidget = new ShowTexContainerWidget(painterWidget_);
	texContainerWidget->setTextureContainer(tc_);
    texContainerWidget->setMinimumSize(200, 200);
    VoreenToolWindow* tc = addToolWindow(new QAction(QIcon(":/icons/grid.png"), tr("Texture Container"), this),
                                         texContainerWidget, "TextureContainer");
    tc->resize(500, 500);

    
    // Restore visiblity, position and size of tool windows from settings
    if (!resetSettings_) {
        settings_.beginGroup("Windows");
        for (int i=0; i < toolWindows_.size(); ++i) {
            if (!toolWindows_[i]->objectName().isEmpty()) {
                settings_.beginGroup(toolWindows_[i]->objectName());
                if (settings_.contains("size"))
                    toolWindows_[i]->resize(settings_.value("size").toSize());

                // Ignore position (0, 0) for invisible windows as otherwise all previously
                // invisible windows would be placed at (0, 0) after restarting the application.
                if (settings_.contains("pos") &&
                    (settings_.value("pos").toPoint() != QPoint(0, 0) || settings_.value("visible").toBool()))
                {
                    toolWindows_[i]->move(settings_.value("pos").toPoint());
                }
                
                if (settings_.contains("visible")) {
                    toolWindows_[i]->setVisible(settings_.value("visible").toBool());
                }
                else if (toolWindows_[i]->objectName() == "NetworkEditor") {
                    // show network editor on first start
                    toolWindows_[i]->setVisible(true);
#if defined(WIN32) || defined(__APPLE__)
                    // move to the right
                    toolWindows_[i]->move(x() + frameGeometry().width() + 2, y());
#endif
                }
                settings_.endGroup();    
            }
        }
        settings_.endGroup();
    } else {
        for (int i=0; i < toolWindows_.size(); ++i) {
            if (toolWindows_[i]->objectName() == "NetworkEditor") {
                // show network editor
                toolWindows_[i]->setVisible(true);
#if defined(WIN32) || defined(__APPLE__)
                // move to the right
                toolWindows_[i]->move(x()+ frameGeometry().width() + 2, y());
#endif
            }
        }
    }
        
    // Restore toolbar position
    settings_.beginGroup("Windows");
    settings_.beginGroup(toolbar_->objectName());
    if (settings_.contains("pos") && !resetSettings_)
        toolbar_->move(settings_.value("pos").toPoint());
    else
        toolbar_->move(painterWidget_->mapToGlobal(QPoint(20, 20)));

    settings_.endGroup();    
    settings_.endGroup();    

    // Now show the toolbar, after it is fully initialized
    toolbar_->show();

    if (resetSettings_) {
        QMessageBox::information(this, tr("VoreenVE"), tr("Configuration reset."));
        resetSettings_ = false;
    }

    // measuring
#ifdef VRN_MODULE_MEASURING
    // List for SelectionProcessors
    selectionProcessorListWidget_ = new SelectionProcessorListWidget(evaluator_,0);
    selectionProcessorPropertyListWidget_ = new RptPropertyListWidget(0);
	//List of MeasuringProcessors
	measuringProcessorListWidget_ = new MeasuringProcessorListWidget(evaluator_,0);
    measuringProcessorPropertyListWidget_ = new RptPropertyListWidget(0);

    measuringLayout_ = new LayoutContainer(this);
    
	//Add SelectionProcessorList
	measuringLayout_->addWidget(selectionProcessorListWidget_);
	measuringLayout_->addWidget(selectionProcessorPropertyListWidget_);
    //Add MeasuringProcessorListWidgets
    measuringLayout_->addWidget(measuringProcessorListWidget_);
	measuringLayout_->addWidget(measuringProcessorPropertyListWidget_);

	contentTab_->insertTab(3, measuringLayout_, "Measuring Actions");
#endif
}

void RptMainWindow::createMenuAndToolBar() {
	menu_ = menuBar();						//create menuBar

	fileMenu_ = menu_->addMenu(tr("&File"));
	editMenu_ = menu_->addMenu(tr("&Edit"));
	toolsMenu_ = menu_->addMenu(tr("&Tools"));
	optionsMenu_ = menu_->addMenu(tr("&Options"));
    helpMenu_ = menu_->addMenu(tr("&Help"));

	selectAllAction_ = new QAction(tr("Select all"), this);
	selectAllAction_->setShortcut(tr("Ctrl+A"));

	copyAction_ = new QAction(tr("Copy"), this);
	copyAction_->setShortcut(tr("Ctrl+C"));

	pasteAction_ = new QAction(tr("Paste"), this);
	pasteAction_->setShortcut(tr("Ctrl+V"));

    QAction* deleteAction = new QAction(QIcon(":/vrn_app/icons/eraser.png"),tr("Delete"), this);
    deleteAction->setShortcut(tr("Del"));

    insertAggregationAction_ = new QAction(tr("Aggregate"), this);

    showAggregationContentAction_ = new QAction("Show Aggregation Content", this);
    showAggregationContentAction_->setCheckable(true);
    showAggregationContentAction_->setChecked(false);

    connectCanvasModAct_ = new QAction(tr("&Connect CanvasModifier"), this);
    connectCanvasModAct_->setCheckable(true);
    resetSettingsAct_ = new QAction(tr("&Reset Settings (needs restart)"), this);
    resetSettingsAct_->setCheckable(true);
    resetSettingsAct_->setChecked(false);

    deaggregateAction_ = new QAction(tr("Deaggregate"), this);

    createPropSetAction_ = new QAction(tr("Create Property Set"), this);

    connectAction_ = new QAction(QIcon(":/icons/mapping-function.png"),
                                 tr("Auto-connect Processors"), this);
    connectAction_->setShortcut(tr("Alt+C"));
    connectAction_->setStatusTip(tr("Automatically connects selected processors"));

    helpFirstStepsAct_ = new QAction(QIcon(":/vrn_app/icons/wizard.png"),
                                        tr("Getting Started..."), this);
    connect(helpFirstStepsAct_, SIGNAL(triggered()), this, SLOT(helpFirstSteps()));
    helpMenu_->addAction(helpFirstStepsAct_);

    helpMenu_->addSeparator();    
    
    aboutAction_ = new QAction(QIcon(":/vrn_app/icons/about.png"), tr("&About..."), this);
    connect(aboutAction_, SIGNAL(triggered()), this, SLOT(helpAbout()));
    helpMenu_->addAction(aboutAction_);
    
    scriptAction_ = new QAction(QIcon("icons/python.png"), tr("Run Python Script..."), this);
    scriptAction_->setShortcut(tr("F7"));
    scriptAction_->setStatusTip(tr("Select and run a python script"));
    scriptAction_->setToolTip(tr("Run a python script"));
    connect(scriptAction_, SIGNAL(triggered()), this, SLOT(runScript()));

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
    editMenu_->addSeparator();
    editMenu_->addAction(scriptAction_);

    setReuseTargetsAction_ = new QAction("Reuse TC targets (needs rebuild)", this);
    setReuseTargetsAction_->setCheckable(true);
    setReuseTargetsAction_->setChecked(false);

    trackballNaviAction_ = new QAction(tr("Trackball navigation"), this);
    trackballNaviAction_->setCheckable(true);
    trackballNaviAction_->setChecked(true);
    flythroughNaviAction_ = new QAction(tr("Flythrough navigation"), this);
    flythroughNaviAction_->setCheckable(true);
    navigationGroup_ = new QActionGroup(this);
    navigationGroup_->addAction(trackballNaviAction_);
    navigationGroup_->addAction(flythroughNaviAction_);
    
    navigationMenu_ = optionsMenu_->addMenu(tr("Select camera navigation..."));
    navigationMenu_->addAction(trackballNaviAction_);
    navigationMenu_->addAction(flythroughNaviAction_);
    optionsMenu_->addSeparator();
    optionsMenu_->addAction(setReuseTargetsAction_);
    optionsMenu_->addAction(connectCanvasModAct_);
    optionsMenu_->addAction(resetSettingsAct_);

    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteSlot()));

    //actions to be put into the toolbar
    evaluatorAction_ = new QAction(QIcon(":/icons/player_play.png"),"Use the current network to render the data set.", this);
    evaluatorAction_->setShortcut(tr("F9"));

    clearAction_ = new QAction(QIcon(":/icons/clear.png"),tr("New Network"), this);
    clearAction_->setStatusTip(tr("Create a new network"));
    clearAction_->setShortcut(tr("Ctrl+N"));

    openFileAction_ = new QAction(QIcon(":/vrn_app/icons/open_dicom.png"),"Open Data Set...", this);
    openFileAction_->setShortcut(tr("Ctrl+O"));
    openFileAction_->setStatusTip(tr("Open a volume data set"));

    openDicomDirAct_ = new QAction(QIcon(":/vrn_app/icons/open_dicom.png"), tr("&Open DICOMDIR Data Set..."), this);
    openDicomDirAct_->setStatusTip(tr("Open an existing DICOMDIR file"));
    openDicomDirAct_->setToolTip(tr("Open an existing DICOMDIR file"));
    connect(openDicomDirAct_, SIGNAL(triggered()),
            volumeSetWidget_, SLOT(buttonAddDICOMDirClicked()));

  	openDicomFilesAct_ = new QAction(QIcon(":/vrn_app/icons/open_dicom.png"), tr("Open DICOM Slices..."), this);
  	openDicomFilesAct_->setStatusTip(tr("Open DICOM slices"));
  	openDicomFilesAct_->setToolTip(tr("Open existing DICOM slices"));
  	connect(openDicomFilesAct_, SIGNAL(triggered()),
            volumeSetWidget_, SLOT(buttonAddDICOMClicked()));

    quitAction_ = new QAction(QIcon(":/vrn_app/icons/exit.png"), tr("&Quit"), this);
    quitAction_->setShortcut(tr("Ctrl+Q"));
    quitAction_->setStatusTip(tr("Exit the application"));
    quitAction_->setToolTip(tr("Exit the application"));
    connect(quitAction_, SIGNAL(triggered()), this, SLOT(close()));
    
    openNetworkFileAction_ = new QAction(QIcon(":/vrn_app/icons/openNetwork.png"),tr("Open Network..."), this);
    saveNetworkAction_ = new QAction(QIcon(":/vrn_app/icons/save.png"), tr("Save Network"), this);
    saveNetworkAction_->setToolTip(tr("Save Current Network"));
    saveNetworkAction_->setShortcut(tr("Ctrl+S"));
    saveNetworkAsAction_ = new QAction(tr("Save Network As..."), this);

    rebuildShadersAction_ = new QAction(QIcon(":/icons/rebuildshaders.png"), tr("Rebuild All Shaders"),  this);
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

    fileMenu_->addSeparator();
    setLoadVolumeSetContainerAction_ = new QAction("Load datasets with networks", this);
    setLoadVolumeSetContainerAction_->setCheckable(true);
    setLoadVolumeSetContainerAction_->setChecked(false);
    fileMenu_->addAction(setLoadVolumeSetContainerAction_);

    // toolbar
    toolbar_ = new VoreenToolBar(this);   
    toolbar_->hide(); // will be shown later, when fully initialized
    
    // add action button (tool buttons will be added later)
    toolbar_->addToolButtonAction(clearAction_);
    toolbar_->addToolButtonAction(openNetworkFileAction_);
    toolbar_->addToolButtonAction(saveNetworkAction_);
    toolbar_->addToolButtonAction(openFileAction_);
    toolbar_->addToolButtonAction(evaluatorAction_);
}

void RptMainWindow::updateRecentFileActions() {
    QStringList files = settings_.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), static_cast<int>(maxRecents));
    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = QString("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < maxRecents; ++j)
        recentFileActs[j]->setVisible(false);
}

void RptMainWindow::addCurrentFileToRecents() {
    if (currentFile_.isEmpty())
        return;
    
    QStringList files = settings_.value("recentFileList").toStringList();
    files.removeAll("");        // delete empty entries
    files.removeAll(currentFile_);
    files.prepend(currentFile_);
    while (files.size() > maxRecents)
        files.removeLast();

    settings_.setValue("recentFileList", files);
    updateRecentFileActions();
}

void RptMainWindow::rebuildShaders() {
    // set to a waiting cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	if (ShdrMgr.rebuildAllShadersFromFile()) {
        LINFOC("RptMainWindow", "Shaders reloaded");
        evaluator_->invalidateRendering();
        painterWidget_->update();
		#ifdef WIN32
			Beep(100, 100);
		#endif
	} else {
        LWARNINGC("RptMainWindow", "Shader reloading failed");
		#ifdef WIN32
			Beep(10000, 100);
		#endif
	}
    QApplication::restoreOverrideCursor();
}

void RptMainWindow::deleteSlot() {

   QList<QGraphicsItem*> selectedItems = graphWidget_->scene()->selectedItems();
   // check for text item (indicates del shortcut during renaming)
   for (int i=0; i < selectedItems.size(); i++) {
       if (selectedItems.at(i)->hasFocus() && selectedItems.at(i)->type() == RptTextItem::Type) {
            return;
        }
    }

   transferFuncPlugin_->setEnabled(false);

   // sort selectedItems by their type to not delete a port/arrow-item
   // that has already been deleted indirectly with the guiitem
   // so at first kick out the ports:
   for (int i=0; i < selectedItems.size(); i++) {
       if (selectedItems.at(i)->type() == RptPortItem::Type)
           selectedItems.removeAt(i--);
   }
   for (int i=0; i < selectedItems.size(); i++) {
       if (selectedItems.at(i)->type() == RptPropertyPort::Type)
           selectedItems.removeAt(i--);
   }
   for (int i=0; i < selectedItems.size(); i++) {
       if (selectedItems.at(i)->type() == RptTextItem::Type)
           selectedItems.removeAt(i--);
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
                            ->disconnect(static_cast<RptPortItem*>(arrow->getSourceNode()),
                                         static_cast<RptPortItem*>(arrow->getDestNode()));
                    }
                    else if (arrow->getSourceNode()->type() == RptPropertyPort::Type) {
                        static_cast<RptPropertySetItem*>(arrow->getSourceNode()->parentItem())
                            ->disconnectGuiItem(arrow->getDestNode());
                    }
                }

                selectedItems.removeAt(i--);
            }
        }
    }

    // eventually delete the guiitems
    for (int i = 0; i < selectedItems.size(); i++) {
        if (selectedItems[i]) {
            QGraphicsItem* item = selectedItems[i];
            if (item->type() == RptProcessorItem::Type) {
                RptProcessorItem* guiItem = static_cast<RptProcessorItem*>(item);
                if (!(guiItem->parentItem() && guiItem->parentItem()->type() == RptAggregationItem::Type)) {

                    for (size_t j = 0; j<processors_.size(); j++) {
                        RptProcessorItem* temp = processors_.at(j);
                        if (temp == guiItem) {
                            evaluator_->removeProcessor(processors_.at(j)->getProcessor());
                            transferFuncPlugin_->removeProcessor(processors_.at(j)->getProcessor());
                            delete  processors_[j];
                            processors_[j] = 0;
                            processors_.erase(processors_.begin() + (j--));
                            propertyListWidget_->setProcessor(0);
                            changeProcessorInfo();
                            break;
                        }
                    }
                }
                selectedItems.removeAt(i--);
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

    setWindowModified(true);
}

void RptMainWindow::closeEvent(QCloseEvent *event) {
    // store setings
    settings_.setValue("ResetSettings", resetSettings_);

    // write version number of the config file format (might be useful someday)
    settings_.setValue("ConfigVersion", 1);

    settings_.beginGroup("MainWindow");
    settings_.setValue("size", size());
    settings_.setValue("pos", pos());
    settings_.endGroup();

    settings_.beginGroup("Paths");
    settings_.setValue("network", networkPath_);
    settings_.setValue("dataset", volumeSetWidget_->getCurrentDirectory().c_str());
    settings_.endGroup();

    settings_.beginGroup("Windows");

    settings_.beginGroup(toolbar_->objectName());
    settings_.setValue("pos", toolbar_->pos());
    settings_.endGroup();

    for (int i=0; i < toolWindows_.size(); ++i) {
        if (!toolWindows_[i]->objectName().isEmpty()) {
            settings_.beginGroup(toolWindows_[i]->objectName());
            settings_.setValue("visible", toolWindows_[i]->isVisible());
            settings_.setValue("pos", toolWindows_[i]->pos());
            settings_.setValue("size", toolWindows_[i]->size());
            settings_.endGroup();
        }
    }
    settings_.endGroup();
    
    if (isWindowModified()) {
        switch (QMessageBox::information(this, tr("VoreenVE Beta"),
                                         tr("Do you want to save the changes to the current network?"),
                                         tr("Yes"), tr("No"), tr("Cancel"), 0, 1))
        {
        case 0:
            saveNetworkAsButtonPushed();
            event->accept();
            break;
        case 1:
            event->accept();
            break;
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
		processorInfoBox_->append(QString::number(evaluator_->getPriorityMap()[processor]));
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
	//for (iter = evaluator_->getPortMap().begin(); iter != evaluator_->getPortMap().end(); iter++) {
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

void RptMainWindow::changeProcessorInfo() {
	QString text;

	//get current list item
	//prevent showing information twice after drag'n drop
	if (processorListWidget_->hasFocus()) {
		//check if a item is selected
		if (!(processorListWidget_->currentItem() == 0)) {
			RptProcessorListItem* selectedTreeWidgetItem
                = dynamic_cast<RptProcessorListItem*>(processorListWidget_->currentItem());
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
		if (selectedGraphItems.at(i)->type() == RptProcessorItem::Type) {
			RptProcessorItem* item = static_cast<RptProcessorItem*>(selectedGraphItems.at(i));
            if (item->getProcessor()) {
			    text += QString("<b><font color='green'>") + QString(item->getName().c_str())
                        + QString("</font></b><br /> ");
                text += QString(item->getProcessor()->getProcessorInfo().c_str()) + QString("<hr />");
            }
		}
	}

	// insert text in info box
	processorInfoBox_->setHtml(text);
}

void RptMainWindow::helpAbout() {
    AboutBox about("VoreenVE", tr("Visualization Environment"), "0.8 beta", this);
    about.exec();
}

void RptMainWindow::helpFirstSteps() {
    HelpBrowser* help = new HelpBrowser("file:///" + QDir::currentPath() + "/doc/gsg.html");
    help->resize(900, 600);
    help->show();
}

void RptMainWindow::updateWindowTitle() {
    QString title = tr("VoreenVE Beta [*]");
    
    if (!currentFile_.isEmpty())
        title += " - " + currentFile_;

    setWindowTitle(title);
}

void RptMainWindow::connectCanvasModifier(bool connect) {
    if (connect)
        canvasMod_->connect();
    else
        canvasMod_->disconnect();
}

void RptMainWindow::setResetSettings(bool value) {
    resetSettings_ = value;
}

void RptMainWindow::dragEnterEvent(QDragEnterEvent* event) {
    // Get all the possible URI's for the object
    QList<QUrl> urls = event->mimeData()->urls();
    // Should not happen, but safe anyway
    if (urls.isEmpty())
        return;
    // The filename is the first one
    QString fileName = urls.first().toLocalFile();
    // If the extension is ".vnw", accept it
    if (fileName.endsWith(".vnw"))
        event->acceptProposedAction();
}

void RptMainWindow::dropEvent(QDropEvent* event) {
    // Get the filename ...
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;
    QString fileName = urls.first().toLocalFile();

    // ... and open the network
    openNetworkFile(fileName);
}

//---------------------------------------------------------------------------

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
