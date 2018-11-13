contract C {
    function f() public {
        (bool success,) = address(this).delegatecall();
        require(success);
        (success,) = address(this).delegatecall(uint(1));
        require(success);
        (success,) = address(this).delegatecall(uint(1), uint(2));
        require(success);
    }
}
// ----
// TypeError: (65-93): Wrong argument count for function call: 0 arguments given but expected 1. This function requires a single bytes argument. Use "" as argument to provide empty calldata.
// TypeError: (169-176): Invalid type for argument in function call. Invalid implicit conversion from uint256 to bytes memory requested. This function requires a single bytes argument. If all your arguments are value types, you can use abi.encode(...) to properly generate it.
// TypeError: (226-270): Wrong argument count for function call: 2 arguments given but expected 1. This function requires a single bytes argument. If all your arguments are value types, you can use abi.encode(...) to properly generate it.
