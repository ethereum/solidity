#!/usr/bin/env sh


if ["$DOCKER_SOLC" = On]; then
	#Build the alpine container with soltest included as well
	docker build --build-arg TEST=On -t ethereum/solc:build -f scripts/Dockerfile .

	#Run soltest against the cpp client container with a link between the two containers
	echo "--> Running tests without optimizer..."
	docker run -it --volumes-from testEth --ipc container:testEth ethereum/solc:build soltest -- --ipcpath /tmp/testeth/geth.ipc
	echo "--> Running tests WITH optimizer..."
	docker run -it --volumes-from testEth --ipc container:testEth ethereum/solc:build soltest -- --optimize --ipcpath /tmp/testeth/geth.ipc
else

fi
ERROR_CODE=$?
exit $ERROR_CODE