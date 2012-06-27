def modifyColorRed(oldSourceValue, newSourceValue, oldDestinationValue):
    oldDestinationValue[0] = newSourceValue * 0.01
    return oldDestinationValue