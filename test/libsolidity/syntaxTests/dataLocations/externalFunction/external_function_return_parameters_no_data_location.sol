contract C {
    function i() external pure returns(uint[]) {}
}
// ----
// TypeError: (52-58): Data location must be "memory" for return parameter in function, but none was given.
