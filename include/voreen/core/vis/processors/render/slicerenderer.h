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

#ifndef VRN_SLICERENDERER_H
#define VRN_SLICERENDERER_H

#include <cstring>

#include "tgt/vector.h"
#include "tgt/matrix.h"
#include "tgt/plane.h"
#include "tgt/shadermanager.h"
#include "tgt/camera.h"

#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/transfunc/transfunc.h"
#include "voreen/core/vis/processors/processor.h"
#include "voreen/core/vis/processors/render/volumerenderer.h"
#include "voreen/core/vis/transfunc/transfuncintensity.h"

namespace voreen {

class EnumProp;

/**
 * Base class for all SliceRendering sub classes.
 * Provides basic functionality.
 */
class SliceRendererBase : public VolumeRenderer {
public:

    /**
     * Constructor
     */
    SliceRendererBase();
    virtual ~SliceRendererBase();

    /// Inits some OpenGL states
    virtual void init();
    /// Sets previously defined OpenGL states back to orignal values
    virtual void deinit();

    /**
     * Handles:
     * - Identifier::setTransferFunction
     * - Identifier::setLowerThreshold
     * - Identifier::setUpperThreshold
     * - Identifier::setSnapFileName
     * - Identifier::paintToFile
     * - Identifier::paintToAVI
     * - Identifier::cleanUpAVI
     * - Identifier::setAVIFileName
     */
    virtual void processMessage(Message* msg, const Identifier& dest =Message::all_);
    virtual void setPropertyDestination(Identifier tag);

    virtual void setCamera(tgt::Camera* camera) {camera_ = camera;}
    virtual tgt::Camera* getCamera() { return camera_; }

    virtual TransFunc* getTransFunc() { return transferFunc_; }

protected:

/*
    internal helpers
*/
    /// Used intenally to update some members
    virtual void setVolumeContainer() {};

    /// Returns true if all necessary data have been initialized so rendering can be started
    bool ready() const;

    /// Pushes the model view and the texture matrix on the matrix stack
    void saveModelViewAndTextureMatrices();

    /// Pops the model view and the texture matrix on the matrix stack
    void restoreModelViewAndTextureMatrices();

    /**
     * Enalbes GL_TEXTURE_3D, binds the transfer function
     * and activates a texture unit for use of the volume data.
     * */
    void setupTextures();

    /// Setups the shader if applicable and sets all uniforms.
    virtual void setupShader();

    /// Constructs the header for the shader depending which features should be used.
    virtual std::string buildHeader();

    /// Recompiles the shader.
    void rebuildShader();

    /// Deactivates the shader
    void deactivateShader();

    /// checks usePreIntegration_ and modifies lookupTable_
    void usePreIntegration(TransFunc* source);



/*
    data
*/
    TransFunc* transferFunc_;
    TransFunc2D* lookupTable_;

    tgt::Shader* transferFuncShader_;

    bool needRecompileShader_;

    BoolProp useLowerThreshold_;
    BoolProp useUpperThreshold_;
    BoolProp usePhongLighting_;
    BoolProp useCalcGradients_;
    BoolProp usePreIntegration_;

    tgt::vec2 threshold_;

    static const Identifier transFuncTexUnit_;
    static const Identifier volTexUnit_;
};

//------------------------------------------------------------------------------

/**
 * This class renders volume datasets 3d with 3d slicing.
*/
class SliceRenderer3D : public SliceRendererBase {
public:

    SliceRenderer3D();
 	virtual const Identifier getClassName() const {return "Raycaster.SliceRenderer3D";}
	virtual const std::string getProcessorInfo() const;
    virtual Processor* create() {return new SliceRenderer3D();}

	virtual ~SliceRenderer3D();

    virtual void setPropertyDestination(Identifier tag);

    /**
     * Handles:
     * - "set.SamplingRate"
     * - all stuff inherited from SliceRendererBase
     */
    virtual void processMessage(Message* msg, const Identifier& dest =Message::all_);

	virtual void process(LocalPortMapping* portMapping);

protected:

    /// Calculates the transformation of the bounding box of the dataset
    virtual void calculateTransformation();
    /// renders the dataset
    virtual void paint();

    virtual void setupShader();
    virtual std::string buildHeader();

    tgt::vec3   trans_;  // stores the translation
    tgt::mat4   rot_;    // stores the rotation of the dataset
    tgt::mat4   matrix_;
    float       sliceDensity_;

    EnumProp*   samplingRateProp_;
    BoolProp    showLookupTable_;

