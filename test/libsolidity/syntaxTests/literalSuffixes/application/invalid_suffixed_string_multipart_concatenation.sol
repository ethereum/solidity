function suffix(string memory) pure suffix returns (string memory) {}

contract C {
    string s = "abcd" suffix "abcd" suffix;
}
// ----
// ParserError 2314: (113-119): Expected ';' but got 'StringLiteral'
