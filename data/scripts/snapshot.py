# Since this is python be sure to configure your text editor properly.
# Do NOT use tabs and replace 1 tab with 4 spaces.

import math
import time
import voreen
import voreenqt

width  = 512
height = 512
filename = "snapshot-%d.png"

print 'starting script'
start = time.time()

#voreen.setViewport(width, height)
count = voreen.canvasCount()
print "canvas count is %d" % count
for i in range(0, count):
    voreen.snapshotCanvas(i, filename % (i+1))

end = time.time()
runtime = (end - start)
print 'total runtime: ' + str(runtime) + 's'

print 'finished script'

#voreenqt.quit()
