contract C {
    function f() public pure
    {
        // PDF RLO
        /*overflow ‬‮*/
    }
}
// ----
// ParserError 8936: (75-86='/*overflow'): Unicode direction override underflow in comment or string literal.
