# external dependency: triangle library
INCLUDEPATH += $${VRN_MODULE_INC_DIR}/triangle/ext"

SOURCES += "$${VRN_MODULE_SRC_DIR}/triangle/ext/del_impl.cpp"

HEADERS += "$${VRN_MODULE_INC_DIR}/triangle/ext/del_interface.hpp"
HEADERS += "$${VRN_MODULE_INC_DIR}/triangle/ext/dpoint.hpp"
HEADERS += "$${VRN_MODULE_INC_DIR}/triangle/ext/triangle.h"
HEADERS += "$${VRN_MODULE_INC_DIR}/triangle/ext/triangle_impl.hpp"

### Local Variables:
### mode:conf-unix
### End:
