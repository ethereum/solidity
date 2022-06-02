function suffix(uint value) pure returns (uint) { return value; }

contract C {
    uint x = 1000 suffix suffix;
}
// ----
// ParserError 2314: (105-111): Expected ';' but got identifier
