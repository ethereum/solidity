#######
安装Solidity
#######

*******
基于浏览器的Solidity
*******


如果你只是想尝试一个使用Solidity的小合约，你不需要安装任何东西，只要访问 [基于浏览器的Solidity](https://chriseth.github.io/browser-solidity)。

如果你想离线使用，你可以保存页面到本地，或者从 http://github.com/chriseth/browser-solidity 克隆一个。

*******
NPM / node.js
*******

这可能安装Solidity到本地最轻便最省事的方法。

在基于浏览器的Solidity上，Emscripten提供了一个跨平台JavaScript库，把C++源码编译为JavaScript，同时也提供NPM安装包。


去安装它就可以简单使用。，

::

    npm install solc

如何使用nodejs包的详细信息可以在[代码库](https://github.com/chriseth/browser-solidity#nodejs-usage)中找到.

*******
二进制安装包
*******


*******
Ethereum.
*******

包括Mix IDE的二进制Solidity安装包在Ethereum网站[C++ bundle](https://github.com/ethereum/webthree-umbrella/releases)中下载。 

*******
从源码构建
*******

在MacOS X、Ubuntu和其它类Unix系统中编译安装Solidity非常相似。这个指南开始讲解如何在每个平台下安装相关的依赖软件，然后构建Solidity。

*******
MacOS X
*******

系统需求:

- OS X Yosemite (10.10.5)

- Homebrew

- Xcode


安装Homebrew：

::

    brew update
    brew install boost --c++11             # 这需要等待一段时间
    brew install cmake cryptopp miniupnpc leveldb gmp libmicrohttpd libjson-rpc-cpp  # 仅仅安装Mix IDE和Alethzero
    brew install xz d-bus
    brew install llvm --HEAD --with-clang 
    brew install qt5 --with-d-bus          # 如果长时间的等待让你发疯，那么添加--verbose输出信息会让你感觉更好。

*******
Ubuntu 系统
*******

下面是在最新版Ubuntu系统上编译安装Solidity的指南。最佳的支持平台是2014年11月发布的64位Ubuntu 14.04，至少需要2GB内存。我们所有的测试都是基于此版本，当然我们也欢迎其它版本的测试贡献者。

*******
安装依赖软件：
*******

在你从源码编译之前，你需要准备一些工具和依赖软件。

首先，升级你的代码库。Ubuntu主代码库不提供所有的包，你需要从Ethereum PPA和LLVM获取。

注意

Ubuntu 14.04的用户需要使用：`sudo apt-add-repository ppa:george-edison55/cmake-3.x`获取最新版本的cmake。

现在加入其它的包：

::

    sudo apt-get -y update
    sudo apt-get -y install language-pack-en-base
    sudo dpkg-reconfigure locales
    sudo apt-get -y install software-properties-common
    sudo add-apt-repository -y ppa:ethereum/ethereum
    sudo add-apt-repository -y ppa:ethereum/ethereum-dev
    sudo apt-get -y update
    sudo apt-get -y upgrade


对于Ubbuntu 15.04（Vivid Vervet）或者更老版本，使用下面的命令获取开发相关的包：

::

    sudo apt-get -y install build-essential git cmake libboost-all-dev libgmp-dev libleveldb-dev libminiupnpc-dev libreadline-dev libncurses5-dev libcurl4-openssl-dev libcryptopp-dev libjson-rpc-cpp-dev libmicrohttpd-dev libjsoncpp-dev libedit-dev libz-dev


对于Ubbuntu 15.10（Wily Werewolf）或者更新版本，使用下面的命令获取开发相关的包：

::

    sudo apt-get -y install build-essential git cmake libboost-all-dev libgmp-dev libleveldb-dev libminiupnpc-dev libreadline-dev libncurses5-dev libcurl4-openssl-dev libcryptopp-dev libjsonrpccpp-dev libmicrohttpd-dev libjsoncpp-dev libedit-dev libz-dev


不同版本使用不同获取命令的原因是，libjsonrpccpp-dev已经在最新版的Ubuntu的通用代码仓库中。

*******
编译
*******

如果你只准备安装solidity，忽略末尾Alethzero和Mix的错误。

::

    git clone --recursive https://github.com/ethereum/webthree-umbrella.git
    cd webthree-umbrella
    ./webthree-helpers/scripts/ethupdate.sh --no-push --simple-pull --project solidity # 更新Solidity库
    ./webthree-helpers/scripts/ethbuild.sh --no-git --project solidity --all --cores 4 -DEVMJIT=0 # 编译Solidity及其它 在OS X系统加上DEVMJIT将不能编译，在Linux系统上则没问题  

如果你选择安装Alethzero和Mix：

::

    git clone --recursive https://github.com/ethereum/webthree-umbrella.git
    cd webthree-umbrella && mkdir -p build && cd build
    cmake ..


如果你想帮助Solidity的开发，你需要分支（fork）Solidity并添加到你的私人远端分支：

::

    cd webthree-umbrella/solidity
    git remote add personal git@github.com:username/solidity.git

注意webthree-umbrella使用的子模块,所以solidity是其自己的git代码库，但是他的设置不是保存在 `.git/config`, 而是在`webthree-umbrella/.git/modules/solidity/config`.
