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

#ifndef VRN_CONNECTIONBUNDLE_H
#define VRN_CONNECTIONBUNDLE_H

#include <QList>
#include <QPointF>
#include "voreen/core/datastructures/meta/metadatabase.h"

#include "voreen/core/network/portconnection.h"

class QGraphicsSceneMouseEvent;

namespace voreen {

class ConnectionBundleHandle;
class NetworkEditor;
class PortArrowGraphicsItem;

/**
 * Helper class for connection bundles
 */
struct ConnectionBundle {

    ConnectionBundle(const QList<PortArrowGraphicsItem*>& arrowList, NetworkEditor* nwe);
    ~ConnectionBundle();

    bool operator==(const ConnectionBundle& rhs) const {
        return rhs.arrowList_ == arrowList_;
    }

    int getArrowIndex(const PortArrowGraphicsItem* arrow) const;
    void updateScene();
    void setBundlePointsDefault();
    void addHandle(const QPointF& p);
    QList<QPointF> getBundlePoints();

    QList<PortArrowGraphicsItem*> arrowList_;
    NetworkEditor* nwe_;
    ConnectionBundleHandle* startHandle_;
    ConnectionBundleHandle* endHandle_;
    QList<ConnectionBundleHandle*> handles_;
};

class ConnectionBundleMetaData : public MetaDataBase {

public:

    ConnectionBundleMetaData(ConnectionBundle* bundle = 0);
    virtual ~ConnectionBundleMetaData();

    virtual std::string getClassName() const { return "ConnectionBundleMetaData"; }
    virtual Serializable* create() const;
    virtual MetaDataBase* clone() const;
    virtual std::string toString() const;
    virtual std::string toString(const std::string& /*component*/) const;

    /**
     * @see Serializable::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Serializable::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

    const std::vector<PortConnection>& getConnections() const {
        return connections_;
    }

    const std::vector<tgt::vec2>& getBundlePoints() const {
        return bundlePoints_;
    }

    bool isStartDetached() const {
        return startDetached_;
    }

    bool isEndDetached() const {
        return endDetached_;
    }

private:

    std::vector<PortConnection> connections_;
    std::vector<tgt::vec2> bundlePoints_;
    bool startDetached_;
    bool endDetached_;

};

}

#endif

