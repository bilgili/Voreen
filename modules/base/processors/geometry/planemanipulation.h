/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2013 University of Muenster, Germany.                        *
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>        *
 * For a list of authors please refer to the file "CREDITS.txt".                   *
 *                                                                                 *
 * This file is part of the Voreen software package. Voreen is free software:      *
 * you can redistribute it and/or modify it under the terms of the GNU General     *
 * Public License version 2 as published by the Free Software Foundation.          *
 *                                                                                 *
 * Voreen is distributed in the hope that it will be useful, but WITHOUT ANY       *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR   *
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.      *
 *                                                                                 *
 * You should have received a copy of the GNU General Public License in the file   *
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>. *
 *                                                                                 *
 * For non-commercial academic use see the license exception specified in the file *
 * "LICENSE-academic.txt". To get information about commercial licensing please    *
 * contact the authors.                                                            *
 *                                                                                 *
 ***********************************************************************************/

#ifndef VRN_PLANEMANIPULATION_H
#define VRN_PLANEMANIPULATION_H

#include "voreen/core/processors/geometryrendererbase.h"
#include "voreen/core/properties/eventproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/floatproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/buttonproperty.h"

#include "voreen/core/ports/volumeport.h"
#include "voreen/core/ports/geometryport.h"

#include <vector>

namespace voreen {

/**
 * Renders 3D-control elements to manipulate an arbitrary oriented clipping plane
 * using linking to propagate property changes to @c GeometryClipping processors.
 */
class PlaneManipulation : public GeometryRendererBase {
public:
    PlaneManipulation();
    ~PlaneManipulation();
    virtual Processor* create() const;

    virtual std::string getClassName() const  { return "PlaneManipulation"; }
    virtual std::string getCategory() const   { return "Geometry";          }
    virtual CodeState getCodeState() const    { return CODE_STATE_TESTING;  }

protected:
    virtual void setDescriptions() {
        setDescription("3D control elements to manipulate an arbitrarily oriented geometry clipping plane: \
                <ul> \
                <li>Use linking to connect the properties to a GeometryClipping processor.</li>\
                <li>Use the <b>left mouse button</b> to change the plane orientation by rotating the manipulator handle through \
                grabbing one of its tips or shift the plane along its normal by grabbing the cylinder.</li>\
                <li>Use the <b>right mouse button</b> to shift the manipulator on the plane without changing the plane itself \
                (only if the plane, ie. its intersection with the input bounding box, is visible).</li>\
                </ul>");

        renderPlane_.setDescription("Selects if the geometry of the plane (ie. its intersection with the input bounding box) is rendered. \
                If disabled this also turns off the rendering of the manipulator.");

        renderManipulator_.setDescription("Selects if the manipulator handle is rendered. If disabled this also prevents any interaction with the handle.");

        invert_.setDescription("Link to GeometryClipping processor to invert the plane's orientation.");

        resetManipulatorPos_.setDescription("This resets the manipulator handle to the default position (without changing the plane), \
                which is the point on the plane nearest to the center of the input bounding box.");

        autoReset_.setDescription("If enabled the manipulator handle is reset to the its default position after each interaction with the plane, ie. \
                when the mouse button is released. <br> \
                If the plane, ie. its intersection with the input bounding box, is not visible, the handle will not be reset.");

        resetPlane_.setDescription("This resets the plane so that the plane normal is (0,0,-1) and the plane goes through the center of the input bounding box.\
                The manipulator handle is set to its default position on the plane.");

        planeNormal_.setDescription("Normal of the plane, link with GeometryClipping processor.");

        planePosition_.setDescription("Distance of the plane from the origin (in world coordinates). Link with GeometryClipping processor.");

        planeColor_.setDescription("Color of the plane and manipulator handle rendering.");
        grabbedElementColor_.setDescription("Color of the manipulator element that is currently grabbed.");

        blockedElementColor_.setDescription("Color of an element that is grabbed, but may currently not be moved (e.g. moving the manipulator outside the scene");

        grabbedPlaneOpacity_.setDescription("Opacity of the plane, which is rendered during interaction with the manipulator handle.");

        width_.setDescription("Width of the lines that are rendered for the plane visualization.");

        manipulatorScale_.setDescription("Scaling factor to adjust the size of the manipulator handle, which is computed from the bounding box of the input volume or geometry.");
    }

    virtual void initialize() throw (tgt::Exception);

    virtual bool isReady() const;

