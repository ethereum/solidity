contract C {
    function f() public pure
    {
        // RLO RLO
        /*overflow ‮‮*/
    }
}
// ----
// ParserError 8936: (75-93): Mismatching directional override markers in comment or string literal.
