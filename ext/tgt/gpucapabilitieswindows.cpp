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

GpuCapabilitiesWindows::GpuCapabilitiesWindows() :
#ifdef TGT_WITH_WMI
    pIWbemLocator_(NULL),
    pWbemServices_(NULL),
    WMISecurityInitialized_(false),
#endif
    loggerCat_("tgt.GpuCapabilitiesWindows"),
    GpuCapabilities(),
    videoRamSize_(-1)
{
 
#ifdef TGT_WITH_WMI
    // this has to be done exactly once per process for WMI access
    HRESULT result = ::CoInitializeSecurity( NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_PKT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        0
    );
    if (result == S_OK) {
        LDEBUG("CoInitializeSecurity  succeeded.");
        WMISecurityInitialized_ = true;   
    } 
    else if (result == RPC_E_TOO_LATE) {
        LWARNING("CoInitializeSecurity failed. HRESULT = RPC_E_TOO_LATE.");
        WMISecurityInitialized_ = true;   
    }
    else if (result == RPC_E_NO_GOOD_SECURITY_PACKAGES) {
        LWARNING("CoInitializeSecurity failed. HRESULT = RPC_E_NO_GOOD_SECURITY_PACKAGES.");
    }
    //else if (result == RPC_E_OUT_OF_MEMORY) {
    //    LWARNING("CoInitializeSecurity failed. HRESULT = E_OUT_OF_MEMORY.");
    //}
    else {
        LWARNING("CoInitializeSecurity failed. HRESULT = " << result);
    }
#endif

}

void GpuCapabilitiesWindows::logCapabilities(bool extensionsString, bool osString) {
    GpuCapabilities::logCapabilities(extensionsString, osString);
    
    GraphicsDriverInformation driverInfo = getGraphicsDriverInformation();
    LINFO("Graphics Driver Version: " << driverInfo.driverVersion.versionString);
    LINFO("Graphics Driver Date:    " << driverInfo.driverDate);
    
    LINFO("Graphics Memory Size:    " << getVideoRamSize() << " MB");

    GraphicsDriverSettings driverSettings = getDriverSettings();
    LINFO("Vertical Sync:           " << ( driverSettings.verticalSync ? "enabled" : "disabled") );
    LINFO("Triple Buffering:        " << ( driverSettings.tripleBuffering ? "enabled" : "disabled \n") );
}

