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

#ifndef VRN_TEXTFILEREADER_H
#define VRN_TEXTFILEREADER_H

#include <fstream>
#include <string>

namespace voreen {

/**
 * Helper for parsing simple text files as used by the <tt>.dat</tt> file format.
 *
 * The supported format is:<br>
 * <tt>[whitespace ...] <type> <separator ...> <args> [whitespace ...]</tt>
 */
class TextFileReader {
public:
    /**
     * Opens a text file.
     * @param filename Name of file.
     */
    TextFileReader(const std::string& filename);

    /**
     * Reads from a text file stream.
     */
    TextFileReader(std::istream* stream);

    virtual ~TextFileReader();

    /**
     * Returns the next line with valid content.
     *
     * Try to get the next valid line. Skip empty lines and lines starting with a comment
     * character. Treats the first word of a line as \a type and returns the remaining after
     * the separator(s) as \a args. Convert the \a type to lowercase.
     *
     * @param type stores the returned type
     * @param args stores the returned args
     * @param toLowercase convert \a type to lowercase
     *
     * @return true if a valid line was read.
     */
    bool getNextLine(std::string& type, std::string& arg,
                     bool toLowercase = true);

    /**
     * Convenience method which returns \a arg as a \a istringstream.
     */
    bool getNextLine(std::string& type, std::istringstream& arg,
                     bool toLowercase = true);

    /**
     * Checks validity of the reader.
     */
    bool operator!();

    /**
     * Sets the character(s) used for commenting out a line in the file. Only a single
     * character is checked, so "//" is not possible.
     */
    void setCommentChars(const std::string& commentChars);

    /**
     * Sets the field separators.
     */
    void setSeparators(const std::string& separators);

    /**
     * Sets what is considered as whitespace
     */
    void setWhitespace(const std::string& whitespace);

    std::string getMagicNumber();

protected:
    std::string whitespace_;
    std::string commentChars_;
    std::string separators_;
    bool isMyOwnStream_;
    std::istream* file_;
};

} // namespace voreen

#endif // VRN_TEXTFILEREADER_H
