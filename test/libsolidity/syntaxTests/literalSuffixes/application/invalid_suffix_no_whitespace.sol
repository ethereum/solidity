function suffix(uint x) pure suffix returns (uint) { return x; }

contract C {
    uint x = 1000suffix;
}
// ----
// ParserError 8936: (92-96): Identifier-start is not allowed at end of a number.
