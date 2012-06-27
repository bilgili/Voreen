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

#ifndef VRN_VOREENMODULE_H
#define VRN_VOREENMODULE_H

#include <string>
#include <vector>
#include "voreen/core/utils/exception.h"

namespace voreen {

class VoreenApplication;
class Processor;
class SerializableFactory;
class VolumeReader;
class VolumeWriter;

class VoreenModule {

    friend class VoreenApplication;

public:
    VoreenModule();
    virtual ~VoreenModule();

    virtual std::string getDescription() const {
        return "";
    }

    std::string getName() const;
    const std::vector<Processor*>& getProcessors() const;
    const std::vector<VolumeReader*>& getVolumeReaders() const;
    const std::vector<VolumeWriter*>& getVolumeWriters() const;
    const std::vector<SerializableFactory*>& getSerializerFactories() const;
    const std::vector<std::string>& getShaderPaths() const;

protected:
    virtual void initialize()
        throw (VoreenException);
    virtual void deinitialize()
        throw (VoreenException);

    void setName(const std::string& name);
    void addProcessor(Processor* processor);
    void addVolumeReader(VolumeReader* reader);
    void addVolumeWriter(VolumeWriter* writer);
    void addSerializerFactory(SerializableFactory* factory);
    void addShaderPath(const std::string& path);

    static const std::string loggerCat_;

private:
    std::string name_;
    std::vector<Processor*> processors_;
    std::vector<VolumeReader*> volumeReaders_;
    std::vector<VolumeWriter*> volumeWriters_;
    std::vector<SerializableFactory*> serializerFactories_;
    std::vector<std::string> shaderPaths_;

};

} // namespace

#endif // VRN_VOREENMODULE_H
