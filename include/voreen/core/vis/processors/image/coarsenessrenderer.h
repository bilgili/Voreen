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

#ifndef VRN_COARSENESSRENDERER_H
#define VRN_COARSENESSRENDERER_H

#include "voreen/core/vis/processors/processor.h"

namespace voreen {

class CoarsenessRenderer : public Processor {
public:
	/**
	 * Default constructor. Sets the processorname, creates one in- and one outport and registers
     * the coarsenessfactor property.
	 */
	CoarsenessRenderer();
    ~CoarsenessRenderer();

    /**
    * Gets the identifier for "Miscellaneous.CoarsenessRenderer"
    * @return The identifier
    */
	virtual const Identifier getClassName() const;
	
    /**
    * Gets further information about the processor.
    * @return A short text describing the processor
    */
    virtual const std::string getProcessorInfo() const;
    
    /**
    * Returns a new instance of this processor
    * @return A newly created coarseness renderer
    */
    virtual Processor* create();
    
    /**
    * Loads the appropriate vertex and fragment shaders.
    * @return VRN_OK if everything went fine. VRN_ERROR or VRN_OPENGL_INSUFFICIENT otherwise
    */
    virtual int initializeGL();
	virtual void process(LocalPortMapping* portMapping);
    virtual void processMessage(Message* msg, const Identifier& dest=Message::all_);
    
    /// Getter for useCoarseness_
    bool getUseCoarseness();
    /// Getter for ignoreCoarseness_
    bool getIgnoreCoarseness();
    /// Getter for coarsenessFactior_
    int getCoarsenessFactor();

    /// setter for useCoarseness
    void setUseCoarseness(bool use);

    /**
     * If ignoreCoarseness is set to true, the \c CoarsenessRenderer
     * does not pass through the coarseness factor to its attached
     * pipeline. This is useful, if there is more than one
     * \c CoarsenessRenderer in a pipeline. In this case, all
     * \c CoarsenessRenderer objects except the last have to ignore
     * the coarseness.
     * default: false
     */
    void setIgnoreCoarseness(bool ignoreCoarseness);

private:

	/**
     * Indicates if the coarseness factor is ignored. \see setIgnoreCoarseness
     */
    bool useCoarseness_;

    /**
     * The current coarseness factor. The rendering target's resolution
     * is reduced by this factor during user interaction.
     */
    IntProp coarsenessFactor_;

    /**
     * The shader program used by this \c CoarsenessRenderer.
     */
    tgt::Shader* raycastPrg_;

    static const Identifier setCoarseness_;

};

} // namespace voreen

#endif //VRN_COARSENESSRENDERER_H