    float       samplingRate_;
};

//------------------------------------------------------------------------------

/**
 * This class renders an overview of slices and provides the possibility to zoom-in
 * on single slices.
 */
class OverviewRenderer : public SliceRendererBase {
public:

    /**
     * Standard constructor.
     *
     * @param drawGrid determines wether a grid is drawn around the slices
     * @param innerGridColor the color of the inner part of the grid
     * @param outerGridColor the color of the outer part of the grid
     */
    OverviewRenderer(bool drawGrid = true, tgt::Color innerGridColor = tgt::Color(1.f, 0.f, 0.f, 0.6f),
        tgt::Color outerGridColor = tgt::Color(1.f, 0.f, 0.f, 1.f));

    virtual void process(LocalPortMapping* localPortMapping);

    /**
     * Handles:
     * - Identifier::setVolumeContainer
     * - Identifier::setCurrentDataset
     */
    virtual void processMessage(Message* msg, const Identifier& dest =Message::all_);

    /// With this setter you can influence the translation of the whole overview.
    void setTranslation(const tgt::vec2& translation);

    /**
     * Adds a translation to the current translation vector. This moves the slice plate
     * on the canvas.
     *
     * @param translation the translation vector to add
     * @param canvasTranslation set this to true, if you pass the translation vector
     *   in canvas pixel coordinates and want the shifting of the slice plate to be
     *   synchronized with the canvas translation. E.g.: pass the offset of a mouse
     *   motion in pixel coordinates and set this parameter to true => the slice plate
     *   will move exactly as fast as the mouse cursor on screen
     *
     */
    void addTranslation(const tgt::vec2& translation, bool canvasTranslation = false);

    /**
     * With this setter you can influence the scaling of the whole overview.
     *
     * @param scale scale factor: 1.0 shows the complete overview, any greater
     *      value causes a "zoom-in"
     */
    void setScale(float scale);

    /**
     * Scales the current scale factor
     *
     * @param scale scale factor: any positive value less than 1.0 zooms out,
     *      any positive value greater than 1.0 zooms in
     */
    void scale(float scale);

    /**
     * Centers the slice nearest to the canvas' center on the canvas.
     */
    void snapToGrid();

    /**
     * Shows the whole slice plate centered on the canvas by performing a maximum zoom-out.
     */
    void showOverview();

    /**
     * Performs a maximum zoom-in on a specific slice.
     *
     * @param sliceID the number of the slice to shown
     */
    void showSlice(int sliceID);

    /**
     * Performs a maximum zoom-in on a specific slice. This function can be used to show a
     * slice the user has clicked on in the overview.
     *
     * @param canvasCoordinates the canvas coordinates of the slice to be shown.
     *
     */
    void showSlice(const tgt::ivec2 &canvasCoordinates);

    /**
     * Should be called if the user started a dragging operation on the canvas.
     *
     * @param snapToGrid if true, the slice plate is snapped to grid during dragging
     * @param snapFactor the slice plate is snapped, if the canvas' center is
     *      closer to the next slice's center than max(canvasDim.x, canvasDim.x)*draggingSnapFactor_
     */
    void startDragging(bool snapToGrid = true, float snapFactor = 0.1f);

    /**
     * Indicates that the dragging operation has stopped.
     */
    void stopDragging();

protected:

    /// Calculates the number of rows/columns of the slice plate and calculates the
    /// internal scale vector that is used to keep the slices' aspect ratio
    void calcScaleVector();

    /// Returns the distance between the canvas' center and the nearest slice center
    /// in pixel coordinates
    /// @param slice_x the x-grid-coordinate of the nearest slice is written to
    /// @param slice_y the y-grid-coordinate of the nearest slice is written to
    int distanceToNearestCenter(int &slice_x, int &slice_y);

    /// Calculates the coordinates of a slice on a virtual slice plate with
    /// dimensions [-1,-1][1,1]
    /// @param sliceID the id (depth) of the slice in the dataset
    /// @param ll the world coordinates of the slice's lower left are written to this
    /// @param ur the world coordinates of the slice's upper right are written to this
    void getSliceCoords(int sliceID, tgt::vec2 &ll, tgt::vec2 &ur);

    /// The world space translation of the slice plate relative to the canvas center / origin
    /// Is initially zero.
    tgt::vec2 trans_;

    /// Determines wether the user is currently dragging the slice plate
    bool dragging_;

    /// Determines wether the slice plate should be snapped during dragging
    bool snapToGrid_;

    /// The real translation, i.e. without snapping.
    /// Only needed in dragging mode.
    tgt::vec2 realTrans_;

    /// When in dragging mode, the slice plate is snapped, if the canvas' center is
    /// closer to the next slice's center than slice-diagonal*draggingSnapFactor_
    float draggingSnapFactor_;

    /// Internal scale vector that is used to keep the slices' aspect ratio
    /// Is calculated by calcScaleVector()
    tgt::vec3 scaleVector_;

    /// Scales the slice plate. It is used to zoom-in on the slice plate
    /// and is initially set to 1.f
    float scaleFactor_;

    /// Number of colums / rows of the slice plate. Are calculated by calcScaleVector()
    int numX_, numY_;

    /// Length of the diagonal of one slice
    int sliceDiagonal_;

    /// Determines wether a grid is drawn around the slices.
    bool drawGrid_;

    /// The color that is used for the inner part of the grid.
    tgt::Color innerGridColor_;

    /// The color that is used for the outer part of the grid.
    tgt::Color outerGridColor_;

};

//------------------------------------------------------------------------------

/**
 * This class renders a single slice parallel to the front,
 * side or top of a data set.
 */
class SingleSliceRenderer : public SliceRendererBase {
public:

    enum Alignment {
        AXIAL,          ///< view from the volume's bottom to the top
        SAGITTAL,       ///< view from the volume's right to the left
        CORONAL         ///< view from the volume's front to the back
    };

    /**
     * @param alignment use AXIAL or 0, SAGITTAL or 1 and CORONAL or 2
     */
	SingleSliceRenderer(Alignment alignment=SingleSliceRenderer::AXIAL);

	virtual const Identifier getClassName() const {return "Raycaster.SingleSliceRenderer";}
	virtual const std::string getProcessorInfo() const;
    virtual Processor* create() {return new SingleSliceRenderer();}

	virtual void process(LocalPortMapping* portMapping);
	virtual void processMessage(Message* msg, const Identifier& ident);

    /// Sets the index of the slice to be rendered
    void setSliceIndex(size_t sliceIndex);

    /// Returns the number of slices which can be shown i.e. the depth of the slices
    size_t getNumSlices() const;

	EnumProp* alignmentProp_; 

protected:

    Alignment alignment_;      ///< Which kind of Alignment is used here?
    tgt::Vector3<size_t> xyz_; ///< xyz_[i] gives the i-th dimension in this view.
    size_t numSlices_;         ///< The number of slices in the xyz_[2] direction.
    size_t sliceIndex_;        ///< The sliceIndex-th slice which should be displayed.
};

//------------------------------------------------------------------------------

/**
 * This class renders a single slice parallel to the front,
 * side or top of a data set with two modalities.
*/
class MultimodalSingleSliceRenderer : public SingleSliceRenderer {
public:

	/**
     * @param alignment use AXIAL or 0, SAGITTAL or 1 and CORONAL or 2
     */
    MultimodalSingleSliceRenderer(Alignment alignment);

    virtual void process(LocalPortMapping* localPortMapping);

protected:
    TransFuncIntensity* transferFunc1_;
    static const Identifier transFuncTexUnit1_;
    static const Identifier volTexUnit1_;
};

//------------------------------------------------------------------------------

/// This helper class holds information about slices through datasets.
struct CutData
{
    tgt::vec3   cutPolygon_[12]; // actually only 6 are needed, but for security lets allocate 12
    size_t      numVertices_;
    tgt::plane  cutPlane_;
    tgt::col3   color_;

    CutData()
        : numVertices_(0)
    {}
};

//------------------------------------------------------------------------------

/**
 * This class renders an arbitrary cut of a data set
*/
class CustomSliceRenderer : public SliceRendererBase {
public:

    CustomSliceRenderer(const CutData* cutData);
    virtual Processor* create() {return new CustomSliceRenderer(0);}
    virtual const Identifier getClassName() const { return "CustomSliceRenderer"; }
    virtual void process(LocalPortMapping* localPortMapping);

protected:

    const CutData*  cutData_;
};

//------------------------------------------------------------------------------

/**
 * Used in the Split View. Emphasizes one \e top slice.
 */
class EmphasizedSliceRenderer3D : public SliceRenderer3D {
public:

    EmphasizedSliceRenderer3D(const CutData* cutData);

    virtual void paint();
    virtual void calculateTransformation();
    virtual void processMessage(Message* msg, const Identifier& dest =Message::all_);
    void setMouseY(float mouseY) {
        mouseY_ = mouseY;
    }

protected:

    const CutData* cutData_;
    tgt::quat quat_;
    float mouseY_;
};

} // namespace voreen

#endif // VRN_SLICERENDERER_H
