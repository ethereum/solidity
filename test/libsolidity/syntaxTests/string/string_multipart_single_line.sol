contract test {
    function f() public pure returns (bytes32) {
        bytes32 escapeCharacters = "first" "second" "third";
        return escapeCharacters;
    }
}
// ----
