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

#ifndef VRN_VOLUMESETWIDGET_H
#define VRN_VOLUMESETWIDGET_H

#include <QtGui>
//#include <QComboBox>

#include <string>
#include <vector>
#include <typeinfo>

#ifndef VRN_VOLUMESETCONTAINER_H
#include "voreen/core/volume/volumesetcontainer.h"
#endif

#ifndef VRN_VOLUMESERIALIZERPOPULATOR_H
#include "voreen/core/io/volumeserializerpopulator.h"
#endif

#ifndef VRN_IO_PROGRESS_H
#include "voreen/core/io/ioprogress.h"
#endif

namespace voreen
{

/**
 * Class for managing currently loaded VolumeSet / files in Voreen applications.
 *
 * @author  Dirk Feldmann, 2008
 */
class VolumeSetWidget : public QWidget {

Q_OBJECT

private:
    /**
     * QTreeWidget class containing one column and no fancy stuff.
     */
    class VolumeSetTreeWidget : public QTreeWidget
    {
        public:
            VolumeSetTreeWidget(QWidget* parent = 0) : QTreeWidget(parent) {
                setColumnCount(1);
                setHeaderLabel(tr("Content of VolumeSetContainer"));
                setAnimated(false);
            }

            virtual ~VolumeSetTreeWidget() {}
    };

    /**
     * Abstract base class for QTreeWidgetItems. This only necessary to
     * have a common method <code>getType()</code> to classify the data
     * contained by the different TreeWidgetItems / nodes.
     */
    class AbstractVolumeSetTreeItem : public QTreeWidgetItem
    {
        public:
            virtual ~AbstractVolumeSetTreeItem() {};
            virtual const std::type_info& getType() = 0;
    };

    /**
     * Class for the QTreeWidgetItems, contained additionally a pointer
     * to the objects the are associated with.
     * The QTreeWidget may contain nodes which can be eiterh
     * - the root (no data associated, void pointer)
     * - VolumeSet items (VolumeSet pointers associated)
     * - VolumeSeries items
     * - VolumeHandle items
     */
    template<class T>
    class VolumeSetTreeItem : public AbstractVolumeSetTreeItem
    {
        public:
            VolumeSetTreeItem(const std::string& lable, T itemData) : 
              AbstractVolumeSetTreeItem(), 
              type_(typeid(itemData)),
              itemData_(itemData)
            {
                QTreeWidgetItem::setText(0, tr(lable.c_str()));
            }

            virtual ~VolumeSetTreeItem() { }

            T getItemData() { return itemData_; }
            const std::type_info& getType() { return type_; }

        protected:
            T itemData_;
            const std::type_info& type_;
    };

    // Specialization of the template items described above
    //
    typedef VolumeSetTreeItem<void*> VolumeRootItem;
    typedef VolumeSetTreeItem<VolumeSet*> VolumeSetItem;
    typedef VolumeSetTreeItem<VolumeSeries*> VolumeSeriesItem;
    typedef VolumeSetTreeItem<VolumeHandle*> VolumeHandleItem;

    /**
     * A class for a small dialog indicating the progress of loading a file by
     * using a QProgressDialog. This obsoletes the former class IOSystem in all
     * Voreen applications.
     */
    class IOObserver : public voreen::IOProgress
    {
        public:
            IOObserver(QWidget* parent) : progressDialog_(new QProgressDialog(parent)) {
                progressDialog_->setCancelButton(0);
                progressDialog_->setWindowModality(Qt::WindowModal);
            }

            ~IOObserver() {
                delete progressDialog_;
            }

            virtual void update() {
                progressDialog_->setValue(progress_);
            }

            virtual void setNumSteps(int numSteps) {
                progressDialog_->setRange(0, numSteps - 1);
            }

            void show(const std::string& filename = "") {
                if( progressDialog_ != 0 ) {
                    set(0);
                    std::string title("Loading file");
                    if( filename.empty() == false ) {
                        title += " \"";
                        title += filename;
                        title += "\"";
                    }
                    title += "...";
                    progressDialog_->setLabelText(tr(title.c_str()));
                    progressDialog_->show();
                    progressDialog_->raise();
                    progressDialog_->activateWindow();
                }
            }

            void hide() {
                if( progressDialog_ != 0 ) {
                    set( progressDialog_->maximum() );
                    progressDialog_->hide();
                }
            }
        protected:
            QProgressDialog* progressDialog_;
    };

