contract C {
    uint constant LEN = LEN;
    function f() public {
        uint[LEN] a;
    }
}
// ----
// TypeError: (37-40): Cyclic constant definition (or maximum recursion depth exhausted).
