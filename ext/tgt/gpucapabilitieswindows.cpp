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

#ifdef WIN32

#include "gpucapabilitieswindows.h"

#include <windows.h>
#include <WinBase.h>
#include <tchar.h>
#include <strsafe.h>

#include <iostream>
#include <sstream>

const std::string DRIVER_DLL_NVIDIA = "nvoglnt.dll";
const std::string DRIVER_DLL_ATI = "atioglxx.dll";

namespace tgt {

GpuCapabilitiesWindows::GpuCapabilitiesWindows()
    : loggerCat_("tgt.GpuCapabilitiesWindows")
    , GpuCapabilities()
    , videoRamSize_(-1)
#ifdef TGT_WITH_WMI
    , pIWbemLocator_(0)
    , pWbemServices_(0)
#endif
{
#ifdef TGT_WITH_WMI
    if (!WMIinit()) {
        LWARNING("WMI initialization failed");
    }
#endif
}

void GpuCapabilitiesWindows::logCapabilities(bool extensionsString, bool osString) {
    GpuCapabilities::logCapabilities(extensionsString, osString);
    
    GraphicsDriverInformation driverInfo = getGraphicsDriverInformation();
    LINFO("Graphics Driver Version: " << driverInfo.driverVersion.versionString);
    LINFO("Graphics Driver Date:    " << driverInfo.driverDate);
    
    LINFO("Graphics Memory Size:    " << getVideoRamSize() << " MB");

}

GpuCapabilitiesWindows::~GpuCapabilitiesWindows() {
#ifdef VRN_WITH_WMI
    if (isWMIinited())
        WMIdeinit();
#endif 
}

GpuCapabilitiesWindows::GraphicsDriverInformation GpuCapabilitiesWindows::getGraphicsDriverInformation() {
    GraphicsDriverInformation driverInfo;
    driverInfo.driverVersion = getDriverVersion();
    if (driverInfo.driverVersion.versionString.length() == 0) {
        driverInfo.driverVersion.d1 = 0;
        driverInfo.driverVersion.d2 = 0;
        driverInfo.driverVersion.d3 = 0;
        driverInfo.driverVersion.d4 = 0;
        driverInfo.driverVersion.version = 0;
    }
    driverInfo.driverDate = getDriverDate();
    
    return driverInfo;
}


GpuCapabilitiesWindows::FileVersion GpuCapabilitiesWindows::getDriverVersion() {
    
    FileVersion fileVersion;
    fileVersion.versionString = "";

    // Try to get driver file version
    if ( GpuCapabilities::getVendor() == GPU_VENDOR_NVIDIA ) {

        // Get fileversion of NVIDIA OpenGL dll
        LDEBUG("Reading file version of NVIDIA driver dll ... ");
        FileVersion fileVersion;
        //TODO: windows vista stores the name of the dll in the registry.
        //      its better to get the name from there (cdoer)
        if (getOSVersion() == OS_WIN_VISTA)
            fileVersion = getFileVersion("nvoglv32.dll");
        else
            fileVersion = getFileVersion(DRIVER_DLL_NVIDIA);

        if (fileVersion.versionString.length() == 0) {
            LDEBUG("Reading NVIDIA driver version failed.");
        }
        else {
            LDEBUG("Reading driver version successful.");
            return fileVersion;
        }
    } 
    else if (getVendor() == GPU_VENDOR_ATI) {

        // Get fileversion of ATI OpenGL dll
        LDEBUG("Reading file version of ATI driver dll ... ");
        FileVersion fileVersion = getFileVersion(DRIVER_DLL_ATI);
        if ( fileVersion.versionString.length() == 0 ) {
            LDEBUG("Reading ATI driver version failed.");
        }
        else {
            LDEBUG("Reading driver version successful.");
            return fileVersion;
        }
    }  

#ifdef TGT_WITH_WMI
    // unknown graphics board vendor or reading NVIDIA/Ati driver version failed: 
    // get driver version from WMI
    LDEBUG("Reading driver version from WMI ...");

    // Win32_VideoController class: http://msdn2.microsoft.com/en-us/library/aa394512.aspx
    std::string version = WMIqueryStr("Win32_VideoController", "DriverVersion");
    
    if (version.length() == 0) {
        LDEBUG("Failed to retrieve driver version from WMI.");
		LWARNING("Failed to detect driver version.");
    } 
    else {
        fileVersion.versionString = version;
        // do not parse driver version string since it is not necessarily a file version string
        fileVersion.d1 = 0;
        fileVersion.d2 = 0;
        fileVersion.d3 = 0;
        fileVersion.d4 = 0;
        fileVersion.version = 0;
        LDEBUG("Successfully read driver version from WMI");
    }
    return fileVersion;

#else
    LDEBUG("Compiled without WMI support.");
	LWARNING("Failed to detect driver version.");
    return fileVersion;
#endif

}

char* GpuCapabilitiesWindows::findPrimaryDevicesKey() {
    BOOL res = TRUE;
    DISPLAY_DEVICE dd;
    int count = 0;
    char* devicesKey = 0;
    LDEBUG("Enumerating all installed display devices...");

    do {
        memset(&dd, 0, sizeof(DISPLAY_DEVICE));
        dd.cb = sizeof(DISPLAY_DEVICE);
        res = EnumDisplayDevices(NULL, count, &dd, 0);
        if ( res == TRUE ) {
            LDEBUG("Graphics Adapter #" << count << " is valid:");
            LDEBUG("DeviceName: " << dd.DeviceName);
            LDEBUG("DeviceString: " << dd.DeviceString);
            LDEBUG("StateFlags: " << dd.StateFlags);
            bool primary = (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE);
            bool desktop = (dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP);
            LDEBUG("\tis primary device? " << ((primary == true) ? "true" : "false"));
            LDEBUG("\tis attached to desktop? " << ((desktop == true) ? "true" : "false"));
            LDEBUG("DeviceID: " << dd.DeviceID);
            LDEBUG("DeviceKey: " << dd.DeviceKey << "\n");

            if ( (primary == true) && (desktop == true) ) {
                size_t len = strlen(dd.DeviceKey);
                devicesKey = new char[len + 1];
                devicesKey[len] = 0;
                memcpy(devicesKey, dd.DeviceKey, len);
            }
        }
        ++count;
    }while( res != FALSE );

    return devicesKey;
}

DWORD GpuCapabilitiesWindows::readVRAMSizeFromReg() {
    DWORD memSize = 0;
    char* devicesKey = findPrimaryDevicesKey();
    if ( devicesKey == 0 ) {
        LERROR("Failed to determine primary graphics adapter by calling findPrimaryDevicesKey()!");
        return 0;
    }

    for ( size_t i = 0; i < strlen(devicesKey); i++ )
        devicesKey[i] = tolower(devicesKey[i]);
    
    char* substr = strstr(devicesKey, "\\registry\\machine\\");
    if ( substr != 0 )
        substr += strlen("\\registry\\machine\\");
    else
        substr = devicesKey;
    LDEBUG("registry key: " << substr);

    HKEY hKey = NULL;
    LONG stat = RegOpenKeyEx(HKEY_LOCAL_MACHINE, substr, 0, KEY_READ, &hKey);
    if ( stat == ERROR_SUCCESS ) {
        DWORD type = 0;
        DWORD bufferSize = 4;
        char* data = new char[bufferSize + 1];
        memset(data, 0, bufferSize + 1);
        stat = RegQueryValueEx(hKey, "HardwareInformation.MemorySize", 
            NULL, &type, (BYTE*) data, &bufferSize);
        if ((stat == ERROR_SUCCESS) && ((type == REG_BINARY) ||
            (getOSVersion() == GpuCapabilities::OS_WIN_VISTA && type == REG_DWORD)))
        {
            LDEBUG("read " << bufferSize << " BYTES from key 'HardwareInformation.MemorySize'...");
            for ( DWORD i = (bufferSize - 1); ; i-- ) {
                LDEBUG("data[" << i << "]: " << static_cast<DWORD>(data[i]));
                memSize |= (data[i] << (i * 8));

                // As DWORD is unsigned, the loop would never exit if
                // it would not be broken...
                //
                if ( i == 0 )
                    break;
             }
             LDEBUG("data converted to " << memSize << " (means " << (memSize / (1024 * 1024)) << " MByte)\n");
        }
        RegCloseKey(hKey);
        hKey = NULL;
        delete [] data;
        data = 0;            
    } 
    else
        LERROR("Error opening key " << substr << ". Reason: " << stat);

    delete [] devicesKey;
    devicesKey = 0;
    return memSize;
}

int GpuCapabilitiesWindows::getVideoRamSize() {
    
    if (videoRamSize_ != -1)
        return videoRamSize_;

#ifdef TGT_WITH_WMI
    LDEBUG("Reading video ram size using WMI...");

    if (isWMIinited()) {       
        // Win32_VideoController class: http://msdn2.microsoft.com/en-us/library/aa394512.aspx
        int ramStr = WMIqueryInt("Win32_VideoController", "AdapterRAM");
        
        if (ramStr > 0) {
            LDEBUG("Reading video ram size through WMI succeeded.");
            // return result in MB
            videoRamSize_ = ramStr / 1048576;
        }
        else {
            LDEBUG("Reading video ram size through WMI failed.");
        }
    }
    else {
        LDEBUG("Unable to read Video RAM size from WMI: not inited");
    } 
#else 
    LDEBUG("Compiled without WMI support.");
#endif
    if (videoRamSize_ == -1) {
        LDEBUG("Reading video ram size from registry...");
        videoRamSize_ = readVRAMSizeFromReg();
        // return result in MB
        videoRamSize_ /= 1048576;
        if (videoRamSize_ == 0) {
            videoRamSize_ = -1;
            LDEBUG("Reading video ram size from registry failed.");
            LWARNING("Reading video ram size failed.");
        }
    }
   
    return videoRamSize_;
}

std::string GpuCapabilitiesWindows::getDriverDate() {

    LDEBUG("Retrieving driver date ...");
        
    if ( getVendor() == GPU_VENDOR_NVIDIA ) {

        LDEBUG("Retrieving driver date by querying file date of NVIDIA driver dll ...");

        std::string result;
        //TODO: windows vista stores the name of the dll in the registry.
        //      its better to get the name from there (cdoer)
        if (getOSVersion() == OS_WIN_VISTA)
            result = getFileDate("nvoglv32.dll");
        else
            result = getFileDate(DRIVER_DLL_NVIDIA);

        if (result.length() == 0) {
            LDEBUG("Failed reading driver date from NVIDIA driver dll.");
        }
        else {
            LDEBUG("Reading driver date successful.");
            return result;
        }
    }
    else if (getVendor() == GPU_VENDOR_ATI) {
        LDEBUG("Retrieving driver date by querying file date of ATI driver dll ...");
        std::string result = getFileDate(DRIVER_DLL_ATI);
        if (result.length() == 0) {
            LDEBUG("Failed reading driver date from ATI driver dll.");
        }
        else {
            LDEBUG("Reading driver date successful.");
            return result;
        }
    }
    
#ifdef TGT_WITH_WMI

    // unknown vendor or reading driver date from vendor dll failed: 
    // read driver date from WMI
    LDEBUG("Reading driver date from WMI ...");

    if (isWMIinited()) {
        // Win32_VideoController class: http://msdn2.microsoft.com/en-us/library/aa394512.aspx
        std::string date = WMIqueryStr("Win32_VideoController", "DriverDate");

        if (date.length() > 0) {

            LDEBUG("Reading driver date successful.");

            // convert to yyyy-mm-dd format.
            // see http://msdn2.microsoft.com/en-us/library/aa387237.aspx for specification of CIM_DATETIME
            std::string dateformat = "";
            dateformat.append(date.substr(0,4));
            dateformat.append("-");
            dateformat.append(date.substr(4,2));
            dateformat.append("-");
            dateformat.append(date.substr(6,2));

            return dateformat;
        }
        else {
            LDEBUG("Failed reading driver date.");
			LWARNING("Failed to detect driver date");
            return "";
        }
    }
    else {
        LDEBUG("Unable to read DriverDate from WMI: not inited");
    } 

#else
    LDEBUG("Compiled without WMI support.");
	LWARNING("Failed to detect driver date");
#endif

	return "";

}


#ifdef TGT_WITH_WMI
VARIANT* GpuCapabilitiesWindows::WMIquery(std::string wmiClass, std::string attribute) {

    // Code based upon:  "Example: Getting WMI Data from the Local Computer" 
    // http://msdn2.microsoft.com/en-us/library/aa390423.aspx

    if (!isWMIinited()) {
        LWARNING("WMI not initiated");
        return 0;
    }

    HRESULT hres;
    VARIANT* result = 0;

    // Step 6: --------------------------------------------------
    // Use the IWbemServices pointer to make requests of WMI ----
    IEnumWbemClassObject* pEnumerator = NULL;
    std::string query = "SELECT " + attribute + " FROM " + wmiClass;
    hres = pWbemServices_->ExecQuery(
        bstr_t("WQL"),
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
        NULL,
        &pEnumerator);
    
    if (FAILED(hres)) {
		LWARNING("ERROR: WMI query failed: " << query);
        return 0;
    }

    // Step 7: -------------------------------------------------
    // Get the data from the query in step 6 -------------------
 
    IWbemClassObject *pclsObj;
    ULONG uReturn = 0;
   
    while (pEnumerator && !result) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, 
            &pclsObj, &uReturn);

