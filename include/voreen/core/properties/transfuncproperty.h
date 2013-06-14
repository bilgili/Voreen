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

#ifndef VRN_TRANSFERFUNCPROPERTY_H
#define VRN_TRANSFERFUNCPROPERTY_H

#include "voreen/core/properties/templateproperty.h"

#include "voreen/core/voreencoreapi.h"
#include "voreen/core/utils/observer.h"
#include "voreen/core/datastructures/transfunc/transfunc.h"
#include "voreen/core/datastructures/volume/volume.h"

namespace voreen {

class Volume;

#ifdef DLL_TEMPLATE_INST
template class VRN_CORE_API TemplateProperty<TransFunc*>;
#endif
/**
 * Property for transfer functions. The widget for this property contains several editors
 * to modify the transfer function. You can change the shown editors via the constructor or
 * by calling enableEditor() or disableEditor().
 */
class VRN_CORE_API TransFuncProperty : public TemplateProperty<TransFunc*>, public VolumeObserver {
public:

    ///< enum for all editors that can be used in the widget for this property
    enum Editors {
        NONE               = 0, ///< no editor appears in the widget
        INTENSITY          = 1, ///< general widget for 1D transfer functions
        INTENSITY_RAMP     = 2, ///< widget for 1D transfer functions which only allows 2 keys
        INTENSITY_GRADIENT = 8, ///< widget for 2D transfer functions
        ALL                = 15 ///< aggregation of all editor widgets
    };

    /**
     * Constructor
     *
     * @param ident identifier that is used in serialization
     * @param guiText text that is shown in the gui
     * @param invalidationLevel The owner is invalidated with this InvalidationLevel upon change.
     * @param editors determines the types of editors to be presented to the user
     * @param lazyEditorInstantiation determines whether the transfer function editor of this property is instantiated on
     *        construction of the property widget or when the user is first accessing it (lazy)
     *
     * @note The creation of the encapsulated transfer function object is deferred
     *       to deserialization or initialization, respectively.
     */
    TransFuncProperty(const std::string& ident, const std::string& guiText,
        int invalidationLevel = Processor::INVALID_RESULT,
        Editors editors = Editors(INTENSITY | INTENSITY_GRADIENT), bool lazyEditorInstantiation = true);
    TransFuncProperty();
    virtual ~TransFuncProperty();

    virtual Property* create() const;

    virtual std::string getClassName() const       { return "TransFuncProperty"; }
    virtual std::string getTypeDescription() const { return "TransferFunction"; }
    virtual void reset();

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
     * Sets the stored transfer function to the given one.
     *
     * @note The TransFuncProperty takes ownership of the passed
     *  object. Therefore, the caller must not delete it.
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
    void setVolumeHandle(const VolumeBase* handle);

    /**
     * Returns the volume that is assigned to this property.
     *
     * @return volume that is associated with this property
     */
    const VolumeBase* getVolumeHandle() const;

    /**
     * Executes all member actions that belong to the property. Generally the owner of the
     * property is invalidated and the property's widgets are notified.
     */
    void notifyChange();

    /**
     * Implementation of VolumeObserver interface.
     */
    virtual void volumeDelete(const VolumeBase* source);

    /**
     * Implementation of VolumeObserver interface.
     */
    virtual void volumeChange(const VolumeBase* source);

    /// Listens for the VolumeMinMax computation to finish.
    virtual void derivedDataThreadFinished(const VolumeBase* source, const VolumeDerivedData* derivedData);

    /**
     * @see Property::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Property::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

    /**
     * Returns whether the transfer function editor of this property should be instantiated on
     * construction of the property widget or when the user is first accessing it (lazy)
     */
    bool getLazyEditorInstantiation() const { return lazyEditorInstantiation_; }

    /**
     * Returns whether the tf domain is always adapted when a new volume is passed.
     */
    void setAlwaysFitToDomain(bool b) {
        alwaysFitDomain_ = b;
    }

    /**
     * Turn automatic domain fitting on / off.
     */
    bool getAlwaysFitToDomain() const {
        return alwaysFitDomain_;
    }

    /**
     * Sets the tf domain bounds from the current volume.
     */
    void fitDomainToData();

protected:
    /**
     * Creates an initial transfer function of type TransFuncIntensity, if has not already
     * been created by deserialization
     *
     * @see Property::initialize
     */
    void initialize() throw (tgt::Exception);

    /**
     * Deletes the stored transfer function.
     *
     * @see Property::deinitialize
     */
    void deinitialize() throw (tgt::Exception);

    const VolumeBase* volume_; ///< volumehandle that is associated with the transfer function property

    int editors_; ///< number that indicates what editors will appear in the tf widget

    /// Determines whether the transfer function editor of this property is instantiated on
    /// construction of the property widget or when the user is first accessing it (lazy)
    bool lazyEditorInstantiation_;

    /// Determines whether the transfer function value domain is always apated when an new volumehandle is passed to the
    /// property.
    bool alwaysFitDomain_;

    /// Set to true, if the domain fitting should be executed after the VolumeMinMax computation has finished
    bool fitToDomainPending_;

    static const std::string loggerCat_; ///< logger category
};

} // namespace voreen

#endif // VRN_TRANSFERFUNCPROPERTY_H
