# Since this is python be sure to configure your text editor properly
# Do NOT use tabs and replace 1 tab with 4 spaces

#
# do a little animation
#
# output files pic000.png until pic099.png are generated
# you can transform them with
# ffmpeg -r 10 -b 1800 -i pic%03d.png test1800.mp4
# to a movie
# have fun
#

import math
import voreen

print '\nstarting script'

cycles = 100
counter = 0

angle = 0.0
angleIncr = math.pi / cycles  # do a half rotation

# used for interpolating the transfer functions
t = 0.0
tIncr = 1.0 / cycles


# set transfer functions
voreen.loadTransferFunc1("transferfunc1.png")
voreen.loadTransferFunc2("transferfunc2.png")

# start loop for animation
while (counter < cycles):
    voreen.setBlendedTransferFunc(t, "mainview")
    voreen.postCreateQuatMsg(angleIncr, 1.0, 0.0, 0.0, "set.cameraApplyQuat", "mainview")
    #voreen.postMsg("msg.repaint", "mainview") # I think this is not needed

    # paint to file
    voreen.postStringMsg('pic%(#)03d.png' % { "#": counter }, "set.SnapFileName", "mainview")
    voreen.postIVec2Msg(640, 480, "msg.paintToFile", "mainview")

    # increment neccessary variables
    counter = counter + 1
    t = t + tIncr
    angle = angle + angleIncr

print 'finished script\n'
