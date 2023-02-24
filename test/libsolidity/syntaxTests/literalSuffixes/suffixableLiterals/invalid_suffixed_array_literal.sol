function suffix(uint[1] memory) pure suffix returns (uint[1] memory) {}

contract C {
    uint[1] x = [42] suffix;
}
// ----
// ParserError 2314: (107-113): Expected ';' but got identifier