    /// @see GeometryRendererBase::render
    virtual void render();

    /// @see GeometryRendererBase::renderPicking
    virtual void renderPicking();

    /**
     * Initialize ID manager to work with this @c MeshClippingWidget instance.
     *
     * @param idm ID manager
     */
    void setIDManager(IDManager* idm);

private:

    /**
     * Accuracy for floating point operations within this class.
     */
    static const float EPS;

    /**
     * Definition of @c Anchor data structure.
     */
    typedef tgt::vec3 Anchor;

    /**
     * Definition of @c Line data structure.
     */
    typedef std::pair<Anchor*, Anchor*> Line;

    /**
     * Callback function for arbitrary clipping plane manipulation.
     *
     * @param e mouse event which is translated into arbitrary clipping plane manipulation(s)
     */
    void planeManipulation(tgt::MouseEvent* e);

    /**
     * Paints the manipulator, ie. cylinder (shaft) and tips.
     * Takes into account which parts of the handle are on the other side of the plane and renders those transparent (depending on the angle to the camera).
     */
    void paintManipulator();

    /**
     * Renders the cylinder of the manipulator, ie. the shaft of the handle.
     */
    void paintManipulatorCylinder(const tgt::vec3& center, const tgt::vec3& direction);

    /**
     * Renders the top half of the manipulator cylinder.
     */
    void paintManipulatorCylinderTopHalf(const tgt::vec3& center, const tgt::vec3& direction);

    /**
     * Renders the bottom half of the manipulator cylinder.
     */
    void paintManipulatorCylinderBottomHalf(const tgt::vec3& center, const tgt::vec3& direction);

    /**
     * Renders a manipulator tip, ie. a sphere.
     */
    void paintManipulatorTip(const tgt::vec3& center);

    /**
     * Renders the given anchor.
     *
     * @param anchor Anchor
     */
    void paintAnchor(const Anchor& anchor);

    /**
     * Renders the given line.
     *
     * @param line Line
     */
    void paintLine(const Line& line);

    /**
     * Returns the distance of the given point to the given line in vector equation form.
     *
     * @param point the Point
     * @param linePositionVector position vector of line vector equation
     * @param lineDirectionVector direction vector of line vector equation
     *
     * @return distance between point and line
     */
    //float getPointLineDistance(const tgt::vec3& point, const tgt::vec3& linePositionVector, const tgt::vec3& lineDirectionVector) const;

    /**
     * Return the given line vector equation scalar for the projection of the given point.
     *
     * @param point the Point
     * @param linePositionVector position vector of line vector equation
     * @param lineDirectionVector direction vector of line vector equation
     *
     * @return distance between point and line
     */
    float getPointLineProjectionScalar(const tgt::vec2& point, const tgt::vec2& linePositionVector, const tgt::vec2& lineDirectionVector) const;

    /**
     * Determines the intersection point of the given edge and a plane equation.
     *
     * If the intersection point is outside of the edge (ie. not between start and end vertex),
     * the intersection point is omitted.
     *
     * @param intersectionPoint the variable in which the intersection point should be stored
     * @param startVertex start vertex of the edge
     * @param endVertex end vertex of the edge
     * @param plane the plane equation
     *
     * @return @c true if an intersection point within the edge is found, otherwise @c false.
     */
    bool getIntersectionPointInBB(tgt::vec3& intersectionPoint, const tgt::vec3& startVertex,
            const tgt::vec3& endVertex, const tgt::vec4& plane);

    /**
     * Add the intersection point of the given edge and plane equation,
     * if it is within the edge (ie. between start and end vertex).
     *
     * @param startVertex start of the edge
     * @param endVertex end of the edge
     * @param plane the plane equation
     */
    void addAnchor(const tgt::vec3& startVertex, const tgt::vec3& endVertex, const tgt::vec4& plane);

    /**
     * Add line connecting two anchors lying on the given plane.
     *
     * @note: All anchors must exist before the @c addLine function is called,
     *        since the anchors are checked against the given plane.
     *
     * @param plane plane equation
     */
    void addLine(const tgt::vec4& plane);

    /**
     * Updates anchors and lines.
     *
     * Synchronize anchor and line state with ID manager, recreate anchors and lines,
     * and order them for clipping plane visualization on clipping plane manipulation.
     */
    void updateAnchorsAndLines();

    /**
     * Updates the tip positions using center position, and orientation of the plane manipulator
     */
    void updateManipulatorElements();

