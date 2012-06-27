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

#ifndef VRN_SMARTLABEL_H
#define VRN_SMARTLABEL_H

#include "tgt/logmanager.h"
#include "tgt/vector.h"
#include "tgt/font.h"

#include <string>
#include <vector>
#include <utility>

namespace voreen {

/**
 * A smart label is nothing more than a string to be rendered associated with position,
 * alignment, font size and color.
 **/
class SmartLabel {
public:
    /// enum for label alignment
    enum Alignment {
        TOPLEFT,        ///< label will be placed left-justified above given coordinates
        TOPCENTERED,    ///< label will be placed centered above given coordinates
        TOPRIGHT,       ///< label will be placed right-justified above given coordinates

        MIDDLELEFT,     ///< label will be placed left-justified on vertically centered on given coordinates
        CENTERED,       ///< label will be placed centered on given coordinates
        MIDDLERIGHT,    ///< label will be placed right-justified on vertically centered on given coordinates

        BOTTOMLEFT,     ///< label will be placed left-justified below given coordinates
        BOTTOMCENTERED, ///< label will be placed centered below given coordinates
        BOTTOMRIGHT     ///< label will be placed left-justified below given coordinates
    };

    SmartLabel(std::string text, tgt::vec3 pos, Alignment align, tgt::Color color, int size);

    std::string            text_;          ///< text to render
    tgt::vec3              position_;      ///< position where to render
    Alignment align_;                      ///< alignment of text around position
    tgt::Color             color_;         ///< font color
    int                    size_;          ///< font size

    tgt::Bounds            boundingBox_;   ///< bounding box of the rendered text - has to be maintained by owning object
};


/**
 * \brief   abstract base class for a group of labels which will be layouted that they won't overlap each other
 *
 * Each label to render has to be added via addLabel(). On calling performLayout() the SmartLabelGroup will try
 * to rearrange all labels so that they won't overlap each other. On calling render() all handeled labels will
 * be rendered using the current OpenGL context.
 *
 * All given positions are in viewport coordinates.
 **/
class SmartLabelGroupBase {
public:
    /**
     * \brief   initializes a SmartLabelGroup
     *
     * \param   font        pointer to tgt::Font object which will do the font rendering
     * \param   padding     minimum space between two labels
     * \param   bounds      bounding box of this SmartLabelGroup, no label shall exceed this dimensions
     **/
    SmartLabelGroupBase(tgt::Font* font, float padding, tgt::Bounds bounds);

    virtual ~SmartLabelGroupBase() {};

    /**
     * \brief   adds a label with the given settings to this SmartLabelGroup
     *
     * \param   text        text of the label
     * \param   position    primary position of the label in viewport coordinates
     * \param   color       font color
     * \param   size        font size in points
     * \param   align       text alignment
     **/
    void addLabel(std::string text, tgt::vec3 position, tgt::Color color, int size, SmartLabel::Alignment align);

    /**
     * \brief   adds a label with the given settings to this SmartLabelGroup
     *
     * \param   text        text of the label
     * \param   position    primary position of the label in viewport coordinates
     * \param   color       font color
     * \param   size        font size in points
     * \param   align       text alignment
     **/
    void addLabel(std::string text, tgt::dvec2 position, tgt::Color color, int size, SmartLabel::Alignment align);

    /**
     * \brief   Resets this SmartLabelGroup and clears all handled labels.
     **/
    void reset();

    /**
     * \brief   Sets the bounds of this SmartLabelGroup to \a bounds.
     **/
    void setBounds(tgt::Bounds bounds);

    /**
     * \brief   Performs the layouting process so that no labels overlap; has to be implemented by subclasses.
     **/
    virtual void performLayout() = 0;

    /**
     * \brief   Renders all handled labels using the current OpenGL context.
     **/
    virtual void render();

    /**
     * \brief   renders a single label with given parameters
     *
     * \param   font        font object to use for rendering
     * \param   text        text of the label
     * \param   position    primary position of the label
     * \param   color       font color
     * \param   size        font size in points
     * \param   align       text alignment
     * \param   padding     label padding
     **/
    static void renderSingleLabel(tgt::Font* font, const std::string& text, const tgt::vec3& position,
                                  const tgt::Color& color, int size, SmartLabel::Alignment align, float padding);

protected:
    /**
     * \brief   Aligns the SmartLabel \a label, calculates its bounding box and updates label.boundingBox_.
     *
     * The bounding box of the SmartLabel includes the alignment of the text as well as shifts due
     * to possible exceeding the bounds of this SmartLabelGroup.
     *
     * \param   label   SmartLabel of which to determine the bounding box
     **/
    void calculateBoundingBox(SmartLabel& label);
    /**
     * \brief   renders the SmartLabel \a label using current OpenGL context
     *
     * \param   label   SmartLabel to render
     **/
    void renderLabel(const SmartLabel& label);

    tgt::Font* font_;                       ///< font object to use for rendering
    float padding_;                         ///< spacing between labels
    tgt::Bounds bounds_;                    ///< bounding box of this label group
    std::vector<SmartLabel> labels_;        ///< container for all handled labels

    static const std::string loggerCat_;
};


/**
 * \brief   smart group of labels which will be layouted vertically so that they won't overlap each other
 *
 * On overlapping the upper label will be moved vertically until they do not intersect anymore.
 **/
class SmartLabelGroupVerticalMoving : public SmartLabelGroupBase {
public:
    SmartLabelGroupVerticalMoving(tgt::Font* font, float padding, tgt::Bounds bounds);

    virtual void performLayout();
};


/**
* \brief   smart group of labels which will be layouted horizontally that they won't overlap each other
*
* If any of the labels overlap the labels will be positioned in altering y positions (e.g. up,
* down, up, down) equally spaced out within given bounding box.
**/
class SmartLabelGroupHorizontalMoving : public SmartLabelGroupBase {
    public:
        SmartLabelGroupHorizontalMoving(tgt::Font* font, float padding, tgt::Bounds bounds);

        virtual void performLayout();
};


/**
* \brief   Group of labels which will not be layouted at all
*
* If any of the labels overlap this class will not care.
**/
class SmartLabelGroupNoLayout : public SmartLabelGroupBase {
    public:
        /**
         * \brief Constructor
         *
         * \param   font            pointer to tgt::Font object which will do the font rendering
         * \param   padding         minimum space between two labels
         **/
        SmartLabelGroupNoLayout(tgt::Font* font, float padding);

        virtual void performLayout();
};


/**
* \brief   Group of labels which will be rendered on a background.
*
* If any of the labels overlap this class will not care.
**/
class SmartLabelGroupNoLayoutWithBackground : public SmartLabelGroupBase {
    public:
        /**
         * \brief Constructor
         *
         * \param   font            pointer to tgt::Font object which will do the font rendering
         * \param   padding         minimum space between two labels
         * \param   bounds          bounding box of this SmartLabelGroup, no label shall exceed this dimensions
         * \param   backgroundColor color of the bounding box renderes in background of the labels
         **/
        SmartLabelGroupNoLayoutWithBackground(tgt::Font* font, float padding, tgt::Bounds bounds, tgt::Color backgroundColor);

        virtual void performLayout();
        virtual void render();

    private:
        tgt::Color backgroundColor_;    ///< color of the bounding box renderes in background of the labels
};

} // namespace voreen

#endif // VRN_SMARTLABEL_H
