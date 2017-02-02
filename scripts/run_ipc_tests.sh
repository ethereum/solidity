#!/usr/bin/env sh

#mount volume so that host can access the tmp directory and the geth.ipc process
docker run -di \
	--volume /tmp:/tmp      \
	--name testEth          \
	ethereum/client-cpp     \
	--test -d /tmp/testeth  

echo "--> Running tests without optimizer..."
soltest --run_test=SolidityOptimizer,GasMeterTests,SolidityEndToEndTest -- --ipcpath=/tmp/testeth/geth.ipc && \
	echo "--> Running tests WITH optimizer..." && \
	soltest --run_test=SolidityOptimizer,GasMeterTests,SolidityEndToEndTest -- --optimize --ipcpath=/tmp/testeth/geth.ipc
ERROR_CODE=$?
docker kill testEth
exit $ERROR_CODE