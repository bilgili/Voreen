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

#ifndef VRN_TRANSFUNCPRIMITIVE_H
#define VRN_TRANSFUNCPRIMITIVE_H

class TiXmlElement;

#include "tgt/vector.h"
#include "tgt/tgt_gl.h"

namespace voreen {

/**
 * Abstract base class for all primitives that are used in 2D transfer functions.
 */
class TransFuncPrimitive {
public:
    /**
     * Standard constructor
     */
    TransFuncPrimitive();

    /**
     * Constructor
     *
     * @param col color of the primitive
     * @param scalefactor scaling of the y coordinate of the primitive
     */
    TransFuncPrimitive(tgt::col4 col, float scaleFactor);

    /**
     * Destructor
     */
    virtual ~TransFuncPrimitive();

    /**
     * Sets the scaleFactor of the primitive to the given value. The y coordinates
     * of the primitive are adjusted according to the scaleFactor.
     *
     * @param scaleFactor scaleFactor for the y coordinates of the primitve
     */
    virtual void setScaleFactor(float scaleFactor) = 0;

    /**
     * Paints the primitive.
     */
    virtual void paint() = 0;

    /**
     * Paints the primitive for display in an editor. An outline and control points are added.
     */
    virtual void paintInEditor() = 0;

    /**
     * Paints the primitive to decide whether it was selected or not.
     * Therefor the color that is used in painting is set to [id, 123, 123].
     *
     * @param id id of the primitive
     */
    virtual void paintForSelection(GLubyte id) = 0;

    /**
     * Paints a control point at the given position.
     *
     * @param v position of the control point
     */
    virtual void paintControlPoint(const tgt::vec2& v);

    /**
     * Sets the color of the primitive to the given value.
     *
     * @param c desired color for the primitive
     */
    void setColor(const tgt::col4& c);

    /**
     * Returns the current color of the primitive.
     *
     * @return color of the primitive
     */
    tgt::col4 getColor() const;

    /**
     * Sets the fuzziness of the primitive to the given value. With increasing fuzziness the
     * primitives get an alphagradient towards their border.
     *
     * @param f desired fuzziness
     */
    void setFuzziness(float f);

    /**
     * Returns the current fuzziness of the primitive.
     *
     * @return current fuzziness
     */
    float getFuzziness() const;

    /**
     * Returns the size of a control point.
     *
     * @return size of a control point
     */
    float getControlPointSize() const;

    /**
     * Returns the distance between pos and the closest control point.
     *
     * @param pos position the distance of control points is measured to
     * @param distance between pos and closest control point
     */
    virtual float getClosestControlPointDist(tgt::vec2 pos) = 0;

    /**
     * Returns true if a control point is under the mouse cursor and grabs this control point.
     * False is returned when there is no control point under the mouse cursor.
     *
     * @param pos position of the mouse cursor
     * @return true if a contorl point is grabbed, false otherwise
     */
    virtual bool selectControlPoint(tgt::vec2 pos) = 0;

    /**
     * Moves the primitive by the given offset. If a control point is grabbed only the control
     * point is moved.
     *
     * @param offset offset the primitive is moved by
     * @return true when moved primitive lies completely inside the editor widget, false otherwise
     */
    virtual bool move(tgt::vec2 offset) = 0;

    /**
     * Sets the selection state of this primitive to true. Furthermore is tested whether the
     * mouse cursor is over a control point.
     *
     * @param pos position of the mouse cursor
     */
    virtual void select(tgt::vec2 pos);

    /**
     * Sets the selection state of the primitive to false.
     */
    virtual void deselect();

    /**
     * Saves the primitive in the given Xml element.
     *
     * @param root Xml element the values are saved in
     */
    virtual void saveToXml(TiXmlElement* root) = 0;

    /**
     * Updates the primitive from the values in the xml tree.
     *
     * @param root the xml element with the settings
     */
    virtual void updateFromXml(TiXmlElement* root) = 0;

protected:
    tgt::col4 color_;   ///< color of the primitive
    bool selected_;     ///< indicates whether the primitive is selected or not
    float fuzziness_;   ///< fuzziness of the primitive
    float cpSize_;      ///< size of a control point
    int grabbed_;       ///< number of the grabbed control point. -1 when no control point is grabbed
    float scaleFactor_; ///< scaling of the primitive coordinates in y direction
};

// ----------------------------------------------------------------------------

/**
 * A quad primitive. It consists of 4 vertices that can be moved independently.
 */
class TransFuncQuad : public TransFuncPrimitive {
public:
    /**
     * Constructor
     */
    TransFuncQuad();

    /**
     * Constructor
     *
     * @param center center of the quad
     * @param size size of the quad
     * @param col color of the quad
     * @param scaleFactor scaling of the y coordinate of the primitive
     */
    TransFuncQuad(tgt::vec2 center, float size, tgt::col4 col, float scaleFactor);

    /**
     * Destructor
     */
    ~TransFuncQuad();

    /**
     * Sets the scaleFactor of the primitive to the given value. The y coordinates
     * of the primitive are adjusted according to the scaleFactor.
     *
     * @param scaleFactor scaleFactor for the y coordinates of the primitve
     */
    void setScaleFactor(float scaleFactor);

