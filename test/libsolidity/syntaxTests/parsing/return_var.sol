contract C {
    function f() returns(var) {}
    function f() returns(var x) {}
    function f() public pure returns (var storage) {}
    function f() public pure returns (var storage x) {}
}
// ----
// ParserError: (38-38): Expected explicit type name.
// ParserError: (71-71): Expected explicit type name.
// ParserError: (119-119): Expected explicit type name.
// ParserError: (123-123): Location specifier needs explicit type name.
// ParserError: (173-173): Expected explicit type name.
// ParserError: (177-177): Location specifier needs explicit type name.
