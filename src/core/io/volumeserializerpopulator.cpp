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

#include "voreen/core/io/volumeserializerpopulator.h"

#include "voreen/core/io/datvolumereader.h"
#include "voreen/core/io/datvolumewriter.h"
#include "voreen/core/io/nrrdvolumereader.h"
#include "voreen/core/io/nrrdvolumewriter.h"
#include "voreen/core/io/quadhidacvolumereader.h"
#include "voreen/core/io/rawvolumereader.h"
#include "voreen/core/io/tuvvolumereader.h"
#include "voreen/core/io/interfilevolumereader.h"

#include "voreen/core/io/volumeserializer.h"

#ifdef VRN_WITH_DCMTK
    #include "voreen/core/io/dicomvolumereader.h"
#endif

#ifdef VRN_WITH_MATLAB
#ifdef VRN_MODULE_GLYPHS_MESH
    #include "voreen/modules/glyphs_mesh/matcontourreader.h"
#else
    #include "voreen/core/io/matvolumereader.h"
#endif
#endif

#ifdef VRN_WITH_PVM
    #include "voreen/core/io/pvmvolumereader.h"
#endif

#ifdef VRN_WITH_TIFF
    #include "voreen/core/io/tiffvolumereader.h"
#endif

#ifdef VRN_WITH_ZIP
    #include "voreen/core/io/zipvolumereader.h"
#endif


namespace voreen {

/*
    constructor and destructor
*/

VolumeSerializerPopulator::VolumeSerializerPopulator(IOProgress* progress /*= 0*/)
  : progress_(progress),
    vs_ ( new VolumeSerializer() )
{
    /*
        populate array with all known VolumeReaders
        --> if an FormatClashException occurs here it is an error in this method
    */
    readers_.push_back( new DatVolumeReader(progress_) );
    readers_.push_back( new NrrdVolumeReader() );
    readers_.push_back( new QuadHidacVolumeReader() );
    readers_.push_back( new RawVolumeReader(progress_) );
    readers_.push_back( new TUVVolumeReader() );
    readers_.push_back( new InterfileVolumeReader() );

#ifdef VRN_WITH_DCMTK
    readers_.push_back( new DicomVolumeReader() );
#endif

#ifdef VRN_WITH_MATLAB
#ifdef VRN_MODULE_GLYPHS_MESH
    readers_.push_back( new MatContourReader() );
#else
    readers_.push_back( new MatVolumeReader() );
#endif
#endif

#ifdef VRN_WITH_PVM
    readers_.push_back( new PVMVolumeReader(progress_) );
#endif

#ifdef VRN_WITH_TIFF
    readers_.push_back( new TiffVolumeReader(progress_) );
#endif

#ifdef VRN_WITH_ZIP
    readers_.push_back( new ZipVolumeReader(progress_) );
#endif

    for (size_t i = 0; i < readers_.size(); ++i)
        vs_->registerReader( readers_[i] );

    /*
        populate array with all known VolumeWriters
        --> if an FormatClashException occurs here it is an error in this method
    */
    writers_.push_back( new DatVolumeWriter() );
    writers_.push_back( new NrrdVolumeWriter() );

    for (size_t i = 0; i < writers_.size(); ++i)
        vs_->registerWriter( writers_[i] );
}

VolumeSerializerPopulator::~VolumeSerializerPopulator() {
    delete vs_;

    for (size_t i = 0; i < readers_.size(); ++i)
        delete readers_[i];

    for (size_t i = 0; i < writers_.size(); ++i)
        delete writers_[i];
}

/*
    getter
*/

VolumeSerializer* VolumeSerializerPopulator::getVolumeSerializer() {
    return vs_;
}

} // namespace voreen
