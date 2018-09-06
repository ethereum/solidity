contract C {
    function f() private pure returns(uint[]) {}
}
// ----
// TypeError: (51-57): Data location must be "storage" or "memory" for return parameter in function, but none was given.
