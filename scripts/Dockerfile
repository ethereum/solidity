FROM alpine AS build
MAINTAINER chriseth <chris@ethereum.org>
#Official solidity docker image

#Establish working directory as solidity
WORKDIR /solidity

# Build dependencies
ADD /scripts/install_deps.sh /solidity/scripts/install_deps.sh
RUN ./scripts/install_deps.sh

#Copy working directory on travis to the image
COPY / $WORKDIR

#Install dependencies, eliminate annoying warnings
RUN sed -i -E -e 's/include <sys\/poll.h>/include <poll.h>/' /usr/include/boost/asio/detail/socket_types.hpp
RUN cmake -DCMAKE_BUILD_TYPE=Release -DTESTS=0 -DSOLC_LINK_STATIC=1
RUN make solc -j$(grep -c ^processor /proc/cpuinfo) && install -s  solc/solc /usr/bin
#RUN strip solc/solc

FROM scratch
COPY --from=build /solidity/solc/solc /usr/bin/solc
ENTRYPOINT ["/usr/bin/solc"]
