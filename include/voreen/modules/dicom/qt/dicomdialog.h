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

#ifndef VRN_DICOMDIALOG_H
#define VRN_DICOMDIALOG_H

#include <QDialog>
#include <QString>
#include <QLabel>
#include <QRadioButton>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QListWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QThread>
#include <QProgressDialog>

#include <string>
#include <vector>
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/modules/dicom/dicomseriesinfo.h"

namespace voreen {

/**
 * The dicom download runs in this thread while the gui thread refreshes the progressdialog.
 */
class DownloadThread : public QThread {
    std::string targetPath_;
    std::string callAE_;
    std::string selectedSeriesInstanceUID_;
    bool success_;
public:
    ///See dicommove functions for explanation of the parameters.
    DownloadThread(std::string targetPath, std::string callAE,
                   std::string selectedSeriesInstanceUID)
        : targetPath_(targetPath),
          callAE_(callAE),
          selectedSeriesInstanceUID_(selectedSeriesInstanceUID) {}

    ///Call QThread::start() to start this thread and download.
    void run();
    ///Call this after thread has finished to find out if download was successful
    bool successful() { return success_; }
};

///Lists dicom series. Provides find-as-you-type search field.
class SeriesListWidget : public QWidget {
    Q_OBJECT
public:
    SeriesListWidget();
    ///Fill the list with seriesinfos
    void setSeries(std::vector<voreen::DicomSeriesInfo>& list);
    void setFile(QString file) { chosenFile_ = file; isServer_ = false;}
    void setURL(QString file) { chosenFile_ = file; isServer_ = true;}
    ///Get the currently selected series UID, returns empty string if no series is selected.
    std::string getSelectedSeriesInstanceUID();
    ///Get the number of images in the selected series, returns 0 if no series is selected.
    int getSelectedNumImages();

public slots:
    ///Updates the filtering in the list with the given string.
    void updateFilter(const QString & text);

signals:
    ///This signal is emitted upon selection of an item in the list.
    void seriesSelected();

private:
    QTableWidget* table;
    QString chosenFile_;
    QLineEdit* filter_;
    QComboBox* filterCombo_;
    bool isServer_;
};

///Holds all infos for server bookmark
struct ServerConfig {
    QString name_;
    QString hostname_;
    int serverPort_;
    QString callAE_;
    QString destAE_;
    int inPort_;
    int sslEnabled_;
    QString clientKey_;
    QString clientCert_;
    QString serverCert_;
};

///This Dialog allows editing of server bookmarks.
class ServerConfigDialog : public QDialog {
    Q_OBJECT
public:
    ServerConfigDialog();
    ///Sets the config to edit. The widgets will be filled with this config.
    ///If the user clicks ok it will saved back, otherwise the config is untouched.
    void editConfig(ServerConfig* conf);
public slots:
    ///Enable/Disable the ssl-related widgets
    void enableSSL(int enable);
    ///Opens file dialog for selection
    void selectClientKey();
    ///Opens file dialog for selection
    void selectClientCert();
    ///Opens file dialog for selection
    void selectServerCert();

    ///Save the info back to the ServerConfig and close the dialog
    void save();
    ///Do nothing and close the dialog.
    void cancel();
protected:
    ServerConfig* currentConf_;

    QLabel* hostnameLabel_;
    QLineEdit* hostname_;
    QSpinBox* serverPort_;

    QLineEdit* callAE_;
    QLineEdit* destAE_;
    QSpinBox* incomingPort_;

    QCheckBox* sslCheck_;
    QLineEdit* clientKey_;
    QPushButton* selectClientKey_;
    QLineEdit* clientCert_;
    QPushButton* selectClientCert_;
    QLineEdit* serverCert_;
    QPushButton* selectServerCert_;

    QPushButton* okButton_;
    QPushButton* cancelButton_;
};

///Dicom-Download main dialog.
class ServerDialog : public QDialog {
    Q_OBJECT
public:
    ServerDialog();

public slots:
    ///Add a server to the bookmarks: Ask for a name and then open the edit dialog. The
    ///configuration is saved.
    void addServer();
    ///Delete server from bookmarks. The configuration is saved.
    void deleteServer();
    ///Sets the current config to number i of the bookmarks
    void displayConfig(int i);
    ///Edit the selected config in ServerConfigDialog
    void editConfig();
    ///Fetch serieslist from selected server and display it.
    void updateList();
    ///Download the selected series
    void open();
    ///Load the bookmarks from ini/registry..
    void loadServers();
    ///Save the bookmarks to ini/registry..
    void saveServers();
    void enableOpenButton();
    ///Open directory selection dialog
    void selectOutputDir();
    ///Cancel the current download
    void cancelDownload();
    ///Called by the timer usually; updates the progressbar.
    void updateProgressbar();
    ///Called by the DownloadThread after finishing. Displays success in a messagebox, closses
    ///progressdialog and stops timer.
    void downloadFinished();
    std::string getURL();


signals:
    ///for use in voreenapp. emitted when open button is pressed.
    void dicomServerFinished();

private:
    void fillCombo();

    int currentConf_;
    std::vector<ServerConfig> servers_;

    SeriesListWidget* listWidget_;
    ServerConfigDialog* configDialog_;

    QComboBox* serverCombo_;
    QPushButton* newServerButton_;
    QPushButton* deleteServerButton_;
    QPushButton* editServerButton_;
    QPushButton* updateListButton_;
    QPushButton* openButton_;
    QLineEdit* outputDirectory_;
    QPushButton* changeDirectoryButton_;
    voreen::DicomSecurityOptions security_;

    QTimer* t_;
    QProgressDialog* pd_;
    DownloadThread* dt_;
};

///Dicom-Dir main dialog.
class DicomDirDialog : public QDialog {
    Q_OBJECT
public:
    DicomDirDialog(QWidget* parent = 0);
    ///Fill the list with seriesinfos
    void setSeries(std::vector<voreen::DicomSeriesInfo>& list, std::string filename) {
        listWidget_->setSeries(list);
        filename_ = filename;
    }
    std::string getFilename();

public slots:
    ///Download the selected series
    void open();

signals:
    void dicomDirFinished();

private:
    SeriesListWidget* listWidget_;
    QPushButton* openButton_;
    std::string filename_;
};

} // namespace

#endif
