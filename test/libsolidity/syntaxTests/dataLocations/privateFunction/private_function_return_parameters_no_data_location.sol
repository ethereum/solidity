contract C {
    function f() private pure returns(uint[]) {}
}
// ----
// TypeError 6651: (51-57): Data location must be "storage", "memory" or "calldata" for return parameter in function, but none was given.
