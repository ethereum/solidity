 # Check for Homebrew install and abort if it is not installed.
brew -v > /dev/null 2>&1 || { echo >&2 "ERROR - solidity requires a Homebrew install.  See http://brew.sh."; exit 1; }

brew update
brew upgrade cmake

brew install boost

brew tap ethereum/ethereum
#install with verbose mode to keep travis from timing out
brew install cpp-ethereum -v
brew linkapps cpp-ethereum