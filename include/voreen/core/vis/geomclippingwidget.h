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

#ifndef VRN_GEOMCLIPPINGWIDGET_H
#define VRN_GEOMCLIPPINGWIDGET_H

#include "voreen/core/vis/processors/image/geometryprocessor.h"

namespace voreen {

/**
 * 3D-Widget to manipulate the clipping planes.
 */
class ClippingWidget : public GeometryRenderer, public tgt::EventListener {
public:
    ClippingWidget(tgt::Camera* camera=0, TextureContainer* tc=0);
	~ClippingWidget();
    
    /**
    * Process voreen message, accepted identifiers:
    * - set.ClipSliceColor \c tgt::vec4
    * - set.polyOffsetFact \c float
    * - set.polyOffsetUnit \c float
    * - switch.ClipXLock \c bool
    * - switch.ClipYLock \c bool
    * - switch.ClipZLock \c bool
    * - resetClipPlanes
    */
    void processMessage(Message* msg, const Identifier& dest = Message::all_);

	virtual const Identifier getClassName() const {return "GeometryRenderer.ClippingWidget";}
	virtual const std::string getProcessorInfo() const;
    virtual Processor* create();

    virtual void mousePressEvent(tgt::MouseEvent *e);
    virtual void mouseMoveEvent(tgt::MouseEvent *e);
    virtual void mouseReleaseEvent(tgt::MouseEvent *e);
    virtual void mouseDoubleClickEvent(tgt::MouseEvent* e);

protected:
    void render(LocalPortMapping*  portMapping);
private:
    ///Returns the control that is clicked, or 0 if none
    int isClicked(int x, int y);
    ///Paint the bounding box geometry
    void paintBoundingGeometry(float trans_x,float trans_y,float trans_z, int direction);

    void saveMatrixStacks();
    void restoreMatrixStacks();
    void setLightParams();
    ///Render the transparent quads that show where the clipping planes are.
    void renderQuad(int number);

    float posX, posY, posZ;

    float x_control1, x_control2, y_control1, y_control2, z_control1, z_control2; // rel positions of Clipp-Widgets
    bool bx_control1, bx_control2, by_control1, by_control2, bz_control1, bz_control2; // rel positions of Clipp-Widgets

    int isClicked_;
    int x_prev ,y_prev;

    ColorProp sliceColor_;
    FloatProp polyOffsetFact_;
    FloatProp polyOffsetUnit_;
    BoolProp x_lock_;
    BoolProp y_lock_;
    BoolProp z_lock_;

    bool shift_lock_;
    GLint viewport_[4];
    GLdouble modelview_[16];
    GLdouble projection_[16];
    tgt::vec3 boundLeft, boundTop, boundBack;

    //Light Parameter
	GLfloat light_pos[4];
    GLfloat light_pos2[4];
	GLfloat light_ambient[4];
	GLfloat light_diffuse[4];
	GLfloat light_specular[4];

	//Material green plastic
	GLfloat gr_plastic_ambient[4];
	GLfloat gr_plastic_diffuse[4];
	GLfloat gr_plastic_specular[4];
	GLfloat gr_plastic_shininess;

    //Material red plastic
	GLfloat re_plastic_ambient[4];
	GLfloat re_plastic_diffuse[4];
	GLfloat re_plastic_specular[4];
	GLfloat re_plastic_shininess;

    //Material blue plastic
    GLfloat bl_plastic_ambient[4];
	GLfloat bl_plastic_diffuse[4];
	GLfloat bl_plastic_specular[4];
	GLfloat bl_plastic_shininess;

    GLuint arrowDisplayList_;

    tgt::vec3 volumeSize_;
    bool volumeSizeValid_;
};

} // namespace voreen

#endif // VRN_CLIPPINGWIDGET_H
