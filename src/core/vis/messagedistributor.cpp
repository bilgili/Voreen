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

#include "voreen/core/vis/messagedistributor.h"

#include <sstream>
#include <utility> // ensure that std::make_pair is available

#include "tgt/logmanager.h"

#include "voreen/core/vis/voreenpainter.h"

namespace voreen {

int MessageDistributor::freeTagCounter_ = 0;
const std::string MessageDistributor::loggerCat_ = "voreen.core.vis.MessageDistributor";

MessageDistributor::MessageDistributor()
    : currViewId_(Message::all_)
{
    allowRepaints_ = true;
}

void MessageDistributor::processMessage(Message* msg, const Identifier& dest/*=Message::all_*/) {
    if (msg->id_ == VoreenPainter::repaint_ && !allowRepaints_) {
        LDEBUG("MessageDistributor::processMessage() got repaint while disallowed!\n"
            << "   (this means somewhere a repaint was triggered while the mainframe layout\n"
            << "    is not yet complete. Put a breakpoint on "
            << __FILE__ << " line " << __LINE__  << "\n"
            << "    and try to find out where this comes from using the stack-trace/backtrace)");
        return;
    }

    MessageReceiver::processMessage(msg, dest);

    if ( msg->isDiscarded() )
        return;

    // if the destination "all" was specified all message receivers should receive the message
    if (dest == Message::all_) {
        for (iterator iter = begin(); iter != end(); ++iter) {
            MessageReceiver* mr = iter->second;
            mr->processMessage(msg, dest);
            if ( msg->isDiscarded() )
                return;
        }
    }
    // the special dest visibleView was specified; substitute with currently visible view
    else if (dest == VoreenPainter::visibleViews_) {
        std::pair<iterator, iterator> range;
        if (currViewId_ == Message::all_) // if no special view identifier is specified, take all
            range = make_pair(begin(), end());
        else
            range = equal_range(currViewId_);

        for(iterator iter = range.first; iter != range.second; ++iter) {
            iter->second->processMessage(msg, currViewId_);
            if ( msg->isDiscarded() )
                return;
        }
    }
    // a destination was specified so give only message receivers of this tag the message
    else {
        std::pair<iterator, iterator> range = equal_range(dest);

        for(iterator iter = range.first; iter != range.second; ++iter) {
            iter->second->processMessage(msg, dest);
            if ( msg->isDiscarded() )
                return;
        }
    }
}

MessageDistributor::iterator MessageDistributor::insert(MessageReceiver* mr) {

    tgtAssert( !contains(mr), "Message receiver already inserted");

    return ReceiverMap::insert( std::make_pair(mr->getTag(), mr) );
}

void MessageDistributor::remove(MessageReceiver* mr) {
    // TODO use equal_range here for performance
    iterator iter = begin();
    while ( (iter != end()) && (iter->second != mr)  )
        ++iter;

    if (iter != end())
        ReceiverMap::erase(iter);
    else
        LWARNING("tried to remove a non-existent MessageReceiver from the map");
}

bool MessageDistributor::contains(MessageReceiver* mr) {

    iterator iter = begin();
    while ( (iter != end()) && (iter->second != mr) )
        ++iter;

    return (iter != end());
}

Identifier MessageDistributor::getFreeTag() {
    ++freeTagCounter_;

    std::ostringstream oss;
    // do not use "_@!" for your own group tags
    oss << "_@!" << freeTagCounter_;
    return oss.str();
}

} // namespace voreen
