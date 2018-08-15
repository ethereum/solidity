contract C {
    function f() public {
        (bool success,) = address(this).call();
        require(success);
        (success,) = address(this).call(bytes4(0x12345678));
        require(success);
        (success,) = address(this).call(uint(1));
        require(success);
        (success,) = address(this).call(uint(1), uint(2));
        require(success);
    }
}
// ----
// TypeError: (65-85): Wrong argument count for function call: 0 arguments given but expected 1. This function requires a single bytes argument. Use "" as argument to provide empty calldata.
// TypeError: (153-171): Invalid type for argument in function call. Invalid implicit conversion from bytes4 to bytes memory requested. This function requires a single bytes argument. If all your arguments are value types, you can use abi.encode(...) to properly generate it.
// TypeError: (240-247): Invalid type for argument in function call. Invalid implicit conversion from uint256 to bytes memory requested. This function requires a single bytes argument. If all your arguments are value types, you can use abi.encode(...) to properly generate it.
// TypeError: (297-333): Wrong argument count for function call: 2 arguments given but expected 1. This function requires a single bytes argument. If all your arguments are value types, you can use abi.encode(...) to properly generate it.
