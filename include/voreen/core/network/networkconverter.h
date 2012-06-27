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

#ifndef VRN_NETWORKCONVERTER_H
#define VRN_NETWORKCONVERTER_H

#include <string>

class TiXmlElement;
class TiXmlNode;

namespace voreen {

/**
 * This abstract class serves as the base of several incrementally working NetworkConverters. Each NetworkConverter should change
 * the Network only by one version. This way it isn't necessary to write n new Converters if a network change occurs, but
 * only one - from the latest but one to the now latest.</br>
 * In this class there are some basic tools necessary to manipulate the TiXmlElement itself. A subclass should only
 * overwrite the NetworkConverter::convert(TiXmlElement*) method and don't add any other public methods.</br>
 * some outside method should care about specific version numbers and which NetworkConverter's method is to be called.</br>
 * Note: As far as this system is implemented now, the NetworkConverter doesn't check, if the version is really the one
 * it is meant for. So it might get jumbled up if the supervising class makes an error.</br>
 * This class presents one method called NetworkConverter::changeVersion(TiXmlNode*, int, int) which should be
 * called at the end of the overwritten NetworkConverter::convert(TiXmlElement*) method with the appropriate parameters.
 */
class NetworkConverter {
public:

    virtual ~NetworkConverter() {}

    /**
     * <b>The</b> method of this class. It takes the whole document and manipulates it, so it is viable for the new version it is
     * intended for. At the end of this method you have to call NetworkConverter::changeVersion(TiXmlNode*, int, int) with the
     * correct parameters, so the TiXmlElement will be consistent
     * \param elem The TiXmlElement which should be converted to a new version
     */
    virtual void convert(TiXmlElement* elem) = 0;

protected:
    void changePropertyName(TiXmlElement* elem, const std::string& ofProcessor, const std::string& from, const std::string& to);
    void changePropertyValue(TiXmlElement* elem, const std::string& ofProcessor, const std::string& from, const std::string& to);
    void changeProcessorType(TiXmlElement* elem, const std::string& from, const std::string& to);

    static const std::string loggerCat_; ///< The logging information used in LINFO etc.
};

//-------------------------------------------------------------------------------------------------

/**
 * The @c NetworkConverter4to5 converts the serialized XML data for a
 * version 4 @c ProcessorNetwork to XML data for a version 5 @c ProcessorNetwork.
 *
 * @see NetworkConverter
 */
class NetworkConverter4to5 : public NetworkConverter {
public:
    virtual void convert(TiXmlElement* elem);
};

/**
 * The @c NetworkConverter4to5 converts the serialized XML data for a
 * version 4 @c ProcessorNetwork to XML data for a version 5 @c ProcessorNetwork.
 *
 * @see NetworkConverter
 */
class NetworkConverter5to6 : public NetworkConverter {
public:
    virtual void convert(TiXmlElement* elem);
};

} // namespace

#endif // VRN_NETWORKCONVERTER_H
