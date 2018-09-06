contract C {
    function f() external pure returns (mapping(uint=>uint) storage m) {
    }
}
// ----
// TypeError: (53-82): Data location must be "memory" for return parameter in function, but "storage" was given.
