function suffix(uint x) view suffix returns (uint) {}

contract C {
    uint a = 1000 suffix;
}
// ----
// TypeError 1716: (0-53): Only pure functions can be used as literal suffixes
