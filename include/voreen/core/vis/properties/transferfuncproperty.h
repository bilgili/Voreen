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

#ifndef VRN_TRANSFERFUNCPROPERTY_H
#define VRN_TRANSFERFUNCPROPERTY_H

#include "voreen/core/vis/properties/templateproperty.h"

#include "voreen/core/volume/observer.h"
#include "voreen/core/volume/volumehandle.h"

namespace voreen {

class TransFunc;
class VolumeHandle;
class Volume;

/**
 * Property for transfer functions. The widget for this property contains several editors
 * to modify the transfer function. You can change the shown editors via the constructor or 
 * by calling enableEditor() or disableEditor().
 */
class TransFuncProp : public TemplateProperty<TransFunc*>, Observer {
public:
    
    ///< enum for all editors that can be used in the widget for this property
    enum Editors {
        NONE               = 0, ///< no editor appears in the widget
        INTENSITY          = 1, ///< general widget for 1D transfer functions
        INTENSITY_RAMP     = 2, ///< widget for 1D transfer functions which only allows 2 keys
        INTENSITY_PET      = 4, ///< widget for 1D transfer functions for pet datasets
        INTENSITY_GRADIENT = 8, ///< widget for 2D transfer functions
        ALL                = 15 ///< aggregation of all editor widgets
    };

    /**
     * Constructor
     *
     * @param ident identifier that is used in serialization
     * @param guiText text that is shown in the gui
     * @param invalidate if true the owner of the property is invalidated when the property changes
     * @param invalidateShader if true the shader of the owner of the property owner is invalidated when the property changes
     * @param editors determines the types of editors to be presented to the user
     * @param lazyEditorInstantiation determines whether the transfer function editor of this property is instantiated on 
     *        construction of the property widget or when the user is first accessing it (lazy)
     */
    TransFuncProp(const Identifier& ident, const std::string& guiText, bool invalidate = true, bool invalidateShader = false,
        Editors editors = ALL, bool lazyEditorInstantiation = false);

    /**
     * Destructor
     */
    ~TransFuncProp();

    /**
     * Enables the given editor in the widget for the property. Must be called before creation
     * of the widget.
     *
     * @param editor editor that is enabled
     */
    void enableEditor(Editors editor);

    /**
     * Disables the given editor in the widget for the property. Must be called before creation
     * of the widget.
     *
     * @param editor editor that is disabled
     */
    void disableEditor(Editors editor);

    /**
     * Returns true when the given editor is enabled and false otherwise.
     *
     * @param editor editor which activation status is returned
     * @return true if the given editor is enabled, false otherwise
     */
    bool isEditorEnabled(Editors editor);

    /**
     * Enables or disables the automatic repaint of the volume rendering when the transfer
     * function changes.
     *
     * @param manual enable or disable manual repaint of volume rendering
     */
    void setManualRepaint(bool manual);

    /**
     * Returns the status of manual repaint.
     *
     * @return true when no automatic repaint of the volume rendering takes place on transfer
     *         function change, false otherwise
     */
    bool getManualRepaint();

    /**
     * Sets the stored transfer function to the given one.
     *
     * @param tf transfer function the property is set to
     */
    void set(TransFunc* tf);

    /**
     * Assigns the given volumehandle to this property. It is tested whether the given volumehandle
     * is different to the already stored one. If this is the case, the texture of the transfer
     * function is resized according to the bitdepth of the volume
     *
     * @param handle volumehandle that should be assigned to this property
     */
    void setVolumeHandle(VolumeHandle* handle);

    /**
     * Returns the volume handle that is assigned to this property.
     *
     * @return volume handle that is associated with this property
     */
    VolumeHandle* getVolumeHandle() const;
    
    /**
     * Returns the volume that is assigned to this property.
     *
     * @return volume that is associated with this property
     */
    Volume* getVolume() const;

    /**
     * Executes all member actions that belong to the property. Generally the owner of the
     * property is invalidated.
     */
    void notifyChange();

    /**
     * Reads a transfer function from the given xml element. The stored transfer function is
     * updated to the read transfer function.
     *
     * @param propElem the xml description the property is updated from
     */
    void updateFromXml(TiXmlElement* propElem);

    /**
     * Serializes the property to xml. Therefore the serialize methods of the transfer function
     * itself are used.
     *
     * @return xml description of this property
     */
    TiXmlElement* serializeToXml() const;

    /**
     * Creates a widget for the transfer function property using the given factory.
     *
     * @param f the factory that is used to create a widget for the transfer function property
     * @return widget that was created for the property
     */
    PropertyWidget* createWidget(PropertyWidgetFactory* f);

    /** 
     * Returns whether the transfer function editor of this property should be instantiated on 
     * construction of the property widget or when the user is first accessing it (lazy)
     */
    bool getLazyEditorInstantiation() const { return lazyEditorInstantiation_; }

    virtual std::string toString() const { return ""; }

    /**
     * Gets called when something changes in the VolumeSetContainer while the background thread
     * is calculating the histogram. Checks whether the volume handle is still contained in the
     * container.
     */
    void notify(const Observable* const source = 0);

protected:
    VolumeHandle* volumeHandle_; ///< volumehandle that is associated with the transfer function property

    int editors_; ///< number that indicates what editors will appear in the tf widget

    bool manualRepaint_; ///< indicates whether the tf widget automatically emits repaints for the
                         ///< volume rendering or whether the user has to do it manually

    /// Determines whether the transfer function editor of this property is instantiated on 
    /// construction of the property widget or when the user is first accessing it (lazy)
    bool lazyEditorInstantiation_;

};

} // namespace voreen

#endif // VRN_TRANSFERFUNCPROPERTY_H
