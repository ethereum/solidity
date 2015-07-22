FROM ubuntu:14.04

ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update
RUN apt-get upgrade -y

# Ethereum dependencies
RUN apt-get install -qy build-essential git cmake libcurl4-openssl-dev wget
RUN apt-get install -qy automake libtool yasm scons 

RUN useradd -ms /bin/bash user
USER user
ENV HOME /home/user
WORKDIR /home/user

# Emscripten SDK
RUN wget -c https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz
RUN tar xzf emsdk-portable.tar.gz
WORKDIR /home/user/emsdk_portable
RUN ./emsdk update && ./emsdk install latest && ./emsdk activate latest
ENV PATH $PATH:/home/user/emsdk_portable:/home/user/emsdk_portable/clang/fastcomp/build_master_64/bin:/home/user/emsdk_portable/emscripten/master

USER root
RUN apt-get install -qy nodejs
USER user
RUN sed -i "s/NODE_JS = 'node'/NODE_JS = 'nodejs'/g" ~/.emscripten 

# CryptoPP
WORKDIR /home/user
RUN git clone https://github.com/mmoss/cryptopp.git
WORKDIR /home/user/cryptopp
RUN emcmake cmake -DCRYPTOPP_LIBRARY_TYPE=STATIC -DCRYPTOPP_RUNTIME_TYPE=STATIC && emmake make -j 4
RUN ln -s . src/cryptopp

# Boost
WORKDIR /home/user
RUN wget 'http://downloads.sourceforge.net/project/boost/boost/1.57.0/boost_1_57_0.tar.bz2?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fboost%2Ffiles%2Fboost%2F1.57.0%2F&ts=1421887207&use_mirror=cznic' -O boost_1_57_0.tar.bz2
RUN tar xjf boost_1_57_0.tar.bz2
WORKDIR /home/user/boost_1_57_0
RUN ./bootstrap.sh --with-libraries=thread,system,regex
RUN sed -i 's/using gcc ;/using gcc : : \/home\/user\/emsdk_portable\/emscripten\/master\/em++ ;/g' ./project-config.jam
RUN sed -i 's/$(archiver\[1\])/\/home\/user\/emsdk_portable\/emscripten\/master\/emar/g' ./tools/build/src/tools/gcc.jam
RUN sed -i 's/$(ranlib\[1\])/\/home\/user\/emsdk_portable\/emscripten\/master\/emranlib/g' ./tools/build/src/tools/gcc.jam
RUN ./b2 link=static variant=release threading=single runtime-link=static thread system regex

# Json-CPP
WORKDIR /home/user
RUN git clone https://github.com/open-source-parsers/jsoncpp.git
WORKDIR /home/user/jsoncpp
RUN emcmake cmake -DJSONCPP_LIB_BUILD_STATIC=ON -DJSONCPP_LIB_BUILD_SHARED=OFF -DJSONCPP_WITH_TESTS=OFF -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF -G "Unix Makefiles" .
RUN emmake make

## Build soljs
WORKDIR /home/user
ADD https://api.github.com/repos/ethereum/cpp-ethereum/git/refs/heads/develop unused.txt
RUN git clone --depth=1 https://github.com/ethereum/cpp-ethereum
WORKDIR /home/user/cpp-ethereum
RUN git config --global user.email "me@example.com"
RUN git config --global user.name "Jane Doe"
ADD https://api.github.com/repos/chriseth/cpp-ethereum/git/refs/heads/solidity-js unused2.txt
RUN git remote add -f solidityjs https://github.com/chriseth/cpp-ethereum
# TODO this should be a proper merge but somehow causes problems
# NOTE that we only get the latest commit of that branch
RUN git cherry-pick solidityjs/solidity-js
RUN emcmake cmake -DETH_STATIC=1 -DSOLIDITY=ON -DGUI=0 -DCMAKE_CXX_COMPILER=/home/user/emsdk_portable/emscripten/master/em++ -DCMAKE_C_COMPILER=/home/user/emsdk_portable/emscripten/master/emcc
RUN emmake make -j 6 soljson

WORKDIR /home/user/cpp-ethereum/solc
ENTRYPOINT cat soljson.js