    /**
     * Paints the quad. The fuzziness factor is obeyed.
     */
    void paint();

    /**
     * Paints the quad for selection purposes. Therefor the color used in painting is set
     * to [id, 123, 123].
     *
     * @param id id that is used in the first component of the color
     */
    void paintForSelection(GLubyte id);

    /**
     * Paints the quad for display in an editor. An outline and control points are added.
     */
    void paintInEditor();

    /**
     * Returns the distance between pos and the closest control point.
     *
     * @param pos position the distance of control points is measured to
     * @param distance between pos and closest control point
     */
    float getClosestControlPointDist(tgt::vec2 pos);

    /**
     * Grabs a control point when the mouse cursor is over one. Returns true when a control
     * point is grabbed and false otherwise.
     *
     * @param pos position of the mouse cursor
     * @return true when mouse cursor is over a control point, false otherwise
     */
    bool selectControlPoint(tgt::vec2 pos);

    /**
     * Moves the 4 vertices by the given offset. Returns true when the moved vertices
     * are lying in the range [0,1] and false otherwise.
     *
     * @param offset offset the coordinates are moved by
     * @return true when moved coordinates are lying in the interval [0,1] and false otherwise
     */
    bool move(tgt::vec2 offset);

    /**
     * Saves the coordinates of the quad in the given Xml element.
     *
     * @param root Xml element the coordinates are saved in
     */
    void saveToXml(TiXmlElement* root);

    /**
     * Updates the coordinates from the values in the xml tree.
     *
     * @param root the xml element with the coordinates
     */
    void updateFromXml(TiXmlElement* root);

protected:
    tgt::vec2 coords_[4]; ///< the coordinates of the 4 vertices
    bool scaleCoords_;    ///< indicates whether the coordinates must be scaled in paint() or not
};

// ----------------------------------------------------------------------------

/**
 * A banana primitive. It consists of 4 vertices that are connected by 2 splines.
 * The control points are arranged in the following way.
 *       1
 *
 *       2
 * 0          3
 * The first spline starts at point 0, goes through 1 and ends in 3.
 * The second spline starts at point 0, goes through 2 and ends in 3.
 */
class TransFuncBanana : public TransFuncPrimitive {
public:
    /**
     * Constructor
     */
    TransFuncBanana();

    /**
     * Constructor
     *
     * @param a  coordinate of the left control point
     * @param b1 coordinate of the upper middle control point
     * @param b2 coordinate of the lower middle control point
     * @param c  coordinate of the right control point
     * @param col color of the primitive
     * @param scaleFactor scaling of the y coordinate of the primitive
     */
    TransFuncBanana(tgt::vec2 a, tgt::vec2 b1, tgt::vec2 b2, tgt::vec2 c, tgt::col4 col, float scaleFactor);

    /**
     * Destructor
     */
    ~TransFuncBanana();

    /**
     * Sets the scaleFactor of the primitive to the given value. The y coordinates
     * of the primitive are adjusted according to the scaleFactor.
     *
     * @param scaleFactor scaleFactor for the y coordinates of the primitve
     */
    void setScaleFactor(float scaleFactor);

    /**
     * Paints the banana. The fuzziness is obeyed.
     */
    void paint();

    /**
     * Paints the banana for selection purposes. Therefor the color used in painting is set
     * to [id, 123, 123].
     *
     * @param id id that is used in the first component of the color
     */
    void paintForSelection(GLubyte id);

    /**
     * Paints the banana for display in an editor. An outline and control points are added.
     */
    void paintInEditor();

    /**
     * Returns the distance between pos and the closest control point.
     *
     * @param pos position the distance of control points is measured to
     * @param distance between pos and closest control point
     */
    float getClosestControlPointDist(tgt::vec2 pos);

    /**
     * Grabs a control point when the mouse cursor is over one. Returns true when a control
     * point is grabbed and false otherwise.
     *
     * @param pos position of the mouse cursor
     * @return true when mouse cursor is over a control point, false otherwise
     */
    bool selectControlPoint(tgt::vec2 m);

    /**
     * Moves the 4 vertices by the given offset. Returns true when the moved vertices
     * are lying in the range [0,1] and false otherwise.
     *
     * @param offset offset the coordinates are moved by
     * @return true when moved coordinates are lying in the interval [0,1] and false otherwise
     */
    bool move(tgt::vec2 offset);

    /**
     * Saves the coordinates of the 4 vertices in the given Xml element.
     *
     * @param root Xml element the coordinates are saved in
     */
    void saveToXml(TiXmlElement* root);

    /**
     * Updates the coordinates from the values in the xml tree.
     *
     * @param root the xml element with the coordinates
     */
    void updateFromXml(TiXmlElement* root);

protected:
    /**
     * Paints the space between the both splines. steps_ triangles in a
     * trianglestrip are used for that.
     */
    void paintInner();


    tgt::vec2 coords_[4]; ///< coordinates of the 4 vertices

    int steps_; ///< number of triangles used to fill the space between the both splines

    bool scaleCoords_; ///< indicates whether the coordinates must be scaled in paintInner() or not
};

} // namespace voreen

#endif // VRN_TRANSFUNCPRIMITIVE_H
