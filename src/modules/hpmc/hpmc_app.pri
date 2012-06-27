# libraries to link

win32 {  
  CONFIG(debug, debug|release) {
    MSC_CONFIG = Debug
  } else {
    MSC_CONFIG = Release
  }

  visual_studio {
    # Contains "Release" or "Debug" as selected in the IDE
    MSC_CONFIG = $(ConfigurationName)
  }
  
  LIBS += "$${VRN_MODULE_SRC_DIR}/hpmc/ext/hpmc/win32/$${MSC_CONFIG}/hpmc.lib"
}

unix {  
  LIBS += -lhpmc
}  

### Local Variables:
### mode:conf-unix
### End:
