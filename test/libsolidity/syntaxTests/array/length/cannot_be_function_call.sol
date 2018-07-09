contract C {
    function f(uint x) public {}
    uint constant LEN = f();
    uint[LEN] ids;
}
// ----
// TypeError: (84-87): Invalid array length, expected integer literal or constant expression.
