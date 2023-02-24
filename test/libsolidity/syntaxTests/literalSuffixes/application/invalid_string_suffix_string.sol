function suffix(string memory s) pure suffix returns (string memory) { return s; }

contract C {
    uint x = "abcd" suffix "abcd";
}
// ----
// ParserError 2314: (124-130): Expected ';' but got 'StringLiteral'
