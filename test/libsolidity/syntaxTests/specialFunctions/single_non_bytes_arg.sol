contract C {
    function f() pure public {
        g(keccak256(uint(2)));
        g(sha256(uint(2)));
        g(ripemd160(uint(2)));
    }
    function g(bytes32) pure internal {}
}
// ----
// Warning: (54-72): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data.
// Warning: (54-72): The provided argument of type uint256 is not implicitly convertible to expected type bytes memory.
// Warning: (85-100): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data.
// Warning: (85-100): The provided argument of type uint256 is not implicitly convertible to expected type bytes memory.
// Warning: (113-131): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data.
// Warning: (113-131): The provided argument of type uint256 is not implicitly convertible to expected type bytes memory.
