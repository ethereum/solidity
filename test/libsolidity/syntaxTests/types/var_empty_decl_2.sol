contract C {
    function f() public pure {
        var (b, c);
        b.WeMustNotReachHere();
        c.FailsToLookupToo();
    }
}
// ----
// ParserError 6933: (52-55): Expected primary expression.
