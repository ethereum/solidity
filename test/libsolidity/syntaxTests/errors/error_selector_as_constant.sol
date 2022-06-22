contract C {
    error Er();
    function f() external {}
    bytes4 constant errorSelector = Er.selector;
}
// ----
// TypeError 8349: (95-106): Initial value for constant variable has to be compile-time constant.
