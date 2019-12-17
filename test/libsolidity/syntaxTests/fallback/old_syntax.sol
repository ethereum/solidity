contract C {
    function() external {}
}
// ----
// ParserError: (37-38): Expected a state variable declaration. If you intended this as a fallback function or a function to handle plain ether transactions, use the "fallback" keyword or the "ether" keyword instead.
