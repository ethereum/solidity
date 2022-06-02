function suffix(uint value) pure returns (uint) { return value; }

contract C {
    uint x = 000.10 suffix;
}
// ----
// ParserError 8936: (93-94): Octal numbers not allowed.
