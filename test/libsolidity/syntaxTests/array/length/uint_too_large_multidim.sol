contract C {
    uint[8**90][500] ids;
}
// ----
// TypeError 1847: (22-27): Array length too large, maximum is 2**256 - 1.
