contract C {
    uint constant LEN = LEN;
    function f() public {
        uint[LEN] a;
    }
}
// ----
// TypeError 5210: (17-40='uint constant LEN = LEN'): Cyclic constant definition (or maximum recursion depth exhausted).
