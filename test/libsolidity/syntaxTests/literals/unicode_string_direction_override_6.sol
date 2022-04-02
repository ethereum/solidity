contract C {
    function f() public pure
    {
        // PDF RLO
        bytes memory m = unicode" underflow ‬‮";
    }
}
// ----
// ParserError 8936: (92-111='unicode" underflow'): Unicode direction override underflow in comment or string literal.
