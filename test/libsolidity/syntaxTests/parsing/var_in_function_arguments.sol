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
// ParserError: (28-31): Expected explicit type name.
// ParserError: (63-66): Expected explicit type name.
// ParserError: (100-103): Expected explicit type name.
// ParserError: (107-110): Expected explicit type name.
// ParserError: (152-155): Expected explicit type name.
// ParserError: (189-192): Expected explicit type name.
// ParserError: (234-237): Expected explicit type name.
// ParserError: (238-245): Location specifier needs explicit type name.
// ParserError: (277-280): Expected explicit type name.
// ParserError: (281-288): Location specifier needs explicit type name.
// ParserError: (322-325): Expected explicit type name.
// ParserError: (326-333): Location specifier needs explicit type name.
// ParserError: (337-340): Expected explicit type name.
// ParserError: (341-348): Location specifier needs explicit type name.