        if(uReturn) {
            // Get the value of the attribute and stored it in result
            result = new VARIANT;
            hr = pclsObj->Get(LPCWSTR(str2wstr(attribute).c_str()), 0, result, 0, 0);
        }
    }

    if(!uReturn) {
		LWARNING("ERROR: No WMI query result");
    }

    // Clean enumerator and pclsObject
    pEnumerator->Release();
    pclsObj->Release();

    return result;

}

std::string GpuCapabilitiesWindows::WMIqueryStr(std::string wmiClass, std::string attribute) {

    VARIANT* variant = WMIquery(wmiClass, attribute);
    if (!variant) {
        return "";
    }
    else {
        std::string result = wstr2str(std::wstring(variant->bstrVal));
        VariantClear(variant);
        return result;
    }
}

int GpuCapabilitiesWindows::WMIqueryInt(std::string wmiClass, std::string attribute) {

    VARIANT* variant = WMIquery(wmiClass, attribute);
    if (!variant) {
        return -1;
    }
    else {
        int result = variant->intVal;
        VariantClear(variant);
        return result;
    }
}

bool GpuCapabilitiesWindows::WMIinit() {
    
    // Code based upon:  "Example: Getting WMI Data from the Local Computer" 
    // http://msdn2.microsoft.com/en-us/library/aa390423.aspx

    if (isWMIinited()) {
        LWARNING("The WMI connection has already been inited.");
        return false;
    }

    HRESULT hRes;

    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------

    hRes = CoInitializeEx(0, COINIT_APARTMENTTHREADED); 
    if (FAILED(hRes)) {
        LWARNING("CoInitializeEx() failed");
        return false;       
    }

    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------
    // Note: If you are using Windows 2000, you need to specify -
    // the default authentication credentials for a user by using
    // a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
    // parameter of CoInitializeSecurity ------------------------

    hRes =  CoInitializeSecurity(
        NULL, 
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
        );

    if (FAILED(hRes)) {

        // Failure of CoInitializeSecurity is not necessarily critical 
        // => do not abort initialization

        if (hRes == RPC_E_TOO_LATE) {
            LDEBUG("CoInitializeSecurity failed. HRESULT = RPC_E_TOO_LATE.");
        }
        else if (hRes == RPC_E_NO_GOOD_SECURITY_PACKAGES) {
            LDEBUG("CoInitializeSecurity failed. HRESULT = RPC_E_NO_GOOD_SECURITY_PACKAGES.");
        }
        /*else if (hres == E_OUT_OF_MEMORY) {
            LDEBUG("CoInitializeSecurity failed. HRESULT = E_OUT_OF_MEMORY.");
        } */
        else {
            LDEBUG("CoInitializeSecurity failed. HRESULT = " << hRes);
        }  
    }
    else {
        LDEBUG("CoInitializeSecurity successful");

    }
    
    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    hRes = CoCreateInstance(
        CLSID_WbemLocator,             
        0, 
        CLSCTX_INPROC_SERVER, 
        IID_IWbemLocator, (LPVOID *) &pIWbemLocator_);
 
    if (FAILED(hRes)) {
        LWARNING("Failed to create IWbemLocator object. Err code = 0x" << hRes);
        pIWbemLocator_ = 0;
        CoUninitialize();
        return false;     // error is critical
    }

    LDEBUG("IWbemLocator object successfully created");

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method
	
    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hRes = pIWbemLocator_->ConnectServer(
         _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
         NULL,                    // User name. NULL = current user
         NULL,                    // User password. NULL = current
         0,                       // Locale. NULL indicates current
         NULL,                    // Security flags.
         0,                       // Authority (e.g. Kerberos)
         0,                       // Context object 
         &pWbemServices_          // pointer to IWbemServices proxy
         );
    
    if (FAILED(hRes)) {
        LWARNING("Could not connect to WMI server. Error code = 0x" << hRes);
        pIWbemLocator_->Release(); 
        CoUninitialize();
        pIWbemLocator_ = 0;
        pWbemServices_ = 0;
        return false;            // error is critical
    }

    LDEBUG("Connected to ROOT\\CIMV2 WMI namespace");


    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------

    hRes = CoSetProxyBlanket(
       pWbemServices_,              // Indicates the proxy to set
       RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
       RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
       NULL,                        // Server principal name 
       RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
       RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
       NULL,                        // client identity
       EOAC_NONE                    // proxy capabilities 
    );

    if (FAILED(hRes)) {
        LWARNING("Could not set proxy blanket. Error code = 0x" << hRes);
        pWbemServices_->Release();
        pIWbemLocator_->Release();     
        CoUninitialize();
        pIWbemLocator_ = 0;
        pWbemServices_ = 0;
        return false;               // error is critical
    }
    LDEBUG("WMI successfully inited");
    return true;   

}