    IOObserver* ioObserver_;

public:
    /**
     * Allows to omit certain levels in the hierarchy of the VolumeSet concept. E.g. if
     * an application only deals with volume data of the type "CT", you need not to create
     * the hierarchy level for VolumeSeries, as there is only one. The the timesteps from
     * the VolumeHandles would be displayed directly below the VolumeSet and would be added
     * there.
     */
    enum AvailableLevels {
        LEVELS_NONE = 0,
        LEVEL_VOLUMESETS = 1,
        LEVEL_VOLUMESERIES = 2,
        LEVEL_VOLUMEHANDLES = 4,
        LEVEL_ALL = 7
    };

public:
    /**
     * Ctor.
     * @param   volumeSetContainer  The VolumeSetContainer which shall be managed.
     * @param   parent  The parent widget of this widget. If parent is NULL, the widget
     *  is displayed in its own window.
     * @param   leves   levels of the hierarchy which shall be displayed and managed.
     * @param   flags   QWindow flags for the window appearance
     */
	VolumeSetWidget(VolumeSetContainer* const volumeSetContainer, QWidget* parent = 0, 
        int levels = LEVEL_ALL, Qt::WFlags flags = 0);

    /**
     * Dtor. Does not delete the VolumeSetContainer.
     */
	~VolumeSetWidget();

    /**
     * Clears the QTreeWidget and updates its content based on the current
     * content of the VolumeSetContainer.
     */
    void updateContent();

    /**
     * Sets whether multiple file selection is enabled or disabled.
     */
    void setAllowAddingMultipleFiles(const bool allow);

    /**
     * Indicates whether multiple file selection is enabled or disabled.
     */
    bool getAllowAddingMultipleFiles() const;

    /**
     * Sets the current directory which is used for file selection.
     */
    void setCurrentDirectory(const std::string& dir);

    /**
     * Returns the current directory which is used for file selection.
     */
    const std::string& getCurrentDirectory() const;

    /**
     * Sets the filter for allowed file extension on file selection.
     */
    void setFileExtensionFilter(const std::string& fileExtFilter);

    /**
     * Returns the filter for allowed file extension on file selection.
     */
    const std::string& getFileExtensionFilter() const;

    /**
     * Sets whether to generate VolumeGL hardware volumes or not.
     */
    void setGenerateVolumeGL(const bool generateVolumeGL);

    /**
     * Indicates whether VolumeGL hardware volumes are generated or not.
     */
    bool getGenerateVolumeGL() const;

    // TODO: add methods for indicating the generation VolumCUDA

    /**
     * Adds the content of the files from the given file names to the VolumeSetContainer
     * as new VolumeSet objects. A VolumeSet is created for each file.
     */
    void addVolumeSets(const std::vector<std::string>& filenames);

    /**
     * Adds the content of the files from the given file names to the given VolumeSet as new
     * VolumeSeries objects. A VolumeSeries is created for each file.
     */
    void addVolumeSeries(const std::vector<std::string>& filenames, VolumeSet* volumeSet);

    /**
     * Adds the content of the files from the given file names to the given VolumeSeries as new
     * VolumeHandle objects. A VolumeHandle is created for each file.
     */
    void addVolumeHandles(const std::vector<std::string>& filenames, VolumeSeries* volumeSeries);

    /**
     * Directly add the content of the file from the given file name to the VolumeSetContainer
     * as new VolumeSet and returns it. Use this method for initial loading data sets from your
     * application.
     */
    VolumeSet* loadVolumeSet(const std::string& filename);

    /** Shows a dialog for opening files and returns the selected file names.
     * Use <code>setAllowMultipleFileSelection()</code> and <code>setFileExtensionFilter()</code>
     * to determine the capability of multiple file selection and the supported file extensions.
     */
    std::vector<std::string> openFileDialog();

public slots:
    /**
     * The action performed when clicking the button for loading files.
     */
    void buttonAddClicked();

    /**
     * The action performed when clicking the button for removing data sets.
     */
    void buttonRemoveClicked();

    /**
     * The action performed when altering the selection of the QTreeWidget.
     */
    void treeItemSelectionChanged();

protected:
    VolumeSerializerPopulator* volumeSerializerPopulator_;
    VolumeSetContainer* volumeSetContainer_;
    bool generateVolumeGL_;
    // TODO: add support for VolumeCUDA

    // appearance
    //
    int availableLevelsMask_;
    std::string fileExtFilter_;
    std::string currentDir_;
    bool allowAddingMultipleFiles_;

    // QT controls
    //
	QLabel* lblVolumeSets_;
	VolumeSetTreeWidget* treeVolumeSets_;
	QPushButton* btnLoad_;
	QPushButton* btnUnload_;
	
private:
    void appendVolumeSets(VolumeRootItem* node);
    void appendVolumeSeries(QTreeWidgetItem* node, VolumeSet* const volumeSet);
    void appendVolumeHandles(QTreeWidgetItem* node, VolumeSeries* const volumeSeries);
	void createWidgets();
}; 

}   // namespace
#endif // VOLUMESETWIDGET_H
