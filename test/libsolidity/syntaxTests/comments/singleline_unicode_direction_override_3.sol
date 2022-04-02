contract C {
    function f() public pure
    {
        // RLO
        // overflow ‮
    }
}
// ----
// ParserError 8936: (71-86='// overflow ‮'): Mismatching directional override markers in comment or string literal.