bool GpuCapabilitiesWindows::WMIdeinit() {
    
    if (!isWMIinited()) {
        LWARNING("WMI is not inited");
        return false;
    }

    LDEBUG("Deinitializing WMI.");

    if (pIWbemLocator_) 
        pIWbemLocator_->Release();
    if (pWbemServices_)
        pWbemServices_->Release();

    pIWbemLocator_ = 0;
    pWbemServices_ = 0;

    CoUninitialize();

    return true;    
}

bool GpuCapabilitiesWindows::isWMIinited() const {
    return (pIWbemLocator_ && pWbemServices_);
}

std::wstring GpuCapabilitiesWindows::str2wstr(const std::string& str) {

    int len;
    int slength = (int)str.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, 0, 0); 
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;

}

std::string GpuCapabilitiesWindows::wstr2str(const std::wstring& wstr) {

    int len;
    int slength = (int)wstr.length() + 1;
    len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), slength, 0, 0, 0, 0); 
    char* buf = new char[len];
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), slength, buf, len, 0, 0);
    std::string r(buf);
    delete[] buf;
    return r;

}

#endif // VRN_WITH_WMI

GpuCapabilitiesWindows::FileVersion GpuCapabilitiesWindows::getFileVersion(std::string filename) {
    LDEBUG("Reading file version of file '" << filename << "' ...");

    FileVersion fileVersion;
    fileVersion.versionString = "";

    LPVOID vData = NULL;
    VS_FIXEDFILEINFO *fInfo = NULL;
    DWORD dwHandle;

    DWORD dwSize = GetFileVersionInfoSize(filename.c_str(), &dwHandle);
    if (dwSize <= 0) {
		LDEBUG("Error: Invalid file version info size. File " << filename.c_str() << " not existing?");
        return fileVersion;
    }

    // try to allocate memory
    vData = malloc(dwSize);
    if (vData == NULL) {
		LDEBUG("Error: Allocating memory for file version info failed.");
        SetLastError(ERROR_OUTOFMEMORY);
        return fileVersion;
    }

    // try to the the version-data
    if (GetFileVersionInfo(filename.c_str(), dwHandle, dwSize, vData) == 0) {
        DWORD gle = GetLastError();
        free(vData);
        SetLastError(gle);
        LDEBUG("Error reading file version info.");
        return fileVersion;
    }

    // try to query the root-version-info
    UINT len;
    if (VerQueryValue(vData, TEXT("\\"), (LPVOID*) &fInfo, &len) == 0) {
        DWORD gle = GetLastError();
        free(vData);
        SetLastError(gle);
        LDEBUG("Error reading file version info.");
        return fileVersion;
    } 

    // extract hex digits
    int ls = static_cast<int>(fInfo->dwFileVersionLS);
    int ms = static_cast<int>(fInfo->dwFileVersionMS);
    fileVersion.d1 = (ls % 0x10000);
    fileVersion.d2 = (ls / 0xffff);
    fileVersion.d3 = (ms % 0x10000);
    fileVersion.d4 = (ms / 0xffff);

    if ( !createFileVersionFromDigits(fileVersion) ) {
        fileVersion.versionString = "";
        LDEBUG("Error in file version conversion.");
        return fileVersion;
    }

    if ( !createVersionStringFromDigits(fileVersion) ) {
        fileVersion.versionString = "";
        LDEBUG("Error in file version conversion.");
        return fileVersion;
    }

    LDEBUG("Reading file version successful.");
    return fileVersion;
}

