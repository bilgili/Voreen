
# root dir
INSTALL(FILES   
            Changelog.txt
            CREDITS.txt
            LICENSE.txt
            LICENSE-academic.txt
        DESTINATION .
)

# data dir
INSTALL(FILES  
            data/voreenve.cfg.sample
        DESTINATION data
)
INSTALL(DIRECTORY   
            data/screenshots
        DESTINATION data
        PATTERN "data/screenshots/*" EXCLUDE
)

# further directories
INSTALL(DIRECTORY
            resource
            doc
        DESTINATION .
        PATTERN "doc/docbook" EXCLUDE
)

