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

#ifndef VRN_COPYTOSCREENRENDERER
#define VRN_COPYTOSCREENRENDERER

#include "tgt/glmath.h"

#include "voreen/core/vis/processors/processor.h"

namespace voreen {

/**
 * A CopyToScreenRenderer is the root node of the rendering pipeline.
 * Its main purpose is to render (copy) its producer's rendering result to screen.
 * Additionally it is able to cache the producer's rendering,
 * if no parameters regarding any element of the producer pipeline have been
 * changed since last \a render() call.
 * An object of this class can also be used within the pipeline in order to cache
 * a pipeline's fork. The usage of multiple CopyToScreenRenderer objects in the
 * same pipeline is possible.
 * \note A pipeline element (Renderer object) has to indicate a change of
 * its rendering parameters that necessiates a new rendering by calling \c invalidate().

 * THIS CLASS IS ONLY NEEDED BECAUSE FINAL AND COARSENESSRENDER INHERITE FROM IT!
 */
class CopyToScreenRenderer : public Processor
{
public:

	/**
	 * Default constructor.
	 * @param camera the \c Camera object used in this pipeline.
	 * @param tc the \c TextureContainer object to use. \see TextureContainer
	 */
	CopyToScreenRenderer(tgt::Camera* camera=0, TextureContainer* tc = 0);
    ~CopyToScreenRenderer();

	virtual const Identifier getClassName() const {return "Miscellaneous.CopyToScreenRenderer";}
	virtual const std::string getProcessorInfo() const;
    virtual Processor* create() {return new CopyToScreenRenderer();}

    virtual int initializeGL();

    virtual void process(LocalPortMapping* /*portMapping*/) {}
    virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);

    virtual void invalidate();

    /**
	 * Indicates if this \c CopyToScreenRenderer caches its producer's
	 * rendering result. If caching is true, the CopyToScreenRenderer
	 * only updates it current rendering, if it has received an \c
	 * invalidate() since last \c render() call.
	 * default:true
	 */
	void setCaching(bool caching);

    /**
     * If ignoreCoarseness is set to true, the \c CopyToScreenRenderer
     * does not pass through the coarseness factor to its attached
     * pipeline. This is useful, if there is more than one
     * \c CopyTosScreenRenderer in a pipeline. In this case, all
     * \c CopyToScreenRenderer objects except the last have to ignore
     * the coarseness.
     * default: false
     */
    void setIgnoreCoarseness(bool ignoreCoarseness);

//TODO:  protected:
    /**
     * True if the coarseness factor is not passed through to the attached
     * producer. \see coarsenessFactor_
     */
    bool ignoreCoarseness_;

	/**
     * Indicates if the coarseness factor is ignored. \see setIgnoreCoarseness
     */
    BoolProp useCoarseness_;

    /**
     * The current coarseness factor. The rendering target's resolution
     * is reduced by this factor during user interaction.
     */
    IntProp coarsenessFactor_;


protected:   

    /**
     * The shader program used by this \c CopyToScreenRenderer.
     */
    tgt::Shader* raycastPrg_;

	/**
	 * True if the rendering of the attached producer is cached.
	 */
    bool caching_;

    /**
     * The id of the render target that is used for the caching of
     * the producer's image.
     */
    int cachedImage_;

    /**
     * The id of the render target that is used for the caching of
     * the left image in stereo viewing.
     */
    int cachedStereoImageLeft_;

    /**
     * The id of the render target that is used for the caching of
     * the right image in stereo viewing.
     */
    int cachedStereoImageRight_;
};

} // namespace voreen

#endif //VRN_COPYTOSCREENRENDERER
