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

#ifndef Q_DEBUG_H
#define Q_DEBUG_H

#include <iostream>
#include <ostream>
#include <sstream>

#include <streambuf>

#include <QTextEdit>

class QDebugStream: public std::basic_streambuf< char, std::char_traits< char > > {
  typedef
    std::basic_streambuf< char, std::char_traits< char > >
      ::
        int_type
    int_type
  ;
  typedef std::char_traits< char > traits_t;
public:

	void setConsole(QTextEdit* console) {
		console_ = console;
	}

	virtual int_type overflow(int_type v) {
		if (v == '\n') {
			console_->append(infoStr_.c_str());
			infoStr_.clear();
		} else infoStr_.push_back(v);
		return v;
	}

	virtual std::streamsize xsputn(const char *p, std::streamsize n) {
		infoStr_.append(p, p + n);
		size_t pos = 0;
		while (pos != std::string::npos) {
			pos = infoStr_.find('\n');
			if (pos != std::string::npos) {
				std::string tmp(infoStr_.begin(), infoStr_.begin() + pos);
				console_->append(tmp.c_str());
				console_->ensureCursorVisible();
				infoStr_.erase(infoStr_.begin(), infoStr_.begin() + pos + 1);
			}
		}
		return n;
	}

private:
	QTextEdit*  console_;
	std::string infoStr_;
}; 
#endif
