contract C {
    function i() external pure returns(uint[]) {}
}
// ----
// TypeError: (52-58): Data location must be "memory" or "calldata" for return parameter in function, but none was given.
