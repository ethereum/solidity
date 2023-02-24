function suffix(uint value) pure suffix returns (uint) { return value; }

contract C {
    int x = int(1000) suffix;
}
// ----
// ParserError 2314: (109-115): Expected ';' but got identifier
