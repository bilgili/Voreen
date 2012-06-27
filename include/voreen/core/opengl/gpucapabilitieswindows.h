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

#ifndef VRN_GPUCAPABILITIESWINDOWS_H
#define VRN_GPUCAPABILITIESWINDOWS_H

#include "tgt/gpucapabilities.h"

#ifdef VRN_WITH_WMI
    #include <wbemidl.h>
    #include <comdef.h>
#endif

namespace voreen {

/**
 * A singleton that performs Windows specific hardware detection. It provides
 * information about: 
 *  - Graphics driver version and date
 *  - Driver settings
 *  - Size of graphics memory
 *
 * The information is collected from several sources like
 * Windows Management Instrumentation (WMI), Windows registry,
 * and driver dlls.
 *
 * @note WMI is only used if VRN_WITH_WMI is defined
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
     * Holds information about graphics driver
     * settings.
     */
    struct GraphicsDriverSettings {
        bool verticalSync;      ///< Is vertical synchronization enabled?
        bool tripleBuffering;   ///< Is triple buffering enabled?
    };

    /**
     * Access the class' singleton by this function.
     * The singleton is instantiated when this function
     * is called the first time.
     *
     * @warning Instantiating the singleton before OpenGL is
     *      is initialized can lead to unpredictable results.
     */
    static GpuCapabilitiesWindows* getInstance();

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
     * If voreen was compiled without WMI support (VRN_WITH_WMI),
     * 0 is returned.
     */
    int getVideoRamSize();
    
    /**
     * Returns information about graphics driver settings. 
     * At this time this is the state of vertical synchronization
     * and triple buffering only.
     *
     * @note The information is retrieved from the Windows 
     *      registry for NVIDIA and ATI GPUs only. For other
     *      vendors, false is returned for both settings.
     *      Futhermore, the registry keys containing these settings
     *      are not documented and can be expected to change in the future.
     *      In short: Do not rely on this function's results too much!
     */
    GraphicsDriverSettings getDriverSettings();

protected:
    /**
     * Protected constructor called by getInstance().
     * Also performs the WMI security initialization,
     * if compiled with WMI support (VRN_WITH_WMI).
     */
    GpuCapabilitiesWindows();

    std::string loggerCat_;

#ifdef VRN_WITH_WMI
    
    /**
     * Has to be called before a \sa WMIquery() can be done.
     *
     * @return true, if WMI initialization succeeded
     *
     * @note call \sa WMIdeinit() after finished wmi queries and
     *      before calling \sa WMIinit() again
     *
     * @warning function is only defined if VRN_WITH_WMI is set!
     */
    bool WMIinit();
    
    /**
     * Closes the WMI connection. Call this after finished wmi queries.
     *
     * @return true, if WMI deinitialization succeeded
     *
     * @warning function is only defined if VRN_WITH_WMI is set!
     */
    bool WMIdeinit();
    
    /**
     * This function is used to query the Windows
     * Management Instrumentation (WMI).
     *
     * @see http://msdn2.microsoft.com/en-us/library/aa394582(VS.85).aspx
     *
     * @return the query result
     *
     * @note You have to call \sa WMIinit() before using this function.
     *
     * @warning function is only defined if VRN_WITH_WMI is set!
     */
    std::string WMIquery(std::string wmiclass, std::string attribute);

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

    // Singleton reference
    static GpuCapabilitiesWindows* instance_;

    // Internal helper functions
    bool parseFileVersionString(FileVersion &fileVersion);
    bool createFileVersionFromDigits(FileVersion &fileVersion);
    bool createVersionStringFromDigits(FileVersion &fileVersion);

#ifdef VRN_WITH_WMI
   // WMI internals. Should not be touched outside the present WMI functions.
   IWbemLocator* pIWbemLocator_;
   IWbemServices* pWbemServices_;
   bool WMISecurityInitialized_;
#endif

};

} // namespace voreen

#endif // VRN_GPUCAPABILITIESWINDOWS_H
