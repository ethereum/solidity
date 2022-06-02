function suffix(uint x) pure returns (uint) { return x; }

contract C {
    uint x = 1000suffix;
}
// ----
// ParserError 8936: (85-89): Identifier-start is not allowed at end of a number.
