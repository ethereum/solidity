contract C {
    function f() public pure returns (mapping(uint=>uint) memory m) {
    }
}
// ----
// TypeError: (51-79): Mapping types for parameters or return variables can only be used in internal or library functions.
