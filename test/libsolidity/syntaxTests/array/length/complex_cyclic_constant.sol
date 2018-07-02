contract C {
    uint constant L2 = LEN - 10;
    uint constant L1 = L2 / 10;
    uint constant LEN = 10 + L1 * 5;
    function f() public {
        uint[LEN] a;
    }
}
// ----
// TypeError: (36-39): Cyclic constant definition (or maximum recursion depth exhausted).
