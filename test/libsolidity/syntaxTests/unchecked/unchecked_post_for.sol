contract C {
    function f() public pure {
        for (uint x = 2; x < 2; unchecked { x ++; }) {

        }
    }
}
// ----
// ParserError 6933: (76-85): Expected primary expression.
