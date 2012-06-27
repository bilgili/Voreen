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

#ifndef VRN_SYMBOLTABLE_H
#define VRN_SYMBOLTABLE_H

#include "voreen/core/utils/GLSLparser/symbol.h"

#include <map>

namespace voreen {

namespace glslparser {

template<class T = Symbol>
class SymbolTable {
public:
    SymbolTable();
    SymbolTable(SymbolTable<T>* const parent);

    virtual ~SymbolTable();

    const std::map<std::string, T*>& getSymbolsMap() const { return symbols_; }
    bool empty() const { return symbols_.empty(); }

    /**
     * Removes and deletes the symbol with the given name from the table
     * if it exists.
     */
    bool deleteSymbol(const std::string& symbol);

    /**
     * Return a pointer to the symbol with the given name from the table
     * if it exists.
     */
    T* findSymbol(const std::string& symbol) const;

    /**
     * Inserts the given symbol into the table, if it is not NULL and if it
     * is not already present.
     */
    bool insertSymbol(T* const symbol);

    /**
     * Removes the given symbol from the table without deleting it.
     */
    bool removeSymbol(T* const symbol);

protected:
    SymbolTable<T>* parent_;
    //std::list<SymbolTable<T>*> children_;

    typedef std::map<std::string, T*> SymbolMap;
    SymbolMap symbols_;
};

// ----------------------------------------------------------------------------

template<class T>
SymbolTable<T>::SymbolTable()
    : parent_(0)
{
}

template<class T>
SymbolTable<T>::SymbolTable(SymbolTable<T>* const parent)
    : parent_(parent)
{
}

template<class T>
SymbolTable<T>::~SymbolTable() {
    for (typename SymbolMap::iterator it = symbols_.begin(); it != symbols_.end(); ++it)
        delete it->second;
}

template<class T>
bool SymbolTable<T>::deleteSymbol(const std::string& symbol) {
    if (! symbol.empty()) {
        typename SymbolMap::iterator it = symbols_.find(symbol);
        if (it != symbols_.end()) {
            delete it->second;
            symbols_.erase(it);
            return true;
        }
    }

    return false;
}

template<class T>
T* SymbolTable<T>::findSymbol(const std::string& symbol) const {

    typename SymbolMap::const_iterator it = symbols_.find(symbol);
    if (it != symbols_.end())
        return it->second;
    else if (parent_ != 0)
        return parent_->findSymbol(symbol);
    else
        return 0;
}

template<class T>
bool SymbolTable<T>::insertSymbol(T* const symbol) {
    if (symbol == 0)
        return false;

    std::pair<typename SymbolMap::iterator, bool> res = symbols_.insert(std::make_pair(symbol->getID(), symbol));
    return res.second;
}

template<class T>
bool SymbolTable<T>::removeSymbol(T* const symbol) {
    if ((symbol != 0) && (symbols_.find(symbol->getID()) != symbols_.end())) {
        symbols_.erase(symbol->getID());
        return true;
    }

    return false;
}

}   // namespace glslparser

}   // namespace voreen

#endif // VRN_SYMBOLTABLE_H
