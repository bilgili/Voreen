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

#ifndef VRN_RPTMAINWINDOW_H
#define VRN_RPTMAINWINDOW_H

#include <QtGui>

#include "voreen/core/io/iosystem.h"
#include "voreen/core/io/volumeserializerpopulator.h"
#include "voreen/core/vis/idmanager.h"
#include "voreen/core/vis/message.h"

namespace voreen {

struct ConnectionInfoForOneProcessor;

class CanvasModifier;
class GeometryContainer;
class NetworkEvaluator;
class NetworkSerializer;
class RptAggregationItem;
class RptAggregationListWidget;
class RptGraphWidget;
class RptGuiItem;
class RptNetwork;
class RptNetworkSerializerGui;
class RptPainterWidget;
class RptProcessorItem;
class RptProcessorListWidget;
class RptPropertyListWidget;
class RptPropertySetItem;
class TextureContainer;
class TransFuncPlugin;
class Volume;
class VolumeSetContainer;
class VolumeSetWidget;

#ifdef VRN_MODULE_GLYPHS
class GlyphsPlugin;
class DataSourceContainer;
#endif

#ifdef VRN_WITH_DCMTK
class DicomDirDialog;
#endif

/**
 * The Layout Container is neccesary for combining multiple widgets in a dock widget
 */
class LayoutContainer : public QWidget {
    Q_OBJECT
public:
    LayoutContainer(QWidget* parent = 0, QBoxLayout::Direction direction = QBoxLayout::LeftToRight);
    virtual ~LayoutContainer();

    void addWidget(QWidget*);

protected:
    QBoxLayout* layout_;

};

//================================================================================================================

/**
 * The Main Window that contains all the other widgets, tool bar, menu bar, etc.
 */
class RptMainWindow : public QMainWindow, public MessageReceiver {

Q_OBJECT

public:
    /**
     * Default Constructor
     */
    RptMainWindow();
	~RptMainWindow();

    /**
     * Create all Dock Widgets. Has to be called after tgt::initGL() and initCanvas().
     */
    void createDockWidgets();
    void createConnections();

    /**
     * Initializes the TextureContainer and IdManager
     */
    void initTextureContainer();

	virtual void processMessage(Message* msg, const Identifier& = Message::all_);

public slots:
    /**
     * Adds a new processor item to the processoritem-vector and adds it to the scene.
     */
    void addProcessorItem(Identifier type, QPoint pos);

    /**
     * Connects all selected Processors depending on their position.
     */
    void connectButtonPushed();
    void clearNetwork();
    void evaluatorButtonPushed();
    void helpAbout();
    void helpFirstSteps();
    void openFileButtonPushed();
    void addAggregationSlot(std::string filename, QPoint position);
    void openNetworkFileButtonPushed();
    void openNetworkFile(const QString& filename, bool loadVolumeSetContainer = false);
    void sendProcessorToTable(Processor*,QVector<int>);
    void saveNetworkButtonPushed();
    void saveNetworkAsButtonPushed();
    void saveCurrentNetwork();
    void saveAggregationSlot(RptAggregationItem* aggregation);
    void createAggregation(std::vector<RptProcessorItem*> processors, std::string name, QPoint pos = QPoint(10000,10000));
    void insertAggregation();
    void editAggregation();
    void deaggregate();
    void selectAll();
    void showAggregationContent();
    void setReuseTargets();
    void copyButtonPushed();
    void pasteButtonPushed();
    void createPropertySet();
    void deleteSlot();
    void rebuildShaders();
    void changeProcessorInfo();
    void networkModified();
    void openRecentFile();
    //void openRecentFile(const QString& filename, const QPoint& position);
    void showProperties();
    void dicomDirFinished();
    bool getFileDialog(QStringList& filenames, QDir& dir);
    void connectCanvasModifier(bool connect);
    void detachCanvasSlot();
    void reAttachCanvasSlot();

private slots:
    void fileOpenDicomDir();
    void fileOpenDicomFiles();

private:
    void updateWindowTitle();
    
    Volume* loadDataset(const std::string& filename);

    void createMenuAndToolBar();
    void keyPressEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent *event);
    void sortByPosition(std::vector<RptGuiItem*> &v);
    void createLoadedAggregations(std::map<int,std::vector<RptProcessorItem*> *> aggroList,std::map<int,std::string> aggroNameMap, QPoint pos = QPoint(10000,10000));
    void addToScene(RptNetwork& rptnet);
    void clearScene();
    void askSaveNetwork();
    bool askLoadVolumeSetContainer(QString filename);
    void showNetworkErrors(const RptNetwork& rptnet);

    void scaleView(float maxFactor);
    std::vector<RptProcessorItem*> copyProcessors();
    void addCurrentFileToRecents();
    void updateRecentFileActions();
    static const int maxRecents = 5;
    QAction *recentFileActs[maxRecents];

    /**
    * A drag event for loading network files
    * \param event The event happening on entering the window
    */
    void dragEnterEvent(QDragEnterEvent* event);
    /**
    * The drop event for loading network files
    * \param event The event happening on dropping the file
    */
    void dropEvent(QDropEvent* event);

