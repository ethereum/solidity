contract C {
    uint[8**90] ids;
    uint[2**256-1] okay;
    uint[2**256] tooLarge;
}
// ----
// TypeError 1847: (22-27): Array length too large, maximum is 2**256 - 1.
// TypeError 1847: (68-74): Array length too large, maximum is 2**256 - 1.
