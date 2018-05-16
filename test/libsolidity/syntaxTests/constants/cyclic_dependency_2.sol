contract C {
    uint constant a = b * c;
    uint constant b = 7;
    uint constant c = b + uint(keccak256(d));
    uint constant d = 2 + a;
}
// ----
// Warning: (98-110): This function only accepts a single "bytes" argument. Please use "abi.encodePacked(...)" or a similar function to encode the data.
// Warning: (98-110): The provided argument of type uint256 is not implicitly convertible to expected type bytes memory.
// TypeError: (17-40): The value of the constant a has a cyclic dependency via c.
// TypeError: (71-111): The value of the constant c has a cyclic dependency via d.
// TypeError: (117-140): The value of the constant d has a cyclic dependency via a.
