#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script for publishing Solidity Emscripten binaries to Github.
#
# The results are committed to https://github.com/ethereum/solc-bin.
#
# The documentation for solidity is hosted at:
#
# http://solidity.readthedocs.io/
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

VER=$(cat CMakeLists.txt | grep 'set(PROJECT_VERSION' | sed -e 's/.*set(PROJECT_VERSION "\(.*\)".*/\1/')
test -n "$VER"
VER="v$VER"
COMMIT=$(git rev-parse --short=8 HEAD)
DATE=$(date --date="$(git log -1 --date=iso --format=%ad HEAD)" --utc +%Y.%-m.%-d)

# remove leading zeros in components - they are not semver-compatible
COMMIT=$(echo "$COMMIT" | sed -e 's/^0*//')

ENCRYPTED_KEY_VAR="encrypted_${ENCRYPTION_LABEL}_key"
ENCRYPTED_IV_VAR="encrypted_${ENCRYPTION_LABEL}_iv"
ENCRYPTED_KEY=${!ENCRYPTED_KEY_VAR}
ENCRYPTED_IV=${!ENCRYPTED_IV_VAR}
openssl aes-256-cbc -K $ENCRYPTED_KEY -iv $ENCRYPTED_IV -in scripts/travis-emscripten/deploy_key.enc -out deploy_key -d
chmod 600 deploy_key
eval `ssh-agent -s`
ssh-add deploy_key

git clone --depth 2 git@github.com:ethereum/solc-bin.git
cd solc-bin
git config user.name "travis"
git config user.email "chris@ethereum.org"
git checkout -B gh-pages origin/gh-pages
git clean -f -d -x


FULLVERSION=INVALID
if [ "$TRAVIS_BRANCH" = release ]
then
    # We only want one file with this version
    if ls ./bin/soljson-"$VER+"*.js
    then
      echo "Not publishing, we already published this version."
      exit 0
    fi
    FULLVERSION="$VER+commit.$COMMIT"
elif [ "$TRAVIS_BRANCH" = develop ]
then
    # We only want one release per day and we do not want to push the same commit twice.
    if ls ./bin/soljson-"$VER-nightly.$DATE"*.js || ls ./bin/soljson-*"commit.$COMMIT.js"
    then
      echo "Not publishing, we already published this version today."
      exit 0
    fi
    FULLVERSION="$VER-nightly.$DATE+commit.$COMMIT"
else
    echo "Not publishing, wrong branch."
    exit 0
fi


NEWFILE=./bin/"soljson-$FULLVERSION.js"

# Prepare for update script
npm install

# This file is assumed to be the product of the build_emscripten.sh script.
cp ../soljson.js "$NEWFILE"

# Run update script
npm run update

# Publish updates
git add "$NEWFILE"
git commit -a -m "Added compiler version $FULLVERSION"
git push origin gh-pages
