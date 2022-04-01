contract C {
    bytes32[8**90] ids;
}
// ----
// TypeError 1847: (25-30): Array length too large, maximum is 2**256 - 1.
