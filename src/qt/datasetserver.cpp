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

#include "voreen/qt/datasetserver.h"

#include <iostream>
#include <limits>

#include <QRegExp>
#include <QString>

#include "tgt/vector.h"

#include "voreen/core/volume/volumeatomic.h"


/*
    FIXME I guess this code is not little/big endian friendly
*/

namespace voreen {

const std::string DatasetServer::loggerCat_("voreen.qt.DatasetServer");

DatasetServer::DatasetServer(MessageReceiver* messageReceiver, quint16 port)
    : messageReceiver_(messageReceiver)
{
    // connect signal with slot
    connect(&server_, SIGNAL(newConnection()), this, SLOT(newConnection()));

    // allow only one connection
//     server_.setMaxPendingConnections(1);
    server_.listen(QHostAddress::Any, port);
}

DatasetServer::~DatasetServer() {
}

void DatasetServer::newConnection() {
    LINFO("new connection");
    socket_ = server_.nextPendingConnection();

    // connect signals of new socket with proper slots
    connect(socket_, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket_, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

void DatasetServer::readyRead() {
    // append new incoming data chunk to this object's data buffer
    QByteArray data = socket_->readAll();
    data_.append(data);
}

void DatasetServer::disconnected() {
    // search for end marker "DATA\n"
    QByteArray end("DATA\n");
    int endIndex = data_.indexOf(end);
    if (endIndex < 0) {
        LERROR("end marker DATA not found");
        return;
    }

    // else
    QString str(data_);
    str.resize(endIndex);

    // log meta data
    LDEBUG( str.toStdString() );

    // erase non meta data + "DATA\n" from data_
    data_.remove(0, endIndex + 5);

    /*
        parse meta data
    */

    // parse magic
    QRegExp magic("Magic *= *B7344782\n", Qt::CaseInsensitive);
    if (magic.indexIn(str) == -1) {
        LERROR("wrong magic number found");
        return;
    }

    // parse dimension
    QRegExp dim("Dimension *= *", Qt::CaseInsensitive);
    int dimIndex = dim.indexIn(str);
    if (dimIndex == -1) {
        LERROR("no dimension tag found");
        return;
    }
    dimIndex += dim.matchedLength();

    // parse 3 dimension numbers
    tgt::ivec3 vDim;
    for (int i = 0; i < 3; ++i) {
        int firstIndex = dimIndex;
        QRegExp number("\\d+[ \n]+"); // match integer + space or new line
        dimIndex = number.indexIn(str, firstIndex);

        if (dimIndex == -1) {
            LERROR("could not find a proper number");
            return;
        }

        dimIndex += number.matchedLength();

        QString strNumber;
        for (int j = firstIndex; str[j] != ' ' && str[j] != '\n' && j < dimIndex; ++j)
            strNumber.push_back(str[j]);

        // store number in vDim
        bool ok;
        vDim[i] = strNumber.toInt(&ok);
        if (!ok) {
            LERROR("could not parse number");
            return;
        }
    }

    // check whether found dimensions fit
    if (tgt::hmul(vDim)  * 4 != data_.size()) {
        LERROR("data chunk does not fit with proposed dimensions");
        return;
    }

    /*
        everything seems to be ok, so build new data set
    */
    VolumeUInt16* dataset = new VolumeUInt16(vDim);
    for (int i = 0; i < data_.size(); i += 4) {
        // unpack float
        union Float_4xUint8 {
            float f_;
            uint8_t ui8_[4];
        } f_4xui8;

        f_4xui8.ui8_[0] = data_[i+0];
        f_4xui8.ui8_[1] = data_[i+1];
        f_4xui8.ui8_[2] = data_[i+2];
        f_4xui8.ui8_[3] = data_[i+3];

        uint16_t ui16;

        // catch corner cases
        if (f_4xui8.f_ <= 0.f)
            ui16 = 0;
        else if (f_4xui8.f_ >= 1.f)
            ui16 = std::numeric_limits<uint16_t>::max();
        else
            ui16 = static_cast<uint16_t>(f_4xui8.f_ * std::numeric_limits<uint16_t>::max());

        // pack to new data set
        dataset->voxel()[i/4] = ui16;
    }

    // post to messageReceiver_
    messageReceiver_->postMessage( new VolumePtrMsg("setNewDataset", dataset) );

    // clean up
    data_.clear();
}

} // namespace voreen
