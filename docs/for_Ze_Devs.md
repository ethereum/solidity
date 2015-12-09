### Preparing for Compiler Development on OS X

# Requirements
- OS X Yosemite (10.10.5)
- Homebrew
- Xcode

# Set up Homebrew
```
brew update
brew install boost --c++11 # this takes a while
brew install cmake qt5 cryptopp miniupnpc leveldb gmp libmicrohttpd libjson-rpc-cpp 
brew install llvm --HEAD --with-clang
```

# Clone source repo and run this script
```
git clone --recursive https://github.com/ethereum/webthree-umbrella.git
webthree-helpers/scripts/ethupdate.sh --no-push --simple-pull --project solidity #update solidity repo
./webthree-helpers/scripts/ethbuild.sh --no-git --build-type Debug --project solidity --all --cores 4 -DFATDB=0 -DEVMJIT=0 -DETHASHCL=0 #install solidity
```

# Bringing it all together
clone your forked repo, then:

```
cd ~/webthree-helpers/solidity
mv build ../../yourForkedRepo
```

you are now ready to develop :) 
