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

#include "voreen/core/vis/message.h"

namespace voreen {

const Identifier Message::all_("all");
const Identifier Message::discarded_("discarded");

using std::vector;

Message::Message(const Identifier& id)
  : id_(id),
    stamped_(false)
{}

void Message::discard() {
    id_ = discarded_;
}

bool Message::isDiscarded() {
    return id_ == discarded_;
}

//------------------------------------------------------------------------------

void MessageReceiver::processMessage(Message* msg, const Identifier& /*dest*/)
{
    if (!msg->stamped_)
        throw message_error("voreen::message is not stamped! MessageReceiver::processMessage() was called directly, "
                            "call MessageReceiver::postMessage() instead.");
}

void MessageReceiver::processMessage(Message* msg, const vector<Identifier>& dest)
{
	for (vector<Identifier>::const_iterator iter = dest.begin(); iter != dest.end(); ++iter)
		processMessage(msg, *iter);
}

void MessageReceiver::postMessage(Message* msg, const Identifier& dest)
{
    if (msg->stamped_) {
        throw message_error("voreen::Message already stamped! Was MessageReceiver::postMessage() called twice?");
    } else {
        msg->stamped_ = true;
    }
    processMessage(msg, dest);
    delete msg;
    msg = 0;
}

void MessageReceiver::postMessage(Message* msg, const vector<Identifier>& dest)
{
    if (msg->stamped_) {
        throw message_error("voreen::Message already posted! Was MessageReceiver::postMessage() called twice?");
    } else {
        msg->stamped_ = true;
    }
    processMessage(msg, dest);
    delete msg;
    msg = 0;
}

} // namespace voreen
