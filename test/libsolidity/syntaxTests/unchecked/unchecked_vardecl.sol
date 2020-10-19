contract C {
    uint x = unchecked { f() + 2 }
    function f() public pure returns (uint) {
        return 4;
    }
}
// ----
// ParserError 6933: (26-35): Expected primary expression.
