contract C {
    function f() public pure returns (mapping(uint=>uint) storage m) {
    }
}
// ----
// TypeError 6651: (51-80): Data location must be "memory" or "calldata" for return parameter in function, but "storage" was given.
