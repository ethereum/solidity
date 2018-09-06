contract C {
    function f() public {}
    uint[f] ids;
}
// ----
// TypeError: (49-50): Invalid array length, expected integer literal or constant expression.
