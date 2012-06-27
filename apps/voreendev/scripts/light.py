# Since this is python be sure to configure your text editor properly.
# Do NOT use tabs and replace 1 tab with 4 spaces.

import math
import time
import voreen
import voreenqt

cycles = 400
width  = 512
height = 512
#width  = 700
#height = 560

counter = 0
angleIncr = 2.0 * 180.0 / cycles  # do a half rotation


#voreen.postIVec2Msg(width, height, "set.viewport", "mainview")
voreen.postVec3Msg(2.0, 2.0, 1.0, "set.lightPosition", "mainview")

# start loop for animation
while (counter < cycles):
    voreen.postFloatMsg(angleIncr, "rotate.lightYAxis", "mainview")
    #voreen.postMsg("msg.repaint", "mainview") # I think this is not needed
    voreen.postStringMsg('pic%(#)03d.png' % { "#": counter }, "set.SnapFileName", "mainview")
    voreen.postIVec2Msg(640, 480, "msg.paintToFile", "mainview")

    # increment neccessary variables
    counter = counter + 1

