function suffix(string memory s) pure returns (string memory) { return s; }

contract C {
    uint x = "abcd" suffix "abcd";
}
// ----
// ParserError 2314: (117-123): Expected ';' but got 'StringLiteral'
