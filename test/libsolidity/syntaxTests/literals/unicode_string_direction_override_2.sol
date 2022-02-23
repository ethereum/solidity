contract C {
    function f() public pure
    {
        // PDF PDF
        bytes memory m = unicode"underflow ‬‬";
    }
}
// ----
// ParserError 8936: (92-110): Unicode direction override underflow in comment or string literal.
