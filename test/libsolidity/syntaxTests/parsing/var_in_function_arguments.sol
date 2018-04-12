contract C {
    function f(var) public pure {}
    function f(var x) public pure {}
    function f(var storage) public pure {}
    function f(var storage x) public pure {}
}
// ----
// ParserError: (28-28): Expected explicit type name.
// ParserError: (63-63): Expected explicit type name.
// ParserError: (100-100): Expected explicit type name.
// ParserError: (104-104): Location specifier needs explicit type name.
// ParserError: (143-143): Expected explicit type name.
// ParserError: (147-147): Location specifier needs explicit type name.
