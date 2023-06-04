interface I {
    error Er1();
}
contract C {
    function f() external {}
    error Er2();
    bytes4 constant errorSelector1 = I.Er1.selector;
    bytes4 constant errorSelector2 = Er2.selector;
}
// ----
