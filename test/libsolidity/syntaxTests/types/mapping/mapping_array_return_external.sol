contract C {
    function f() external pure returns (mapping(uint=>uint)[] storage m) {
    }
}
// ----
// TypeError: (53-84): Location has to be memory for publicly visible functions (remove the "storage" or "calldata" keyword).
