contract C {
    bytes32[8**90][500] ids;
}
// ----
// TypeError 5462: (25-30='8**90'): Invalid array length, expected integer literal or constant expression.
