function suffix(uint a, uint b) pure suffix returns (uint) { return a + b; }

contract C {
    uint x = 1000 1 suffix;
}
// ----
// ParserError 2314: (109-110): Expected ';' but got 'Number'
