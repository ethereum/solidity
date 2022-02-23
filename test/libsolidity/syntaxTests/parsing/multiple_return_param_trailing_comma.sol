contract test {
    function() returns (uint a, uint b,) {}
}
// ----
// ParserError 7591: (54-55): Unexpected trailing comma in parameter list.
