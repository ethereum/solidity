#!/usr/bin/env bash

create_bin_artifacts() {
	for file in {*.sol,*}; do 
		# eliminate warnings
		echo 'pragma solidity >=0.0;' >> "$file"
		output=$("$2" --bin "$file" 2>&1)
		output=$(echo "$output" | grep -v 'pre-release')
	    echo "$output"
	done
}

SOLTEST="soltest --run_test=SolidityASTJSON,Assembly,SolidityImports,SolidityInlineAssembly,SemVerMatcher,SolidityNameAndTypeResolution,SolidityABIJSON,SolidityExpressionCompiler,SolidityTypes,SolidityNatspecJSON,SolidityParser,SolidityScanner"

cd "$TRAVIS_BUILD_DIR"
mkdir -p solcbins
cd ./solcbins
../scripts/isolate_tests.py \
	../test/libsolidity/SolidityEndToEndTest.cpp \
	../test/libsolidity/SolidityOptimizer.cpp    \
	../test/libsolidity/GasMeter.cpp

case "$OS_VERSION" in
	"OSX-xcode6.4"|"OSX-xcode7.3"|"OSX-xcode8"|"OSX-xcode8.1"|"OSX-xcode8.2")
		eval "$SOLTEST"
		create_bin_artifacts "$OS_VERSION" "solc"
		;;
	"Ubuntu-Clang"|"Ubuntu-Gcc")
		eval "$SOLTEST"
		create_bin_artifacts "$OS_VERSION" "solc"
		;;
	"Emscripten")
		eval "$SOLTEST"
		create_bin_artifacts "$OS_VERSION" "solcjs"
		;;
	"Docker")
		docker run -it ethereum/solc:build "$SOLTEST"
		docker create --volume "$TRAVIS_BUILD_DIR"/..:/ --env OS_VERSION="Docker" --name dockerSolc ethereum/solc:build ./solidity/scripts/get_binaries.sh
		;;
esac