FROM alpine AS build

#Establish working directory as solidity
WORKDIR /solidity

# Build dependencies 
RUN apk update && \
    apk add --no-cache --no-install-recommends \
        boost-dev \
        boost-static \
        build-base \
        cmake \
        git \
        clang && \
    rm -rf /var/cache/apk/*

#Copy working directory to the image
COPY . /solidity

# Number of parallel jobs during build
# or 0 for auto-computing (max(1, CPU_core_count * 2/3), a greedy value)
ARG BUILD_CONCURRENCY="0"

#Install dependencies, eliminate annoying warnings
RUN sed -i -E -e 's/include <sys\/poll.h>/include <poll.h>/' /usr/include/boost/asio/detail/socket_types.hpp
RUN cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release -DTESTS=0 -DSOLC_LINK_STATIC=1
RUN make solc \
    -j$(awk "BEGIN {                                       \
        if (${BUILD_CONCURRENCY} != 0) {                   \
            print(${BUILD_CONCURRENCY});                   \
        } else {                                           \
            x=($(grep -c ^processor /proc/cpuinfo) * 2/3); \
            if (x > 1) {                                   \
                printf(\"%d\n\", x);                       \
            } else {                                       \
                print(1);                                  \
            }                                              \
        }                                                  \
    }")
RUN strip solc/solc

FROM scratch
LABEL org.opencontainers.image.source=https://github.com/argotorg/solidity
LABEL maintainer="Solidity Team <solidity@argot.org>"
COPY --from=build /solidity/solc/solc /usr/bin/solc
ENTRYPOINT ["/usr/bin/solc"]
