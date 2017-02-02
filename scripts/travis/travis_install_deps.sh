#!/usr/bin/env sh

set -e

# Check for 'uname' and abort if it is not available.
uname -v > /dev/null 2>&1 || { echo >&2 "ERROR - solidity requires 'uname' to identify the platform."; exit 1; }

case $(uname -s) in

#------------------------------------------------------------------------------
# macOS
#------------------------------------------------------------------------------

    Darwin)
        case $(sw_vers -productVersion | awk -F . '{print $1"."$2}') in
            10.9)
                echo "Installing solidity dependencies on OS X 10.9 Mavericks."
                ;;
            10.10)
                echo "Installing solidity dependencies on OS X 10.10 Yosemite."
                ;;
            10.11)
                echo "Installing solidity dependencies on OS X 10.11 El Capitan."
                ;;
            10.12)
                echo "Installing solidity dependencies on macOS 10.12 Sierra."
                ;;
            *)
                echo "Unsupported macOS version."
                echo "We only support Mavericks, Yosemite and El Capitan, with work-in-progress on Sierra."
                exit 1
                ;;
        esac

        # Check for Homebrew install and abort if it is not installed.
        brew -v > /dev/null 2>&1 || { echo >&2 "ERROR - solidity requires a Homebrew install.  See http://brew.sh."; exit 1; }

      	brew update
		brew upgrade cmake

		brew install boost

        ;;

#------------------------------------------------------------------------------
# Linux
#------------------------------------------------------------------------------

    Linux)
#Since it's Travis, we assume that this is Ubuntu Trusty Tahr for now
        sudo apt-get -y update
        sudo apt-get -y install \
            build-essential \
            cmake \
            git \
            libboost-all-dev
        ;;
esac
