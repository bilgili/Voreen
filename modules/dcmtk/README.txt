The DCMTK module requires the DCMTK 3.5.4 or DCMTK 3.6.0 libraries, 
which are not included.

Unix: 
- install DCMTK 3.5.4 or DCMTK 3.6.0 to your path.

Win32:
- copy the DCMTK headers to:   modules/dcmtk/ext/dcmtk/include
- copy the OpenSSL headers to: modules/dcmtk/ext/openssl/include (only for DCMTK 3.6.0)
- copy the DCMTK libs to:      modules/dcmtk/ext/dcmtk/lib/win32
- copy the OpenSSL libs to:    modules/dcmtk/ext/openssl/lib/win32 (only for DCMTK 3.6.0)

Note: A DCMTK (+OpenSSL) build for MS Visual Studio 2008/2010 
      is available at http://voreen.uni-muenster.de

