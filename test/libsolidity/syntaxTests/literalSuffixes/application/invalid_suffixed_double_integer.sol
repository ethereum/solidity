function suffix(uint a, uint b) pure returns (uint) { return a + b; }

contract C {
    uint x = 1000 1 suffix;
}
// ----
// ParserError 2314: (102-103): Expected ';' but got 'Number'
