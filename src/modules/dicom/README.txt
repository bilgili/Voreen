The DICOM module requires the DCMTK 3.5.4 library, which is not included.

Win32:
- copy the DCMTK headers to: include/voreen/modules/dicom/ext/dcmtk
- copy the DCMTK libs to:    src/modules/dicom/ext/dcmtk/win32
The paths can be adapted in dicom_config.txt.

Note: The pre-compiled DCMTK library for MS Visual Studio 2008 
      is available at www.voreen.org.


Unix: 
- install DCMTK 3.5.4 to your path.