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

#include "connectionbundle.h"
#include "connectionbundlehandle.h"
#include "networkeditor.h"
#include "portarrowgraphicsitem.h"
#include "portgraphicsitem.h"

#include <limits>

namespace voreen {

ConnectionBundle::ConnectionBundle(const QList<PortArrowGraphicsItem*>& arrowList, NetworkEditor* nwe) : arrowList_(arrowList), nwe_(nwe) {
    startHandle_ = new ConnectionBundleHandle(this);
    endHandle_   = new ConnectionBundleHandle(this);
    nwe_->scene()->addItem(startHandle_);
    nwe_->scene()->addItem(endHandle_);

    setBundlePointsDefault();
}

ConnectionBundle::~ConnectionBundle() {
    foreach(ConnectionBundleHandle* cbh, handles_) {
        nwe_->scene()->removeItem(cbh);
        delete cbh;
    }
    nwe_->scene()->removeItem(startHandle_);
    nwe_->scene()->removeItem(endHandle_);
    delete startHandle_;
    delete endHandle_;
}

int ConnectionBundle::getArrowIndex(const PortArrowGraphicsItem* arrow) const {
    for(int i = 0; i < arrowList_.size(); i++) {
        if(arrow == arrowList_.at(i))
            return i;
    }
    return -1;
}

void ConnectionBundle::setBundlePointsDefault() {
    float avgSourceX = 0.f;
    float avgDestX   = 0.f;

    float maxYS = -std::numeric_limits<float>::max();
    float minYD = std::numeric_limits<float>::max();

    //tgtAssert(arrowList_.size() >= 2, "Too few bundle points");
    if(arrowList_.size() < 2)
        return;

    foreach (PortArrowGraphicsItem* arrow, arrowList_) {
        QPointF source = arrow->mapFromItem(arrow->getSourceItem(), 0.f, 0.f);
        QPointF dest   = arrow->mapFromItem(arrow->getDestinationItem(), 0.f, 0.f);

        avgSourceX += source.x();
        avgDestX   += dest.x();
        if(source.y() > maxYS)
            maxYS = source.y();
        if(dest.y() < minYD)
            minYD = dest.y();
    }

    avgSourceX /= (float)(arrowList_.size());
    avgDestX   /= (float)(arrowList_.size());
    maxYS += 50.f;
    minYD -= 50.f;

    float radius = arrowList_.size() * 3.f;
    QRectF handleRect = QRectF(-radius, -10.f, 2.f * radius, 20.f);
    startHandle_->setRect(handleRect);
    foreach(ConnectionBundleHandle* handle, handles_)
        handle->setRect(handleRect);
    endHandle_->setRect(handleRect);

    QPointF bS(avgSourceX, maxYS);
    QPointF bD(avgDestX, minYD);

    if(!startHandle_->isDetached())
        startHandle_->setPos(bS);
    if(!endHandle_->isDetached())
        endHandle_->setPos(bD);

    foreach(PortArrowGraphicsItem* arrow, arrowList_)
        arrow->setBundleInfo(true, this);

    nwe_->scene()->update();
}

void ConnectionBundle::updateScene() {
    nwe_->updateCurrentBundles();
    nwe_->scene()->update();
}

QList<QPointF> ConnectionBundle::getBundlePoints() {
    QList<QPointF> res;
    res.append(startHandle_->pos());
    foreach(ConnectionBundleHandle* handle, handles_)
        res.append(handle->pos());
    res.append(endHandle_->pos());
    return res;
};

void ConnectionBundle::addHandle(const QPointF& p) {
    float minDist = QLineF(p, startHandle_->pos()).length();
    ConnectionBundleHandle* minHandle = startHandle_;
    foreach(ConnectionBundleHandle* handle, handles_) {
        float dist = QLineF(p, handle->pos()).length();
        if(dist < minDist) {
            minDist = dist;
            minHandle = handle;
        }
    }

    ConnectionBundleHandle* newHandle = new ConnectionBundleHandle(this);
    float radius = arrowList_.size() * 3.f;
    QRectF handleRect = QRectF(-radius, -10.f, 2.f * radius, 20.f);
    newHandle->setRect(handleRect);
    newHandle->setPos(p);
    nwe_->scene()->addItem(newHandle);
    handles_.insert(handles_.indexOf(minHandle) + 1, newHandle);
}

ConnectionBundleMetaData::ConnectionBundleMetaData(ConnectionBundle* bundle) {
    if(bundle) {
        foreach(PortArrowGraphicsItem* arrow, bundle->arrowList_)
            connections_.push_back(PortConnection(arrow->getSourceItem()->getPort(), arrow->getDestinationItem()->getPort()));

        bundlePoints_.push_back(tgt::vec2(bundle->startHandle_->pos().x(), bundle->startHandle_->pos().y()));
        foreach(ConnectionBundleHandle* handle, bundle->handles_) {
            QPointF pos = handle->pos();
            bundlePoints_.push_back(tgt::vec2(pos.x(), pos.y()));
        }
        bundlePoints_.push_back(tgt::vec2(bundle->endHandle_->pos().x(), bundle->endHandle_->pos().y()));

        startDetached_ = bundle->startHandle_->isDetached();
        endDetached_ = bundle->endHandle_->isDetached();
    }
}

ConnectionBundleMetaData::~ConnectionBundleMetaData() {}

void ConnectionBundleMetaData::serialize(XmlSerializer& s) const {
    s.serialize("Connections", connections_, "Connection");
    s.serialize("BundlePoints", bundlePoints_, "BundlePoint");
    s.serialize("StartHandleDetached", startDetached_);
    s.serialize("EndHandleDetached", endDetached_);
}

void ConnectionBundleMetaData::deserialize(XmlDeserializer& s) {
    s.deserialize("Connections", connections_, "Connection");
    s.deserialize("BundlePoints", bundlePoints_, "BundlePoint");
    s.deserialize("StartHandleDetached", startDetached_);
    s.deserialize("EndHandleDetached", endDetached_);
}

MetaDataBase* ConnectionBundleMetaData::clone() const {
    return new ConnectionBundleMetaData(*this);
}

Serializable* ConnectionBundleMetaData::create() const {
    return new ConnectionBundleMetaData();
}

std::string ConnectionBundleMetaData::toString() const {
    return "";
}

std::string ConnectionBundleMetaData::toString(const std::string& /*component*/) const {
    return toString();
}

} // namespace voreen

