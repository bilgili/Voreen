/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2008 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#include "memorymanager.h"

#if defined(TGT_DEBUG) && defined(__GNUC__) && defined(TGT_USE_MEMORY_MANAGER)

#include <sstream>
#include <fstream>

#include "tgt/assert.h"

namespace tgt {

std::string MemoryManager::MemoryManagerValue::toString(bool free) const {
    std::ostringstream oss;

    if (!free) {
        switch (allocInfo_) {
        case (NEW):
            oss << "new";
            break;
        case (ARRAY_NEW):
            oss << "new[]";
            break;
        default:
            tgtAssert(false, "Illegal enum-value");
            break;
        }
    }
    else {
        switch (allocInfo_) {
        case (NEW):
            oss << "delete";
            break;
        case (ARRAY_NEW):
            oss << "delete[]";
            break;
        default:
            tgtAssert(false, "Illegal enum-value");
            break;
        }
    }

    oss << " in function at address " << caller_ << ". Call #" << callNumber_;

    return oss.str();
}

/*
    init static stuff
*/

bool    MemoryManager::isReady_             = false;
uint    MemoryManager::breakpoints_[10]     = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint    MemoryManager::callCounter_         = 0;
size_t  MemoryManager::breakpointCounter_   = 0;
MemoryManager::MAP  MemoryManager::map_     = MAP();
volatile long       MemoryManager::lock_    = 0;

void MemoryManager::init() {
    isReady_ = true;
}

void MemoryManager::deinit() {
    isReady_ = false;

    std::ofstream mem_leaks("mem_leaks");
    std::ofstream calls("calls");

    if (map_.empty()) {
        std::cout << std::endl << std::endl << "Tiny Graphics Toolbox has not found any memory leaks :)" << std::endl;
        return;
    }

    std::cout << std::endl << "WARNING: Tiny Graphics Toolbox has found memory leaks." << std::endl;
    std::cout << "Please run" << std::endl;
    std::cout << "\t$ tgtleakfinder <EXECUTABLE>" << std::endl;

    for (MAP::iterator iter = map_.begin(); iter != map_.end(); ++iter) {
        // skip infos in unknown locations
        //Skip infos in paths starting '/' or '\' (these things are bugs, or strange or magic things in libs)
        mem_leaks << iter->second.caller_ << std::endl;
        calls  << iter->second.callNumber_ << std::endl;
/*        if (iter->second.line_ <= 0 || iter->second.filename_[0] == '/' || iter->second.filename_[0] == '\\') {
            mem_leaks_full  << "   " << iter->second.toString() << " at address: " << (uintptr_t) iter->first << std::endl;
        }
        else {
            mem_leaks           << "   " << iter->second.toString() << " at address: " << (uintptr_t) iter->first << std::endl;
            mem_leaks_full      << "   " << iter->second.toString() << " at address: " << (uintptr_t) iter->first << std::endl;
        }*/
    }

    calls.close();
    mem_leaks.close();
}

std::string MemoryManager::toString() {
    std::ostringstream oss;
    MAP::iterator iter = map_.begin();
    while (iter != map_.end()) {
        oss << "   " << iter->second.toString() << std::endl;
        ++iter;
    }
    return oss.str();
}

void MemoryManager::add(void* p, AllocInfo info, void* caller) {
    if (!isReady_ || lock_ > 0)
        return;

    volatile MemoryManager::MemoryManagerLock lock;

    MemoryManagerValue val(info, caller, callCounter_);
    map_[p] = val;
}

bool MemoryManager::remove(void* p, AllocInfo info, void* caller) {
    if (!isReady_ || lock_ > 0)
        return true;

    volatile MemoryManagerLock lock;

    MAP::iterator iter = map_.find(p);

    if (iter == map_.end()) {
        switch (info) {
        case (NEW):
            std::cout << "ERROR: 'delete' used with free memory!!!" << std::endl;
            std::cout << "   delete called from " << caller << " at address: " << p << std::endl;
            break;
        case (ARRAY_NEW):
            std::cout << "ERROR: 'delete[]' used with free memory!!!" << std::endl;
            std::cout << "   delete[] called from " << caller << " at address: " << p << std::endl;
            break;
        default:
            tgtAssert(false, "Illegal enum-value");
            break;
        }
        return false;
    }

    if (iter->second.allocInfo_ == info) {
        map_.erase(iter);
        return true;
    }

    if ((iter->second.allocInfo_ == NEW)       && (info == ARRAY_NEW))
        std::cout << "ERROR: You called 'new' but deleted with 'delete[]'" << std::endl;
    if ((iter->second.allocInfo_ == ARRAY_NEW) && (info == NEW))
        std::cout << "ERROR: You called 'new[]' but deleted with 'delete'" << std::endl;

    std::cout << "   " << iter->second.toString() << std::endl;
    std::cout << "   " << MemoryManagerValue(info, caller, callCounter_).toString(true) << std::endl;

    map_.erase(iter);

    return false;
};

} //namespace tgt

// New implementation of global new, delete and new[] and delete[]

void* operator new(size_t size) {
    ++tgt::MemoryManager::callCounter_;

    void* p = malloc(size);
    // Is here a breakpoint?
    for (size_t i = 0; i < tgt::MemoryManager::breakpointCounter_; ++i) {
        if (tgt::MemoryManager::breakpoints_[i] == tgt::MemoryManager::callCounter_) {
            std::cout << "Breakpoint at adress: " << uintptr_t(p) << std::endl;
            TGT_THROW_BREAKPOINT;
            break;
        }
    }

    if (tgt::MemoryManager::isReady_)
        tgt::MemoryManager::add(p, tgt::MemoryManager::NEW, __builtin_return_address(0));

    return p;
};

void* operator new[](size_t size) {
  ++tgt::MemoryManager::callCounter_;

    void* p = malloc(size);
    //Has been set a breakpoint here?
    for (size_t i = 0; i < tgt::MemoryManager::breakpointCounter_; ++i) {
        if (tgt::MemoryManager::breakpoints_[i] == tgt::MemoryManager::callCounter_) {
            std::cout << "Breakpoint at adress: " << uintptr_t(p) << std::endl;
            TGT_THROW_BREAKPOINT;
            break;
        }
    }

    if (tgt::MemoryManager::isReady_)
        tgt::MemoryManager::add(p, tgt::MemoryManager::ARRAY_NEW, __builtin_return_address(0));

    return p;
}

void operator delete(void* p) {
    if (tgt::MemoryManager::isReady_) {
        if (p == NULL) {
            std::cout << "ERROR. You try to delete a NULL-pointer." << std::endl;
            std::cout << " in function at address " <<  __builtin_return_address(0) << ". Call #" << tgt::MemoryManager::callCounter_ << std::endl;
            return;
        }
        tgt::MemoryManager::remove(p, tgt::MemoryManager::NEW,  __builtin_return_address(0));
    }

    free(p);
}

void operator delete[](void* p) {
    if (tgt::MemoryManager::isReady_) {
        if (p == NULL) {
            std::cout << "ERROR. You try to delete[] a NULL-pointer." << std::endl;
            std::cout << " in function at address " <<  __builtin_return_address(0) << ". Call #" << tgt::MemoryManager::callCounter_ << std::endl;
            return;
        }
        tgt::MemoryManager::remove(p, tgt::MemoryManager::ARRAY_NEW,  __builtin_return_address(0));
    }

    free(p);
}

#endif //if defined(TGT_DEBUG) && defined(__GNUC__) && defined(TGT_USE_MEMORY_MANAGER)
