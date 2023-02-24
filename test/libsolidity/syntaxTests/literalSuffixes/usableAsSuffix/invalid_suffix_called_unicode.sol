function unicode(string memory) pure suffix returns (string memory) {}

contract C {
    string s = unicode"😃"unicode;
}
// ----
// ParserError 2314: (9-16): Expected identifier but got 'ILLEGAL'
