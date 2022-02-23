contract C {
    function f() public pure
    {
        // PDF RLO
        bytes memory m = unicode" underflow ‬‮";
    }
}
// ----
// ParserError 8936: (92-111): Unicode direction override underflow in comment or string literal.
