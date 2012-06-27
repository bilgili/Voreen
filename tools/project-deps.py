import re
import os

dependencies = {}
############### config ###############
dependencies['voreen_qt'] = ['voreen_core']
dependencies['voreenve'] = ['voreen_core','voreen_qt']
######################################

#
# This script is supposed to be executed within voreen/tools (or other directory, as long its one above the voreen-root
#

# Explanation:
# A visual studio solution file is structured as follows:
#
# Microsoft Visual Studio Solution File, Format Version 10.00
# # Visual Studio 2008
# Project("{some id}") = "project name", "project source", "{project id}"
# EndProject
# Project("{some id}") = "project name", "project source", "{project id}"
#	ProjectSection(ProjectDependencies) = postProject
#		{other project id} = {other project id}
#	EndProjectSection
# EndProject
#
# All this script has to do is to extract the project ids affiliated with the project names, and add the "ProjectSection" parts
# in the appropriate projects.
# In addition to that, we have to:
#   - add the "# Visual Studio 2008" line because qmake doesn't include it
#   - convert absolute to relative paths
#

# main()

# first read the solution file
f = open('..\\voreen.sln', 'r')
allLines = f.readlines()
f.close()

# then add the line necessary for the double-click start-ability
allLines[1:1] = ['# Visual Studio 2008\n']

# fill the dictionary that will hold all the "name":id pairs found in the solution
dictionary = {}
for line in allLines:
    # only look in lines beginning with Project(
    if (line[:8] == 'Project('):
        name = re.findall('"[a-zA-Z0-9_]*"', line)[0]
        name = name[1:len(name)-1] # Remove the surrounding "
        id = re.findall( '{[-A-Z0-9]*}', line)
        id = id[1] # The first entry is the solution(?) id and constant
        dictionary[name] = id

# now scan through the file and add the corresponding dependencies
for i in range(len(allLines)-1, 1, -1):
    line = allLines[i]
    if (line[:8] == 'Project('):
        name = re.findall('"[a-zA-Z0-9_]*"', line)[0]
        name = name[1:len(name)-1]
        if (name in dependencies):
            j = 1
            allLines[i+j:i+j] = ['\tProjectSection(ProjectDependencies) = postProject\n']
            for dep in dependencies[name]:
                j += 1
                allLines[i+j:i+j] = ['\t\t' + dictionary[dep] + ' = ' + dictionary[dep] + '\n']
            allLines[i+j+1:i+j+1] = ['\tEndProjectSection\n']

# lastly, convert absolute to relative paths
pwd = os.getcwd() + '\\'
pwd = pwd.replace('\\', '/')
for i in range(1, len(allLines)):
    allLines[i] = allLines[i].replace(pwd, '')

# and save the file
f = open('..\\voreen.sln', 'w')
for line in allLines:
    f.write(line)
f.close()
