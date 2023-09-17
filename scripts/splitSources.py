#!/usr/bin/env python3
#
# This script reads a syntaxTest file and writes all
# sources into their own files. If one source-name specifies subdirectories
# those will be created too.

# Usage: scripts/splitSources.py pathToTestfile
# as a result prints
# -  string of created files separated by whitespaces
# -  'false' if the file only had one source

import sys
import os
import traceback


def uncaught_exception_hook(exc_type, exc_value, exc_traceback):
    # The script `scripts/ASTImportTest.sh` will interpret return code 3
    # as a critical error (because of the uncaught exception) and will
    # terminate further execution.
    print("Unhandled exception: %s", "".join(traceback.format_exception(exc_type, exc_value, exc_traceback)))
    sys.exit(3)


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
        for idx, line in enumerate(lines[1:]):
            # write to file
            if line[:12] != "==== Source:":
                f.write(line + '\n')

            # recursive call if there is another source
            else:
                return [srcName] + writeSourceToFile(lines[1+idx:])

    return [srcName]

def split_sources(filePath, suppress_output = False):
    sys.excepthook = uncaught_exception_hook

    try:
        # decide if file has multiple sources
        with open(filePath, mode='r', encoding='utf8', newline='') as f:
            lines = f.read().splitlines()
        if len(lines) >= 1 and lines[0][:12] == "==== Source:":
            srcString = ""
            for src in writeSourceToFile(lines):
                srcString += src + ' '
            if not suppress_output:
                print(srcString)
            return 0
        return 1

    except UnicodeDecodeError as ude:
        print("UnicodeDecodeError in '" + filePath + "': " + str(ude))
        print("This is expected for some tests containing invalid utf8 sequences. "
              "Exception will be ignored.")
        return 2


if __name__ == '__main__':
    sys.exit(split_sources(sys.argv[1]))
