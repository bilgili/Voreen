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

#ifndef VRN_TRANSFUNCEDITOR_H
#define VRN_TRANSFUNCEDITOR_H

#include "voreen/core/vis/voreenpainter.h"
#include "voreen/core/vis/processors/render/volumerenderer.h"

#include "voreen/core/vis/messagedistributor.h"
#include "voreen/core/vis/message.h"

#include "voreen/core/vis/transfunc/transfunc.h"

namespace voreen {
class Volume;

/**
* Interface for all TF editor widgets. Implement colorchooser with the currently used toolkit.
*/
class TransFuncEditor {
public:
    /**
    * Standard constructor
    */
    TransFuncEditor(MessageReceiver* msgReceiver, Identifier target = "PrimaryRaycaster", Identifier id = VolumeRenderer::setTransFunc_);
    
    /**
    * Standard destructor
    */
    virtual ~TransFuncEditor();

    virtual void transFuncChanged();
    virtual void dataSourceChanged(Volume* newDataset) = 0;
    virtual TransFunc* getTransFunc() = 0;
    
    /**
    * Sets the display thesholds for appropriate transfer function.
    * All values x < lower and x > upper will be ignored
    * \param lower The lower threshold
    * \param upper The upper threshold
    */
    virtual void setThresholds(int lower, int upper) = 0;
    
   // virtual tgt::col4 colorChooser(tgt::col4 c, bool &changed);
    
    /**
    * Standard setter method
    * \param target The target for the transfer function editor
    */
    virtual void setTarget(Identifier target);
	
    /**
    * Standard getter method
    * \return The target for the transfer function editor
    */
    virtual Identifier getTarget();
    
    /**
    * Standard setter method
    * \param id The new identifier for this object
    */
    virtual void setIdentifier(Identifier id);
    
    /**
    * Standard getter method
    * \return The current identifier for this object
    */
    virtual Identifier getIdentifier();
    
    /**
    * Sends TF Msg (with id) to target and sends repaint to all visible views
    */
    virtual void sendTFMessage();
	
    /**
    * Sets the receiver for any messages being sent out
    * \param receiver The message receiver
    */
    virtual void setReceiver(MessageReceiver* receiver);
    
    /**
    * Sends an interaction coarseness and repaint message to all visible views
    * \param interactionCoarseness Active?
    */
    void setInteractionCoarseness(bool interactionCoarseness);
    
    //TODO: elaborate doxygen comment necessary
    virtual void selected();
    //TODO: elaborate doxygen comment necessary
    virtual void deselected();
protected:
    Identifier target_;
    Identifier id_;
    MessageReceiver* receiver_;
};


} // namespace

#endif // VRN_TRANSFUNCEDITOR_H
