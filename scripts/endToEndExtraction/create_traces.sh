BASE_PATH="$( cd "$(dirname "$0")" >/dev/null 2>&1 || exit ; pwd -P )"

mkdir -p build
cd build || exit
cmake ../../../
make soltest
cd test/ || exit
echo "running soltest on 'semanticTests/extracted'..."
./soltest --color_output=false --log_level=test_suite -t semanticTests/extracted/ -- --testpath ${BASE_PATH}/../../test --no-smt --evmonepath /Users/alex/evmone/lib/libevmone.dylib --show-messages --show-metadata > ${BASE_PATH}/extracted-tests.trace
echo "running soltest on 'semanticTests/extracted'... done"

cd $BASE_PATH || exit
git clone git@github.com:ethereum/solidity.git solidity-develop
cd solidity-develop || exit
mkdir -p build
cd build || exit
cmake ..
make soltest
cd test/ || exit
echo "running soltest on 'SolidityEndToEndTest'..."
./soltest --color_output=false --log_level=test_suite -t SolidityEndToEndTest/ -- --testpath ${BASE_PATH}/solidity-develop/test --no-smt --evmonepath /Users/alex/evmone/lib/libevmone.dylib --show-messages --show-metadata > ${BASE_PATH}/endToEndExtraction-tests.trace
echo "running soltest on 'SolidityEndToEndTest'... done"
