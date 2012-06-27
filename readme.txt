The Voreen directory structure:
===============================
data                - contains volume datasets
data/cache          - a temporary folder used for volume caching
data/fonts          - (possibly) necessary fonts
data/networks       - predefined networks which can be used in VoreenVE
data/scripts        - python scripts which can be executed from within voreen
data/workspaces     - predefined workspace to be used with VoreenVE
data/transferfuncs  - contains transfer functions in various formats

ext                 - external dependencies to other libraries

apps                - contains applications using voreen
apps/glutexample    - a simple glut example
apps/voreenve       - the main voreen application - the Voreen Visualization Environment
apps/voltool        - perform various preprocessings on volume data

include             - contains all necessary headers
src                 - contains the source files