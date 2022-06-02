function suffix(uint value) pure returns (uint) { return value; }

contract C {
    uint x = 01_23_45 suffix;
}
// ----
// ParserError 8936: (93-94): Octal numbers not allowed.
