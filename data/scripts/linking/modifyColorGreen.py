def modifyColorGreen(oldSourceValue, newSourceValue, oldDestinationValue):
    oldDestinationValue[2] = newSourceValue * 0.01
    return oldDestinationValue