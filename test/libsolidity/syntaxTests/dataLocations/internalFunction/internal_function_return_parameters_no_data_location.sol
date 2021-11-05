contract C {
    function g() internal pure returns(uint[]) {}
}
// ----
// TypeError 6651: (52-58): Data location must be "storage", "memory" or "calldata" for return parameter in internal function, but none was given.
