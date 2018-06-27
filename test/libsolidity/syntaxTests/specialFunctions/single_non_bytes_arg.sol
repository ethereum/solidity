contract C {
    function f() pure public {
        g(keccak256(uint(2)));
        g(sha256(uint(2)));
        g(ripemd160(uint(2)));
    }
    function g(bytes32) pure internal {}
}
// ----
// TypeError: (64-71): Invalid type for argument in function call. Invalid implicit conversion from uint256 to bytes memory requested. This function requires a single bytes argument. Use abi.encodePacked(...) to obtain the pre-0.5.0 behaviour or abi.encode(...) to use ABI encoding.
// TypeError: (92-99): Invalid type for argument in function call. Invalid implicit conversion from uint256 to bytes memory requested. This function requires a single bytes argument. Use abi.encodePacked(...) to obtain the pre-0.5.0 behaviour or abi.encode(...) to use ABI encoding.
// TypeError: (123-130): Invalid type for argument in function call. Invalid implicit conversion from uint256 to bytes memory requested. This function requires a single bytes argument. Use abi.encodePacked(...) to obtain the pre-0.5.0 behaviour or abi.encode(...) to use ABI encoding.
