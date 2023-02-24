function suffix(uint value) pure suffix returns (uint) { return value; }

contract C {
    uint x = 1000 suffix suffix;
}
// ----
// ParserError 2314: (112-118): Expected ';' but got identifier
