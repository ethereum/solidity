contract C {
    function f() external pure returns (mapping(uint=>uint)[] storage m) {
    }
}
// ----
// TypeError 6651: (53-84='mapping(uint=>uint)[] storage m'): Data location must be "memory" or "calldata" for return parameter in function, but "storage" was given.
