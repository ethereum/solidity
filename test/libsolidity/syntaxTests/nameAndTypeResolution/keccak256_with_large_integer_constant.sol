contract C {
    function f() public { keccak256(2**500); }
}
// ----
// Warning: (39-56): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data.
// Warning: (39-56): The provided argument of type int_const 3273...(143 digits omitted)...9376 is not implicitly convertible to expected type bytes memory.
// TypeError: (49-55): Invalid rational number (too large or division by zero).
