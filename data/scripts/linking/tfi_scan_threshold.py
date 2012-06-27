def tfi_scan_threshold (sourceOld, sourceNew, targetOld):
    sourceNew = sourceNew / 255.0
    thresholds = targetOld[0]
    thresholds[0] = sourceNew
    thresholds[1] = sourceNew + 0.33
    return targetOld