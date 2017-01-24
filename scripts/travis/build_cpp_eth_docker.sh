#!/usr/bin/env sh

#pull docker image of cpp client
docker pull ethereum/client-cpp

#mount a volume for the ipc testing and set up a testing node
if ["$DOCKER_SOLC" = On]; then
	#mount volume so that docker solc container can access the tmp directory and the geth.ipc process
	docker run -d \
		--volume /tmp          \
		--name testEth         \
		ethereum/client-cpp    \
		--test -d /tmp/testeth
else
	#mount volume so that host can access the tmp directory and the geth.ipc process
	docker run -di \
		--volume /tmp:/tmp      \
		--name testEth          \
		ethereum/client-cpp     \
		--test -d /tmp/testeth  
fi

ERROR_CODE=$?
exit $ERROR_CODE