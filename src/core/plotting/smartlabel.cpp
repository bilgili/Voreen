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

#include "voreen/core/plotting/smartlabel.h"

#include <algorithm>
#include <set>
#include <cmath>
#include <limits>

#include "tgt/font.h"
#include "tgt/vector.h"
#include "tgt/tgt_gl.h"

namespace voreen {

const std::string SmartLabelGroupBase::loggerCat_("voreen.plotting.SmartLabel");

namespace {
struct sorterY {
    bool operator() (const SmartLabel& lhs, const SmartLabel& rhs) {
        return (lhs.boundingBox_.center().y < rhs.boundingBox_.center().y);
    };
};

struct sorterX {
    bool operator() (const SmartLabel& lhs, const SmartLabel& rhs) {
        return (lhs.boundingBox_.center().x < rhs.boundingBox_.center().x);
    };
};
};

SmartLabel::SmartLabel(std::string text, tgt::vec3 pos, Alignment align, tgt::Color color, int size)
    : text_(text)
    , position_(pos)
    , align_(align)
    , color_(color)
    , size_(size)
{}

SmartLabelGroupBase::SmartLabelGroupBase(tgt::Font* font, float padding, tgt::Bounds bounds)
    : font_(font)
    , padding_(padding)
    , bounds_(bounds)
{}

void SmartLabelGroupBase::addLabel(std::string text, tgt::vec3 position, tgt::Color color, int size, SmartLabel::Alignment align) {
    SmartLabel sm(text, position, align, color, size);
    calculateBoundingBox(sm);
    labels_.push_back(sm);
}

void SmartLabelGroupBase::addLabel(std::string text, tgt::dvec2 position, tgt::Color color, int size, SmartLabel::Alignment align) {
    addLabel(text, tgt::vec3((float)position.x, (float)position.y, 0), color, size, align);
}

void SmartLabelGroupBase::reset() {
    labels_.clear();
}

void SmartLabelGroupBase::setBounds(tgt::Bounds bounds) {
    bounds_ = bounds;
}

void SmartLabelGroupBase::render() {
    for (std::vector<SmartLabel>::const_iterator it = labels_.begin(); it < labels_.end(); ++it)
        renderLabel(*it);
}

void SmartLabelGroupBase::renderSingleLabel(tgt::Font* font, const std::string& text, const tgt::vec3& position,
                                            const tgt::Color& color, int size, SmartLabel::Alignment align, float padding) {

    if (font->getSize() != size)
        font->setSize(size);

    tgt::vec3 drawingPos = position;
    glColor4f(color.r, color.g, color.b, color.a);
    tgt::Bounds labelBounds = font->getBounds(position, text);
    float textHeight = labelBounds.getURB().y - labelBounds.getLLF().y;
    float textWidth  = labelBounds.getURB().x - labelBounds.getLLF().x;

    switch (align) {
        case SmartLabel::TOPLEFT:
            drawingPos.x += padding;
            drawingPos.y += padding;
            break;
        case SmartLabel::TOPCENTERED:
            drawingPos.x -= textWidth/2;
            drawingPos.y += padding;
            break;
        case SmartLabel::TOPRIGHT:
            drawingPos.x -= textWidth + padding;
            drawingPos.y += padding;
            break;
        case SmartLabel::MIDDLELEFT:
            drawingPos.y -= textHeight/2;
            drawingPos.x += padding;
            break;
        case SmartLabel::CENTERED:
            drawingPos.x -= textWidth/2;
            drawingPos.y -= textHeight/2;
            break;
        case SmartLabel::MIDDLERIGHT:
            drawingPos.x -= textWidth + padding;
            drawingPos.y -= textHeight/2;
            break;
        case SmartLabel::BOTTOMLEFT:
            drawingPos.x += padding;
            drawingPos.y -= textHeight + padding;
            break;
        case SmartLabel::BOTTOMCENTERED:
            drawingPos.x -= textWidth/2;
            drawingPos.y -= textHeight + padding;
            break;
        case SmartLabel::BOTTOMRIGHT:
            drawingPos.x -= textWidth + padding;
            drawingPos.y -= textHeight + padding;
            break;
    }
    font->render(drawingPos, text);
}

void SmartLabelGroupBase::calculateBoundingBox(SmartLabel& sm) {
    if (font_->getSize() != sm.size_)
        font_->setSize(sm.size_);

    tgt::vec3 drawingPos = sm.position_;
    tgt::vec3 enlargement(padding_/2, padding_/2, 0);
    tgt::Bounds labelBounds = font_->getBounds(sm.position_, sm.text_);

    tgt::vec3 urb = labelBounds.getURB();
    tgt::vec3 llf = labelBounds.getLLF();

    float textHeight = urb.y - llf.y;
    float textWidth  = urb.x - llf.x;

    switch (sm.align_) {
        case SmartLabel::TOPLEFT:
            drawingPos.x += padding_;
            drawingPos.y += padding_;
            break;
        case SmartLabel::TOPCENTERED:
            drawingPos.x -= textWidth/2;
            drawingPos.y += padding_;
            break;
        case SmartLabel::TOPRIGHT:
            drawingPos.x -= textWidth + padding_;
            drawingPos.y += padding_;
            break;
        case SmartLabel::MIDDLELEFT:
            drawingPos.x += padding_;
            drawingPos.y -= textHeight/2;
            break;
        case SmartLabel::CENTERED:
            drawingPos.x -= textWidth/2;
            drawingPos.y -= textHeight/2;
            break;
        case SmartLabel::MIDDLERIGHT:
            drawingPos.x -= textWidth + padding_;
            drawingPos.y -= textHeight/2;
            break;
        case SmartLabel::BOTTOMLEFT:
            drawingPos.x += padding_;
            drawingPos.y -= textHeight + padding_;
            break;
        case SmartLabel::BOTTOMCENTERED:
            drawingPos.x -= textWidth/2;
            drawingPos.y -= textHeight + padding_;
            break;
        case SmartLabel::BOTTOMRIGHT:
            drawingPos.x -= textWidth + padding_;
            drawingPos.y -= textHeight + padding_;
            break;
    }

    // check label position within bounds
    if (drawingPos.x + textWidth > bounds_.getURB().x - padding_)
        drawingPos.x = bounds_.getURB().x - textWidth - padding_;
    if (drawingPos.y + textHeight > bounds_.getURB().y - padding_)
        drawingPos.y = bounds_.getURB().y - textHeight - padding_;
    if (drawingPos.x < bounds_.getLLF().x + padding_)
        drawingPos.x = bounds_.getLLF().x + padding_;
    if (drawingPos.y < bounds_.getLLF().y + padding_)
        drawingPos.y = bounds_.getLLF().y + padding_;

    sm.boundingBox_ = tgt::Bounds(drawingPos - enlargement, drawingPos + enlargement + tgt::vec3(textWidth, textHeight, 0));
    sm.position_ = drawingPos;
}

void SmartLabelGroupBase::renderLabel(const SmartLabel& sm) {
    // note: the font size may not be smaller than 8
    if (font_->getSize() != sm.size_)
        font_->setSize(sm.size_);

    glColor4f(sm.color_.r, sm.color_.g, sm.color_.b, sm.color_.a);
    font_->render(sm.position_, sm.text_);
}

/* ------------------------------------------------------------------------- */

SmartLabelGroupVerticalMoving::SmartLabelGroupVerticalMoving(tgt::Font* font, float padding, tgt::Bounds bounds)
    : SmartLabelGroupBase(font, padding, bounds)
{}

void SmartLabelGroupVerticalMoving::performLayout() {
    if (labels_.empty())
        return;

    // first sort array by y scale
    sorterY sorter;
    std::sort(labels_.begin(), labels_.end(), sorter);

    std::vector<SmartLabel>::iterator it;
    for (it = labels_.begin(); it < labels_.end() && (it+1) < labels_.end(); ++it) {
        tgt::Bounds bottom = it->boundingBox_;
        tgt::Bounds top = (it+1)->boundingBox_;
        float diff = bottom.getURB().y - top.getLLF().y;
        if (diff > 0) {
            (it+1)->position_.y += diff;
            calculateBoundingBox(*(it+1));
        }
    }

    // some labels may have been pushed out of bounds_, we try to fix this:
    tgt::Bounds top = labels_.rbegin()->boundingBox_;
    float diff = top.getURB().y - bounds_.getURB().y;
    if (diff > 0) {
        labels_.rbegin()->position_.y -= diff;
        calculateBoundingBox(*(labels_.rbegin()));
        for (std::vector<SmartLabel>::reverse_iterator rit = labels_.rbegin(); rit < labels_.rend() && (rit+1) < labels_.rend(); ++rit) {
            tgt::Bounds top = rit->boundingBox_;
            tgt::Bounds bottom = (rit+1)->boundingBox_;
            diff = bottom.getURB().y - top.getLLF().y;
            if (diff <= 0) {
                break;
            }
            else {
                (rit+1)->position_.y -= diff;
                calculateBoundingBox(*(rit+1));
            }
        }
    }
}

/* ------------------------------------------------------------------------- */

SmartLabelGroupHorizontalMoving::SmartLabelGroupHorizontalMoving(tgt::Font* font, float padding, tgt::Bounds bounds)
: SmartLabelGroupBase(font, padding, bounds)
{}

void SmartLabelGroupHorizontalMoving::performLayout() {
    if (labels_.empty())
        return;

    // first sort array by x scale
    sorterX sorter;
    std::sort(labels_.begin(), labels_.end(), sorter);

    std::vector<SmartLabel>::iterator it;

    // first get maximum count of intersecting labels
    int intersectionLevel = 1;
    for (it = labels_.begin(); it < labels_.end(); ++it) {
        tgt::Bounds current = it->boundingBox_;

        // check with how many of the following labels current label intersects
        int i = 1;
        while ((it+1) < labels_.end() && current.intersects((it+1)->boundingBox_)) {
            ++i;
            ++it;
        }
        intersectionLevel = std::max(intersectionLevel, i);
    }

    if (intersectionLevel > 1) {
        float step = (bounds_.getURB().y - bounds_.getLLF().y - (2*padding_))/intersectionLevel;
        float start = bounds_.getLLF().y + padding_;

        float* positions = new float[intersectionLevel];
        for (int i=0; i<intersectionLevel; ++i)
            positions[i] = start + step*(intersectionLevel-i-1);

        int count = 0;
        // now positon the labels in intersectionLevel different steps
        for (it = labels_.begin(); it < labels_.end(); ++it) {
            it->position_.y = positions[count];
            //calculateBoundingBox(*it);
            count = (count+1) % intersectionLevel;
        }

        delete[] positions;
    }
}

/* ------------------------------------------------------------------------- */

SmartLabelGroupNoLayout::SmartLabelGroupNoLayout(tgt::Font* font, float padding)
    : SmartLabelGroupBase(font,
                          padding,
                          tgt::Bounds(tgt::Vector(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()),
                                      tgt::Vector( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max(),  std::numeric_limits<float>::max())))
{}

void SmartLabelGroupNoLayout::performLayout() {
    /* its a feature, not a bug: nothing happens here - really */
}

/* ------------------------------------------------------------------------- */

SmartLabelGroupNoLayoutWithBackground::SmartLabelGroupNoLayoutWithBackground(tgt::Font* font, float padding, tgt::Bounds bounds, tgt::Color backgroundColor)
    : SmartLabelGroupBase(font, padding, bounds)
    , backgroundColor_(backgroundColor)
{}

void SmartLabelGroupNoLayoutWithBackground::performLayout() {
    /* its a feature, not a bug: nothing happens here - really */
}

void SmartLabelGroupNoLayoutWithBackground::render() {
    for (std::vector<SmartLabel>::const_iterator it = labels_.begin(); it < labels_.end(); ++it) {
        // first render background bounding box
        glColor4fv(backgroundColor_.elem);
        float bbWidth = it->boundingBox_.getURB().x - it->boundingBox_.getLLF().x;
        glBegin(GL_POLYGON);
            glVertex2fv(it->boundingBox_.getLLF().elem);
            glVertex2f(it->boundingBox_.getLLF().x + bbWidth, it->boundingBox_.getLLF().y);
            glVertex2fv(it->boundingBox_.getURB().elem);
            glVertex2f(it->boundingBox_.getURB().x - bbWidth, it->boundingBox_.getURB().y);
        glEnd();

        // then render label
        renderLabel(*it);
    }
}

} // namespace

