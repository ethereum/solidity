contract C {
    function f() public {
        require(address(this).delegatecall());
        require(address(this).delegatecall(uint(1)));
        require(address(this).delegatecall(uint(1), uint(2)));
    }
}
// ----
// TypeError: (55-83): Wrong argument count for function call: 0 arguments given but expected 1. This function requires a single bytes argument. Use "" as argument to provide empty calldata.
// TypeError: (129-136): Invalid type for argument in function call. Invalid implicit conversion from uint256 to bytes memory requested. This function requires a single bytes argument. If all your arguments are value types, you can use abi.encode(...) to properly generate it.
// TypeError: (156-200): Wrong argument count for function call: 2 arguments given but expected 1. This function requires a single bytes argument. If all your arguments are value types, you can use abi.encode(...) to properly generate it.
