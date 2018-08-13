contract C {
    function f() public pure returns (mapping(uint=>uint)[] storage m) {
    }
}
// ----
// TypeError: (51-82): Location has to be memory for publicly visible functions (remove the "storage" or "calldata" keyword).
