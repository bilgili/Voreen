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

#include "voreen/core/vis/processors/render/sliceproxygeometry.h"

namespace voreen {

using tgt::vec3;

const Identifier SliceProxyGeometry::setSlicePosX_("set.slicePosX");
const Identifier SliceProxyGeometry::setSlicePosY_("set.slicePosY");
const Identifier SliceProxyGeometry::setSlicePosZ_("set.slicePosZ");

SliceProxyGeometry::SliceProxyGeometry()
  : ProxyGeometry(),
    dl_(0),
    slicePosX_(setSlicePosX_, "X Slice", 1.0f, 0.0f, 2.0f, true),
    slicePosY_(setSlicePosY_, "Y Slice", 1.0f, 0.0f, 2.0f, true),
    slicePosZ_(setSlicePosZ_, "Z Slice", 1.0f, 0.0f, 2.0f, true)
{
    setName("Slice-ProxyGeometry");

    addProperty(&slicePosX_);
    addProperty(&slicePosY_);
    addProperty(&slicePosZ_);

    createInport("volumehandle.volumehandle");
	createCoProcessorOutport("coprocessor.proxygeometry", &Processor::call);

	setIsCoprocessor(true);
}

const std::string SliceProxyGeometry::getProcessorInfo() const {
	return "Provides a slice proxy geometry. The resulting geometry depends on the ratio \
        of the values in dim.";
}

void SliceProxyGeometry::setPropertyDestination(Identifier tag) {
    MessageReceiver::setTag(tag);
    slicePosX_.setMsgDestination(tag);
    slicePosY_.setMsgDestination(tag);
    slicePosZ_.setMsgDestination(tag);
    MsgDistr.insert(this);
}

SliceProxyGeometry::~SliceProxyGeometry() {
	if (dl_)
		glDeleteLists(dl_, 1);

	if (MessageReceiver::getTag() != Message::all_)
		MsgDistr.remove(this);
}

/**
 * Renders the OpenGL list (and creates it, when needed).
 */
void SliceProxyGeometry::render() {
    if (volume_) {
        if (needsBuild_) {
            if (!dl_) {
                dl_ = glGenLists(1);
            }
            revalidateSliceGeometry();
            needsBuild_ = false;
        }
        glCallList(dl_);
    }
}

void SliceProxyGeometry::revalidateSliceGeometry() {
    vec3 geomLlf = -volumeSize_;
    vec3 geomUrb = volumeSize_;

    // recreate display list
    glNewList(dl_, GL_COMPILE);

    glBegin(GL_QUADS);
        glVertex3f(geomLlf[0]+slicePosX_.get(), geomLlf[1], geomLlf[2]);
        glVertex3f(geomLlf[0]+slicePosX_.get(), geomUrb[1], geomLlf[2]);
        glVertex3f(geomLlf[0]+slicePosX_.get(), geomUrb[1], geomUrb[2]);
        glVertex3f(geomLlf[0]+slicePosX_.get(), geomLlf[1], geomUrb[2]);

        glVertex3f(geomLlf[0], geomLlf[1]+slicePosY_.get(), geomLlf[2]);
        glVertex3f(geomLlf[0], geomLlf[1]+slicePosY_.get(), geomUrb[2]);
        glVertex3f(geomUrb[0], geomLlf[1]+slicePosY_.get(), geomUrb[2]);
        glVertex3f(geomUrb[0], geomLlf[1]+slicePosY_.get(), geomLlf[2]);

        glVertex3f(geomLlf[0], geomLlf[1], geomLlf[2]+slicePosZ_.get());
        glVertex3f(geomUrb[0], geomLlf[1], geomLlf[2]+slicePosZ_.get());
        glVertex3f(geomUrb[0], geomUrb[1], geomLlf[2]+slicePosZ_.get());
        glVertex3f(geomLlf[0], geomUrb[1], geomLlf[2]+slicePosZ_.get());
	glEnd();

    glEndList();

    LGL_ERROR;
}

void SliceProxyGeometry::processMessage(Message *msg, const Identifier& dest) {
    ProxyGeometry::processMessage(msg, dest);

    if (msg->id_ == setSlicePosX_) {
		slicePosX_.set(msg->getValue<float>());
		needsBuild_ = true;
        invalidate();
	}
    else if (msg->id_ == setSlicePosY_) {
		slicePosY_.set(msg->getValue<float>());
		needsBuild_ = true;
        invalidate();
	}
    else if (msg->id_ == setSlicePosZ_) {
		slicePosZ_.set(msg->getValue<float>());
		needsBuild_ = true;
        invalidate();
	}
}

} // namespace voreen