    CanvasModifier* canvasMod_;

	/**
	* Returns the position the given processor has in the given vector. If not found, -1 is returned. This is
	* used in copy/paste.
	*/
	int findProcessor(std::vector<RptProcessorItem*> vector, Processor* processor);

    // TESTING
	void dumpProcessorConnectionInfos();

    /** Clears the vectors for processors, aggregations and proeperty set and
     * calls delete on every single pointer.
     * This is needed in order to load a network when there has already been one:
     * the processor's, aggregation's and property set's dtors need to be called
     * in order to free memory and to prevent unwanted effects.
     * Method is called at the beginning of finishOpen().
     */
    void clearDataVectors();

    // extracts the processor from the processors_ - vector
	std::vector<Processor*> getAllProcessors();

    // central widget: graph editor
    RptGraphWidget* graphWidget_;
    // widget that contains the list of processors, which can be added to the scene
    RptProcessorListWidget* processorListWidget_;
    RptAggregationListWidget* aggregationListWidget_;
    // widget for infos
    QTextBrowser* processorInfoBox_;
    // widget for preview using the voreen engine
    RptPainterWidget* painterWidget_;

    // vector with created processor wrappers
    std::vector<RptProcessorItem*> processors_;
	// vector containing all RptPropertySetItems in the scene
	std::vector<RptPropertySetItem*> propertySets_;
    // vector with aggregations
    std::vector<RptAggregationItem*> aggregations_;

    // counter for copies. needed to move items reasonably after repeated copying
    int copyCounter_;

    // analyzes and evaluates the drawn graph
    NetworkEvaluator* evaluator_;

    // loads and saves networks
    RptNetworkSerializerGui* graphSerializer_;
    NetworkSerializer* networkserializer_;

    // displays the properties of a given processor
    RptPropertyListWidget* propertyListWidget_;

    // Qt menus and tool bar
    QMenuBar* menu_;
    QToolBar* toolBar_;

    QMenu* fileMenu_;
    QMenu* dicomMenu_;
    QMenu* editMenu_;
    QMenu* helpMenu_;
    QMenu* optionsMenu_;

    QAction* aboutAction_;
    QAction* clearAction_;
    QAction* connectAction_;
    QAction* evaluatorAction_;
    QAction* helpFirstStepsAct_;
    QAction* openFileAction_;
    QAction* openNetworkFileAction_;
    QAction* saveNetworkAction_;
    QAction* saveNetworkAsAction_;
    QAction* insertAggregationAction_;
    QAction* deaggregateAction_;
    QAction* showAggregationContentAction_;
    QAction* createPropSetAction_;
    QAction* openDicomDirAct_;
    QAction* openDicomFilesAct_;
    QAction* quitAction_;

    QAction* selectAllAction_;
    QAction* copyAction_;
    QAction* pasteAction_;
    QAction* setReuseTargetsAction_;
    QAction* rebuildShadersAction_;
    QAction* connectCanvasModAct_;

    QDockWidget* sceneDock_;
    QTabWidget* contentTab_;
    QDockWidget *contentDock_;
    QTabWidget *canvasTab_;
    QDockWidget *canvasDock_;
    VolumeSetWidget* volumeSetWidget_;

    LayoutContainer *processorLayout_;
    LayoutContainer *propertyLayout_;

	//The TextureContainer
	TextureContainer* tc_;

    GeometryContainer* geoContainer_;
    VolumeSetContainer* volsetContainer_;

    //The camera
    tgt::Camera* camera_;

    //Not sure yet what this is for
    IDManager id1_;

    IOSystem* ioSystem_;
    TransFuncPlugin* transferFuncPlugin_;

    // array for session management. has to be a member, because
    // session can only be resored after creation of widgets
    QByteArray sessionArray_;

#ifdef VRN_MODULE_GLYPHS
    GlyphsPlugin* glyphsPlugin_;
    voreen::DataSourceContainer* datasourceContainer_;
#endif
    VolumeSerializerPopulator volumeSerializerPopulator_;
//voreen::VolumeContainer* volumeContainer_;

    /// Current network path
    QString networkPath_;
    /// Current data set path
    QString datasetPath_;

    QString currentFile_; //filename of current network
	std::vector<RptProcessorItem*> copyPasteProcessors_;
	std::vector<RptProcessorItem*> pastedProcessors_;
	std::vector<RptAggregationItem*> copyPasteAggregations_;
	std::vector<RptPropertySetItem*> copyPastePropertySets_;
	std::vector<ConnectionInfoForOneProcessor*> copyPasteConnectionInfos_;
	std::map<int,std::vector<int> > aggregationMap_;
	std::map<int,std::string> aggregationNameMap_;
	int numberOfAggregations_;
	int numberOfPropertySets_;
	std::map<int,std::vector<int> > propertySetMap_;
	std::map<int,std::vector<int> > propertySetAggregationMap_;
	std::map<int,std::string> propertySetNameMap_;

#ifdef VRN_WITH_DCMTK
    DicomDirDialog* dicomDirDialog_;
#endif
    QDir fileDialogDir_;

};

} // namespace 

#endif // VRN_RPTMAINWINDOW_H
