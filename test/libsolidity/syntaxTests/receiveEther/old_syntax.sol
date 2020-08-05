contract C {
    function() external payable {}
}
// ----
// ParserError 2915: (45-46): Expected a state variable declaration. If you intended this as a fallback function or a function to handle plain ether transactions, use the "fallback" keyword or the "receive" keyword instead.
