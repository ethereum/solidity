function suffix(uint x) pure returns (uint) { return x; }

contract C {
    uint v = 42;
    uint x = v suffix;
}
// ----
// ParserError 2314: (104-110): Expected ';' but got identifier
