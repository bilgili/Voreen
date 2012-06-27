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

#ifndef VRN_MESSAGE_H
#define VRN_MESSAGE_H

#include "tgt/quaternion.h"

#include "voreen/core/vis/identifier.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace tgt {

class GLCanvas;
class Texture;
class Camera;

}

namespace voreen {

template<class T>
struct TemplateMessage;
class Processor;

/// Exception class for Messages
struct message_error : public std::runtime_error {

	/// @param msg The error message.
    message_error(const std::string& msg)
		: std::runtime_error(msg)
	{}
};

/**
 * This class is used by MessageReceiver::postMessage and
 * MessageReceiver::processMessage. It is used for the type of Message
 * and the Receiver.
 */
struct Message {
    Identifier id_;   ///< Specifies the type of the message (but not the data type)
    bool stamped_;    ///< A message gets stamped when it is posted.

    /// @param id The identifier for this message
    Message(const Identifier& id);
    virtual ~Message() {}

    /// Gets the value which this message holds.
    template<class T>
    T getValue() {
        if(TemplateMessage<T>* msg = dynamic_cast<TemplateMessage<T>*>(this))
            return msg->value_;
        else
            throw message_error("type mismatch in voreen::Message::getValue()");
    }

    /**
     * Gets the value which this message is holding.
     *
     * @param value The return value.
     */
    template<class T>
    void getValue(T& value) {
        if(TemplateMessage<T>* msg = dynamic_cast<TemplateMessage<T>*>(this))
            value = msg->value_;
        else
            throw message_error("type mismatch in voreen::Message::getValue()");
    }

    /// Discards this Message. \a id_ will be set to Identifier::discarded.
    void discard();

    /**
     * Use this method to find out, whether further propagating is not necessary.
     *
     * @return true if this message was discarded, false otherwise
     */
    bool isDiscarded();

    static const Identifier all_;
    static const Identifier discarded_;
};

/**
 * This TemplateMessage holds a value of type T.
*/
template<class T>
struct TemplateMessage : public Message {
    TemplateMessage(Identifier id, T value)
        : Message(id), value_(value)
    {}
    T value_;
};


struct CoarsenessStruct {
	Processor* processor;
	float coarsenessFactor;
};

typedef TemplateMessage<bool> BoolMsg;
typedef TemplateMessage<size_t> SizeTMsg;
typedef TemplateMessage<int> IntMsg;
typedef TemplateMessage<float> FloatMsg;
typedef TemplateMessage<std::string> StringMsg;
typedef TemplateMessage<tgt::vec2> Vec2Msg;
typedef TemplateMessage<tgt::vec3> Vec3Msg;
typedef TemplateMessage<tgt::vec4> Vec4Msg;
typedef TemplateMessage<tgt::ivec2> IVec2Msg;
typedef TemplateMessage<tgt::ivec3> IVec3Msg;
typedef TemplateMessage<tgt::ivec4> IVec4Msg;
typedef TemplateMessage<tgt::quat> QuatMsg;
typedef TemplateMessage<tgt::Color> ColorMsg;
typedef TemplateMessage<tgt::Texture*> TexturePtrMsg;
typedef TemplateMessage< std::vector<tgt::vec3> > stdVectorVec3Msg;
typedef TemplateMessage<tgt::GLCanvas*> GLCanvasPtrMsg;
typedef TemplateMessage<tgt::Camera*> CameraPtrMsg;
class Glyphs;
typedef TemplateMessage<Glyphs*> GlyphPtrMsg;
typedef TemplateMessage<Processor*> ProcessorPointerMsg;
typedef TemplateMessage<CoarsenessStruct*> CoarsenessMsg;
typedef TemplateMessage<std::vector<std::string> > StringVectorMsg;

/**
 * All classes which should receive messages must inherit from this class.
*/
class MessageReceiver {
public:

	/**
	 * Every MessageReceiver has a Tag. This Tag can be used to address
	 * different MessageReceiver objects in the MessageDistributer.
	 *
	 * @param tag The Identifier of the tag for this MessageReceiver.
	 * If it is set to Message::all_ (default argument), the MessageDistributer
	 * will address this class on all events.
	 */
    MessageReceiver(const Identifier& tag = Message::all_)
        : tag_(tag)
    {}
    virtual ~MessageReceiver() {}

    /**
     * Processes a message. If the message is not stamped (i.e., postMessage() was not called),
     * a voreen::message_error is thrown.
     *
     * @param msg Message to be processed, must be stamped.
     * @param dest Destination of the message (optional)
     */
    virtual void processMessage(Message* msg, const Identifier& dest = Message::all_);

    /**
     * Processes a message. If the message is not stamped (i.e., postMessage() was not called),
     * a voreen::message_error is thrown.
     *
     * @param msg Message to be processed
     * @param dest Destinations of the message
     */
    virtual void processMessage(Message* msg, const std::vector<Identifier>& dest);

    /**
     * Stamps, processes and deletes a message. If the message is already stamped, a
     * voreen::message_error is thrown.
     *
     * @param msg Message to be processed, it is deleted after processing.
     * @param dest Destination of the message (optional)
     */
    virtual void postMessage(Message* msg, const Identifier& dest = Message::all_);

    /**
     * Stamps, processes and deletes a message. If the message is already stamped, a
     * voreen::message_error is thrown.
     *
     * @param msg Message to be processed, it is deleted after processing.
     * @param dest Destinations of the message
     */
    virtual void postMessage(Message* msg, const std::vector<Identifier>& dest);

	/// Gets the tag for this class.
    Identifier getTag() {
        return tag_;
    }

	/// Sets the tag for this class.
    void setTag(Identifier tag) {
        tag_ = tag;
    }

protected:
    Identifier tag_;

};

} // namespace voreen

#endif // VRN_MESSAGE_H
