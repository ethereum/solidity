function suffix(uint x) pure suffix returns (uint) { return x; }

contract C {
    uint v = 42;
    uint x = v suffix;
}
// ----
// ParserError 2314: (111-117): Expected ';' but got identifier
