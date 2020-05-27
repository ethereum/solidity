contract C {
    function g() internal pure returns(uint[]) {}
}
// ----
// TypeError: (52-58): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
