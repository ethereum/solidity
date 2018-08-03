contract C {
    function i() external pure returns(uint[]) {}
}
// ----
// TypeError: (52-58): Storage location must be "memory" for parameter in external function, but none was given.