std::string GpuCapabilitiesWindows::getFileDate(std::string filename) {
    LDEBUG("Retrieving file date (last write date) of system file '" << filename << "' ...");
    
    // add system path to filename
    LPTSTR lpBuffer = static_cast<LPTSTR>(new char[1024]);
    int len = GetSystemDirectory(
        lpBuffer,
        1024
    );
    if (len == 0) {
        LWARNING("Failed to detect system directory.");
        delete lpBuffer;
        return "";
    }
    std::string systemPath = std::string( static_cast<char*>(lpBuffer), len);
    delete lpBuffer;
    systemPath.append("\\");
    systemPath.append(filename);
    filename = systemPath;

    // create file handle for reading attributes
    LDEBUG("Acquiring handle for file '" << filename << "' ...");
    HANDLE filehandle = CreateFile( filename.c_str(), 
        FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ, 
        0,
        OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, 
        0 );
    if ( filehandle == INVALID_HANDLE_VALUE ) {
        LDEBUG("Failed to acquire handle for file '" << filename << "'.");
        return "";
    }
    
    // read file information
    LDEBUG("Reading file information ...");
    LPBY_HANDLE_FILE_INFORMATION fileInformation = static_cast<LPBY_HANDLE_FILE_INFORMATION>(new BY_HANDLE_FILE_INFORMATION);
    bool success = GetFileInformationByHandle(
        filehandle,
        fileInformation 
    );
    if ( !success ) {
        LDEBUG("Failed to read file information of file '" << filename << "'.");
        delete fileInformation;
        return "";
    }

    // convert file time to local time
    SYSTEMTIME stUTC, stLocal;
    DWORD dwRet;
    FileTimeToSystemTime(&(fileInformation->ftLastWriteTime), &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

    // build a string showing the date
    LPTSTR lpszString = static_cast<LPTSTR>(new char[1024]);
    dwRet = StringCchPrintf(lpszString, 1024, 
        TEXT("%d-%02d-%02d"),
        stLocal.wYear, stLocal.wMonth, stLocal.wDay);
    std::string result = std::string( static_cast<char*>(lpszString));
    delete lpszString;

    LDEBUG("Reading file date successful.");

    return result;
}
    

bool GpuCapabilitiesWindows::parseFileVersionString(FileVersion &fileVersion) {
    std::string verStr = std::string( fileVersion.versionString );
    std::string substr;
    std::istringstream converter;

    int pos;

    // fourth (= most significant) hex digit
    pos = verStr.find( ".", 0 );
    if (pos > 0 && pos < 5) {
        substr = verStr.substr( 0, pos );
        converter.clear();
        converter.str(substr);
        converter >> fileVersion.d4;
        if (verStr.length() == pos) {
			LDEBUG("ERROR: Malformed file version string.");
            return false;
        }
        verStr = verStr.substr(pos+1, verStr.length());
    } else {
		LDEBUG("ERROR: Malformed file version string.");
        return false;
    }

    // third hex digit
    pos = verStr.find( ".", 0 );
    if (pos > 0 && pos < 5) {
        substr = verStr.substr( 0, pos );
        converter.clear();
        converter.str(substr);
        converter >> fileVersion.d3;
        if (verStr.length() == pos) {
            LWARNING("Malformed file version string.");
            return false;
        }
        verStr = verStr.substr(pos+1, verStr.length());
    } else {
		LDEBUG("ERROR: Malformed file version string.");
        return false;
    }

    // second hex digit
    pos = verStr.find( ".", 0 );
    if (pos > 0 && pos < 5) {
        substr = verStr.substr( 0, pos );
        converter.clear();
        converter.str(substr);
        converter >> fileVersion.d2;
        if (verStr.length() == pos) {
			LDEBUG("ERROR: Malformed file version string.");
            return false;
        }
        verStr = verStr.substr(pos+1, verStr.length());
    } else {
		LDEBUG("ERROR: Malformed file version string.");
        return false;
    }

    // first hex digit
    if ( verStr.length() > 0 && verStr.length() < 5) {
        converter.clear();
        converter.str(verStr);
        converter >> fileVersion.d1;
    } else {
		LDEBUG("ERROR: Malformed file version string.");
        return false;
    }

    return createFileVersionFromDigits(fileVersion);

}

bool GpuCapabilitiesWindows::createFileVersionFromDigits(FileVersion &fileVersion) {
    fileVersion.version = ( static_cast<uint64_t>(fileVersion.d4) << 48 )
        + ( static_cast<uint64_t>(fileVersion.d3) << 32 )
        + ( static_cast<uint64_t>(fileVersion.d2) << 16 )
        + ( static_cast<uint64_t>(fileVersion.d1) );

    return true;
}

bool GpuCapabilitiesWindows::createVersionStringFromDigits(FileVersion &fileVersion) {
    std::ostringstream converter;
    std::string substr;

    fileVersion.versionString = "";
    
    converter.clear();
    converter << fileVersion.d4;
    fileVersion.versionString.append(converter.str());
    
    fileVersion.versionString.append(".");
    converter.str("");
    converter.clear();
    converter << fileVersion.d3;
    fileVersion.versionString.append(converter.str());
    
    fileVersion.versionString.append(".");
    converter.str("");
    converter.clear();
    converter << fileVersion.d2;
    fileVersion.versionString.append(converter.str());
    
    fileVersion.versionString.append(".");
    converter.str("");
    converter.clear();
    converter << fileVersion.d1;
    fileVersion.versionString.append(converter.str());
    
    return true;
}

} // namespace tgt

#endif //WIN32
