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

#include "voreen/core/voreenapplication.h"
#include "voreen/core/datastructures/transfunc/transfuncmappingkey.h"
#include "voreen/core/datastructures/transfunc/transfuncintensity.h"
#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/utils/voreenpainter.h"

#include "modules/sphericalharmonics/include/shraycaster.h"
#include "modules/sphericalharmonics/include/transfuncmappingcanvastexture.h"
#include "modules/sphericalharmonics/include/transfunceditorintensitytexture.h"

#include "tgt/texturemanager.h"

#include <QFileDialog>
#include <QMouseEvent>

namespace voreen {

TransFuncMappingCanvasTexture::TransFuncMappingCanvasTexture(QWidget* parent, TransFuncIntensity* tf, TransFuncProperty* prop,
                                               bool noColor, QString xAxisText, QString yAxisText)
    : TransFuncMappingCanvas(parent, tf, noColor, xAxisText, yAxisText)
    , property_(prop)
{
    if (!noColor_) {
        assignAction_ = new QAction(tr("Assign texture to key"), this);
        keyContextMenu_.addAction(assignAction_);
        connect(assignAction_, SIGNAL(triggered()), this, SLOT(changeCurrentTexture()));
        disableAction_ = new QAction(tr("Disable key-texture"), this);
        keyContextMenu_.addAction(disableAction_);
        connect(disableAction_, SIGNAL(triggered()), this, SLOT(disableCurrentTexture()));
    }
}

void TransFuncMappingCanvasTexture::changeCurrentTexture() {
    if (!selectedKey_ || noColor_)
        return;

    QFileDialog fileDialog(this);
    fileDialog.setWindowTitle(tr("Choose a texture to open"));
    fileDialog.setDirectory(VoreenApplication::app()->getUserDataPath("brdfs/").c_str());
    //fileDialog.setFilter("Textures (*.tga, *.png)");
    //fileDialog.setFilter("Textures (*.png)");
    fileDialog.setFilter("Textures (*.tga)");

    if (fileDialog.exec() && !fileDialog.selectedFiles().empty()) {
        std::string fn = fileDialog.selectedFiles()[0].toStdString();

        // HACK there is a bug in FileSystem (at least for me): under windows, relativeFilename doesn't work,
        // it just replaces the "/"s with "\"s
        std::ostringstream o;
        o << VoreenApplication::app()->getUserDataPath("brdfs/") << tgt::FileSystem::fileName(fn);
        fn = o.str();
        //std::cout << fn << std::endl;

        if (selectedKey_->isSplit() && !selectedLeftPart_) {
            selectedKey_->setTextureR(TexMgr.load(fn, tgt::Texture::LINEAR, false, true, false));
            selectedKey_->setTexNameRight(fn);
        } else {
            selectedKey_->setTextureL(TexMgr.load(fn, tgt::Texture::LINEAR, false, true, false));
            selectedKey_->setTexNameLeft(fn);
        }
        update();
        emit changed();
    }
}

void TransFuncMappingCanvasTexture::disableCurrentTexture() {
    if (!selectedKey_ || noColor_)
        return;

    std::string empty("");
    if (selectedKey_->isSplit() && !selectedLeftPart_) {
        TexMgr.dispose(selectedKey_->getTextureR());
        selectedKey_->setTextureR(0);
        selectedKey_->setTexNameRight(empty);
    } else {
        TexMgr.dispose(selectedKey_->getTextureL());
        selectedKey_->setTextureL(0);
        selectedKey_->setTexNameLeft(empty);
    }
    update();
    emit changed();
}

void TransFuncMappingCanvasTexture::showKeyContextMenu(QMouseEvent* event) {
    // Set context-dependent text for menu items

    // Split/merge
    QString splitMergeText;
    if (selectedKey_->isSplit())
        splitMergeText = tr("Merge this key");
    else
        splitMergeText = tr("Split this key");
    splitMergeAction_->setText(splitMergeText);

    // Zero/unzero
    QString zeroText;
    if (selectedLeftPart_)
        zeroText = tr("Zero to the left");
    else
        zeroText = tr("Zero to the right");
    zeroAction_->setText(zeroText);

    // allow deletion of keys only if there are more than two keys
    deleteAction_->setEnabled(tf_->getNumKeys() > 2);

    if(selectedKey_->isSplit()) {
        if(selectedKey_->getTextureL() && selectedLeftPart_)
            assignAction_->setText(QString("Assign Texture (Current: ").append(selectedKey_->getTexNameLeft().c_str()).append(")"));
        else if(selectedKey_->getTextureR() && !selectedLeftPart_)
            assignAction_->setText(QString("Assign Texture (Current: ").append(selectedKey_->getTexNameRight().c_str()).append(")"));
        else
            assignAction_->setText(QString("Assign Texture (Current: none)"));
    }
    else {
        if(selectedKey_->getTextureL())
            assignAction_->setText(QString("Assign Texture (Current: ").append(selectedKey_->getTexNameLeft().c_str()).append(")"));
        else
            assignAction_->setText(QString("Assign Texture (Current: none)"));
    }

    if(selectedKey_->isSplit()) {
        if(selectedKey_->getTextureL() && selectedLeftPart_)
            disableAction_->setEnabled(true);
        else if(selectedKey_->getTextureR() && !selectedLeftPart_)
            disableAction_->setEnabled(true);
        else
            disableAction_->setEnabled(false);
    } else if(selectedKey_->getTextureL())
        disableAction_->setEnabled(true);
    else
        disableAction_->setEnabled(false);

    keyContextMenu_.popup(event->globalPos());
}

// FIXME horrible hack
void TransFuncMappingCanvasTexture::mousePressEvent(QMouseEvent* event) {
    //SHRaycaster* shrc = 0;
    //shrc = dynamic_cast<SHRaycaster*>(property_->getOwner());

    //if(shrc)
        //shrc->getSHClass()->setSHInteraction(true);

    TransFuncMappingCanvas::mousePressEvent(event);
}

} // namespace voreen
