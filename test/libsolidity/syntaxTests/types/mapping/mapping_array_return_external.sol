contract C {
    function f() external pure returns (mapping(uint=>uint)[] storage m) {
    }
}
// ----
// TypeError 6651: (53-84): Data location must be "memory" or "calldata" for return parameter in function, but "storage" was given.
