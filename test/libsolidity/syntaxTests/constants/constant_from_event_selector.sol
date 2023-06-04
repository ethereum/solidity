interface I {
    event Ev1();
}
contract C {
    function f() external {}
    event Ev2();
    bytes32 constant eventSelector1 = I.Ev1.selector;
    bytes32 constant eventSelector2 = Ev2.selector;
}
// ----
