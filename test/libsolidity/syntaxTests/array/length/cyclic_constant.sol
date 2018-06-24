contract C {
    uint constant LEN = LEN;
    function f() {
        uint[LEN] a;
    }
}
// ----
// TypeError: (37-40): Cyclic constant definition (or maximum recursion depth exhausted).
