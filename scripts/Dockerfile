FROM alpine
MAINTAINER chriseth <chris@ethereum.org>

#Copy working directory on travis to the image
ADD solidity/ /

#Establish working directory as soliditiy
WORKDIR solidity/

#Install dependencies, eliminate annoying warnings, test if we are in a test environment, if so, 
#add in soltest to the binary. If not, assume this is basic solidity image, build it and ship it out.
RUN \
  ./scripts/install_deps.sh                                           									    										&& \
  sed -i -E -e 's/include <sys\/poll.h>/include <poll.h>/' /usr/include/boost/asio/detail/socket_types.hpp  										&& \
  [[$TEST=On]] && cmake -DCMAKE_BUILD_TYPE=Release -DSTATIC_LINKING=1 || cmake -DCMAKE_BUILD_TYPE=Release -DTESTS=0 -DSTATIC_LINKING=1      	    && \
  [[$TEST=On]] && (make solc && install -s solc/solc /usr/bin && install -s test/soltest /usr/bin) || (make solc && install -s  solc/solc /usr/bin) && \
  cd / && rm -rf solidity                                                                                   										&& \
  apk del sed build-base git make cmake gcc g++ musl-dev curl-dev boost-dev                                 										&& \
  rm -rf /var/cache/apk/*
