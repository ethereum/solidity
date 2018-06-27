contract C {
    function f() public {
        require(keccak256() != 0);
        require(keccak256(uint(1)) != 0);
        require(keccak256(uint(1), uint(2)) != 0);
    }
}
// ----
// TypeError: (55-66): Wrong argument count for function call: 0 arguments given but expected 1. This function requires a single bytes argument. Use abi.encodePacked(...) to obtain the pre-0.5.0 behaviour or abi.encode(...) to use ABI encoding.
// TypeError: (100-107): Invalid type for argument in function call. Invalid implicit conversion from uint256 to bytes memory requested. This function requires a single bytes argument. Use abi.encodePacked(...) to obtain the pre-0.5.0 behaviour or abi.encode(...) to use ABI encoding.
// TypeError: (132-159): Wrong argument count for function call: 2 arguments given but expected 1. This function requires a single bytes argument. Use abi.encodePacked(...) to obtain the pre-0.5.0 behaviour or abi.encode(...) to use ABI encoding.
