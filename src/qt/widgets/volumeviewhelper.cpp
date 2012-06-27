/***********************************************************************************
 *                                                                                 *
 * Voreen - The Volume Rendering Engine                                            *
 *                                                                                 *
 * Copyright (C) 2005-2012 University of Muenster, Germany.                        *
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

#include "voreen/qt/widgets/volumeviewhelper.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumepreview.h"

#include "tgt/filesystem.h"

#include <QDir>

namespace voreen {

std::string VolumeViewHelper::getVolumeType(const VolumeRAM* volume) {

    if (dynamic_cast<const VolumeRAM_UInt8*>(volume))       return "UInt8";
    if (dynamic_cast<const VolumeRAM_UInt16*>(volume))      return "UInt16";
    if (dynamic_cast<const VolumeRAM_UInt32*>(volume))      return "UInt32";
    if (dynamic_cast<const VolumeRAM_UInt64*>(volume))      return "UInt64";
    if (dynamic_cast<const VolumeRAM_Int8*>(volume))        return "Int8";
    if (dynamic_cast<const VolumeRAM_Int16*>(volume))       return "Int16";
    if (dynamic_cast<const VolumeRAM_Int32*>(volume))       return "Int32";
    if (dynamic_cast<const VolumeRAM_Int64*>(volume))       return "Int64";
    if (dynamic_cast<const VolumeRAM_Float*>(volume))       return "Float";
    if (dynamic_cast<const VolumeRAM_Double*>(volume))      return "Double";

    if (dynamic_cast<const VolumeRAM_2xUInt8*>(volume))     return "2xUInt8";
    if (dynamic_cast<const VolumeRAM_2xUInt16*>(volume))    return "2xUInt16";
    if (dynamic_cast<const VolumeRAM_2xUInt32*>(volume))    return "2xUInt32";
    if (dynamic_cast<const VolumeRAM_2xUInt64*>(volume))    return "2xUInt64";
    if (dynamic_cast<const VolumeRAM_2xInt8*>(volume))      return "2xInt8";
    if (dynamic_cast<const VolumeRAM_2xInt16*>(volume))     return "2xInt16";
    if (dynamic_cast<const VolumeRAM_2xInt32*>(volume))     return "2xInt32";
    if (dynamic_cast<const VolumeRAM_2xInt64*>(volume))     return "2xInt64";
    if (dynamic_cast<const VolumeRAM_2xFloat*>(volume))     return "2xFloat";
    if (dynamic_cast<const VolumeRAM_2xDouble*>(volume))    return "2xDouble";

    if (dynamic_cast<const VolumeRAM_3xUInt8*>(volume))     return "3xUInt8";
    if (dynamic_cast<const VolumeRAM_3xUInt16*>(volume))    return "3xUInt16";
    if (dynamic_cast<const VolumeRAM_3xUInt32*>(volume))    return "3xUInt32";
    if (dynamic_cast<const VolumeRAM_3xUInt64*>(volume))    return "3xUInt64";
    if (dynamic_cast<const VolumeRAM_3xInt8*>(volume))      return "3xInt8";
    if (dynamic_cast<const VolumeRAM_3xInt16*>(volume))     return "3xInt16";
    if (dynamic_cast<const VolumeRAM_3xInt32*>(volume))     return "3xInt32";
    if (dynamic_cast<const VolumeRAM_3xInt64*>(volume))     return "3xInt64";
    if (dynamic_cast<const VolumeRAM_3xFloat*>(volume))     return "3xFloat";
    if (dynamic_cast<const VolumeRAM_3xDouble*>(volume))    return "3xDouble";

    if (dynamic_cast<const VolumeRAM_4xUInt8*>(volume))     return "4xUInt8";
    if (dynamic_cast<const VolumeRAM_4xUInt16*>(volume))    return "4xUInt16";
    if (dynamic_cast<const VolumeRAM_4xUInt32*>(volume))    return "4xUInt32";
    if (dynamic_cast<const VolumeRAM_4xUInt64*>(volume))    return "4xUInt64";
    if (dynamic_cast<const VolumeRAM_4xInt8*>(volume))      return "4xInt8";
    if (dynamic_cast<const VolumeRAM_4xInt16*>(volume))     return "4xInt16";
    if (dynamic_cast<const VolumeRAM_4xInt32*>(volume))     return "4xInt32";
    if (dynamic_cast<const VolumeRAM_4xInt64*>(volume))     return "4xInt64";
    if (dynamic_cast<const VolumeRAM_4xFloat*>(volume))     return "4xFloat";
    if (dynamic_cast<const VolumeRAM_4xDouble*>(volume))    return "4xDouble";

    return "<unknown>";
}

QPixmap VolumeViewHelper::generatePreview(const VolumeBase* volume, int height) {
    return generateBorderedPreview(volume, height, 0);
}

QPixmap VolumeViewHelper::generateBorderedPreview(const VolumeBase* handle, int height, int border) {

    VolumePreview* prev = handle->getDerivedData<VolumePreview>();
    int internHeight = prev->getHeight();

    QImage origImg = QImage(internHeight, internHeight, QImage::Format_ARGB32);
    for (int y=0; y<internHeight; y++) {
        for (int x=0; x<internHeight; x++) {
            int previewIndex = y * internHeight + x;
            int greyVal = prev->getData()[previewIndex];
            origImg.setPixel(x, y, qRgb(greyVal, greyVal, greyVal));
        }
    }
    QImage* preview = new QImage(origImg.scaled(height, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    // draw border
    for (int y = 0; y < preview->height(); y++){
        for (int x = 0; x < border; x++)
            preview->setPixel(preview->width()-x-1, y, qRgb(255, 255, 255));
    }
    for (int x = 0; x < preview->width(); x++){
        for (int y = 0; y < border; y++)
               preview->setPixel(x, preview->height()-y-1, qRgb(255, 255, 255));
    }

    QPixmap pixmap = QPixmap::fromImage(*preview);
    delete preview;

    return pixmap;
}

std::string VolumeViewHelper::volumeInfoString(const VolumeBase* handle) {
    std::string outString;
    if (handle && handle->getRepresentation<VolumeRAM>()) {

        const VolumeRAM* volume = handle->getRepresentation<VolumeRAM>();

        std::string filename = getStrippedVolumeName(handle);
        if(filename.length() > 13) {
            QString ext = QString::fromStdString(filename);
            filename.resize(13);
            filename+="...";
            filename+=ext.section('.',-1).toStdString();
        }
        std::string spacing = getVolumeSpacing(handle);
        std::string dimension = getVolumeDimension(handle);
        std::string type = getVolumeType(volume);
        outString+="<style type='text/css'> table {margin-left:0px; margin-top:1px; font-size: 9px;}</style><table><tr><td>"+filename+"</td><td></td></tr><tr><td>type </td><td>" \
            +type+"</td></tr><tr><td>dimension </td><td>" \
            +dimension+"</td></tr><tr><td>spacing </td><td>" \
            +spacing+"</td></tr><tr><td>memSize </td><td>" \
    +getVolumeMemorySize(volume)+" bytes</td></tr> </table>";

    }
    else {
        outString = "<strong>Volume:</strong> no Volume";
    }
    return outString;
}

std::string VolumeViewHelper::getVolumeName(const VolumeBase* handle){
    std::string volumeName;
    if (handle) {
        const Volume* vh = dynamic_cast<const Volume*>(handle);
        if(vh)
            volumeName = vh->getOrigin().getPath();
    }
    else {
        volumeName = "no Volume";
    }
    return volumeName;
}

std::string VolumeViewHelper::getStrippedVolumeName(const VolumeBase* handle) {
    if (!handle)
        return "no volume";
    else {
        std::string volumePath = getVolumeName(handle);

        //  remove trailing slash (dicom)
        std::string::size_type separator = volumePath.find_last_of("/\\");
        if (separator == volumePath.size()-1)
            volumePath = volumePath.substr(0, volumePath.size()-1);

        return tgt::FileSystem::fileName(volumePath);
    }
}

std::string VolumeViewHelper::getVolumePath(const VolumeBase* handle) {
    std::string volumeName;
    if (handle) {
        const Volume* vh = dynamic_cast<const Volume*>(handle);
        if(vh) {
            QString fn = QString::fromStdString(vh->getOrigin().getPath());
            volumeName = fn.section('/',0, -2).toStdString();
        }
    }
    else {
        volumeName = "no Volume";
    }
    return volumeName;

}

std::string VolumeViewHelper::getVolumeDimension(const VolumeBase* volume) {
    std::stringstream out;

    out << volume->getDimensions()[0] << " x " << volume->getDimensions()[1] << " x " << volume->getDimensions()[2];

    return out.str();
}

std::string VolumeViewHelper::getVolumeSpacing(const VolumeBase* volume) {
    std::stringstream out;
    out << volume->getSpacing()[0] << " x " << volume->getSpacing()[1] << " x " << volume->getSpacing()[2];
    return out.str();
}

std::string VolumeViewHelper::getVolumeMemorySize(const VolumeRAM* volume) {
    std::stringstream out;

    size_t bytes = volume->getNumBytes();
    float mb = tgt::round(bytes/104857.6f) / 10.f;    //calculate mb with 0.1f precision
    float kb = tgt::round(bytes/102.4f) / 10.f;
    if (mb >= 0.5f) {
        out << mb << " MB";
    }
    else if (kb >= 0.5f) {
        out << kb << " kB";
    }
    else {
        out << bytes << " bytes";
    }
    return out.str();
}


size_t VolumeViewHelper::getVolumeMemorySizeByte(const VolumeRAM* volume) {
    size_t volumeBytes = 0;

    volumeBytes = volume->getNumBytes();
    return volumeBytes;

}

} // namespace
