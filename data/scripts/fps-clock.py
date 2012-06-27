import math
import time
import voreen
import voreenqt

cycles = 100
width  = 512
height = 512
#width  = 700
#height = 560

def benchmark():
    counter = 0
    angle = 0.0
    angleIncr = math.pi / cycles  # do a half rotation

    t = 0.0
    tIncr = 1.0 / cycles

    voreen.setViewport(width, height)
    voreenqt.processEvents()
    voreen.resetCamera()
    voreenqt.repaintCanvas()

    print "clock started"
    start = time.time()

    # start loop for animation
    while (counter < cycles):
        voreen.tickClockProcessor()

        # increment necessary variables
        counter = counter + 1
        t = t + tIncr
        angle = angle + angleIncr

        voreenqt.repaintCanvas()
    end = time.time()
    print "clock stopped"

    runtime = (end - start)
    if runtime > 0:
        fps = cycles / runtime
    else:
        fps = -1

    message = 'frames: ' + str(cycles) + '\n'
    message += 'total runtime: ' + str(runtime) + 's\n'
    message += 'fps: ' + str(fps)
    print message

    voreenqt.messageBox("Finished!\n" + message)


# Start

if voreenqt.questionBox("Welcome to fps-clock.py!\n" + "Will run for " + str(cycles) + " frames with "
                      + str(width) + "x" + str(height) + " pixels.\n" 
                      + "Start?"):
    benchmark()
else:
    print "aborting on user request"

print 'finished.'
