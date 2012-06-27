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

#ifndef TGT_GPUCAPABILITIESWINDOWS_H
#define TGT_GPUCAPABILITIESWINDOWS_H

#include "gpucapabilities.h"
#include "tgt/singleton.h"

#ifdef TGT_WITH_WMI
    #include <wbemidl.h>
    #include <comdef.h>
#endif

namespace tgt {

/**
 * A singleton that performs Windows specific hardware detection. It provides
 * information about: 
 *  - Graphics driver version and date
 *  - Size of graphics memory
 *  - Driver settings
 *
 * The information is collected from several sources like
 * Windows Management Instrumentation (WMI), Windows registry,
 * and driver dlls.
 *
 * @note WMI is only used if TGT_WITH_WMI is defined
 */
class GpuCapabilitiesWindows : public tgt::GpuCapabilities {
public:
    
    /**
     * Holds several representations of a Windows file version.
     *
     * Windows file versions are usually represented as a string 
     * of the form d1.d2.d3.d4 where d1,d2,d3,d4 are 16-bit unsigned integers,
     * e.g. 6.14.10.6389.
     */
    struct FileVersion {
        int d1, d2, d3, d4;             ///< Parts d1,d2,d3,d4 of the file version, separated
        std::string versionString;      ///< String representation of file version: 'd1.d2.d3.d4'
        uint64_t version;               ///< Binary representation of file version: d1d2d3d4
    };

    /**
     * Contains the graphics driver's version and date.
     */
    struct GraphicsDriverInformation {
        FileVersion driverVersion;      ///< Version of driver dll (the dll is vendor specific)
        std::string driverDate;         ///< Date of last modification of driver dll, format: YYYY-MM-DD
    };

    /**
     * Constructor performs the WMI initialization,
     * if compiled with WMI support (TGT_WITH_WMI).
     */
    GpuCapabilitiesWindows();

    /**
     * The destructor closes the WMI connection, if it is open.
     */
    virtual ~GpuCapabilitiesWindows();

    /**
     * Returns information about the graphics driver, 
     * retrieved from driver dlls (NVIDIA and ATI only).
     *
     * The Windows Management Instrumentation (WMI) is used as 
     * fallback option only since it turned out to be less reliable than
     * querying the graphics driver directly.
     */
    GraphicsDriverInformation getGraphicsDriverInformation();
    
    /**
     * @see getGraphicsDriverInformation
     */
    FileVersion getDriverVersion();
    
    /**
     * @see getGraphicsDriverInformation
     */
    std::string getDriverDate();
    
    /**
     * Returns the size of the graphics memory in MB. This
     * information is obtained by querying the 
     * Windows Management Instrumentation (WMI), hence it is
     * vendor independent.
     *
     * If voreen was compiled without WMI support (TGT_WITH_WMI) or the WMI data retrieval fails,
     * the information is read from the registry. If both methods fail, -1 is returned.
     */
    int getVideoRamSize();
    
    virtual void logCapabilities(bool extensionsString = false, bool osString = true);

protected:
    
    std::string loggerCat_;

#ifdef TGT_WITH_WMI
    
    /**
     * Has to be called before a \sa WMIquery() can be done.
     *
     * @return true, if WMI initialization succeeded
     *
     */
    bool WMIinit();
    
    /**
     * Closes the WMI connection. Call this after finished wmi queries.
     *
     * @return true, if WMI deinitialization succeeded
     *
     * @warning function is only defined if TGT_WITH_WMI is set!
     */
    bool WMIdeinit();
    
    /**
     * Returns whether the WMI connection has been setup.
     */
    bool isWMIinited() const;

    /**
     * This function is used to query the Windows
     * Management Instrumentation (WMI).
     *
     * @see http://msdn2.microsoft.com/en-us/library/aa394582(VS.85).aspx
     *
     * @return the query result as a pointer to a VARIANT type, 
     *         or NULL if the query was unsuccessful. The return value has to be freed
     *         by the caller by \c VariantClear().
     *
     * @note Instead of calling this function directly, use \sa WMIqueryStr and \sa WMIqueryInt,
     *       which return result values of a specific type.
     */
    VARIANT* WMIquery(std::string wmiclass, std::string attribute);

    /**
     * WMI query returning a string. \sa WMIquery.
     */
    std::string WMIqueryStr(std::string wmiclass, std::string attribute);

    /**
     * WMI query returning an int. \sa WMIquery.
     */
    int WMIqueryInt(std::string wmiclass, std::string attribute);

    /**
     * Converts a STL string to a STL wide-string.
     */
    static std::wstring str2wstr(const std::string& s);

    /**
     * Converts a STL wide-string to a STL string.
     */
    static std::string wstr2str(const std::wstring& s);

#endif
    
    /**
     * Detects the Windows file version of a file.
     *
     * @param filename complete path to the file 
     *
     * @see FileVersion
     */
    FileVersion getFileVersion(std::string filename);
    
    /**
     * Detects the date of a file.
     *
     * @param filename complete path to the file 
     *
     * @return file date as string in the form YYYY-MM-DD, e.g. 2007-12-15
     */
    std::string getFileDate(std::string filename);


private:

    // Internal helper functions
    bool parseFileVersionString(FileVersion &fileVersion);
    bool createFileVersionFromDigits(FileVersion &fileVersion);
    bool createVersionStringFromDigits(FileVersion &fileVersion);

    DWORD readVRAMSizeFromReg();
    char* findPrimaryDevicesKey();

#ifdef TGT_WITH_WMI
   // WMI internals. Should not be touched outside the present WMI functions.
   IWbemLocator* pIWbemLocator_;
   IWbemServices* pWbemServices_;
#endif

    int videoRamSize_;

};

} // namespace tgt

#define GpuCapsWin tgt::Singleton<tgt::GpuCapabilitiesWindows>::getRef()

#endif // TGT_GPUCAPABILITIESWINDOWS_H
