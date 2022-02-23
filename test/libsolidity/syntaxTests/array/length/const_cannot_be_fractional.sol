contract C {
    fixed constant L = 10.5;
    uint[L] ids;
}
// ----
// TypeError 5462: (51-52): Invalid array length, expected integer literal or constant expression.
