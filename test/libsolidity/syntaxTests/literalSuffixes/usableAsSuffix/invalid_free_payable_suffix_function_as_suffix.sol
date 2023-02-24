function suffix(uint x) payable suffix returns (uint) {}

contract C {
    uint a = 1000 suffix;
}
// ----
// TypeError 1716: (0-56): Only pure functions can be used as literal suffixes
// TypeError 9559: (0-56): Free functions cannot be payable.
