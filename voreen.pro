TEMPLATE = subdirs
SUBDIRS = sub_core sub_qt sub_voreenrpt sub_voreendev

!exists(config.txt) {
  error("config.txt not found! copy config-default.txt to config.txt and edit!")
}

sub_core.file = src/core/voreenlib_core.pro
sub_qt.file = src/qt/voreenlib_qt.pro

sub_voreenrpt.file = apps/rptgui/rptgui.pro
sub_voreenrpt.depends = sub_core sub_qt

sub_voreendev.file = apps/voreendev/voreendev.pro
sub_voreendev.depends = sub_core sub_qt

unix {
  # update browser file for the emacs class hierarchy browser
  update-ebrowse.target = update-ebrowse
  EBROWSE_DIRS=include src apps/dicomloader apps/vol2dicom apps/voltool apps/voreendev ext/tgt
  update-ebrowse.commands = @echo \"Updating ebrowse file...\"; \
                            (find $$EBROWSE_DIRS -name \"*.h\"; \
                             find $$EBROWSE_DIRS -not -name \"moc_*\" -and -name \"*.cpp\") | \
                            ebrowse --output-file=BROWSE
  QMAKE_EXTRA_TARGETS += update-ebrowse
}

### Local Variables:
### mode:conf-unix
### End:
