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

#ifndef VRN_TRANSFUNCPROPERTYWIDGET_H
#define VRN_TRANSFUNCPROPERTYWIDGET_H

#include "voreen/qt/widgets/property/qpropertywidgetwitheditorwindow.h"
#include "voreen/core/datastructures/volume/volume.h"

#include "voreen/core/datastructures/transfunc/transfunc.h"

class QToolButton;
namespace tgt {
    class QtCanvas;
}

namespace voreen {

class TransFuncPlugin;
class VoreenToolWindow;
class TransFuncProperty;
class TransFuncPropertyWidgetPainter;

/**
 * Property widget used to represent 1D transfer functions.
 */
class TransFuncPropertyWidget : public QPropertyWidgetWithEditorWindow, public VolumeObserver {
    Q_OBJECT
public:
    /** Constructor */
    TransFuncPropertyWidget(TransFuncProperty* prop, QWidget* parent = 0);

    virtual void disconnect();

    /// Returns the null pointer, since this widget does not need a separate label.
    virtual CustomLabel* getNameLabel() const;

    void setVisible(bool visible);

    // VolumeObserver methods:
    virtual void volumeDelete(const VolumeBase* source);
    virtual void volumeChange(const VolumeBase* source);
    virtual void derivedDataThreadFinished(const VolumeBase* source, const VolumeDerivedData* derivedData);

public slots:
    void setProperty();
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitToData();

    virtual void showNameLabel(bool);
    void invalidateProperty();
    void updateZoomMeta();
    /**
     * Starts or stops the interaction mode.
     *
     * @param on interaction mode on or off?
     */
    void toggleInteractionMode(bool on);


protected:
    virtual QWidget* createEditorWindowWidget();
    virtual void customizeEditorWindow();
    virtual Property* getProperty();

    void populateMenuFromDirectory(QMenu* menu, std::string directory);

    TransFuncPlugin* plugin_;
    TransFuncProperty* property_;

    tgt::QtCanvas* textureCanvas_;              ///< canvas that is used for displaying the texture of the transfer function
    TransFuncPropertyWidgetPainter* texturePainter_;   ///< painter for texture display of transfer function

    bool ignoreSlideUpdates_;
    bool viewInitialized_;

    //--------------------
    //  gui member
    //--------------------
    QToolButton* tfBt_;             ///< button to load tf presets
        QMenu* loadTFMenu_;         ///< tf menu
    QToolButton* windowBt_;         ///< button to load domain windows
        QMenu* loadWindowMenu_;     ///< window menu
    QToolButton* alphaBt_;          ///< button to enable alpha
        QMenu* alphaMenu_;          ///< menu for alpha value

    QToolButton* zoomInBt_;         ///< button to zoom in
    QToolButton* zoomOutBt_;        ///< button to zoom out

    QToolButton* advancedBt_;       ///< button with advanced settings
        QMenu* advancedMenu_;       ///< advanced menu

    //--------------------
    //  menu handling
    //--------------------
protected slots:
    /** Generates the TF menu on click. */
    void populateLoadTFMenu();
    /** Loads the TF. Is called from the menu. */
    void loadTF(QAction* action);
    /** Generates the window menu on click. */
    void populateLoadWindowMenu();
    /** Loads the window. Is called from the menu. */
    void loadWindow(QAction* action);
    /** Creates the alpha menu. */
    void populateAlphaMenu();
    /** action being clled by the alphaMenu_. */
    void setAlpha(QAction* action);
    /** Creates the advanced menu. */
    void populateAdvancedMenu();
    /** Action being clled by the advancedMenu_. */
    void doAdvancedAction(QAction* action);

private:
         /** Used to generate the menu entries. */
        void populateTFMenuFromDirectory(QMenu* menu, std::string directory);
        /** Used to generate the menu entries. */
        void populateWindowMenuFromDirectory(QMenu* menu, std::string directory);
        /** Updates the property and sets the menu icon */
        void updateAlpha(TransFunc::AlphaMode mode);



protected slots:
    virtual void updateFromPropertySlot();
    /** Shows the tooltip. Is connected to fransfuncpropertywidgetpainter. */
    void showToolTipSlot(QPoint pos, QString tip);
    /** Hides the tooltip. Is connected to fransfuncpropertywidgetpainter. */
    void hideToolTipSlot();

};

} // namespace

#endif // VRN_TRANSFUNCPROPERTYWIDGET_H
