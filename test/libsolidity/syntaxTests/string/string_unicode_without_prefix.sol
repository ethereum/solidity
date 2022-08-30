contract test {
    function f() public pure returns (string memory) {
        return "😃, 😭, and 😈";
    }
}
// ----
// ParserError 8936: (86-88): Invalid character in string. If you are trying to use Unicode characters, use a unicode"..." string literal.
