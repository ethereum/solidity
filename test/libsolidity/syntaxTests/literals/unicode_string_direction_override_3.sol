contract C {
    function f() public pure
    {
        // RLO
        bytes memory m = unicode"overflow ‮";
    }
}
// ----
// ParserError 8936: (88-108='unicode"overflow ‮'): Mismatching directional override markers in comment or string literal.
