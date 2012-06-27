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

#ifndef VRN_MESSAGEDISTRIBUTOR_H
#define VRN_MESSAGEDISTRIBUTOR_H

#include <map>

#include "tgt/singleton.h"

#include "voreen/core/vis/message.h"

namespace voreen {

typedef std::multimap<Identifier, MessageReceiver*> ReceiverMap;

/**
 *  Broadcasts messages to message-receivers.  MessageReceiver-Objects can be added, and messages will be forwarded to them.
 */
class MessageDistributor : public MessageReceiver, private ReceiverMap {
public:

    MessageDistributor();

    /**
     * Broadcast an incoming message to all managed MessageReceivers that have the correct tag.
     *
     * @param msg The incoming message.
     * @param dest The destination of the message.
     */
    void processMessage(voreen::Message* msg, const voreen::Identifier& dest = voreen::Message::all_);

    /**
     * Always produces a unique Identifier. This is convenient if you need many identifiers and don't always want to think of a name.
     *
     * @return Returns the new Identifer.
     */
    voreen::Identifier getFreeTag();

    /**
     * There are always only certain views that are visible, so, for example, repaint messages need only be forwarded to them.
     * Using this method, you can specifiy which views are currently visible.
     *
     * @param id The visible-views-id.
     */
    void setCurrentViewId(Identifier id) {
        currViewId_ = id;
    }

    const voreen::Identifier& getCurrentViewId() {
        return currViewId_;
    }

    void setAllowRepaints(bool allow) { allowRepaints_ = allow; }

     /**
      * Add a MessageReceiver that this distributor is supposed to manage.
      *
      * @param mr The MessageReceiver that is to be added.
      */
    iterator insert(MessageReceiver* mr);

    /**
      * Removes a MessageReceiver from this distributor.
      *
      * @param mr The MessageReceiver that is to be removed.
      */
    void remove(MessageReceiver* mr);

    /**
     * Returns wether a certain \c MessageReceiver is handled by this
     * distributor.
     *
     * @param mr the message receiver to check
     */
    bool contains(MessageReceiver* mr);

protected:

    static int      freeTagCounter_; ///< counts the generated tag-names so it is always possible to create a unique one
    Identifier      currViewId_; ///< The id of the currently visible views.
    bool            allowRepaints_; ///< Are repaint-messages supposed to be forwarded?

private:

    static const std::string loggerCat_;
};

} // namespace voreen

#define MsgDistr tgt::Singleton<voreen::MessageDistributor>::getRef()

#endif // VRN_MESSAGEDISTRIBUTOR_H
