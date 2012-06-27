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

#include "voreen/core/processors/processorwidget.h"

#include "voreen/core/processors/processor.h"
#include "voreen/core/io/serialization/serialization.h"

#include "tgt/logmanager.h"

namespace voreen {

voreen::ProcessorWidget::ProcessorWidget(Processor* processor)
    : processor_(processor)
    , initialized_(false)
{
    tgtAssert(processor, "Null pointer passed as processor");
}

ProcessorWidget::~ProcessorWidget() {
    tgtAssert(processor_, "No processor");
    processor_->deregisterWidget();
}

void ProcessorWidget::initialize() {
    initialized_ = true;
    restoreGeometryFromMeta();
}

void ProcessorWidget::updateFromProcessor(){
}

void ProcessorWidget::processorNameChanged() {
}

void ProcessorWidget::onShow() {
    if (isInitialized())
        saveGeometryToMeta();
}

void ProcessorWidget::onHide() {
    if (isInitialized())
        saveGeometryToMeta();
}

void ProcessorWidget::onMove() {
    if (isInitialized())
        saveGeometryToMeta();
}

void ProcessorWidget::onResize() {
    if (isInitialized())
        saveGeometryToMeta();
}

bool ProcessorWidget::isInitialized() const {
    return initialized_;
}

void ProcessorWidget::saveGeometryToMeta() {
    tgtAssert(processor_, "No processor");

    WindowStateMetaData* meta = new WindowStateMetaData(
        isVisible(),
        getPosition().x,
        getPosition().y,
        getSize().x,
        getSize().y);

    processor_->getMetaDataContainer().addMetaData("ProcessorWidget", meta);
}

void ProcessorWidget::restoreGeometryFromMeta() {
    WindowStateMetaData* meta = dynamic_cast<WindowStateMetaData*>(processor_->getMetaDataContainer().getMetaData("ProcessorWidget"));
    if (!meta) {
        LDEBUGC("voreen.core.ProcessorWidget", "restoreGeometryFromMeta(): No meta data object returned");
        return;
    }

    if (meta->getX() != -1 && meta->getY() != -1)
        setPosition(meta->getX(), meta->getY());

    if (meta->getWidth() != -1 && meta->getHeight() != -1)
        setSize(meta->getWidth(), meta->getHeight());

    setVisible(meta->getVisible());
}

} // namespace voreen

