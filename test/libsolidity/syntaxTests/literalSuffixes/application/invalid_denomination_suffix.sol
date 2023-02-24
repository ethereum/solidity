function suffix(uint) pure suffix returns (uint) { return 1; }

contract C {
    uint x = 2 ether suffix;
}
// ----
// ParserError 2314: (98-104): Expected ';' but got identifier
