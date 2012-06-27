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

#ifndef VRN_CLIPPINGPLANEWIDGET_H
#define VRN_CLIPPINGPLANEWIDGET_H

#include "voreen/core/vis/processors/image/geometryprocessor.h"

#include <vector>

namespace voreen {

/**
 * 3D-Widget to manipulate the clipping planes.
 */
class ClippingPlaneWidget : public GeometryRenderer, public tgt::EventListener {
public:
    ClippingPlaneWidget();
    virtual ~ClippingPlaneWidget();

    /**
    * Process voreen message, accepted identifiers:
    * - resetClipPlanes
    */
    virtual void processMessage(Message* msg, const Identifier& dest = Message::all_);

    virtual const Identifier getClassName() const;
    virtual const std::string getProcessorInfo() const;
    virtual Processor* create() const;

    virtual void mousePressEvent(tgt::MouseEvent *e);
    virtual void mouseMoveEvent(tgt::MouseEvent *e);
    virtual void mouseReleaseEvent(tgt::MouseEvent *e);

protected:
    virtual void render(LocalPortMapping* portMapping);

private:
    ///Returns the id of the control element that was clicked, or 0 if none
    int isClicked(int x, int y);

    ///Paints an arrow that denotes a control element
    void paintArrow(tgt::vec3 translation_first, tgt::vec3 translation_second,
                     tgt::vec3 rotationAxis, float rotationAngle);

    ///Initializes light and material parameter
    void initLightAndMaterial();

    ///Renders transparent quads that show the position of the clipping planes
    void renderQuad(int number);

    ///Calculates the value of one clippingplane depending on given parameter
    void calculateClippingValues(GLdouble pos, float& clip1, float& clip2,
                                 bool lock, bool switched);

    ///calculates clipping values by calling \s calculateClippingValues()
    ///and sends appropriate messages to messagedistributor
    void updateClippingValues(GLdouble clipPoint[3]);

    int isClicked_;            ///< id of clicked control element
    GLuint arrowDisplayList_;  ///< displaylist for rendering of an arrow
    bool shift_lock_;          ///< was shift pressed to lock clipping planes?

    tgt::vec3 volumeSize_;     ///< size of the volume in each direction
    bool volumeSizeValid_;     ///< is the size of volume valid?

    ColorProp sliceColor_;     ///< property for choosing the color of highlighted planes
    FloatProp polyOffsetFact_; ///< property for polygonoffsetfactor
    FloatProp polyOffsetUnit_; ///< property for polygonoffsetunit
    BoolProp x_lock_;          ///< property for locking left and right clipping plane
    BoolProp y_lock_;          ///< property for locking bottom and top clipping plane
    BoolProp z_lock_;          ///< property for locking front and back clipping plane

    struct ControlElement {
        ControlElement(tgt::vec3 clipValue, std::string name, bool activated,
                       tgt::vec3 translation, tgt::vec3 rotationAxis, float rotationAngle,
                       tgt::vec3 adjacentEdge1, tgt::vec3 adjacentEdge2, tgt::vec3 color);

        tgt::vec3 clipValue;
        std::string name;
        bool activated;
        tgt::vec3 translation;
        tgt::vec3 rotationAxis;
        float rotationAngle;
        tgt::vec3 adjacentEdge1;
        tgt::vec3 adjacentEdge2;
        tgt::vec3 color;
    };

    std::vector<ControlElement> controlElements_; ///< vector with all controlelements

    float clipValueLeft_;   ///< value of left clipping plane
    float clipValueRight_;  ///< value of right clipping plane
    float clipValueBottom_; ///< value of bottom clipping plane
    float clipValueTop_;    ///< value of top clipping plane
    float clipValueFront_;  ///< value of front clipping plane
    float clipValueBack_;   ///< value of back clipping plane

    //Light Parameter
    GLfloat light_pos[4];      ///< position of the light source
    GLfloat light_ambient[4];  ///< ambient light parameter
    GLfloat light_diffuse[4];  ///< diffuse light parameter
    GLfloat light_specular[4]; ///< specular light parameter

    struct Material {
        GLfloat ambient[4];
        GLfloat diffuse[4];
        GLfloat specular[4];
        GLfloat shininess;
    };

    Material materials_[3]; ///< array with 3 materials

};

} // namespace voreen

#endif // VRN_CLIPPINGPLANEWIDGET_H
