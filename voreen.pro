TEMPLATE = subdirs

# check qmake version
QMAKE_VERS = $$[QMAKE_VERSION]
QMAKE_VERSION_CHECK = $$find(QMAKE_VERS, "^[234]\.")
isEmpty(QMAKE_VERSION_CHECK) {
   error("Your qmake version '$$QMAKE_VERS' is too old, qmake from Qt 4 is required!")
}

# Default projects
VRN_PROJECTS = core qt voreenve serializertest

!exists(config.txt) {
  error("config.txt not found! copy config-default.txt to config.txt and edit!")
}
include(config.txt)

contains(VRN_PROJECTS, core):     SUBDIRS += sub_core
contains(VRN_PROJECTS, qt):       SUBDIRS += sub_qt
contains(VRN_PROJECTS, voreenve): SUBDIRS += sub_voreenve
contains(VRN_PROJECTS, voltool):  SUBDIRS += sub_voltool
contains(VRN_PROJECTS, serializertest):  SUBDIRS += sub_serializertest

sub_core.file = src/core/voreenlib_core.pro

sub_qt.file = src/qt/voreenlib_qt.pro
sub_qt.depends = sub_core

sub_voreenve.file = apps/voreenve/voreenve.pro
sub_voreenve.depends = sub_core sub_qt

sub_voltool.file = apps/voltool/voltool.pro
sub_voltool.depends = sub_core


sub_serializertest.file = apps/serializertest/serializertest.pro
sub_serializertest.depends = sub_core

unix {
  # update browser file for the emacs class hierarchy browser
  ebrowse.target = ebrowse
  EBROWSE_DIRS=include src apps/voreenve apps/voltool ext/tgt
  ebrowse.commands = @echo \"Updating ebrowse file...\"; \
                     (find $$EBROWSE_DIRS -name \"*.h\"; \
                      find $$EBROWSE_DIRS -not -name \"moc_*\" -and -name \"*.cpp\") | \
                      ebrowse --output-file=BROWSE
  QMAKE_EXTRA_TARGETS += ebrowse

  # clean up old Makefiles also
  mrproper.target = mrproper
  MRPROPER_DIRS=src/core src/qt apps/*
  mrproper.commands = @echo \"Removing Makefiles...\"; \
                            find $$MRPROPER_DIRS -name \"Makefile*\" -or -name "Makefile" | \
                            xargs /bin/rm -fv
  QMAKE_EXTRA_TARGETS += mrproper

}

### Local Variables:
### mode:conf-unix
### End:
