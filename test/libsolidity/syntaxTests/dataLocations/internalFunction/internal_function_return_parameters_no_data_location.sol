contract C {
    function g() internal pure returns(uint[]) {}
}
// ----
// TypeError: (52-58): Location must be specified as either "memory" or "storage" for parameters.
