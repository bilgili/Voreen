TEMPLATE = subdirs

# check qmake version
QMAKE_VERS = $$[QMAKE_VERSION]
QMAKE_VERSION_CHECK = $$find(QMAKE_VERS, "^[234]\\.")
isEmpty(QMAKE_VERSION_CHECK) {
   error("Your qmake version '$$QMAKE_VERS' is too old, qmake from Qt 4 is required!")
}

# Default projects
VRN_PROJECTS = core qt voreenve

!exists(config.txt) {
  error("config.txt not found! copy config-default.txt to config.txt and edit!")
}
include(config.txt)

contains(VRN_PROJECTS, core):     SUBDIRS += sub_core
contains(VRN_PROJECTS, qt):       SUBDIRS += sub_qt
contains(VRN_PROJECTS, voreenve): SUBDIRS += sub_voreenve
contains(VRN_PROJECTS, voltool):  SUBDIRS += sub_voltool
contains(VRN_PROJECTS, simple-qt):  SUBDIRS += sub_simple-qt
contains(VRN_PROJECTS, simple-glut):  SUBDIRS += sub_simple-glut
contains(VRN_PROJECTS, simple-memcheck):  SUBDIRS += sub_simple-memcheck
contains(VRN_PROJECTS, simple-memcheckinit):  SUBDIRS += sub_simple-memcheckinit

sub_core.file = src/core/voreenlib_core.pro

sub_qt.file = src/qt/voreenlib_qt.pro
sub_qt.depends = sub_core

sub_voreenve.file = apps/voreenve/voreenve.pro
sub_voreenve.depends = sub_core sub_qt

sub_voltool.file = apps/voltool/voltool.pro
sub_voltool.depends = sub_core

sub_simple-qt.file = apps/simple/simple-qt.pro
sub_simple-qt.depends = sub_core sub_qt

sub_simple-glut.file = apps/simple/simple-glut.pro
sub_simple-glut.depends = sub_core

sub_simple-memcheck.file = apps/simple/simple-memcheck.pro
sub_simple-memcheck.depends = sub_core

sub_simple-memcheckinit.file = apps/simple/simple-memcheckinit.pro
sub_simple-memcheckinit.depends = sub_core

sub_serializertest.file = apps/tests/serializertest/serializertest.pro
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
  MRPROPER_DIRS=src/core src/qt include/voreen/modules apps/*
  mrproper.commands = @echo \"Removing Makefiles and module registration file...\"; \
                            find $$MRPROPER_DIRS -name \"Makefile*\" -or -name "Makefile" -or -name "gen_moduleregistration.h" | \
                            xargs /bin/rm -fv
  QMAKE_EXTRA_TARGETS += mrproper

}

### Local Variables:
### mode:conf-unix
### End:
