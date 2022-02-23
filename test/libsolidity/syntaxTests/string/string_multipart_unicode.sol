contract test {
    function f() public pure returns (bytes32) {
        bytes32 escapeCharacters = unicode"foo" unicode"😃, 😭, and 😈" unicode"!";
        return escapeCharacters;
    }
}
// ----
