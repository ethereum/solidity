contract C {
    function g() internal pure returns(uint[]) {}
}
// ----
// TypeError: (52-58): Storage location must be "storage" or "memory" for parameter in internal function, but none was given.
