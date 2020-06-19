contract C {
    function f() public pure returns (mapping(uint=>uint) memory m) {
    }
}
// ----
// TypeError 3442: (51-79): Mapping types can only have a data location of "storage" and thus only be parameters or return variables for internal or library functions.
