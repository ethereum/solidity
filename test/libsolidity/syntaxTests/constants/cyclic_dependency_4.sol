contract C {
    uint constant a = b * c;
    uint constant b = 7;
    uint constant c = 4 + uint(keccak256(d));
    uint constant d = 2 + b;
}
// ----
// Warning: (98-110): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data. The provided argument of type uint256 is not implicitly convertible to expected type bytes memory.
