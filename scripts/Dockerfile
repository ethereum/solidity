FROM alpine
MAINTAINER chriseth <chris@ethereum.org>
#Official solidity docker image

#Establish working directory as soliditiy
WORKDIR /solidity
#Determine whether or not we are in the testing suite
ARG TEST=Off
#Determine what build type we are using
ARG BUILD=Release
#Copy working directory on travis to the image
COPY / $WORKDIR

#Install dependencies, eliminate annoying warnings, test if we are in a test environment, if so, 
#add in soltest to the binary. If not, assume this is basic solidity image, build it and ship it out.
RUN ./scripts/install_deps.sh && sed -i -E -e 's/include <sys\/poll.h>/include <poll.h>/' /usr/include/boost/asio/detail/socket_types.hpp
RUN test $TEST == On && cmake -DCMAKE_BUILD_TYPE=$BUILD -DSTATIC_LINKING=1 || cmake -DCMAKE_BUILD_TYPE=$BUILD -DTESTS=0 -DSTATIC_LINKING=1
RUN test $TEST == On && (make && install -s solc/solc /usr/bin && install -s test/soltest /usr/bin) || (make solc && install -s  solc/solc /usr/bin)
RUN cd / && rm -rf solidity
RUN apk del sed build-base git make cmake gcc g++ musl-dev curl-dev boost-dev
RUN rm -rf /var/cache/apk/*
