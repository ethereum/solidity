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
// ParserError 7059: (28-31): Expected explicit type name.
// ParserError 7059: (63-66): Expected explicit type name.
// ParserError 7059: (100-103): Expected explicit type name.
// ParserError 7059: (107-110): Expected explicit type name.
// ParserError 7059: (152-155): Expected explicit type name.
// ParserError 7059: (189-192): Expected explicit type name.
// ParserError 7059: (234-237): Expected explicit type name.
// ParserError 7439: (238-245): Location specifier needs explicit type name.
// ParserError 7059: (277-280): Expected explicit type name.
// ParserError 7439: (281-288): Location specifier needs explicit type name.
// ParserError 7059: (322-325): Expected explicit type name.
// ParserError 7439: (326-333): Location specifier needs explicit type name.
// ParserError 7059: (337-340): Expected explicit type name.
// ParserError 7439: (341-348): Location specifier needs explicit type name.
