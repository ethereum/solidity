function suffix(uint[1] memory) pure returns (uint[1] memory) {}

contract C {
    uint[1] x = [42] suffix;
}
// ----
// ParserError 2314: (100-106): Expected ';' but got identifier
