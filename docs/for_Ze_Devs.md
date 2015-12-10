### Preparing for Compiler Development on OS X

# Requirements
- OS X Yosemite (10.10.5)
- Homebrew
- Xcode

# Set up Homebrew
```
brew update
brew install boost --c++11             # this takes a while
brew install cmake cryptopp miniupnpc leveldb gmp libmicrohttpd libjson-rpc-cpp 
#for Mix IDE and Alethzero only
brew install xz d-bus
brew install llvm --HEAD --with-clang 
brew install qt5 --with-d-bus          # add --verbose if long waits with a stale screen drive you crazy as well
```

# The Long Wait

Run this if you plan on installing solidity only, ignore errors at the end as they relate only to Alethzero and Mix
```
git clone --recursive https://github.com/ethereum/webthree-umbrella.git
webthree-helpers/scripts/ethupdate.sh --no-push --simple-pull --project solidity #update solidity repo
./webthree-helpers/scripts/ethbuild.sh --no-git --project solidity --all --cores 4 #install solidity
```
if you opted to install Alethzero and Mix:
```
git clone --recursive https://github.com/ethereum/webthree-umbrella.git
cd webthree-umbrella && mkdir -p build && cd build
cmake ..
```

# Bringing it all together
clone your forked repo, then:

```
cd ~/webthree-helpers/solidity
mv build ../../yourForkedRepo
```

you are now ready to develop :) 
