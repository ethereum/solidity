contract C {
    function f(uint[] transient transient x) public pure { }
}

// ----
// ParserError 2314: (45-54): Expected ',' but got identifier
