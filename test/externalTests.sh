#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script to run external Solidity tests.
#
# Argument: Path to soljson.js to test.
#
# Requires npm, networking access and git to download the tests.
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
#------------------------------------------------------------------------------

set -e

if [ ! -f "$1" ]
then
  echo "Usage: $0 <path to soljson.js>"
  exit 1
fi

SOLJSON="$1"

DIR=$(mktemp -d)
(
    echo "Running Zeppelin tests..."
    git clone --depth 1 https://github.com/OpenZeppelin/zeppelin-solidity.git "$DIR"
    cd "$DIR"
    npm install
    find . -name soljson.js -exec cp "$SOLJSON" {} \;

    # This is a patch that lets truffle ignore the pre-release compiler warning
    cat > truffle.patch <<EOF
--- node_modules/truffle/build/cli.bundled.js	2017-11-27 16:56:47.114830112 +0100
+++ /tmp/patched	2017-11-27 16:52:31.887064115 +0100
@@ -313846,9 +313846,12 @@
     });
 
     output = JSON.parse(output);
+	var errors = output.errors.filter(function(solidity_error) {
+		return solidity_error.formattedMessage.indexOf("pre-release compiler") < 0;
+    });
 
-    if (output.errors) {
-      throw new CompileError(output.errors[0].formattedMessage);
+    if (errors) {
+      throw new CompileError(errors[0].formattedMessage);
     }
 
     return {
@@ -313901,9 +313904,13 @@
       return {error: importErrorKey};
     });
 
-    output = JSON.parse(output);
+	output = JSON.parse(output);
+	
+	var errors = output.errors.filter(function(solidity_error) {
+		return solidity_error.formattedMessage.indexOf("pre-release compiler") < 0;
+    });
 
-    var nonImportErrors = output.errors.filter(function(solidity_error) {
+    var nonImportErrors = errors.filter(function(solidity_error) {
       // If the import error key is not found, we must not have an import error.
       // This means we have a *different* parsing error which we should show to the user.
       // Note: solc can return multiple parsing errors at once.
@@ -313917,7 +313924,7 @@
 
     // Now, all errors must be import errors.
     // Filter out our forced import, then get the import paths of the rest.
-    var imports = output.errors.filter(function(solidity_error) {
+    var imports = errors.filter(function(solidity_error) {
       return solidity_error.message.indexOf(failingImportFileName) < 0;
     }).map(function(solidity_error) {
       var matches = solidity_error.formattedMessage.match(/import[^'"]+("|')([^'"]+)("|');/);
EOF

    patch node_modules/truffle/build/cli.bundled.js ./truffle.patch
    npm run test
)
rm -rf "$DIR"
