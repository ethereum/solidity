contract test {
    function f() public pure returns (string memory) {
        return "😃, 😭, and 😈";
    }
}
// ----
// ParserError 8936: (86-88): Invalid character in string.
