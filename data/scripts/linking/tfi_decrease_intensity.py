def tfi_decrease_intensity (sourceOld, sourceNew, targetOld):
    sourceNew = sourceNew / 255.0
    keys = targetOld[2]
    key0 = keys[0]
    key0[0] = 0.126 - sourceNew 
    key1 = keys[1]
    key1[0] = 0.168 - sourceNew
    key2 = keys[2]
    key2[0] = 0.409 - sourceNew
    key3 = keys[3]
    key3[0] = 0.446 - sourceNew
    key4 = keys[4]
    key4[0] = 0.752 - sourceNew
    key5 = keys[5]
    key5[0] = 0.875 - sourceNew
    return targetOld