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

#ifndef VRN_IMAGESEQUENCESOURCE_H
#define VRN_IMAGESEQUENCESOURCE_H

#include "voreen/core/processors/renderprocessor.h"
#include "voreen/core/ports/allports.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/intproperty.h"
#include "voreen/core/properties/boolproperty.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/buttonproperty.h"

namespace voreen {

class ImageSequence;

/**
 * Loads all image files from a directory and puts them out as ImageSequence
 * containing one OpenGL texture per image.
 */
class ImageSequenceSource : public RenderProcessor {

public:
    ImageSequenceSource();
    ~ImageSequenceSource();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "ImageSequenceSource"; }
    virtual std::string getCategory() const  { return "Data Source"; }
    virtual CodeState getCodeState() const   { return CODE_STATE_STABLE; }
    virtual std::string getProcessorInfo() const;

    /**
     * Issues a reload of the sequence during next beforeProcess(), if the directory path has changed.
     */
    virtual void invalidate(int inv = INVALID_RESULT);

    /**
     * Loads all images contained by the passed directory.
     *
     * @note The processor must have been initialized
     *       before calling this function.
     *
     * @param dir the directory containing the images to load
     */
    virtual void loadImageSequence(const std::string& dir)
        throw (tgt::FileException, std::bad_alloc);

    /**
     * Assigns the passed ImageSequence to this processor.
     *
     * @note The processor must have been initialized
     *       before calling this function.
     */
    virtual void setImageSequence(ImageSequence* sequence);

    /**
     * Returns the currently assigned/hold image sequence.
     */
    const ImageSequence* getImageSequence() const;

    /**
     * Issues a reload of the image sequence during next beforeProcess() call.
     */
    virtual void reloadImageSequence();

protected:
    virtual void process();
    virtual void initialize() throw (VoreenException);
    virtual void deinitialize() throw (VoreenException);

    /**
     * Clears the assigned image sequence.
     *
     * @note The processor must have been initialized
     *       before calling this function.
     */
    virtual void clearSequence();

    /**
     * Enforces a reload of the current image sequence during next beforeProcess() call.
     */
    virtual void forceReload();

    /**
     * Sets the imageDirectory_ property to an empty string,
     * thereby causing the sequence to be cleared during next beforeProcess().
     */
    virtual void unsetDirectoryName();

    ImageSequencePort outport_;         ///< The port the generated image sequence is written to.

    FileDialogProperty imageDirectory_; ///< Directory the images are loaded from.
    BoolProperty textureFiltering_;     ///< Enable linear filtering of loaded textures.
    BoolProperty uploadTextureData_;    ///< Determines whether the images' texture data is uploaded to the GPU.
    BoolProperty showProgressBar_;      ///< Determines whether a progress dialog is shown during image loading.
    ButtonProperty reloadSequence_;     ///< Button for reloading the current sequence.
    ButtonProperty clearSequence_;      ///< Button for clearing the current sequence.
    IntProperty numImages_;             ///< Read-only property providing the size of the sequence.

    ImageSequence* imageSequence_;      ///< Image sequence assigned to this processor.
    std::string currentDir_;            ///< Current image directory.

    /// Determines whether the processor owns the sequence's textures,
    /// which is only the case, if it has loaded them.
    bool sequenceOwner_;

    bool forceReload_;
    static const std::string loggerCat_;
};

} // namespace

#endif
