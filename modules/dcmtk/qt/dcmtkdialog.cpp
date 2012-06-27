/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#include "dcmtkdialog.h"

#include "modules/dcmtk/io/dcmtkvolumereader.h"
#include "modules/dcmtk/io/dcmtkmovescu.h"
#include "modules/dcmtk/io/dcmtkfindscu.h"

#include <QtGui>
#include <QStringList>

namespace voreen {

void DcmtkDownloadThread::run() {
    std::vector<std::string> files;
    if (voreen::DcmtkMoveSCU::moveSeries(selectedSeriesInstanceUID_,
                                         callAE_, &files, targetPath_) != 0)
        success_ = false;
    else
        success_ = true;
}

//
// Server config dialog
//

DcmtkServerConfigDialog::DcmtkServerConfigDialog() {
    setWindowTitle("Dicom Server Parameters");

    hostname_ = new QLineEdit();
    serverPort_ = new QSpinBox();
    serverPort_->setMinimum(0);
    serverPort_->setMaximum(65535);
    callAE_ = new QLineEdit();
    destAE_ = new QLineEdit();
    incomingPort_ = new QSpinBox();
    incomingPort_->setMinimum(0);
    incomingPort_->setMaximum(65535);

    okButton_ = new QPushButton("Ok");
    okButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    cancelButton_ = new QPushButton("Cancel");
    cancelButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(new QLabel(tr("Remote address:")),1,0);
    layout->addWidget(hostname_,1,1,1,3);

    layout->addWidget(new QLabel(tr("Server port")),2,0);
    layout->addWidget(serverPort_,2,1,1,3);

    layout->addWidget(new QLabel(tr("Remote AE title")),3,0);
    layout->addWidget(callAE_,3,1,1,3);

    layout->addWidget(new QLabel(tr("Destination AE title")),4,0);
    layout->addWidget(destAE_,4,1,1,3);

    layout->addWidget(new QLabel(tr("Incoming port")),5,0);
    layout->addWidget(incomingPort_,5,1,1,3);

    sslCheck_ = new QCheckBox(tr("SSL"));
    layout->addWidget(sslCheck_,6,0);
    layout->addWidget(new QLabel("Client key file"),7,0);
    clientKey_ = new QLineEdit();
    layout->addWidget(clientKey_,7,1);
    selectClientKey_ = new QPushButton(tr("..."));
    layout->addWidget(selectClientKey_,7,2);
    layout->addWidget(new QLabel("Client certificate file"),8,0);
    clientCert_ = new QLineEdit();
    layout->addWidget(clientCert_,8,1);
    selectClientCert_ = new QPushButton(tr("..."));
    layout->addWidget(selectClientCert_,8,2);
    layout->addWidget(new QLabel("Server certificate file"),9,0);
    serverCert_ = new QLineEdit();
    layout->addWidget(serverCert_,9,1);
    selectServerCert_ = new QPushButton(tr("..."));
    layout->addWidget(selectServerCert_,9,2);

    layout->addWidget(okButton_,11,0);
    layout->addWidget(cancelButton_,11,1);

    enableSSL(false);

    connect(sslCheck_, SIGNAL(stateChanged(int)), this, SLOT(enableSSL(int)));
    connect(selectClientKey_, SIGNAL(clicked()), this, SLOT(selectClientKey()));
    connect(selectClientCert_, SIGNAL(clicked()), this, SLOT(selectClientCert()));
    connect(selectServerCert_, SIGNAL(clicked()), this, SLOT(selectServerCert()));

    connect(okButton_, SIGNAL(clicked()), this, SLOT(save()));
    connect(cancelButton_, SIGNAL(clicked()), this, SLOT(cancel()));
}


void DcmtkServerConfigDialog::selectClientKey() {
    QString s = QFileDialog::getOpenFileName(this, "Choose a key file", "",
                                             "Keys/Certificates (*.pem *.crt)");
    if (s != "")
        clientKey_->setText(s);
}

void DcmtkServerConfigDialog::selectClientCert() {
    QString s = QFileDialog::getOpenFileName(this, "Choose a certificate file", "",
                                             "Keys/Certificates (*.pem *.crt)");
    if (s != "")
        clientCert_->setText(s);
}

void DcmtkServerConfigDialog::selectServerCert() {
    QString s = QFileDialog::getOpenFileName(this, "Choose a certificate file", "",
                                             "Keys/Certificates (*.pem *.crt)");
    if (s != "")
        serverCert_->setText(s);
}

void DcmtkServerConfigDialog::enableSSL(int enable) {
    clientKey_->setEnabled(enable);
    selectClientKey_->setEnabled(enable);
    clientCert_->setEnabled(enable);
    selectClientCert_->setEnabled(enable);
    serverCert_->setEnabled(enable);
    selectServerCert_->setEnabled(enable);
}

void DcmtkServerConfigDialog::editConfig(DcmtkServerConfig* conf) {
    currentConf_ = conf;

    hostname_->setText(conf->hostname_);
    serverPort_->setValue(conf->serverPort_);
    callAE_->setText(conf->callAE_);
    destAE_->setText(conf->destAE_);
    incomingPort_->setValue(conf->inPort_);

    sslCheck_->setCheckState((Qt::CheckState)conf->sslEnabled_);
    clientKey_->setText(conf->clientKey_);
    clientCert_->setText(conf->clientCert_);
    serverCert_->setText(conf->serverCert_);

    enableSSL(sslCheck_->checkState());
}


void DcmtkServerConfigDialog::save() {
    currentConf_->hostname_ = hostname_->text();
    currentConf_->serverPort_ = serverPort_->value();
    currentConf_->callAE_ = callAE_->text();
    currentConf_->destAE_ = destAE_->text();
    currentConf_->inPort_ = incomingPort_->value();
    //
    currentConf_->sslEnabled_ = sslCheck_->checkState();
    currentConf_->clientKey_ = clientKey_->text();
    currentConf_->clientCert_ = clientCert_->text();
    currentConf_->serverCert_ = serverCert_->text();
    done(1);
}

void DcmtkServerConfigDialog::cancel() {
    done(0);
}


//
// Server
//

DcmtkServerDialog::DcmtkServerDialog() {
    setWindowTitle("Dicom Downloader");


    listWidget_ = new DcmtkSeriesListWidget();
    configDialog_ = new DcmtkServerConfigDialog();

    serverCombo_ = new QComboBox();
    serverCombo_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    newServerButton_ = new QPushButton("New");
    newServerButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    deleteServerButton_ = new QPushButton("Delete");
    deleteServerButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    editServerButton_ = new QPushButton("Edit");
    editServerButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    updateListButton_ = new QPushButton("Update");
    updateListButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    openButton_ = new QPushButton("Open");
    openButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    openButton_->setDisabled(true);
    outputDirectory_ = new QLineEdit("");
    outputDirectory_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    changeDirectoryButton_ = new QPushButton("...");
    changeDirectoryButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    loadServers();

    connect(newServerButton_, SIGNAL(clicked()), this, SLOT(addServer()));
    connect(deleteServerButton_, SIGNAL(clicked()), this, SLOT(deleteServer()));
    connect(serverCombo_, SIGNAL(currentIndexChanged(int)), this, SLOT(displayConfig(int)));
    connect(editServerButton_, SIGNAL(clicked()), this, SLOT(editConfig()));
    connect(updateListButton_, SIGNAL(clicked()), this, SLOT(updateList()));
    connect(openButton_, SIGNAL(clicked()), this, SLOT(open()));
    connect(listWidget_, SIGNAL(seriesSelected()), this, SLOT(enableOpenButton()));
#ifdef VRN_DICOMLOADER
    connect(changeDirectoryButton_, SIGNAL(clicked()), this, SLOT(selectOutputDir()));
#endif
    QGridLayout *layout = new QGridLayout;

    layout->addWidget(new QLabel("Server:"),0,0,(Qt::Alignment)0);
    layout->addWidget(serverCombo_,0,1,(Qt::Alignment)0);
    fillCombo();
    layout->addWidget(editServerButton_,0,2,(Qt::Alignment)0);
    layout->addWidget(newServerButton_,0,3,(Qt::Alignment)0);
    layout->addWidget(deleteServerButton_,0,4,(Qt::Alignment)0);
    layout->addWidget(listWidget_,1,0,1,5,(Qt::Alignment)0);
    layout->addWidget(updateListButton_,2,3,1,1,(Qt::Alignment)0);
    layout->addWidget(openButton_,2,4,1,1,(Qt::Alignment)0);
#ifdef VRN_DICOMLOADER
    layout->addWidget(new QLabel("Output Directory:"),4,0,(Qt::Alignment)0);
    layout->addWidget(outputDirectory_,4,1,1,3,(Qt::Alignment)0);
    layout->addWidget(changeDirectoryButton_,4,4,1,1,(Qt::Alignment)0);
#endif

    listWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setLayout(layout);
//     if (servers_.size() > 0)
//         displayConfig(servers_[0]);

    dt_ = 0;
}

void DcmtkServerDialog::enableOpenButton() {
    openButton_->setEnabled(true);
}

void DcmtkServerDialog::selectOutputDir() {
    QString targetPath = QFileDialog::getExistingDirectory(this, "Choose a directory");
    if (targetPath.isEmpty())
        return;
    else {
        outputDirectory_->setText(targetPath);
        QSettings settings;
        settings.setValue("outputDir", targetPath);
    }
}

void DcmtkServerDialog::loadServers() {
    QSettings settings;//("servers.ini", QSettings::IniFormat);
    servers_.clear();
    int numservers = settings.value("numServers", 0).toInt();
    QString targetPath = settings.value("outputDir").toString();
    outputDirectory_->setText(targetPath);
    DcmtkServerConfig conf;

    for (int i=0; i<numservers; ++i) {
        QString keyname = "server";
        keyname += (i+1);
        conf.name_ = settings.value(keyname+"/name", "").toString ();
        conf.hostname_ = settings.value(keyname+"/hostname", "").toString ();
        conf.serverPort_ = settings.value(keyname+"/serverport", 27620).toInt();
        conf.callAE_ = settings.value(keyname+"/callae", "").toString ();
        conf.destAE_ = settings.value(keyname+"/destae", "").toString ();
        conf.inPort_ = settings.value(keyname+"/inport", 5679).toInt();

        conf.sslEnabled_ = settings.value(keyname+"/sslenabled", 0).toInt();
        conf.clientKey_ = settings.value(keyname+"/clientkey", "").toString ();
        conf.clientCert_ = settings.value(keyname+"/clientcert", "").toString ();
        conf.serverCert_ = settings.value(keyname+"/servercert", "").toString ();

        servers_.push_back(conf);
    }
}

void DcmtkServerDialog::saveServers() {
    QSettings settings;//("servers.ini", QSettings::IniFormat);
    settings.setValue("numServers", (int)servers_.size());
    for (size_t i=0; i<servers_.size(); ++i) {
        QString keyname = QString("server %1").arg(i+1);
        settings.setValue(keyname+"/name", servers_[i].name_);
        settings.setValue(keyname+"/hostname", servers_[i].hostname_);
        settings.setValue(keyname+"/serverport", servers_[i].serverPort_);
        settings.setValue(keyname+"/callae", servers_[i].callAE_);
        settings.setValue(keyname+"/destae", servers_[i].destAE_);
        settings.setValue(keyname+"/inport", servers_[i].inPort_);

        settings.setValue(keyname+"/sslenabled", servers_[i].sslEnabled_);
        settings.setValue(keyname+"/clientkey", servers_[i].clientKey_);
        settings.setValue(keyname+"/clientcert", servers_[i].clientCert_);
        settings.setValue(keyname+"/servercert", servers_[i].serverCert_);
    }
}

void DcmtkServerDialog::fillCombo() {
    serverCombo_->clear();
    for (size_t i=0; i<servers_.size(); ++i)
        serverCombo_->addItem(servers_[i].name_, QVariant((int)i));
}

void DcmtkServerDialog::displayConfig(int i) {
    currentConf_ = i;
    serverCombo_->setCurrentIndex(i);
}

void DcmtkServerDialog::editConfig() {
    configDialog_->editConfig(&servers_[currentConf_]);
    configDialog_->exec();
    saveServers();
}

void DcmtkServerDialog::addServer() {
    bool ok;
    QString name = QInputDialog::getText(this, tr("Add server config"), tr("Config name:"),
                                         QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;
    DcmtkServerConfig conf;
    conf.name_ = name;
    servers_.push_back(conf);
    serverCombo_->addItem(name, QVariant((int)servers_.size()-1));
    displayConfig(static_cast<int>(servers_.size())-1);
    editConfig();
}

void DcmtkServerDialog::deleteServer() {
    int cc = currentConf_;
    displayConfig(0);
    serverCombo_->removeItem(cc);
    servers_.erase(servers_.begin()+cc);
    saveServers();
}

void DcmtkServerDialog::updateList()
{
    QString url = QString("%1:%2/%3").arg(servers_[currentConf_].hostname_)
        .arg(servers_[currentConf_].serverPort_).arg(servers_[currentConf_].callAE_);

    voreen::DcmtkVolumeReader volumeReader;
    if (servers_[currentConf_].sslEnabled_) {
        security_ = voreen::DcmtkSecurityOptions(servers_[currentConf_].clientKey_.toStdString(),
                                                 servers_[currentConf_].clientCert_.toStdString());
        security_.addCertificateFile(servers_[currentConf_].serverCert_.toStdString());
    } else {
        security_ = voreen::DcmtkSecurityOptions();
    }
    volumeReader.setSecurityOptions(security_);
    std::vector<voreen::DcmtkSeriesInfo> series
        = volumeReader.listSeries("dicom://FINDSCU@" + url.toStdString());

    if (series.size() > 0) {
        url = QString("dicom://%1:%2@") .arg(servers_[currentConf_].destAE_).
            arg(servers_[currentConf_].inPort_) + url;
        listWidget_->setURL(url);
        listWidget_->setSeries(series);
    } else {
        QMessageBox::warning(this, "Voreen", "No series found.");
    }
}

namespace {

// Helper for analyzing a Dicom URL for C-MOVE
bool analyze_URL_path(const std::string &path, std::string &peerTitle,
                      std::string &seriesInstanceUID)
{
    // ^/([A-Za-z0-9.\-_]+)\?seriesInstanceUID=[0-9\.]$
    const std::string number_point = "0123456789.";
    const std::string peer_name = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-"
        + number_point;
    std::string s(path);

    // ^/
    if (path.substr(0, 1) != "/") return false;

    // ([A-Za-z0-9.\-_]+)\?seriesInstanceUID=
    size_t pos = s.find_first_not_of(peer_name, 1);
    if (pos <= 1) return false;
    peerTitle = s.substr(1, pos - 1);
    size_t pos2 = s.find("?seriesInstanceUID=", pos);
    if (pos2 != pos) return false;

    //[0-9\.]$
    if (s.find_first_not_of(number_point, pos2 + 19) != std::string::npos) return false;
    seriesInstanceUID = s.substr(pos2 + 19);

    return true;
}

} // namespace

void DcmtkServerDialog::open() {
#ifdef VRN_DICOMLOADER
    if (listWidget_->getSelectedSeriesInstanceUID().empty())
        return;

    int numImages = listWidget_->getSelectedNumImages();
    std::cout << "Downloading " << numImages << " images\n";
    pd_ = new QProgressDialog("Downloading...", "Cancel", 0, numImages);
    connect(pd_, SIGNAL(canceled()), this, SLOT(cancelDownload()));
    pd_->setValue(0);

    t_ = new QTimer(this);
    connect(t_, SIGNAL(timeout()), this, SLOT(updateProgressbar()));
    t_->start(100);


    QString targetPath = outputDirectory_->text();
    if (targetPath.isEmpty())
        return;

    repaint();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    std::string configFile = (QApplication::applicationDirPath()
                              + "/dicomloader-storescp.cfg").toStdString();

    if (!voreen::DcmtkMoveSCU::init(servers_[currentConf_].destAE_.toStdString(),
                                    servers_[currentConf_].inPort_,
                                    servers_[currentConf_].hostname_.toStdString(),
                                    servers_[currentConf_].serverPort_,
                                    security_, configFile))
    {
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(this, "Init Connection",
                              QString("Invalid connection parameters for DicomMoveSCU"),
                              QMessageBox::Ok, QMessageBox::NoButton);

        t_->stop();
        pd_->setValue(numImages);
        return;
    }
    delete dt_;
    dt_ = new DcmtkDownloadThread(targetPath.toStdString(),
                             servers_[currentConf_].callAE_.toStdString(),
                             listWidget_->getSelectedSeriesInstanceUID());
    connect(dt_, SIGNAL(finished()), this, SLOT(downloadFinished()));
    dt_->start();
#else
    hide();
    emit dicomServerFinished();
#endif
}

void DcmtkServerDialog::cancelDownload() {
    voreen::DcmtkMoveSCU::cancelMove();
}

void DcmtkServerDialog::updateProgressbar() {
    pd_->setValue(voreen::DcmtkMoveSCU::getNumDownloaded());
}


void DcmtkServerDialog::downloadFinished() {
    t_->stop();

    QApplication::restoreOverrideCursor();
    if (pd_->wasCanceled())
        QMessageBox::information(0, QString("Series move"),
                              QString("Download canceled."),
                              QMessageBox::Ok, QMessageBox::NoButton);
    else if (dt_->successful())
        QMessageBox::information(0, QString("Series move"),
                              QString("Download finished."),
                              QMessageBox::Ok, QMessageBox::NoButton);
    else
        QMessageBox::critical(0, QString("Series move"),
                              QString("Failed to move series"),
                              QMessageBox::Ok, QMessageBox::NoButton);

    pd_->close();
}

std::string DcmtkServerDialog::getURL() {
    if (listWidget_->getSelectedSeriesInstanceUID().empty())
        return "";
    else
        return "dicom://" + servers_[currentConf_].callAE_.toStdString() + ":"
            + (QString::number(servers_[currentConf_].inPort_)).toStdString() + "@"
            + servers_[currentConf_].destAE_.toStdString() + ":"
            + (QString::number(servers_[currentConf_].serverPort_)).toStdString()+ "/"
            + servers_[currentConf_].hostname_.toStdString() + "?seriesInstanceUID="
            + listWidget_->getSelectedSeriesInstanceUID();
}

//
// Series
//

DcmtkSeriesListWidget::DcmtkSeriesListWidget() {
    QString str = "id,NumImages,Patient Name,Patient ID,Date,Time,Modality,Description";
    QStringList headers = str.split(",");
    table = new QTableWidget(20, headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->setSortingEnabled(true);

    table->setSelectionBehavior(QAbstractItemView::SelectRows);

    filter_ = new QLineEdit();
    connect(filter_, SIGNAL(textChanged(const QString&)),
            this, SLOT(updateFilter(const QString&)));
    filterCombo_ = new QComboBox();
    filterCombo_->addItem("UID");
    filterCombo_->addItem("Images");
    filterCombo_->addItem("PatientsName");
    filterCombo_->addItem("PatientID");
    filterCombo_->addItem("StudyDate");
    filterCombo_->addItem("StudyTime");
    filterCombo_->addItem("Modality");

    connect(table, SIGNAL(itemSelectionChanged ()), this, SIGNAL(seriesSelected()));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(new QLabel("Search:"),0,0);
    layout->addWidget(filter_,0,1);
    layout->addWidget(new QLabel("Attribute:"),1,0);
    layout->addWidget(filterCombo_,1,1,1,1);
    layout->addWidget(table, 2, 0, 1, 2);
    setLayout(layout);

    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

namespace {
    class QTableWidgetItemDate : public QTableWidgetItem {
    public:
        QTableWidgetItemDate(QDate date)
            : QTableWidgetItem(date.toString("dd.MM.yyyy")) {}

        bool operator< ( const QTableWidgetItem & other ) const {
            QDate date = QDate::fromString(other.text(), "dd.MM.yyyy");
            return (QDate::fromString(text(), "dd.MM.yyyy") < date);
        }
    };
}

void DcmtkSeriesListWidget::setSeries(std::vector<voreen::DcmtkSeriesInfo>& list) {
//     table->clear();
    table->setSortingEnabled(false);  // disable sorting temporarly for inserting new items
    std::vector<voreen::DcmtkSeriesInfo>::iterator theIterator;
    int c = 0;
    table->setRowCount(static_cast<int>(list.size()));
    QDate date;
    std::string temp;
    for (theIterator = list.begin(); theIterator != list.end(); theIterator++) {
        QTableWidgetItem* it = new QTableWidgetItem(QString::fromStdString((*theIterator).uid_));
        it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        table->setItem(c, 0, it);

        it = new QTableWidgetItem(QString::fromStdString((*theIterator).numImages_));
        it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        table->setItem(c, 1, it);

        it = new QTableWidgetItem(QString::fromStdString((*theIterator).patientsName_));
        it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        table->setItem(c, 2, it);

        it = new QTableWidgetItem(QString::fromStdString((*theIterator).patientId_));
        it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        table->setItem(c, 3, it);

        temp = (*theIterator).studyDate_;
        if (temp.length() == 8) {
            int y = (temp[0]-'0')*1000+(temp[1]-'0')*100+(temp[2]-'0')*10+(temp[3]-'0');
            int m = (temp[4]-'0')*10+(temp[5]-'0');
            int d = (temp[6]-'0')*10+(temp[7]-'0');
            date.setYMD(y,m,d);
            it = new QTableWidgetItemDate(date);
        }
        else
            it = new QTableWidgetItem(QString::fromStdString(temp));
        it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        table->setItem(c, 4, it);

        temp = (*theIterator).studyTime_;
        if (temp.length() >= 6) {
            temp.insert(4,":");
            temp.insert(2,":");
        }
//         else
            it = new QTableWidgetItem(QString::fromStdString(temp));
        it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        table->setItem(c, 5, it);

        it = new QTableWidgetItem(QString::fromStdString((*theIterator).modality_));
        it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        table->setItem(c, 6, it);

        it = new QTableWidgetItem(QString::fromStdString((*theIterator).description_));
        it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        table->setItem(c, 7, it);

        ++c;
    }
    table->setSortingEnabled(true);  // re-enable sorting
}

std::string DcmtkSeriesListWidget::getSelectedSeriesInstanceUID() {
    if (table->item(table->currentRow(), 0))
        return table->item(table->currentRow(), 0)->text().toStdString();
    else
        return "";
}

int DcmtkSeriesListWidget::getSelectedNumImages() {
    if (table->item(table->currentRow(), 1))
        return table->item(table->currentRow(), 1)->text().toInt();
    else
        return 0;
}

void DcmtkSeriesListWidget::updateFilter(const QString & text) {
    QTableWidgetItem* it;
    int cur = filterCombo_->currentIndex();

    if (text == "") {
        for (int r=0; r<table->rowCount(); ++r) {
            for (int c=0; c<table->columnCount(); ++c) {
                it = table->item(r,c);
                it->setBackgroundColor(QColor(255, 255, 255, 127));
            }
        }
    }
    else {
        bool foundInRow;
        for (int r=0; r<table->rowCount(); ++r) {
            foundInRow = false;
             for (int c=0; c < table->columnCount(); ++c) {
                it = table->item(r,c);
                if ((c == cur) && (it->text().contains(text, Qt::CaseInsensitive))) {
                    foundInRow = true;
                }
                it->setBackgroundColor(QColor(255, 255, 255, 127));
             }
            if (foundInRow) {
                for (int c=0; c<table->columnCount(); ++c) {
                    it = table->item(r,c);
//                     if (it->text().contains(text, Qt::CaseInsensitive)) {
                    if (c == cur) {
                        it->setBackgroundColor(QColor(255, 0, 0, 127));
                    }
                    else {
                        it->setBackgroundColor(QColor(0, 0, 255, 127));
                    }
                }
            }
        }
    }
}

//
// Dicomdir
//

DcmtkDicomDirDialog::DcmtkDicomDirDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("DicomDir");

    listWidget_ = new DcmtkSeriesListWidget();
    openButton_ = new QPushButton("Open");
    openButton_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(openButton_, SIGNAL(clicked()), this, SLOT(open()));

    QGridLayout *layout = new QGridLayout;

    layout->addWidget(listWidget_,0,0,(Qt::Alignment)0);
    layout->addWidget(openButton_,1,0,(Qt::Alignment)0);

    listWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setLayout(layout);

    // set initial size
    resize(600, 300);

//     if (servers_.size() > 0)
//         displayConfig(servers_[0]);

}

void DcmtkDicomDirDialog::open() {
    hide();
    emit dicomDirFinished();
}

std::string DcmtkDicomDirDialog::getFilename() {
    if (listWidget_->getSelectedSeriesInstanceUID().empty())
        return filename_;
    else
        return filename_ + "?" + "seriesInstanceUID=" + listWidget_->getSelectedSeriesInstanceUID();
}

} // namespace
