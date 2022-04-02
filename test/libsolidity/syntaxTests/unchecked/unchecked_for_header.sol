contract C {
    function f() public pure {
        for (unchecked { uint x = 2 }; x < 2; x ++) {

        }
    }
}
// ----
// ParserError 6933: (57-66='unchecked'): Expected primary expression.
