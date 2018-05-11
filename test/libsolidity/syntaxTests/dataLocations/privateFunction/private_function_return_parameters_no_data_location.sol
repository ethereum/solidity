contract C {
    function f() private pure returns(uint[]) {}
}
// ----
// TypeError: (51-57): Location must be specified as either "memory" or "storage" for parameters.
