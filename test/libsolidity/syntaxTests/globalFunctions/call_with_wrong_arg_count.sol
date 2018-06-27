contract C {
    function f() public {
        require(address(this).call());
        require(address(this).call(bytes4(0x12345678)));
        require(address(this).call(uint(1)));
        require(address(this).call(uint(1), uint(2)));
    }
}
// ----
// TypeError: (55-75): Wrong argument count for function call: 0 arguments given but expected 1. This function requires a single bytes argument. Use "" as argument to provide empty calldata.
// TypeError: (113-131): Invalid type for argument in function call. Invalid implicit conversion from bytes4 to bytes memory requested. This function requires a single bytes argument. If all your arguments are value types, you can use abi.encode(...) to properly generate it.
// TypeError: (170-177): Invalid type for argument in function call. Invalid implicit conversion from uint256 to bytes memory requested. This function requires a single bytes argument. If all your arguments are value types, you can use abi.encode(...) to properly generate it.
// TypeError: (197-233): Wrong argument count for function call: 2 arguments given but expected 1. This function requires a single bytes argument. If all your arguments are value types, you can use abi.encode(...) to properly generate it.
