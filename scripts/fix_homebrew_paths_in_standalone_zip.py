# ------------------------------------------------------------------------------
# This Python script is used within the OS X release process, to ensure
# that the standalone OS X ZIP files which we make are actually
# standalone, and not implicitly dependent on Homebrew installs for
# external libraries which we use.
#
# This implicit dependencies seem to show up only where we have
# external dependencies which are dependent on each other, and the
# path from one to another is an absolute path to "/usr/local/opt",
# the Homebrew install location.   External dependencies which only
# depend on system libraries are fine.   Our main applications seem
# to be fine.
#
# An example of a dependency which requires this fix-up at the time
# of writing is the following dependency edge:
#
# libjsonrpccpp-client.0.dylib
#     -> /usr/local/opt/jsoncpp/lib/libjsoncpp.0.dylib
#
# See https://blogs.oracle.com/dipol/entry/dynamic_libraries_rpath_and_mac
# for a little overview of "install_name_tool" and "otool".
#
# ------------------------------------------------------------------------------
# This file is part of solidity.
#
# solidity is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# solidity is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with solidity.  If not, see <http://www.gnu.org/licenses/>
#
# (c) 2016 solidity contributors.
# -----------------------------------------------------------------------------

import os
import subprocess
import sys


def readDependencies(fname):
    with open(fname) as f:
        o = subprocess.Popen(['otool', '-L', fname], stdout=subprocess.PIPE)
        for line in o.stdout:
            if line[0] == '\t':
                library = line.split(' ', 1)[0][1:]
                if library.startswith("/usr/local/lib") or library.startswith("/usr/local/opt") or library.startswith("/Users/"):
                    if (os.path.basename(library) != os.path.basename(fname)):
                        command = "install_name_tool -change " + \
                            library + " @executable_path/./" + \
                            os.path.basename(library) + " " + fname
                        print command
                        os.system("chmod +w " + fname)
                        os.system(command)

root = sys.argv[1]
for (dirpath, dirnames, filenames) in os.walk(root):
    for filename in filenames:
        readDependencies(os.path.join(root, filename))
