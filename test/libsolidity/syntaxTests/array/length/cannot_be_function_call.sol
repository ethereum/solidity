contract C {
    function f(uint x) {}
    uint constant LEN = f();
    uint[LEN] ids;
}
// ----
// TypeError: (77-80): Invalid array length, expected integer literal or constant expression.