    /**
     * Checks if the scene bounding box changed.
     * Computes the new plane position range from the scene bounds and sets the position if necessary.
     */
    void checkInportsForBoundingBox();

    /**
     * Sets the normal to (0,0,-1) and the position to the center of the bounding box
     */
    void resetPlane();

    /**
     * updates the manipulator length, diameter, and tip radius
     */
    void updateManipulatorScale();

    /**
     * Computes the default position of the manipulator on the plane if no manipulator element is grabbed.
     */
    void setManipulatorToStdPos();

    /**
     * List of all anchors (clipping plane intersection points with the volume's / geometry's bounding box).
     */
    std::vector<Anchor> anchors_;

    /**
     * List of all lines (connection of two anchors to a line, which is in at least one
     * of the bounding box surface planes)
     */
    std::vector<Line> lines_;

    VolumePort volumeInport_;               ///< Volume inport to determine AABB dimension.
    GeometryPort geometryInport_;           ///< alternative for using volumeInport_

    BoolProperty enable_;                   ///< Enable flag to be linked with the clipping processor.

    BoolProperty renderPlane_;              ///< Determines whether the widget is shown.

    BoolProperty renderManipulator_;        ///< determines wether the manipulator is shown

    ButtonProperty resetManipulatorPos_;    ///< reset position of the manipulator to normal * position

    BoolProperty autoReset_;                ///< always reset manipulator after interaction (only if plane visible)?
    ButtonProperty resetPlane_;             ///< reset the plane to normal = (0,0,-1) and the position to the center of the bounding box

    BoolProperty invert_;                   ///< for linking with GeometryClipping processor, has no functionality on its own

    FloatVec3Property planeNormal_;         ///< Clipping plane normal
    FloatProperty planePosition_;           ///< Clipping plane position (distance to the world origin)

    FloatVec4Property planeColor_;          ///< Color of lines, anchors and the actual plane (if it is rendered during manipulation)
    FloatVec4Property grabbedElementColor_; ///< Grabbed color for manipulator parts

    FloatVec4Property blockedElementColor_; ///< color of a grabbed element that may currently not be moved

    FloatProperty grabbedPlaneOpacity_;     ///< opacity of the plane that is rendered while the manipulator is grabbed

    FloatVec2Property positionRange_;       ///< determines the range for the plane position, depending on the scene bounds

    FloatProperty width_;                   ///< Clipping plane boundary lines width.
    FloatProperty manipulatorScale_;        ///< scale to resize the manipulator

    EventProperty<PlaneManipulation>* moveEventProp_;  ///< Mouse event property.

    tgt::vec4 actualPlane_;                 ///< Actually visualized clipping plane equation.

    tgt::vec3 manipulatorCenter_;           ///< center position of the manipulator
    tgt::vec3 topManipulatorPos_;           ///< position of the top manipulator tip
    tgt::vec3 bottomManipulatorPos_;        ///< position of the bottom manipulator tip
    tgt::vec3 manipulatorOrientation_;      ///< orientation of the manipulator (ie. normalized plane normal)

    float shiftOffset_;                     ///< offset between the mouse position and the manipulator center when shifting the handle
    float scale_;                           ///< scale factor for shifting the manipulator to avoid numerical problems with very small / large scenes

    //computed from scene bounding box
    float manipulatorLength_;
    float manipulatorDiameter_;
    float manipulatorTipRadius_;

    static const tgt::vec4 lightPosition_;

    //flags to determine if the manipulator is currently grabbed
    bool manipulatorTopTipIsGrabbed_;
    bool manipulatorBottomTipIsGrabbed_;
    bool manipulatorCylinderIsGrabbed_;
    bool wholeManipulatorIsGrabbed_;

    //flag that determines if current movement is invalid (e.g. moving the handle outside of the scene)
    bool invalidMovement_;

    tgt::Bounds physicalSceneBounds_;           ///< bounds of input geometry in physical coordinates
    tgt::Bounds worldSceneBounds_;              ///< bounds of input geometry in world coordinates
    tgt::mat4 physicalToWorld_;                 ///< physical-to-world matrix for the input geometry
    tgt::mat4 worldToPhysical_;                 ///< inverse of physical-to-world matrix

    tgt::Bounds manipulatorBounds_;             ///< bounds for the manipulator handle center position

    static const std::string loggerCat_;
};

} // namespace voreen

#endif // VRN_PLANEMANIPULATION_H
