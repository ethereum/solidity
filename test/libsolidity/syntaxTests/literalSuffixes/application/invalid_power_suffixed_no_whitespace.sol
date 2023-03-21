function suffix(uint x) pure suffix returns (uint) { return x; }

contract C {
    uint x = 1000.0e-5suffix;
}
// ----
// ParserError 8936: (92-101): Identifier-start is not allowed at end of a number.
