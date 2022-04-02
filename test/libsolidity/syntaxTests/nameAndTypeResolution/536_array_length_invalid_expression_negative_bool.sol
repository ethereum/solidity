contract C {
    uint[-true] ids;
}
// ----
// TypeError 5462: (22-27='-true'): Invalid array length, expected integer literal or constant expression.
