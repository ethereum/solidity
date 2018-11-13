contract test {
    function f() public pure returns (bytes32) {
        bytes32 escapeCharacters = "\t\b\n\r\f\'\"\\\b";
        return escapeCharacters;
    }
}
// ----
