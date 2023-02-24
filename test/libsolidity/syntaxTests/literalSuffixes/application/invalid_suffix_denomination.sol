function suffix(uint) pure suffix returns (uint) { return 1; }

contract C {
    uint x = 8 suffix gwei;
}
// ----
// ParserError 2314: (99-103): Expected ';' but got 'gwei'
