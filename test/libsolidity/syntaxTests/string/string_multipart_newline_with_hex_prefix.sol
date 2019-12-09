contract test {
    function f() public pure returns (bytes32) {
        bytes32 escapeCharacters = hex"0000"
        hex"deaf"
        hex"feed";
        return escapeCharacters;
    }
}
// ----
