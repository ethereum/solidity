function suffix(uint value) pure suffix returns (uint) { return value; }

contract C {
    uint x = 000.10 suffix;
}
// ----
// ParserError 8936: (100-101): Octal numbers not allowed.
