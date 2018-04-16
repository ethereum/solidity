contract C {
    function f() {}
    uint[f] ids;
}
// ----
// TypeError: (42-43): Invalid array length, expected integer literal or constant expression.
