contract C {
    function i() external pure returns(uint[]) {}
}
// ----
// TypeError: (52-58): Location must be specified as "memory" for parameters in publicly visible functions.
