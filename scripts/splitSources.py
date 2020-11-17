#!/usr/bin/env python3
#
# This script reads a syntaxTest file and writes all
# sources into their own files. If one source-name specifies subdirectories
# those will be created too.

# Usage: scripts/splitSources.py pathToTestfile
# as a result prints
# -  string of created files separated by whitespaces
# -  nothing if the input file contains only one source

import sys
import os
import traceback

hasMultipleSources = False
createdSources = []


def extractSourceName(line):
    if line.find("/") > -1:
        filePath = line[13: line.rindex("/")]
        # fileName = line[line.rindex("/")+1: line.find(" ====")]
        srcName = line[line.find(":")+2: line.find(" ====")]
        return filePath, srcName
    return False, line[line.find(":")+2 : line.find(" ====")]


# expects the first line of lines to be "==== Source: sourceName ===="
# writes the following source into a file named sourceName
def writeSourceToFile(lines):
    filePath, srcName = extractSourceName(lines[0])
    # print("sourceName is ", srcName)
    # print("filePath is", filePath)
    if filePath:
        os.system("mkdir -p " + filePath)
    with open(srcName, mode='a+', encoding='utf8', newline='') as f:
        createdSources.append(srcName)
        for idx, line in enumerate(lines[1:]):
            # write to file
            if line[:12] != "==== Source:":
                f.write(line)

            # recursive call if there is another source
            else:
                writeSourceToFile(lines[1+idx:])
                break


if __name__ == '__main__':
    filePath = sys.argv[1]

    # decide if file has multiple sources
    with open(filePath, mode='rb', encoding='utf8', newline='') as f:
        lines = f.read().splitlines()
    if len(lines) >= 1 and lines[0][:12] == "==== Source:":
        hasMultipleSources = True
        writeSourceToFile(lines)

    if hasMultipleSources:
        srcString = ""
        for src in createdSources:
            srcString += src + ' '
        print(srcString)
    else:
        print(filePath)
