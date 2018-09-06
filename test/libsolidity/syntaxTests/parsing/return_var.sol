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
// ParserError: (38-41): Expected explicit type name.
// ParserError: (71-74): Expected explicit type name.
// ParserError: (106-109): Expected explicit type name.
// ParserError: (157-160): Expected explicit type name.
// ParserError: (192-195): Expected explicit type name.
// ParserError: (199-202): Expected explicit type name.
// ParserError: (247-250): Expected explicit type name.
// ParserError: (251-258): Location specifier needs explicit type name.
// ParserError: (301-304): Expected explicit type name.
// ParserError: (305-312): Location specifier needs explicit type name.
// ParserError: (357-360): Expected explicit type name.
// ParserError: (361-368): Location specifier needs explicit type name.
// ParserError: (372-375): Expected explicit type name.
// ParserError: (376-383): Location specifier needs explicit type name.
