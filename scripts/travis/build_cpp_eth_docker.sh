#!/usr/bin/env sh

#pull docker image of cpp client
docker pull ethereum/client-cpp

#mount a volume for the ipc testing and set up a testing node
docker run -d -v /tmp --name testEth ethereum/client-cpp --test -d /tmp/testeth