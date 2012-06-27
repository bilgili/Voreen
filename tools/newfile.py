import os
import sys

path = {}

#
# Settings
#
headerSource = '../header.txt'
negativeAnswers = ['NO', 'no', 'false', '0', 'n']
path['corePro'] = '../src/core/voreenlib_core.pro'
path['qtPro'] = '../src/qt/voreenlib_qt.pro'

#
# This script is supposed to be executed within voreen/tools (or other directory, as long its one level above the voreen-root
#

def addGPL(file):
    headerFile = open(headerSource, 'r')
    header = headerFile.readlines()

    for line in header:
        file.write(line)

    file.flush()

def writeHFileInformation(file):
    file.write(
        '\n' +
        '#ifndef VRN_' + fileNameUpperCase + '_H\n' +
        '#define VRN_' + fileNameUpperCase + '_H\n' +
        '\n' +
        'namespace voreen {\n' +
        '\n\n' +
        '} // namespace\n' +
        '\n' +
        '#endif // VRN_' + fileNameUpperCase + '_H\n'
        )
    file.flush()

def writeShortCPPFileInformation(file):
    file.write(
        '\n' +
        'namespace voreen {\n' +
        '\n\n' +
        '} // namespace\n'
    )
    file.flush()

def writeCPPFileInformation(file):
    file.write(
        '\n' +
        '#include "' + location + fileName + '.h' '"\n' +
        '\n' +
        'namespace voreen {\n' +
        '\n\n' +
        '} // namespace\n'
    )
    file.flush()

def addToProFile(hPath, cppPath, fileName):
    # adjust pathes for use in pro file
    hPath = '../' + hPath
    if (cppPath.startswith('../src/core/')):
        cppPath = cppPath[12:]
        f = open(path['corePro'])
        directory = 'core'
    elif (cppPath.startswith('../src/qt/')):
        cppPath = cppPath[10:]
        f = open(path['qtPro'])
        directory = 'qt'

    fullCPP = cppPath + fileName + '.cpp \\\n'
    fullH = hPath + fileName + '.h \\\n'


    allLines = f.readlines()
    f.close()

    # iterate over all lines, find the first matching one and insert the cpp line
    for i in range(len(allLines)-1):
        line = allLines[i]
        line = line.lstrip()
        if (line[:len(cppPath)] == cppPath):
            if (line > fullCPP):
                allLines.insert(i, '    ' + fullCPP)
                break

    # iterate over all lines, find the first matching one and insert the h line
    for i in range(len(allLines)-1):
        line = allLines[i]
        line = line.lstrip()
        if (line[:len(hPath)] == hPath):
            if (line > fullH):
                allLines.insert(i, '    ' + fullH)
                break


    # write the lines back to the file
    if (directory == 'core'):
        f = open(path['corePro'], 'w')
    elif (directory == 'qt'):
        f = open(path['qtPro'], 'w')

    for line in allLines:
        f.write(line)

    f.close()
    
location = ''
hLocation = ''
cppLocation = ''
copyFiles = ''
shouldAddToProFile = ''
directory = ''
fullCPP = ''
fullH = ''

# main():
fileName = raw_input('Enter the future filename (without .h or .cpp if you want to have both):\n')
fileNameUpperCase = fileName.upper()

# only proceed if there is a filename
if (fileName != ''):
    # if the filename ends in '.h' or '.cpp' only create this file
    if (fileName.endswith('.h')):
        fileNameUpperCase = fileNameUpperCase[:-2]
        file = open(fileName, 'w')
        addGPL(file)
        writeHFileInformation(file)
        file.close()
        sys.exit()
        
    if (fileName.endswith('.cpp')):
        file = open(fileName, 'w')
        fileNameUpperCase = fileNameUpperCase[:-4]
        addGPL(file)
        writeShortCPPFileInformation(file)
        file.close()
        sys.exit()
        
    location = raw_input('Please enter the future location of the header file as it will appear in the cpp-file (or leave it blank):\n')

    # if the location is not empty and doesn't end with a delimiter, add one
    if (location != '' and location[-1] != '/'):
        location += '/'

    # if the location starts with 'voreen/' we can guess the future file location
    if (location.startswith('voreen/')):
        copyFiles = raw_input('Copy the files to their destinations?\nIf not so, enter "NO", "no", "false", "0" or "n"\n')
        if (not (copyFiles in negativeAnswers)):
            cppLocation = '../src/' + location[location.find('/')+1:]
            hLocation = '../include/voreen/' + location[location.find('/')+1:]

    # don't proceed if one of the files already exists
    if (not(os.path.exists(hLocation + fileName + '.h') or os.path.exists(cppLocation + fileName + '.cpp'))):
        hFile = open(hLocation + fileName + '.h', 'w')
        cppFile = open(cppLocation + fileName + '.cpp', 'w')

        addGPL(hFile)
        addGPL(cppFile)

        writeHFileInformation(hFile)
        writeCPPFileInformation(cppFile)

        hFile.close()
        cppFile.close()

        # only display the pro-file question, if the user already wants the files to be moved
        if ((not (copyFiles in negativeAnswers)) and (location.startswith('voreen/core/') or location.startswith('voreen/qt/'))):
            shouldAddToProFile = raw_input('Enter the files in the appropriate pro-file?\nIf not so, enter "NO", "no", "false", "0" or "n"\n')
            if (not (shouldAddToProFile in negativeAnswers)):
                addToProFile(hLocation, cppLocation, fileName)
        
    else:
        print 'ERROR: Either the .h or .cpp file already existed'

else:
    print 'ERROR: There must be a filename'
     
