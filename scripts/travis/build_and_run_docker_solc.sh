#!/usr/bin/env sh

case $DOCKER_OS in
Alpine)
	#Build the alpine container with soltest included as well
	docker build --build-arg TEST=On -t ethereum/solc:build -f scripts/Dockerfile .
	;;
Xenial)
	#Build the xenial container for testing
	docker build --build-arg TEST=On -t ethereum/solc:build -f scripts/Dockerfile.UbuntuXenial .
	;;
Arch)
	#Build the arch linux container for testing
	docker build --build-arg TEST=On -t ethereum/solc:build -f scripts/Dockerfile.Arch .
	;;
Fedora)
	#Build the fedora container for testing
	docker build --build-arg TEST=On -t ethereum/solc:build -f scripts/Dockerfile.Fedora .
	;;
Jessie)
	#Build the debian jessie container for testing
	docker build --build-arg TEST=On -t ethereum/solc:build -f scripts/Dockerfile.DebianJessie .
	;;
*)
    #other
    echo "ERROR - Unsupported or unidentified operating system."
    echo "See http://solidity.readthedocs.io/en/latest/installing-solidity.html for manual instructions."
    echo "If you would like to get your operating system working, that would be fantastic."
    echo "Drop us a message at https://gitter.im/ethereum/solidity."
    ;;

#Run soltest against the cpp client container with a link between the two containers
docker run -it --volumes-from testEth --ipc container:testEth ethereum/solc:build soltest -- --ipcpath /tmp/testeth/geth.ipc