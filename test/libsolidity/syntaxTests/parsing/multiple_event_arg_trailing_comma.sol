contract test {
    event Test(uint a, uint b,);
    function(uint a) {}
}
// ----
// ParserError: (45-46): Unexpected trailing comma in parameter list.
