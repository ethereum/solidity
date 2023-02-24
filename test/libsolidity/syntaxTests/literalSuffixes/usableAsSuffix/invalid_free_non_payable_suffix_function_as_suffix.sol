function suffix(uint x) suffix returns (uint) {}

contract C {
    uint a = 1000 suffix;
}
// ----
// TypeError 1716: (0-48): Only pure functions can be used as literal suffixes
