contract C {
    function f() returns(var) {}
    function f() returns(var x) {}
    function f() returns(var x, uint y) {}
    function f() returns(uint x, var y) {}
    function f() returns(var x, var y) {}
    function f() public pure returns (var storage) {}
    function f() public pure returns (var storage x) {}
    function f() public pure returns (var storage x, var storage y) {}
}
// ----
// ParserError: (38-38): Expected explicit type name.
// ParserError: (71-71): Expected explicit type name.
// ParserError: (106-106): Expected explicit type name.
// ParserError: (157-157): Expected explicit type name.
// ParserError: (192-192): Expected explicit type name.
// ParserError: (199-199): Expected explicit type name.
// ParserError: (247-247): Expected explicit type name.
// ParserError: (251-251): Location specifier needs explicit type name.
// ParserError: (301-301): Expected explicit type name.
// ParserError: (305-305): Location specifier needs explicit type name.
// ParserError: (357-357): Expected explicit type name.
// ParserError: (361-361): Location specifier needs explicit type name.
// ParserError: (372-372): Expected explicit type name.
// ParserError: (376-376): Location specifier needs explicit type name.