GpuCapabilitiesWindows* GpuCapabilitiesWindows::getInstance() {
    if (!instance_)
        instance_ = new GpuCapabilitiesWindows();
    return instance_;
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
        FileVersion fileVersion = getFileVersion(DRIVER_DLL_NVIDIA);
        if ( fileVersion.versionString.length() == 0 ) {
            LWARNING("Reading NVIDIA driver version failed.");
        }
        else {
            LDEBUG("Reading driver version successful.");
            return fileVersion;
        }
    } 
    else if ( getVendor() == GPU_VENDOR_ATI ) {

        // Get fileversion of ATI OpenGL dll
        LDEBUG("Reading file version of ATI driver dll ... ");
        FileVersion fileVersion = getFileVersion(DRIVER_DLL_ATI);
        if ( fileVersion.versionString.length() == 0 ) {
            LWARNING("Reading ATI driver version failed.");
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

    std::string version = "";
    if (WMIinit()) {
        // Win32_VideoController class: http://msdn2.microsoft.com/en-us/library/aa394512.aspx
        version = WMIquery("Win32_VideoController", "DriverVersion");
    }
    WMIdeinit();
    if (version.length() == 0) {
        LWARNING("Failed to retrieve driver version from WMI.");
    } else {
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
        if ( (stat == ERROR_SUCCESS) && (type == REG_BINARY) ) {
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

    if (WMIinit()) {       
        // Win32_VideoController class: http://msdn2.microsoft.com/en-us/library/aa394512.aspx
        std::string ramStr = WMIquery("Win32_VideoController", "AdapterRAM");
        WMIdeinit();
        
        if (ramStr.length() > 0) {
            LDEBUG("Reading video ram size succeeded.");
            std::istringstream istream(ramStr);
            istream >> videoRamSize_;
            // return result in MB
            videoRamSize_ /= 1048576;
        }
        else {
            LWARNING("Reading video ram size failed.");
        }
    }
    else {
        WMIdeinit();
        LWARNING("Reading video ram size failed.");
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
            LWARNING("Reading video ram size from registry failed.");
        }
    }
    return videoRamSize_;
}

std::string GpuCapabilitiesWindows::getDriverDate() {

    LDEBUG("Retrieving driver date ...");
        
    if ( getVendor() == GPU_VENDOR_NVIDIA ) {

        LDEBUG("Retrieving driver date by querying file date of NVIDIA driver dll ...");
        std::string result = getFileDate(DRIVER_DLL_NVIDIA);
        if ( result.length() == 0) {
            LWARNING("Failed reading driver date from NVIDIA driver dll.");
        } else {
            LDEBUG("Reading driver date successful.");
            return result;
        }
    }
    else if ( getVendor() == GPU_VENDOR_ATI ) {
        LDEBUG("Retrieving driver date by querying file date of ATI driver dll ...");
        std::string result = getFileDate(DRIVER_DLL_ATI);
        if ( result.length() == 0) {
            LWARNING("Failed reading driver date from ATI driver dll.");
        } else {
            LDEBUG("Reading driver date successful.");
            return result;
        }
    }
    
#ifdef TGT_WITH_WMI

    // unknown vendor or reading driver date from vendor dll failed: 
    // read driver date from WMI
    LDEBUG("Reading driver date from WMI ...");

    if (!WMIinit()) {
        WMIdeinit();
        LWARNING("Reading driver date failed.");
        return "";
    }
    // Win32_VideoController class: http://msdn2.microsoft.com/en-us/library/aa394512.aspx
    std::string date = WMIquery("Win32_VideoController", "DriverDate");
    WMIdeinit();

    if (date.length() == 25) {

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
        LWARNING("Failed reading driver date.");
        return "";
    }

#else
    LDEBUG("Compiled without WMI support.");
    return "";

#endif
}

GpuCapabilitiesWindows::GraphicsDriverSettings GpuCapabilitiesWindows::getDriverSettings() {
    GraphicsDriverSettings driverSettings;
    driverSettings.verticalSync = false;
    driverSettings.tripleBuffering = false;

    if ( getVendor() != GPU_VENDOR_NVIDIA && getVendor() != GPU_VENDOR_ATI ) {
        LDEBUG("Driver settings for vertical synchronization and triple buffering \
can only be detected for NVIDIA and ATi GPUs.")
    } 
    else {
        std::string vendorStr;
        std::string vSyncStr;
        std::string tripleBufferingStr;

        int vSyncValue = -1;
        int tripleBufferingValue = -1;

        if ( getVendor() == GPU_VENDOR_NVIDIA ) {
            vendorStr = "nv";
            vSyncStr = "OGL_DefaultSwapInterval";
            tripleBufferingStr = "OGL_TripleBuffer";
        }
        else if (getVendor() == GPU_VENDOR_ATI) {
            vendorStr = "ati";
            vSyncStr = "VSyncControl";
            tripleBufferingStr = "EnableTripleBuffering";
        }

        LDEBUG("Reading vsync and triple buffering state from registry ...");

        HKEY hKey, hSubKey;
        if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Control\\Video", 0, KEY_READ, &hKey) == ERROR_SUCCESS ) {
                DWORD subKeyIndex = 0;
                LPTSTR subKeyName = (LPTSTR)(new char[256]);
                LPDWORD bufferSize = (LPDWORD)(new WORD);
                *bufferSize = 256;
                while ( (vSyncValue == -1) && (tripleBufferingValue == -1) && 
                    RegEnumKeyEx(hKey, subKeyIndex, subKeyName, bufferSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {

                    subKeyIndex++;

                    std::string subKeyNameStr = std::string(subKeyName);
                    subKeyNameStr.append("\\0000");
                    if ( RegOpenKeyEx(hKey, subKeyNameStr.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS ) {

                        LPBYTE pBuffer = (LPBYTE)(new byte[256]);
                        *bufferSize = 256;
                        if (RegQueryValueEx(hSubKey, "InstalledDisplayDrivers", NULL, NULL, pBuffer, bufferSize) == ERROR_SUCCESS) {
                            std::string driverStr((char*)pBuffer, *bufferSize);

                            if ( driverStr.find(vendorStr) != std::string::npos ) {

                                *bufferSize = 256;
                                if ( RegQueryValueEx(hSubKey, vSyncStr.c_str(), NULL, NULL, pBuffer, bufferSize) == ERROR_SUCCESS ) {
                                    vSyncValue = static_cast<int>(*pBuffer);
                                    LDEBUG("Entry '" << vSyncStr << "' successfully read.");
                                } else {
                                    LDEBUG("Entry '" << vSyncStr << "' does not exist.");
                                }

                                *bufferSize = 256;
                                if ( RegQueryValueEx(hSubKey, tripleBufferingStr.c_str(), NULL, NULL, pBuffer, bufferSize) == ERROR_SUCCESS ) {
                                    tripleBufferingValue = static_cast<int>(*pBuffer);
                                    LDEBUG("Entry '" << tripleBufferingStr << "' successfully read.");
                                } else {
                                    LDEBUG("Entry '" << tripleBufferingStr << "' does not exist.");
                                }

                            }

                        } else {
                            LDEBUG("Entry 'InstalledDisplayDrivers' does not exist!");
                        }
                        delete[] pBuffer;

                    } else {
                        LDEBUG("Unable to open subkey!");
                    }

                    *bufferSize = 256;
                }

                delete[] subKeyName;

                if (subKeyIndex == 0) { 
                    LDEBUG("Key 'System\\CurrentControlSet\\Control\\Video' has no subkeys!")
                }

        } 
        else {
            LDEBUG("Unable to open key 'System\\CurrentControlSet\\Control\\Video'!")
        }

        // interprete registry values
        if ( getVendor() == GPU_VENDOR_NVIDIA ) {
            if (vSyncValue > -1)
                driverSettings.verticalSync = ( vSyncValue == 1 );
            if (tripleBufferingValue > -1)
                driverSettings.tripleBuffering = ( tripleBufferingValue == 1 );
        }
        else if ( getVendor() == GPU_VENDOR_ATI ) {
            // strange enough, but ATI stores the decimals' ASCII codes
            if (vSyncValue > -1)
                driverSettings.verticalSync = ( (vSyncValue == 0x32) || (vSyncValue == 0x33) );
            if (tripleBufferingValue > -1)
                driverSettings.tripleBuffering = ( tripleBufferingValue == 0x31 );
        }

        if (vSyncValue == -1) {
            LWARNING("Unable to read vertical sync setting.");
        }
        if (tripleBufferingValue == -1) {
            LWARNING("Unable to read triple buffering setting.");
        }

    }

    return driverSettings;

}


#ifdef TGT_WITH_WMI
std::string GpuCapabilitiesWindows::WMIquery(std::string wmiclass, std::string attribute) {

    // Code based upon:  "Example: Getting WMI Data from the Local Computer" 
    // http://msdn2.microsoft.com/en-us/library/aa390423.aspx

    tgtAssert( pIWbemLocator_ && pWbemServices_, "WMI NOT initiated!");

    std::string result;

    HRESULT hRes;
    IEnumWbemClassObject * pEnumObject = NULL;

    LDEBUG("ExecQuery ...");
    std::string query = "Select * from ";
    query.append(wmiclass);
    BSTR strQuery = _bstr_t(query.c_str()).copy();
    BSTR strQL = (L"WQL");
    hRes = pWbemServices_->ExecQuery(strQL, strQuery,
        WBEM_FLAG_RETURN_IMMEDIATELY,NULL,&pEnumObject);

    if ( !SUCCEEDED(hRes) ) {
        LWARNING("WMI ExecQuery failed. (Invalid WQL statement?)");
        return "";
    }

    LDEBUG("Retrieving result from EnumObject ...");
    ULONG uCount = 1, uReturned;
    IWbemClassObject *pClassObject[1];
    pClassObject[0] = 0;
    VARIANT v;
    BSTR strClassProp = SysAllocString( _bstr_t(attribute.c_str()).copy() );
    hRes = pEnumObject->Reset();
    if ( SUCCEEDED(hRes) )
        hRes = pEnumObject->Next(WBEM_INFINITE,uCount, pClassObject, &uReturned);
    if ( SUCCEEDED(hRes) )
        hRes = pClassObject[0]->Get(strClassProp, 0, &v, 0, 0);
    SysFreeString(strClassProp);

    if ( !SUCCEEDED(hRes) ) {
        LWARNING("Retrieving WMI query result failed. (Invalid classname or attribute?)");
        pEnumObject->Release();
        if (pClassObject[0])
            pClassObject[0]->Release(); 
        return "";
    }
    
    _bstr_t bstrPath = &v; //Just to convert BSTR to ANSI
   
    char* strPath=(char*)bstrPath;
    result = std::string( strPath );
    VariantClear( &v );

    pEnumObject->Release();
    //pClassObject->Release(); 

    LDEBUG("Query successful.");

    return result;
}

bool GpuCapabilitiesWindows::WMIinit() {
    // Code based upon:  "Example: Getting WMI Data from the Local Computer" 
    // http://msdn2.microsoft.com/en-us/library/aa390423.aspx

    tgtAssert( !pIWbemLocator_ && !pWbemServices_, "WMI already initiated!");

    if (!WMISecurityInitialized_) {
        LDEBUG("WMI initialization abort due to failed 'CoInitializeSecurity' call!");
        return false;
    }
    
        
    LDEBUG("Initializing WMI ... ");

    LDEBUG("CoInitialize ... ");
    HRESULT hRes = CoInitialize(NULL);
    if (FAILED(hRes)) {
        LWARNING("Initializing WMI failed.");
        return false;
    }

    LDEBUG("CoCreateInstance ... ");
    BSTR bstrNamespace = (L"root\\cimv2");
    hRes = CoCreateInstance (
        CLSID_WbemAdministrativeLocator,
        NULL ,
        CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER , 
        IID_IUnknown ,
        ( void ** ) & pIWbemLocator_
        ) ;

    if (SUCCEEDED(hRes)) {
        LDEBUG("ConnectServer ... ");
        hRes = pIWbemLocator_->ConnectServer(
            bstrNamespace, // Namespace
            NULL, // Userid
            NULL, // PW
            NULL, // Locale
            0, // flags
            NULL, // Authority
            NULL, // Context
            &pWbemServices_
            );
    }
    else {
        LWARNING("Initializing WMI failed.");
        pIWbemLocator_ = NULL;
        return false;
    }

    if (SUCCEEDED(hRes)) {
        LDEBUG("Initializing WMI succeeded.") 
            return true;
    } 
    else {
        LWARNING("Initializing WMI failed."); 
        pWbemServices_ = NULL;
        return false;
    }

}

bool GpuCapabilitiesWindows::WMIdeinit() {
    if (!WMISecurityInitialized_)
        return false;
    
    LDEBUG("Deinitializing WMI.");

    if ( pIWbemLocator_ ) 
        pIWbemLocator_->Release();
    if ( pWbemServices_ )
        pWbemServices_->Release();

    pIWbemLocator_ = NULL;
    pWbemServices_ = NULL;

    CoUninitialize();

    return true;    
}

#endif

GpuCapabilitiesWindows::FileVersion GpuCapabilitiesWindows::getFileVersion(std::string filename) {
    LDEBUG("Reading file version of file '" << filename << "' ...");

    FileVersion fileVersion;
    fileVersion.versionString = "";

    LPVOID vData = NULL;
    VS_FIXEDFILEINFO *fInfo = NULL;
    DWORD dwHandle;

    DWORD dwSize = GetFileVersionInfoSize(filename.c_str(), &dwHandle);
    if (dwSize <= 0) {
        LWARNING("Invalid file version info size.");
        return fileVersion;
    }

    // try to allocate memory
    vData = malloc(dwSize);
    if (vData == NULL) {
        LWARNING("Allocating memory for file version info failed.");
        SetLastError(ERROR_OUTOFMEMORY);
        return fileVersion;
    }

    // try to the the version-data
    if (GetFileVersionInfo(filename.c_str(), dwHandle, dwSize, vData) == 0) {
        DWORD gle = GetLastError();
        free(vData);
        SetLastError(gle);
        LWARNING("Error reading file version info.");
        return fileVersion;
    }

    // try to query the root-version-info
    UINT len;
    if (VerQueryValue(vData, TEXT("\\"), (LPVOID*) &fInfo, &len) == 0) {
        DWORD gle = GetLastError();
        free(vData);
        SetLastError(gle);
        LWARNING("Error reading file version info.");
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
        LWARNING("Error in file version conversion.");
        return fileVersion;
    }

    if ( !createVersionStringFromDigits(fileVersion) ) {
        fileVersion.versionString = "";
        LWARNING("Error in file version conversion.");
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
        LWARNING("Failed to acquire handle for file '" << filename << "'.");
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
        LWARNING("Failed to read file information of file '" << filename << "'.");
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
            LWARNING("Malformed file version string.");
            return false;
        }
        verStr = verStr.substr(pos+1, verStr.length());
    } else {
        LWARNING("Malformed file version string.");
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
        LWARNING("Malformed file version string.");
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
            LWARNING("Malformed file version string.");
            return false;
        }
        verStr = verStr.substr(pos+1, verStr.length());
    } else {
        LWARNING("Malformed file version string.");
        return false;
    }

    // first hex digit
    if ( verStr.length() > 0 && verStr.length() < 5) {
        converter.clear();
        converter.str(verStr);
        converter >> fileVersion.d1;
    } else {
        LWARNING("Malformed file version string.");
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

GpuCapabilitiesWindows* GpuCapabilitiesWindows::instance_ = NULL;

} // namespace tgt

#endif //WIN32
