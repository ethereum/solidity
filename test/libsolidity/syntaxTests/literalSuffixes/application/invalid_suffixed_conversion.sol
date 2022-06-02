function suffix(uint value) pure returns (uint) { return value; }

contract C {
    int x = int(1000) suffix;
}
// ----
// ParserError 2314: (102-108): Expected ';' but got identifier
