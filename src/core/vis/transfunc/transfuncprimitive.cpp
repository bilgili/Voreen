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

#include "voreen/core/vis/transfunc/transfuncprimitive.h"

#include "tinyxml/tinyxml.h"

#include "tgt/glmath.h"

namespace voreen {

TransFuncPrimitive::TransFuncPrimitive()
    : color_(255, 255, 0, 255)
    , selected_(false)
    , fuzziness_(1.f)
    , cpSize_(0.02f)
    , grabbed_(-1)
{
}

TransFuncPrimitive::TransFuncPrimitive(tgt::col4 color)
    : color_(color)
    , selected_(false)
    , fuzziness_(1.f)
    , cpSize_(0.02f)
    , grabbed_(-1)
{
}

TransFuncPrimitive::~TransFuncPrimitive() {
}

void TransFuncPrimitive::setColor(const tgt::col4& c) {
    color_ = c;
}

tgt::col4 TransFuncPrimitive::getColor() const {
    return color_;
}

void TransFuncPrimitive::setFuzziness(float f) {
    fuzziness_ = f;
}

float TransFuncPrimitive::getFuzziness() const {
    return fuzziness_;
}

float TransFuncPrimitive::getControlPointSize() const {
    return cpSize_;
}

void TransFuncPrimitive::select(tgt::vec2 pos) {
    selected_ = true;
    selectControlPoint(pos);
}

void TransFuncPrimitive::deselect() {
    selected_ = false;
    grabbed_ = -1;
}

void TransFuncPrimitive::paintControlPoint(const tgt::vec2& v) {
    glTranslatef(v.x, v.y, 0.1f);
    tgt::vec2 t;

    glBegin(GL_POLYGON);
    glColor4ub(150, 150, 150, 255);
    for (int i = 0; i < 20; ++i) {
        t.x = cosf((i / 20.f) * 2.f * tgt::PIf) * cpSize_;
        t.y = sinf((i / 20.f) * 2.f * tgt::PIf) * cpSize_;
        tgt::vertex(t);
    }
    glEnd();

    glTranslatef(0.f, 0.f, 0.1f);

    glBegin(GL_LINE_LOOP);
    if (selected_)
        glColor4ub(255, 255, 255, 255);
    else
        glColor4ub(128, 128, 128, 255);
    for (int i = 0; i < 20; ++i) {
        t.x = cosf((i / 20.f) * 2.f * tgt::PIf) * cpSize_;
        t.y = sinf((i / 20.f) * 2.f * tgt::PIf) * cpSize_;
        tgt::vertex(t);
    }
    glEnd();

    glTranslatef(-v.x, -v.y, -0.2f);
}

void TransFuncPrimitive::saveToXml(TiXmlElement* root) {
    root->SetDoubleAttribute("fuzzy", fuzziness_);

    TiXmlElement* e = new TiXmlElement("col4");
    e->SetAttribute("r", color_.r);
    e->SetAttribute("g", color_.g);
    e->SetAttribute("b", color_.b);
    e->SetAttribute("a", color_.a);
    root->LinkEndChild(e);
}

void TransFuncPrimitive::updateFromXml(TiXmlElement* root) {
    // read fuzziness
    double tempD;
    if (root->Attribute("fuzzy", &tempD))
        fuzziness_ = static_cast<float>(tempD);

    // read color
    TiXmlElement* pElem = root->FirstChild("col4")->ToElement();
    if (pElem) {
        int tempI;
        if (pElem->Attribute("r", &tempI))
            color_.r = tempI;
        if (pElem->Attribute("g", &tempI))
            color_.g = tempI;
        if (pElem->Attribute("b", &tempI))
            color_.b = tempI;
        if (pElem->Attribute("a", &tempI))
            color_.a = tempI;
    }
}

//-----------------------------------------------------------------------------

TransFuncQuad::TransFuncQuad()
    : TransFuncPrimitive()
{
}

TransFuncQuad::TransFuncQuad(tgt::vec2 center, float size, tgt::col4 col)
    : TransFuncPrimitive(col)
{
    size *= 0.5f;
    coords_[0] = center + tgt::vec2(-size, -size);
    coords_[1] = center + tgt::vec2( size, -size);
    coords_[2] = center + tgt::vec2( size,  size);
    coords_[3] = center + tgt::vec2(-size,  size);
}

TransFuncQuad::~TransFuncQuad() {
}

void TransFuncQuad::paint(float scaleFactor) {
    tgt::vec2 coordsScaled_[4];
    coordsScaled_[0] = tgt::vec2(coords_[0].x, scaleFactor*coords_[0].y);
    coordsScaled_[1] = tgt::vec2(coords_[1].x, scaleFactor*coords_[1].y);
    coordsScaled_[2] = tgt::vec2(coords_[2].x, scaleFactor*coords_[2].y);
    coordsScaled_[3] = tgt::vec2(coords_[3].x, scaleFactor*coords_[3].y);

    tgt::vec2 center = coordsScaled_[0]+coordsScaled_[1]+coordsScaled_[2]+coordsScaled_[3];
    center /= 4.f;

    glTranslatef(0.f, 0.f, -0.5f);
    glBegin(GL_QUADS);
        glColor4ub(color_.r, color_.g, color_.b, 0);
        tgt::vertex(coordsScaled_[0]);
        tgt::vertex(coordsScaled_[1]);
        glColor4ubv(color_.elem);
        tgt::vertex(fuzziness_ * coordsScaled_[1] + (1.f - fuzziness_) * center);
        tgt::vertex(fuzziness_ * coordsScaled_[0] + (1.f - fuzziness_) * center);

        glColor4ub(color_.r, color_.g, color_.b, 0);
        tgt::vertex(coordsScaled_[1]);
        tgt::vertex(coordsScaled_[2]);
        glColor4ubv(color_.elem);
        tgt::vertex(fuzziness_ * coordsScaled_[2] + (1.f - fuzziness_) * center);
        tgt::vertex(fuzziness_ * coordsScaled_[1] + (1.f - fuzziness_) * center);

        glColor4ub(color_.r, color_.g, color_.b, 0);
        tgt::vertex(coordsScaled_[2]);
        tgt::vertex(coordsScaled_[3]);
        glColor4ubv(color_.elem);
        tgt::vertex(fuzziness_ * coordsScaled_[3] + (1.f - fuzziness_) * center);
        tgt::vertex(fuzziness_ * coordsScaled_[2] + (1.f - fuzziness_) * center);

        glColor4ub(color_.r, color_.g, color_.b, 0);
        tgt::vertex(coordsScaled_[3]);
        tgt::vertex(coordsScaled_[0]);
        glColor4ubv(color_.elem);
        tgt::vertex(fuzziness_ * coordsScaled_[0] + (1.f - fuzziness_) * center);
        tgt::vertex(fuzziness_ * coordsScaled_[3] + (1.f - fuzziness_) * center);

        glColor4ubv(color_.elem);
        tgt::vertex(fuzziness_ * coordsScaled_[0] + (1.f - fuzziness_) * center);
        tgt::vertex(fuzziness_ * coordsScaled_[1] + (1.f - fuzziness_) * center);
        tgt::vertex(fuzziness_ * coordsScaled_[2] + (1.f - fuzziness_) * center);
        tgt::vertex(fuzziness_ * coordsScaled_[3] + (1.f - fuzziness_) * center);
    glEnd();
    glTranslatef(0.f, 0.f, 0.5f);
}

void TransFuncQuad::paintForSelection(GLubyte id) {
    glBegin(GL_QUADS);
        glColor3ub(id, 123, 123);
        tgt::vertex(coords_[0]);
        tgt::vertex(coords_[1]);
        tgt::vertex(coords_[2]);
        tgt::vertex(coords_[3]);
    glEnd();
}

void TransFuncQuad::paintInEditor() {
    paint();

    glBegin(GL_LINE_LOOP);
        if (selected_)
            glColor4ub(255, 255, 255, 255);
        else
            glColor4ub(128, 128, 128, 255);
        tgt::vertex(coords_[0]);
        tgt::vertex(coords_[1]);
        tgt::vertex(coords_[2]);
        tgt::vertex(coords_[3]);
    glEnd();

    paintControlPoint(coords_[0]);
    paintControlPoint(coords_[1]);
    paintControlPoint(coords_[2]);
    paintControlPoint(coords_[3]);

}

float TransFuncQuad::getClosestControlPointDist(tgt::vec2 pos) {
    float min = distance(pos, coords_[0]);
    float d;
    for (int i = 1; i < 4; ++i) {
        d = distance(pos, coords_[i]);
        if (d < min)
            min = d;
    }
    return min;
}

bool TransFuncQuad::selectControlPoint(tgt::vec2 pos) {
    grabbed_ = -1;
    int n = 0;
    float min = distance(pos, coords_[0]);
    float d;
    for (int i = 1; i < 4; ++i) {
        d = distance(pos, coords_[i]);
        if (d < min) {
            min = d;
            n = i;
        }
    }
    if (min < cpSize_) {
        grabbed_ = n;
        return true;
    }
    return false;
}

void TransFuncQuad::saveToXml(TiXmlElement* root) {
    TiXmlElement* elem = new TiXmlElement("quad");

    TransFuncPrimitive::saveToXml(elem);

    // save quad coordinates
    for (int i = 0; i < 4; ++i) {
        TiXmlElement* e = new TiXmlElement("vec2");
        e->SetDoubleAttribute("x", coords_[i].x);
        e->SetDoubleAttribute("y", coords_[i].y);
        elem->LinkEndChild(e);
    }

    root->LinkEndChild(elem);
}

void TransFuncQuad::updateFromXml(TiXmlElement* root) {
    TransFuncPrimitive::updateFromXml(root);

    // read coordinates
    TiXmlElement* pElem;
    pElem = root->FirstChild("vec2")->ToElement();
    int i = 0;
    for (; pElem && i < 4; pElem = pElem->NextSiblingElement("vec2")) {
        double temp;
        if (pElem->Attribute("x", &temp))
            coords_[i].x = static_cast<float>(temp);
        if (pElem->Attribute("y", &temp))
            coords_[i].y = static_cast<float>(temp);
        i++;
    }
}

bool TransFuncQuad::move(tgt::vec2 offset) {
    // only move the control point when one is grabbed
    if (grabbed_ > -1) {
        tgt::vec2 temp = coords_[grabbed_] + offset;
        // don't move control point when it is outside of allowed region
        if ((temp.x < 0.f) || (temp.x > 1.f) ||
            (temp.y < 0.f) || (temp.y > 1.f))
        {
            return false;
        }
        else
            coords_[grabbed_] += offset;
    }
    else {
        for (int i = 0; i < 4; ++i) {
            tgt::vec2 temp = coords_[i] + offset;
            //don't move primitive when one point is outside of allowed region
            if ((temp.x < 0.f) || (temp.x > 1.f) ||
                (temp.y < 0.f) || (temp.y > 1.f))
            {
                return false;
            }
        }
        for (int i = 0; i < 4; ++i)
            coords_[i] += offset;
    }

    return true;
}

//-----------------------------------------------------------------------------

TransFuncBanana::TransFuncBanana()
    : TransFuncPrimitive()
    , steps_(20)
{
}

TransFuncBanana::TransFuncBanana(tgt::vec2 a, tgt::vec2 b1, tgt::vec2 b2, tgt::vec2 c, tgt::col4 col)
    : TransFuncPrimitive(col)
    , steps_(20)
{
    coords_[0] = a;
    coords_[1] = b1;
    coords_[2] = b2;
    coords_[3] = c;
}

TransFuncBanana::~TransFuncBanana() {
}

void TransFuncBanana::paint(float scaleFactor) {
    glTranslatef(0.f, 0.f, -0.5f);
    glColor4ubv(color_.elem);
    paintInner(scaleFactor);
    glTranslatef(0.f, 0.f, 0.5f);
}

void TransFuncBanana::paintForSelection(GLubyte id) {
    glColor3ub(id, 123, 123);
    float t;
    tgt::vec2 v1, v2, t1, t2, t3, t4, tc;
    t1 = (2.f * coords_[1]) - (0.5f * coords_[0]) - (0.5f * coords_[3]);
    t2 = (2.f * coords_[2]) - (0.5f * coords_[0]) - (0.5f * coords_[3]);

    //fill the space between the two bezier curves:
    glBegin(GL_TRIANGLE_STRIP);
    tgt::vertex(coords_[0]);
    for (int i = 0; i < steps_; ++i) {
        t = i / static_cast<float>(steps_ - 1);
        v1 = (((1 - t) * (1 - t)) * coords_[0]) + ((2 * (1 - t) * t) * t1) + ((t * t) * coords_[3]);
        v2 = (((1 - t) * (1 - t)) * coords_[0]) + ((2 * (1 - t) * t) * t2) + ((t * t) * coords_[3]);
        tgt::vertex(v1);
        tgt::vertex(v2);
    }
    tgt::vertex(coords_[3]);
    glEnd();
}

void TransFuncBanana::paintInner(float scaleFactor) {
    float t;
    tgt::vec2 v1, v2, t1, t2, t3, t4, tc;

    tgt::vec2 coordsScaled[4];
    coordsScaled[0] = tgt::vec2(coords_[0].x, scaleFactor*coords_[0].y);
    coordsScaled[1] = tgt::vec2(coords_[1].x, scaleFactor*coords_[1].y);
    coordsScaled[2] = tgt::vec2(coords_[2].x, scaleFactor*coords_[2].y);
    coordsScaled[3] = tgt::vec2(coords_[3].x, scaleFactor*coords_[3].y);

    t1 = (2.f * coordsScaled[1]) - (0.5f * coordsScaled[0]) - (0.5f * coordsScaled[3]);
    t2 = (2.f * coordsScaled[2]) - (0.5f * coordsScaled[0]) - (0.5f * coordsScaled[3]);

    tc = (t1 + t2) / 2.f;
    t3 = fuzziness_ * t1 + (1.f - fuzziness_) * tc;
    t4 = fuzziness_ * t2 + (1.f - fuzziness_) * tc;
    //fill the space between the two bezier curves:
    glBegin(GL_TRIANGLE_STRIP);
    glColor4ubv(color_.elem);
    tgt::vertex(coordsScaled[0]);
    for (int i = 0; i < steps_; ++i) {
        t = i / static_cast<float>(steps_ - 1);
        v1 = (((1 - t) * (1 - t)) * coordsScaled[0]) + ((2 * (1 - t) * t) * t1) + ((t * t) * coordsScaled[3]);
        v2 = (((1 - t) * (1 - t)) * coordsScaled[0]) + ((2 * (1 - t) * t) * t3) + ((t * t) * coordsScaled[3]);
        glColor4ub(color_.r, color_.g, color_.b, 0);
        tgt::vertex(v1);
        glColor4ubv(color_.elem);
        tgt::vertex(v2);
    }
    tgt::vertex(coordsScaled[3]);

    glColor4ubv(color_.elem);
    tgt::vertex(coordsScaled[0]);
    for (int i = 0; i < steps_; ++i) {
        t = i / static_cast<float>(steps_ - 1);
        v1 = (((1 - t) * (1 - t)) * coordsScaled[0]) + ((2 * (1 - t) * t) * t3) + ((t * t) * coordsScaled[3]);
        v2 = (((1 - t) * (1 - t)) * coordsScaled[0]) + ((2 * (1 - t) * t) * t4) + ((t * t) * coordsScaled[3]);
        tgt::vertex(v1);
        tgt::vertex(v2);
    }
    tgt::vertex(coordsScaled[3]);

    tgt::vertex(coordsScaled[0]);
    for (int i = 0; i < steps_; ++i) {
        t = i / static_cast<float>(steps_ - 1);
        v1 = (((1 - t) * (1 - t)) * coordsScaled[0]) + ((2 * (1 - t) * t) * t4) + ((t * t) * coordsScaled[3]);
        v2 = (((1 - t) * (1 - t)) * coordsScaled[0]) + ((2 * (1 - t) * t) * t2) + ((t * t) * coordsScaled[3]);
        glColor4ubv(color_.elem);
        tgt::vertex(v1);
        glColor4ub(color_.r, color_.g, color_.b, 0);
        tgt::vertex(v2);
    }
    tgt::vertex(coordsScaled[3]);

    glEnd();
}

void TransFuncBanana::paintInEditor() {
    paint();

    float t;
    tgt::vec2 v, t1, t2;
    t1 = (2.f * coords_[1]) - (0.5f * coords_[0]) - (0.5f * coords_[3]);
    t2 = (2.f * coords_[2]) - (0.5f * coords_[0]) - (0.5f * coords_[3]);

    //draw outer line of double bezier curve:
    glBegin(GL_LINE_LOOP);
        if (selected_)
            glColor4ub(255, 255, 255, 255);
        else
            glColor4ub(128, 128, 128, 255);
        for (int i = 0; i < steps_; ++i) {
            t = i / static_cast<float>(steps_ - 1);
            v = (((1 - t) * (1 - t)) * coords_[0]) + ((2 * (1 - t) * t) * t1) + ((t * t) * coords_[3]);
            tgt::vertex(v);
        }

        for (int i = 0; i < steps_; ++i) {
            t = 1.f - (i / static_cast<float>(steps_ - 1));
            v = (((1 - t) * (1 - t)) * coords_[0]) + ((2 * (1 - t) * t) * t2) + ((t * t) * coords_[3]);
            tgt::vertex(v);
        }
    glEnd();

    paintControlPoint(coords_[0]);
    paintControlPoint(coords_[1]);
    paintControlPoint(coords_[2]);
    paintControlPoint(coords_[3]);
}

float TransFuncBanana::getClosestControlPointDist(tgt::vec2 pos) {
    float min = distance(pos, coords_[0]);
    float d;
    for (int i = 1; i < 4; ++i) {
        d = distance(pos, coords_[i]);
        if (d < min)
            min = d;
    }
    return min;
}

bool TransFuncBanana::selectControlPoint(tgt::vec2 pos) {
    grabbed_ = -1;
    int n = 0;
    float min = distance(pos, coords_[0]);
    float d;
    for (int i = 0; i < 4; ++i) {
        d = distance(pos, coords_[i]);
        if (d < min) {
            min = d;
            n = i;
        }
    }
    if (min < cpSize_) {
        grabbed_ = n;
        return true;
    }
    return false;
}

void TransFuncBanana::saveToXml(TiXmlElement* root) {
    TiXmlElement* elem = new TiXmlElement("banana");

    TransFuncPrimitive::saveToXml(elem);

    // save banana coordinates
    for (int i = 0; i < 4; ++i) {
        TiXmlElement* e = new TiXmlElement("vec2");
        e->SetDoubleAttribute("x", coords_[i].x);
        e->SetDoubleAttribute("y", coords_[i].y);
        elem->LinkEndChild(e);
    }

    root->LinkEndChild(elem);
}

void TransFuncBanana::updateFromXml(TiXmlElement* root) {
    TransFuncPrimitive::updateFromXml(root);

    TiXmlElement* pElem = root->FirstChild("vec2")->ToElement();
    int i = 0;
    for ( ; pElem && i < 4; pElem = pElem->NextSiblingElement("vec2")) {
        double temp;
        if (pElem->Attribute("x", &temp))
            coords_[i].x = static_cast<float>(temp);
        if (pElem->Attribute("y", &temp))
            coords_[i].y = static_cast<float>(temp);
        i++;
    }
}

bool TransFuncBanana::move(tgt::vec2 offset) {
    if (grabbed_ > -1) {
        tgt::vec2 temp = coords_[grabbed_] + offset;
        // don't move control point when it is outside of allowed region
        if ((temp.x < 0.f) || (temp.x > 1.f) ||
            (temp.y < 0.f) || (temp.y > 1.f))
        {
            return false;
        }
        else
            coords_[grabbed_] += offset;
    }
    else {
        for (int i = 0; i < 4; ++i) {
            tgt::vec2 temp = coords_[i] + offset;
            //don't move primitive when one point is outside of allowed region
            if ((temp.x < 0.f) || (temp.x > 1.f) ||
                (temp.y < 0.f) || (temp.y > 1.f))
            {
                return false;
            }
        }
        for (int i = 0; i < 4; ++i)
            coords_[i] += offset;
    }

    return true;
}

} // namespace voreen
