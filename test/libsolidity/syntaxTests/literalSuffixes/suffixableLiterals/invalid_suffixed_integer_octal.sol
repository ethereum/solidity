function suffix(uint value) pure suffix returns (uint) { return value; }

contract C {
    uint x = 01_23_45 suffix;
}
// ----
// ParserError 8936: (100-101): Octal numbers not allowed.
