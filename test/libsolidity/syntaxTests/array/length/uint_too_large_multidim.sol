contract C {
    uint[8**90][500] ids;
}
// ----
// TypeError: (22-27): Invalid array length, expected integer literal or constant expression.
