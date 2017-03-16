FROM alpine
MAINTAINER chriseth <chris@ethereum.org>
#Official solidity docker image

#Establish working directory as solidity
WORKDIR /solidity
#Copy working directory on travis to the image
COPY / $WORKDIR

#Install dependencies, eliminate annoying warnings, and build release, delete all remaining points and statically link.
RUN ./scripts/install_deps.sh && sed -i -E -e 's/include <sys\/poll.h>/include <poll.h>/' /usr/include/boost/asio/detail/socket_types.hpp &&\
cmake -DCMAKE_BUILD_TYPE=Release -DTESTS=0 -DSTATIC_LINKING=1 &&\
make solc && install -s  solc/solc /usr/bin	&&\
cd / && rm -rf solidity &&\
apk del sed build-base git make cmake gcc g++ musl-dev curl-dev boost-dev &&\
rm -rf /var/cache/apk/*

ENTRYPOINT ["/usr/bin/solc"]