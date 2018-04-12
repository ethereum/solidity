contract C {
    function f(var) public pure {}
    function f(var x) public pure {}
    function f(var x, var y) public pure {}
    function f(uint x, var y) public pure {}
    function f(var x, uint y) public pure {}
    function f(var storage) public pure {}
    function f(var storage x) public pure {}
    function f(var storage x, var storage y) public pure {}
}
// ----
// ParserError: (28-28): Expected explicit type name.
// ParserError: (63-63): Expected explicit type name.
// ParserError: (100-100): Expected explicit type name.
// ParserError: (107-107): Expected explicit type name.
// ParserError: (152-152): Expected explicit type name.
// ParserError: (189-189): Expected explicit type name.
// ParserError: (234-234): Expected explicit type name.
// ParserError: (238-238): Location specifier needs explicit type name.
// ParserError: (277-277): Expected explicit type name.
// ParserError: (281-281): Location specifier needs explicit type name.
// ParserError: (322-322): Expected explicit type name.
// ParserError: (326-326): Location specifier needs explicit type name.
// ParserError: (337-337): Expected explicit type name.
// ParserError: (341-341): Location specifier needs explicit type name.
